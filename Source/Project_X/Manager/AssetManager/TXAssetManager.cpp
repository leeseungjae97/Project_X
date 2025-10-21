// Fill out your copyright notice in the Description page of Project Settings.


#include "TXAssetManager.h"

#include "TXPrimaryDataAsset.h"
#include "Kismet/GameplayStatics.h"
#include "UI/TXLoadingWidget.h"

FString UTXAssetManager::MakeBlueprintGeneratedClassPath(const FString& InBasePath)
{
	// 경로가 이미 "_C"를 포함하고 있으면 그대로 반환
	if (InBasePath.EndsWith(TEXT("_C")))
	{
		return InBasePath;
	}

	FString AssetName;
	FString PackagePath;

	if (InBasePath.Split(TEXT("/"), &PackagePath, &AssetName, ESearchCase::IgnoreCase, ESearchDir::FromEnd))
	{
		// ex: /Game/Blueprints/BP_Enemy → /Game/Blueprints/BP_Enemy.BP_Enemy_C
		return FString::Printf(TEXT("%s.%s_C"), *InBasePath, *AssetName);
	}

	// 실패 시 원본 반환
	return InBasePath;
}

UTXAssetManager& UTXAssetManager::Get()
{
	check(GEngine);

	UTXAssetManager* Singleton = Cast<UTXAssetManager>(GEngine->AssetManager);
	if (Singleton)
	{
		return *Singleton;
	}

	UE_LOG(LogTemp, Fatal, TEXT("CustomAssetManager not created yet!"));
	return *NewObject<UTXAssetManager>();
}

void UTXAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();
}

void UTXAssetManager::StartAssetPreLoading(UObject* WorldContextObject)
{
	//AssetManager에서 this로 WorldContext 사용불가. AssetManager는 스스로 GetWorld()가 없기 때문에 필수.
	UWorld* World =  GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
	if (!World)
		return;
	
	//Widget
	UClass* pWidgetClass  = LoadBPClass("/Game/UI/Loading/UMG_Loading.UMG_Loading");
	if(pWidgetClass == nullptr)
		return;
	
	UTXLoadingWidget* LoadingWidget = CreateWidget<UTXLoadingWidget>(World, pWidgetClass, "LoadingWidget");
	if(LoadingWidget == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Cant Load Loading Widget"));
		return;
	}
	LoadingWidget->BindOnButtonClicked([this]
	{
		
	});
	LoadingWidget->AddToViewport();
	UE_LOG(LogTemp, Display, TEXT("Complete Load Loading Widget"));

	const FString Path = "/Game/Assets/DA_PreLoad.DA_PreLoad";
	FSoftObjectPath DataAssetPath(Path);

	//DataAsset
	GetStreamableManager().RequestAsyncLoad(DataAssetPath, [this, DataAssetPath, LoadingWidget, World]()
	{
		auto* DataAsset = Cast<UTXPrimaryDataAsset>(DataAssetPath.ResolveObject());

		if (!DataAsset || DataAsset->AssetsToLoad.Num() == 0)
		{
			UE_LOG(LogTemp, Error, TEXT("Startup loading data asset not found or empty."));
			return;
		}

		//폴더 스캔으로 SoftObjectPath 수집
        TArray<FSoftObjectPath> Paths;// 최종 로드 목록

        if (DataAsset->DirectoriesToScan.Num() > 0)
        {
        	
            IAssetRegistry& AR = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();

            FARFilter Filter;
            for (const FDirectoryPath& Dir : DataAsset->DirectoriesToScan)
	            Filter.PackagePaths.Add(FName(*Dir.Path));
            
            Filter.bRecursivePaths = true;
            TArray<FAssetData> Found;
            AR.GetAssets(Filter, Found);

            for (const FAssetData& AD : Found)
            {
            	UE_LOG(LogTemp, Warning, TEXT("FindObject : %s"),*AD.ToSoftObjectPath().ToString());
                Paths.Add(AD.ToSoftObjectPath());
            }
        }

        // 수동 등록 애셋 합치기
        Paths.Append(DataAsset->AssetsToLoad);

        if (Paths.Num() == 0)
        {
            UE_LOG(LogTemp, Warning, TEXT("No assets to preload"));
            UGameplayStatics::OpenLevel(World, "Lvl_Combat");
            return;
        }

        //비동기 로드
		TSharedPtr<FStreamableHandle> Handle =
			GetStreamableManager().RequestAsyncLoad(
				Paths,
				FStreamableDelegate::CreateLambda([this, World, LoadingWidget]()
				{
					// 로딩완료: 타이머 정리 후, 레벨 전환. OpenWorld라 자동으로 리셋됨. 하지만 추후에 비동기로 바꾸면 문제가 생기기 때문에 넣음.
					World->GetTimerManager().ClearTimer(ProgressUpdateHandle);
					UE_LOG(LogTemp, Log, TEXT("Initial loading completed"));
					
					LoadingWidget->ReadyToStart();
					// 로딩 위젯 안에 OnButtonClicked에 OpenLevel 람다 바인드
					// (정리: 모든 에셋 로드 완료 -> 로딩위젯에 버튼 활성화 및 클릭 이벤트 바인드, 그리고 BindOnButtonClicked함수 반환값이 bool이지만 지금은 사용 안함)
					LoadingWidget->BindOnButtonClicked([this,World]
					{
						UGameplayStatics::OpenLevel(World, "Lvl_Combat");
					});
				}),
				FStreamableManager::AsyncLoadHighPriority);
		
        //진행률 타이머 설정 (0.05s) 및 비동기 로드 중인 에셋 이름 위젯에 표시
        if (Handle.IsValid())
        {
        	Handle->BindUpdateDelegate(FStreamableUpdateDelegate::CreateLambda([this, LoadingWidget](TSharedRef<FStreamableHandle> HandleRef)
	        {
		        if (!LoadingWidget)
		        	return;

		        // Handle 안에서 “이번에 막 끝난” 패키지를 추정
		        // 요청 목록
		        TArray<FSoftObjectPath> Req;
        		HandleRef->GetRequestedAssets(Req);

		        // 이미 끝난 개수
		        int32 Done;
        		// 총 개수
        		int32 Total;
        		HandleRef->GetLoadedCount(Done, Total);

		        // 범위 체크
		        if (Req.IsValidIndex(Done - 1))
		        {
			        const FString Name = Req[Done - 1].GetAssetName();
			        LoadingWidget->SetObjectName(Name, Done, Total); // 위젯에 표시
		        }
		        else
		        {
			        // Fallback: 디버그 네임
			        LoadingWidget->SetObjectName(HandleRef->GetDebugName(), Done, Total);
		        }
	        }));
            // 중복 타이머 방지
            if (ProgressUpdateHandle.IsValid())
                World->GetTimerManager().ClearTimer(ProgressUpdateHandle);

            World->GetTimerManager().SetTimer(
                ProgressUpdateHandle,
                FTimerDelegate::CreateLambda([Handle, LoadingWidget]()
                {
                    if (LoadingWidget)
                        LoadingWidget->SetPercent(Handle->GetProgress());
                }),
                0.05f,
                true);
        }
    });
}

