#include "UI/TXIndicatorHUDWidget.h"
#include "UI/TXEnemyIndicatorWidget.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Engine/GameViewportClient.h"
#include "TXComponents/TXLockOnComponent.h"
#include "Components/DecalComponent.h"
#include "Engine/DecalActor.h"
#include "CombatCharacter.h"
#include "Engine/Engine.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Curves/CurveVector.h"

void UTXIndicatorHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UTXIndicatorHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if(RangeEffectTimeline.IsPlaying())
		RangeEffectTimeline.TickTimeline(InDeltaTime);

	if (!LockOnComponent)
	{
		LockOnComponent = Cast<UTXLockOnComponent>(GetOwningPlayerPawn()->GetComponentByClass(UTXLockOnComponent::StaticClass()));
		SpawnRangeEffectDecal();
	}
	
	SpawnIndicator();
}

void UTXIndicatorHUDWidget::SpawnIndicator()
{
	if (LockOnComponent && !LockOnComponent->IsLockOn())
	{
		for (const TPair<AActor*, UTXEnemyIndicatorWidget*>& Pair : ActiveIndicatorsMap)
		{
			if (Pair.Value)
			{
				Pair.Value->RemoveFromParent();
			}
		}
		ActiveIndicatorsMap.Empty();
		return;
	}
	
	TSet<AActor*> ActorsToUpdateThisFrame;

	for (int i = 0; i < DetectedActorsArray.Num(); ++i)
	{
		const TPair<AActor*, float>& ActorPair = DetectedActorsArray[i];
		AActor* TargetActor = ActorPair.Key;
		if (!TargetActor || !TargetActor->IsValidLowLevelFast())
		{
			continue;
		}

		ActorsToUpdateThisFrame.Add(TargetActor);

		UTXEnemyIndicatorWidget** IndicatorWidgetPtr = ActiveIndicatorsMap.Find(TargetActor);
		UTXEnemyIndicatorWidget* IndicatorWidget = nullptr;

		if (IndicatorWidgetPtr)
		{
			IndicatorWidget = *IndicatorWidgetPtr;
		}

		if (!IndicatorWidget)
		{
			if (IndicatorWidgetClass)
			{
				IndicatorWidget = CreateWidget<UTXEnemyIndicatorWidget>(GetWorld(), IndicatorWidgetClass);
				if (IndicatorWidget)
				{
					IndicatorWidget->AddToViewport();
					IndicatorWidget->SetEnemyIndex(i + 1);
					ActiveIndicatorsMap.Add(TargetActor, IndicatorWidget);
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("Failed to create IndicatorWidget for actor: %s"), *TargetActor->GetName());
					continue;
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("IndicatorWidgetClass is not set in TXIndicatorHUDWidget!"));
				continue;
			}
		}
		else
		{
			IndicatorWidget->AddToViewport();
			IndicatorWidget->SetEnemyIndex(i + 1);
		}

		FVector2D ScreenPosition;
		float RotationAngle = 0.0f;

		bool bIsOnScreen = CalculateIndicatorPositionAndRotation(TargetActor, ScreenPosition, RotationAngle);

		IndicatorWidget->SetPositionInViewport(ScreenPosition);
		//IndicatorWidget->SetRenderTransformAngle(RotationAngle);
		IndicatorWidget->SetVisibility(ESlateVisibility::Visible);
	}

	TArray<AActor*> ActorsToRemove;
	for (const TPair<AActor*, UTXEnemyIndicatorWidget*>& Pair : ActiveIndicatorsMap)
	{
		AActor* Actor = Pair.Key;
		UTXEnemyIndicatorWidget* IndicatorWidget = Pair.Value;

		if (!ActorsToUpdateThisFrame.Contains(Actor))
		{
			if (IndicatorWidget)
			{
				IndicatorWidget->RemoveFromParent();
			}
		}
	}

	//for (AActor* Actor : ActorsToRemove)
	//{
	//	ActiveIndicatorsMap.Remove(Actor);
	//}
}

