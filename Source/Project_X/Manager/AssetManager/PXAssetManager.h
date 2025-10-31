// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "PXAssetManager.generated.h"

/**
 * 
 */
UCLASS() class PROJECT_X_API UPXAssetManager : public UAssetManager
{
	GENERATED_BODY()
	
private:
	UPROPERTY()
	FTimerHandle ProgressUpdateHandle;
	
	FString MakeBlueprintGeneratedClassPath(const FString& InBasePath);
	
public:
	static UPXAssetManager& Get();
	
	virtual void StartInitialLoading() override;

	void StartAssetPreLoading(UObject* WorldContextObject);

	// For Load Any Kind of Asset
	template <typename T>
	T* LoadAsset(const FString& RepPath);
	template <typename T>
	void LoadAssetAsync(const FString& RepPath, TFunction<void(T*)> OnLoaded);
	
	// For BP Based Class
	UClass* LoadBPClass(FString RepPath);
	void LoadBPClassAsync(FString RepPath, TFunction<void(UClass*)> OnLoaded);
	
};

template <typename T>
T* UPXAssetManager::LoadAsset(const FString& RepPath)
{
	if (RepPath.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("LoadAssetByPath_Sync: Empty path"));
		return nullptr;
	}

	FSoftObjectPath SoftPath(RepPath);
	if (!SoftPath.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("LoadAssetByPath_Sync: Invalid path %s"), *RepPath);
		return nullptr;
	}

	UObject* LoadedObj = SoftPath.TryLoad();
	if (!LoadedObj)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load asset at path: %s"), *RepPath);
		return nullptr;
	}

	return Cast<T>(LoadedObj);
}

template <typename T>
void UPXAssetManager::LoadAssetAsync(const FString& RepPath, TFunction<void(T*)> OnLoaded)
{
	FSoftObjectPath AssetPath = RepPath;

	if (!AssetPath.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid asset path for %s"), *RepPath);
		OnLoaded(nullptr);
		return;
	}

	GetStreamableManager().RequestAsyncLoad(AssetPath, [AssetPath, OnLoaded]()
	{
		UObject* LoadedAsset = AssetPath.ResolveObject();

		// Fallback 로딩 (Resolve 실패 시 TryLoad)
		if (!LoadedAsset)
		{
			LoadedAsset = AssetPath.TryLoad();
		}

		if (!LoadedAsset)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to load asset asynchronously: %s"), *AssetPath.ToString());
			OnLoaded(nullptr);
			return;
		}

		T* CastObject = Cast<T>(LoadedAsset);
		if (!CastObject)
		{
			UE_LOG(LogTemp, Error, TEXT("Loaded asset is not of expected type: %s"), *AssetPath.ToString());
		}

		OnLoaded(CastObject);
	});
}