UClass* UTXAssetManager::LoadBPClass(FString RepPath)
{
	if(RepPath.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("UTXAssetManager::LoadBPClass: Invalid RepPath"));
		return nullptr;
	}
	if (!RepPath.EndsWith(TEXT("_C")))
	{
		RepPath += TEXT("_C");
	}

	FSoftClassPath ClassPath(RepPath);
	UClass* LoadedClass = Cast<UClass>(ClassPath.TryLoad());

	if(!LoadedClass)
	{
		UE_LOG(LogTemp, Error, TEXT("UTXAssetManager::LoadBPClass: Invalid RepPath. Class Load is failed. Path is %s"), *RepPath);
		return nullptr;
	}

	return LoadedClass;
}

void UTXAssetManager::LoadBPClassAsync(FString RepPath, TFunction<void(UClass*)> OnLoaded)
{
	if (RepPath.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("UTXAssetManager::LoadBPClassAsync: Empty RepPath"));
		OnLoaded(nullptr);
		return;
	}

	// Ensure _C is at the end (GeneratedClass)
	if (!RepPath.EndsWith(TEXT("_C")))
	{
		RepPath += TEXT("_C");
	}

	FSoftClassPath ClassPath(RepPath);

	if (!ClassPath.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("UTXAssetManager::LoadBPClassAsync: Invalid ClassPath: %s"), *RepPath);
		OnLoaded(nullptr);
		return;
	}

	GetStreamableManager().RequestAsyncLoad(
		ClassPath,
		[ClassPath, OnLoaded]()
		{
			UClass* LoadedClass = ClassPath.ResolveClass();

			if (!LoadedClass)
			{
				UE_LOG(LogTemp, Error, TEXT("UTXAssetManager::LoadBPClassAsync: Failed to resolve class: %s"), *ClassPath.ToString());
			}

			OnLoaded(LoadedClass);
		}
	);
}