bool UTXIndicatorHUDWidget::CalculateIndicatorPositionAndRotation(AActor* TargetActor, FVector2D& OutScreenPosition, float& OutRotationAngle)
{
	APlayerController* PlayerController = GetOwningPlayer();
	if (!PlayerController || !TargetActor)
	{
		return false;
	}

	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}
	else
	{
		return false;
	}

	FVector ActorLocation = TargetActor->GetActorLocation();
	FVector IndicatorLocation = TargetActor->GetActorLocation();
	IndicatorLocation.Z += 100.f;
	ActorLocation.Z += 100.f;

	FVector CameraLocation;
	FRotator CameraRotation;
	PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);

	FVector DirectionToTarget = (ActorLocation - CameraLocation).GetSafeNormal();
	FVector CameraForward = CameraRotation.Vector();

	float DotProduct = FVector::DotProduct(CameraForward, DirectionToTarget);
	bool bIsBehindCamera = (DotProduct < 0);

	FVector2D ScreenLocation;
	bool bProjected = PlayerController->ProjectWorldLocationToScreen(ActorLocation, ScreenLocation);

	bool bIsOnScreen = bProjected && !bIsBehindCamera &&
		ScreenLocation.X >= 0 && ScreenLocation.X <= ViewportSize.X &&
		ScreenLocation.Y >= 0 && ScreenLocation.Y <= ViewportSize.Y;

	if (bIsOnScreen)
	{
		PlayerController->ProjectWorldLocationToScreen(IndicatorLocation, ScreenLocation);
		OutScreenPosition = ScreenLocation;
		OutRotationAngle = 0.0f;
	}
	else
	{
		FVector2D Direction2D = ScreenLocation - (ViewportSize * 0.5f);
		Direction2D.Normalize();

		OutRotationAngle = FMath::RadiansToDegrees(FMath::Atan2(Direction2D.Y, Direction2D.X));

		float Angle = FMath::DegreesToRadians(OutRotationAngle);
		float CosAngle = FMath::Cos(Angle);
		float SinAngle = FMath::Sin(Angle);

		float HalfWidth = ViewportSize.X * 0.5f - IndicatorPadding;
		float HalfHeight = ViewportSize.Y * 0.5f - IndicatorPadding;

		float RatioX = HalfWidth / FMath::Abs(CosAngle);
		float RatioY = HalfHeight / FMath::Abs(SinAngle);

		float Ratio = FMath::Min(RatioX, RatioY);

		OutScreenPosition.X = HalfWidth + Ratio * CosAngle;
		OutScreenPosition.Y = HalfHeight + Ratio * SinAngle;

		OutScreenPosition.X += IndicatorPadding;
		OutScreenPosition.Y += IndicatorPadding;
	}

	return bIsOnScreen;
}

void UTXIndicatorHUDWidget::SpawnRangeEffectDecal()
{
	if (!RangeEffectMaterialDynamic && RangeEffectMaterial)
	{
		RangeEffectMaterialDynamic = UMaterialInstanceDynamic::Create(RangeEffectMaterial, this);
	}

	UWorld* World = GetWorld();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwningPlayer();
	//SpawnParams.Instigator = GetInstigator();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ACombatCharacter* Owner = nullptr;

	if(GetOwningPlayer()->GetPawn())
		Owner = Cast<ACombatCharacter>(GetOwningPlayer()->GetPawn());

	if (!Owner)
		return;

	FVector SpawnLocation = Owner->GetActorLocation();
	FRotator SpawnRotation = FRotator(0.0f, -90.0f, 0.0f);
	//FRotator SpawnRotation = Owner->GetActorRotation();
	FVector SpawnScale = FVector(1.0f, 1.0f, 1.0f);
	FTransform SpawnTransform(SpawnRotation, SpawnLocation, SpawnScale);

	RangeEffectDecalActor = World->SpawnActor<ADecalActor>(ADecalActor::StaticClass(), SpawnTransform, SpawnParams);

	if (RangeEffectDecalActor)
	{
		RangeEffectDecalComponent = RangeEffectDecalActor->GetDecal();

		if (RangeEffectDecalComponent)
		{
			if (RangeEffectMaterialDynamic)
			{
				RangeEffectDecalComponent->SetMaterial(0, RangeEffectMaterialDynamic);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Material!"));
			}

			RangeEffectDecalComponent->DecalSize = FVector(1500.0f, 1500.0f, 1500.0f);
			RangeEffectDecalComponent->SetSortOrder(1);
		}


		RangeEffectDecalActor->SetActorHiddenInGame(true);
		
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("fail decalActor spawn"));
	}
}

void UTXIndicatorHUDWidget::UpdateRangeEffectSize(FVector Size)
{
	if (!RangeEffectDecalActor)
		return;
	RangeEffectDecalActor->SetActorScale3D(Size);
}

void UTXIndicatorHUDWidget::UpdateRangeEffectInnerRadius(float IInnerRadius)
{
	if (!RangeEffectDecalActor || !RangeEffectMaterialDynamic)
		return;

	RangeEffectMaterialDynamic->SetScalarParameterValue(FName("InnerRadius"), IInnerRadius);
}

void UTXIndicatorHUDWidget::UpdateRangeEffectIInnerRadius(float InnerRadius)
{
	if (!RangeEffectDecalActor || !RangeEffectMaterialDynamic)
		return;

	RangeEffectMaterialDynamic->SetScalarParameterValue(FName("IInnerRadius"), InnerRadius);
}


void UTXIndicatorHUDWidget::UpdateRangeEffectRadius(float Radius)
{
	if (!RangeEffectDecalActor || !RangeEffectMaterialDynamic)
		return;

	RangeEffectMaterialDynamic->SetScalarParameterValue(FName("Radius"), Radius);
}

void UTXIndicatorHUDWidget::FinishUpdateRangeEffect()
{
	if (!RangeEffectDecalActor)
		return;

	//if (ACombatCharacter* Character = LockOnComponent->GetOwnerCharacter())
	//{
	//	if (USkeletalMeshComponent* CharacterMesh = Character->GetMesh())
	//	{
	//		CharacterMesh->SetReceivesDecals(true);
	//	}
	//}
	//RangeEffectDecalActor->SetActorHiddenInGame(true);
	//RangeEffectTimeline.Stop();
}

void UTXIndicatorHUDWidget::StartRangeEffectTimeline()
{
	if (!RangeEffectVectorCurve || !RangeEffectDecalActor || !RangeEffectDecalComponent)
		return;

	if (LockOnComponent)
	{
		if (ACombatCharacter* Character = LockOnComponent->GetOwnerCharacter())
		{
			if (USkeletalMeshComponent* CharacterMesh = Character->GetMesh())
			{
				CharacterMesh->SetReceivesDecals(false);
			}

			RangeEffectDecalActor->SetActorLocation(Character->GetActorLocation());
			RangeEffectDecalActor->AttachToActor(Character, FAttachmentTransformRules::KeepWorldTransform);
		}
	}
	const float Size = LockOnComponent->GetLockOnDistance() / 2.f;
	RangeEffectDecalComponent->DecalSize = FVector(Size, Size, Size);

	RangeEffectDecalActor->SetActorHiddenInGame(false);

	//FOnTimelineVector TimelineVectorUpdateDelegate;
	FOnTimelineFloat RadiusDelegate;
	FOnTimelineFloat InnerRadiusDelegate;
	//FOnTimelineFloat IInnerRadiusDelegate;
	FOnTimelineEvent RangeEffectEndDelegate;

	//TimelineVectorUpdateDelegate.BindUFunction(this, FName("UpdateRangeEffectSize"));
	//RangeEffectTimeline.AddInterpVector(RangeEffectVectorCurve, TimelineVectorUpdateDelegate);

	InnerRadiusDelegate.BindUFunction(this, FName("UpdateRangeEffectInnerRadius"));
	RangeEffectTimeline.AddInterpFloat(InnerRadiusFloatCurve, InnerRadiusDelegate);

	//IInnerRadiusDelegate.BindUFunction(this, FName("UpdateRangeEffectIInnerRadius"));
	//RangeEffectTimeline.AddInterpFloat(IInnerRadiusFloatCurve, IInnerRadiusDelegate);

	RadiusDelegate.BindUFunction(this, FName("UpdateRangeEffectRadius"));
	RangeEffectTimeline.AddInterpFloat(RadiusFloatCurve, RadiusDelegate);

	RangeEffectEndDelegate.BindUFunction(this, FName("FinishUpdateRangeEffect"));
	RangeEffectTimeline.AddEvent(IInnerRadiusFloatCurve->FloatCurve.GetLastKey().Time, RangeEffectEndDelegate);

	RangeEffectTimeline.SetPlayRate(1.0f);

	RangeEffectTimeline.PlayFromStart();
}
