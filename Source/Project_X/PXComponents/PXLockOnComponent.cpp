#include "PXComponents/PXLockOnComponent.h"
#include "UI/TXLockOnWidget.h"
#include "Components/WidgetComponent.h"
#include "Algo/Sort.h"
#include "Engine/OverlapResult.h"
#include "Player/Controller/TXPlayerController.h"
#include "UI/TXIndicatorHUDWidget.h"
#include "GameFramework/SpringArmComponent.h"
#include "CombatCharacter.h"

UPXLockOnComponent::UPXLockOnComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	LockOnWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("Lock On Widget Component"));
}

void UPXLockOnComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UPXLockOnComponent::DestroyComponent(bool bPromoteChildren)
{
	Super::DestroyComponent(bPromoteChildren);
}

void UPXLockOnComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (DectedTimeline.IsPlaying())
		DectedTimeline.TickTimeline(DeltaTime);
	
	if (OwnerCharacter && !IndicatorHUDWidget)
	{
		if (ATXPlayerController* PC = Cast<ATXPlayerController>(OwnerCharacter->GetController()))
		{
			if (UTXIndicatorHUDWidget* Widget = PC->GetIndicatorHUDWidget())
			{
				IndicatorHUDWidget = Widget;
			}
		}
	}
	
	TraceEnemy(DeltaTime);

	DrawDebugSphere(GetWorld(), EyeLoc, InGameLockOnTraceDistance, 32, FColor::Blue, false, 0.1f);
}


/**
 *- TargetActor 있을 때
 *	1. 거리 멀어지면 Reset
 *	2. TargetActor의 UI 노출
 *	3. Indicator가 표시 될 Actor 설정, 거리에 따라 Sort
 *	4. IndicatorWidget에 Actor추가, TargetActor는 제외 됨.
 *		@todo 3번 과정에서 통합되게 변경
 *- TargetActor가 없을 때
 *	1. LineTrace
 *	2. LineTrace 실패하면 ConeTrace
 *	3. 없으면 LockOn 해제
 */
void UPXLockOnComponent::TraceEnemy(float DeltaTime)
{
	if (!OwnerCharacter)
		return;

	if (!bIsLockOn)
	{
		SetVisibilityLockOn(false);
		return;
	}

	FVector CameraLoc = FVector::ZeroVector;
	OwnerCharacter->GetActorEyesViewPoint(CameraLoc, EyeRot);
	
	if (USpringArmComponent* CameraBoom = OwnerCharacter->GetCameraBoom())
	{
		EyeLoc = CameraBoom->GetComponentLocation();
	}

	if (TargetActor)
	{
		HandleLockOnTarget(DeltaTime);
	}
	else
	{
		bBeforeLockOn = false;
		TraceLockOnTarget();
	}
}

void UPXLockOnComponent::HandleLockOnTarget(float DeltaTime)
{
	if (!bBeforeLockOn)
	{
		bBeforeLockOn = true;
		SetVisibilityLockOn(true);
	}

	FVector TargetLoc = TargetActor->GetActorLocation();
	if (IPXLockOnTargetInterface* LockOnTarget = Cast<IPXLockOnTargetInterface>(TargetActor))
	{
		TargetLoc = LockOnTarget->Execute_GetLockOnSocketLocation(TargetActor);
		LockOnTarget->Execute_SetLockOnTargeted(TargetActor, true);
	}

	const FVector OwnerLoc = OwnerCharacter->GetActorLocation();
	if (LockOnTraceDistance < FVector::Dist(OwnerLoc, TargetLoc))
	{
		ResetTarget();
		return;
	}

	FVector ImpulseDirection = (TargetLoc - EyeLoc).GetSafeNormal();
	const FRotator LookAtRotation = ImpulseDirection.Rotation();

	ImpulseDirection.Z = 0.0f;
	OwnerCharacter->SetActorRotation(ImpulseDirection.Rotation());

	if (OwnerCharacter->GetController())
	{
		OwnerCharacter->GetController()->SetControlRotation(LookAtRotation);
	}
	
	UpdateDetectedActorsInRange();
	MakeIndicatorArray();
}

/**
 *@todo : Delegate 사용해서 참조없애기
 */
void UPXLockOnComponent::SetVisibilityLockOn(bool InbVisibility)
{
	if (LockOnWidgetComponent)
	{
		LockOnWidgetComponent->SetVisibility(InbVisibility);

		if (!InbVisibility)
			return;

		if (UUserWidget* LockOnWidget = LockOnWidgetComponent->GetWidget())
		{
			if (UTXLockOnWidget* LockOn = Cast<UTXLockOnWidget>(LockOnWidget))
			{
				if (ACharacter* TargetCharacter = Cast<ACharacter>(TargetActor))
				{
					LockOn->AttachToTargetAndPlay(TargetCharacter);
				}
			}
		}
	}
}

void UPXLockOnComponent::TraceLockOnTarget()
{
	FHitResult HitResult;

	if (LineTrace(EyeLoc, EyeRot, LockOnTraceDistance, HitResult, {}, { ECC_Pawn, ECC_WorldDynamic, ECC_WorldStatic, ECC_Visibility })
		&& HitResult.GetActor())
	{
		if (HitResult.GetActor()->Implements<UPXLockOnTargetInterface>())
		{
			ChangeLockOnTarget(HitResult);
			return;
		}
	}

	const bool bConeTrace = ConeTraceMulti(
		EyeLoc,
		EyeRot,
		LockOnTraceDistance,
		LockOnDegree,
		{ ECC_Pawn, ECC_WorldDynamic, ECC_WorldStatic, ECC_Visibility },
		true
	);

	if (!TargetActor)
	{
		bIsLockOn = false;
	}
}

/**
 *주기마다 Raycast확인, 현재 사용안함.
 */
bool UPXLockOnComponent::IntervalTargetTrace(FHitResult& HitResult, float DeltaTime)
{
	if (!TargetActor)
	{
		ResetTarget();
		return false;
	}

	if (TraceIntervalTime <= TraceIntervalElpsedTime)
	{
		TraceIntervalElpsedTime = 0.0f;
		if (LineTrace(EyeLoc, EyeRot, LockOnTraceDistance, HitResult, {}, { ECC_Pawn, ECC_WorldDynamic, ECC_WorldStatic, ECC_Visibility }))
		{
			if (HitResult.GetActor() && HitResult.GetActor()->Implements<UPXLockOnTargetInterface>())
			{
				if (HitResult.GetActor() != TargetActor)
				{
					ResetTarget();
					return false;
				}
			}
			else
			{
				ResetTarget();
				return false;
			}
		}
	}
	else
	{
		TraceIntervalElpsedTime += DeltaTime;
		return false;
	}

	return true;
}

void UPXLockOnComponent::ResetTarget()
{
	if (IPXLockOnTargetInterface* LockOnTarget = Cast<IPXLockOnTargetInterface>(TargetActor))
	{
		LockOnTarget->Execute_SetLockOnTargeted(TargetActor, false);
	}
	SetVisibilityLockOn(false);

	if (IndicatorHUDWidget)
		IndicatorHUDWidget->DetectedActorsArray.Reset();
	DetectedActorsMap.Reset();

	bBeforeLockOn = false;
	TargetActor = nullptr;
	bIsLockOn = false;
}

void UPXLockOnComponent::ChangeLockOnTarget(FHitResult& HitResult)
{
	if (!HitResult.GetActor())
		return;

	if (IPXLockOnTargetInterface* LockOnTarget = Cast<IPXLockOnTargetInterface>(TargetActor))
	{
		LockOnTarget->Execute_SetLockOnTargeted(TargetActor, false);
	}

	TargetActor = HitResult.GetActor();
	bBeforeLockOn = false;
}

void UPXLockOnComponent::ChangeLockOnTarget(AActor* Actor)
{
	if (!Actor)
		return;

	if (IPXLockOnTargetInterface* LockOnTarget = Cast<IPXLockOnTargetInterface>(TargetActor))
	{
		LockOnTarget->Execute_SetLockOnTargeted(TargetActor, false);
	}

	TargetActor = Actor;
	bBeforeLockOn = false;
}

void UPXLockOnComponent::UpdateDetectedActorsInRange()
{
	if (!bIsLockOn || !OwnerCharacter)
		return;

	UWorld* World = GEngine->GetWorldFromContextObject(OwnerCharacter, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
		return;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(UpdateDetectedActorsInRange), true);
	Params.bReturnPhysicalMaterial = false;
	Params.AddIgnoredActor(OwnerCharacter);
	
	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);

	TArray<FOverlapResult> OverlapResults;

	DetectedActorsMap.Reset();
	DetectedActorsArray.Reset();

	if (World->OverlapMultiByObjectType(OverlapResults, OwnerCharacter->GetActorLocation(), FQuat::Identity, ObjectParams, FCollisionShape::MakeSphere(UpdateLockOnTraceDistance), Params))
	{
		for (const FOverlapResult& Overlap : OverlapResults)
		{
			if (Overlap.GetActor()->Implements<UPXLockOnTargetInterface>())
			{
				AActor* Enemy = Overlap.GetActor();
				const float Distance = FVector::Dist(OwnerCharacter->GetActorLocation(), Enemy->GetActorLocation());
				if (Distance < LockOnTraceDistance)
				{
					DetectedActorsArray.Add(TPair<AActor*, float>(Enemy, Distance));
					DetectedActorsMap.Add(Enemy, Distance);
				}
			}
		}
	}

	DetectedActorsArray.Sort([](const TPair<AActor*, float>& A, const TPair<AActor*, float>& B)
	{
		return A.Value < B.Value;
	});
}

/**@deprecated*/
void UPXLockOnComponent::MakeIndicatorArray()
{
	if (!IndicatorHUDWidget)
		return;

	TArray<TPair<AActor*, float>>& DAA = IndicatorHUDWidget->DetectedActorsArray;
	DAA.SetNum(DetectedActorsArray.Num());
	int diffIndex = 0;
	for (int i = 0; i < DetectedActorsArray.Num(); ++i)
	{
			DAA[DAA.IsValidIndex(i - diffIndex) ? i - diffIndex : i] = DetectedActorsArray[i];
	}
}

void UPXLockOnComponent::PickDetectedRangeActors()
{
	if (!bIsLockOn || !OwnerCharacter || !IndicatorHUDWidget)
		return;

	ChangeLockOnTarget(DetectedActorsArray[DetectedActorsArray.IsValidIndex(DetectedActorsIndex) ? DetectedActorsIndex : 0].Key);

	DetectedActorsIndex++;
	DetectedActorsIndex %= DetectedActorsArray.Num();
}

void UPXLockOnComponent::UpdateDetectDistance(float Distance)
{
	UpdateLockOnTraceDistance = FMath::Clamp(Distance, 0, LockOnTraceDistance);
}

bool UPXLockOnComponent::LineTrace(const FVector Start, const FRotator Direction, float Distance, FHitResult& HitResult, const TArray<AActor*>& ActorsToIgnore, const TArray<TEnumAsByte<ECollisionChannel>>& QueryObjectTypes, FLinearColor TraceColor, FLinearColor TraceHitColor, float DrawTime)
{
	if (!OwnerCharacter)
		return false;

	UWorld* World = GEngine->GetWorldFromContextObject(OwnerCharacter, EGetWorldErrorMode::LogAndReturnNull);

	if (!World)
		return false;

	FVector DirectionVector = Direction.Vector();

	FCollisionQueryParams Params(SCENE_QUERY_STAT(RaycastTrace), true);
	Params.bReturnPhysicalMaterial = true;
	Params.AddIgnoredActor(OwnerCharacter);
	Params.AddIgnoredActors(ActorsToIgnore);

	FCollisionObjectQueryParams ObjectParams;

	if (QueryObjectTypes.Num() != 0)
	{
		for (auto QueryObjectType : QueryObjectTypes)
		{
			ObjectParams.AddObjectTypesToQuery(QueryObjectType);
		}
	}

	if (World->LineTraceSingleByObjectType(HitResult, Start, Start + DirectionVector * Distance, ObjectParams, Params))
	{
		return true;
	}
	return false;
}

/**
 *Cone 모양으로 Trace 
 *Sphere로 Sweep하고 Cone 모양에 벗어나는 Actor 제외
 */
bool UPXLockOnComponent::ConeTraceMulti(const FVector Start, const FRotator Direction, float ConeHeight, float ConeHalfAngle, const TArray<TEnumAsByte<ECollisionChannel>>& QueryObjectTypes, bool bTraceComplex, const TArray<AActor*>& ActorsToIgnore, bool bIgnoreSelf, float DrawTime)
{
	if (!OwnerCharacter)
		return false;

	UWorld* World = GEngine->GetWorldFromContextObject(OwnerCharacter, EGetWorldErrorMode::LogAndReturnNull);

	if (!World)
		return false;

	TArray<FHitResult> OutHit;
	TArray<FHitResult> ActorHit;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(ConeTraceMulti), bTraceComplex);
	Params.bReturnPhysicalMaterial = false;
	Params.AddIgnoredActors(ActorsToIgnore);
	if (bIgnoreSelf)
		Params.AddIgnoredActor(OwnerCharacter);

	FCollisionObjectQueryParams ObjectParams;

	if (QueryObjectTypes.Num() != 0)
	{
		for (auto QueryObjectType : QueryObjectTypes)
		{
			ObjectParams.AddObjectTypesToQuery(QueryObjectType);
		}
	}
	ObjectParams.IgnoreMask = ECC_TO_BITFIELD(ECC_WorldStatic) | ECC_TO_BITFIELD(ECC_WorldDynamic);

	const FVector End = Start + (Direction.Vector() * ConeHeight);
	const double ConeHalfAngleRad = FMath::DegreesToRadians(ConeHalfAngle);

	// ConeRadius = height * tan(theta / 2)
	const double ConeBaseRadius = ConeHeight * tan(ConeHalfAngleRad);
	const FCollisionShape SphereSweep = FCollisionShape::MakeSphere(ConeBaseRadius);

	World->SweepMultiByObjectType(
		OutHit,
		Start,
		End,
		Direction.Quaternion(),
		ObjectParams,
		SphereSweep,
		Params
	);

#if ENABLE_DRAW_DEBUG
	const double ConeSlantHeight = FMath::Sqrt((ConeBaseRadius * ConeBaseRadius) + (ConeHeight * ConeHeight)); // s = sqrt(r^2 + h^2)
	DrawDebugCone(World, Start, Direction.Vector(), ConeSlantHeight, ConeHalfAngleRad, ConeHalfAngleRad, 32, FColor::Red, false, 0.2f);
#endif // ENABLE_DRAW_DEBUG

	for (const FHitResult& HitResult : OutHit)
	{
		if (!HitResult.GetActor() || !HitResult.GetActor()->Implements<UPXLockOnTargetInterface>())
			continue;

		// Hit Point 방향 계산
		const FVector HitDirection = (HitResult.ImpactPoint - Start).GetSafeNormal();
		// |A|*|B| = 1, A와 B는 Unit Vector, 내적 값은 cos(theta)
		const double Dot = FVector::DotProduct(Direction.Vector(), HitDirection);

		const double ConeDistance = FVector::Dist(Start, HitResult.ImpactPoint);

		//const double ConeDistance = FVector::Dist(Start, Start + ConeHeight);

		// Dot 값이 1보다 작거나 같고 -1보다 크거나 같게
		const double DeltaAngle = FMath::Acos(Dot);

		// Cone Angle밖에 있는 Hit 제외
		if (DeltaAngle > ConeHalfAngleRad)
		{
			continue;
		}

		const double Distance = (HitResult.ImpactPoint - Start).Length();

		// Hypotenuse = adjacent / cos(theta)
		const double LengthAtAngle = ConeHeight / cos(DeltaAngle);

		// Hit Point가 Cone의 끝에 있는 경우 Continue
		if (Distance > LengthAtAngle)
		{
			continue;
		}

		ActorHit.Add(HitResult);
	}

	float MinDistance = MAX_FLT;
	FHitResult ClosestHit;

	for (const FHitResult& Hit : ActorHit)
	{
		const double ActorDistance = FVector::Dist(Start, Hit.ImpactPoint);

		if (MinDistance >= ActorDistance)
		{
			MinDistance = ActorDistance;
			ClosestHit = Hit;
		}

		//DrawDebugLine(World, Start, Hit.ImpactPoint, FColor::Blue, EDrawDebugTrace::ForOneFrame, DrawTime, 0, 0.5f);
	}

	ChangeLockOnTarget(ClosestHit);

	return true;
}

/**
 * LockOnUpdateDistance 수치 1초동안 LockOnDistance로 증가, 기존 LockOn에서 계산한 변수 초기화
 */
void UPXLockOnComponent::LockOn()
{
	UpdateLockOnTraceDistance = 0.f;
	FOnTimelineFloat DistanceDelegate;
	DistanceDelegate.BindUFunction(this, FName("UpdateDetectDistance"));

	auto fCurve = DetectAreaRadiusCurve->FloatCurve;
	FKeyHandle KeyHandle = fCurve.FindKey(fCurve.GetLastKey().Time);
	fCurve.SetKeyValue(KeyHandle, LockOnTraceDistance);

	DetectAreaRadiusCurve->FloatCurve = fCurve;

	DectedTimeline.AddInterpFloat(DetectAreaRadiusCurve, DistanceDelegate);
	DectedTimeline.PlayFromStart();

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("LockOn"));
	bIsLockOn = !bIsLockOn;
	if (!bIsLockOn)
	{
		ResetTarget();
	}
	else
	{
		UpdateDetectedActorsInRange();

		if (IndicatorHUDWidget)
			IndicatorHUDWidget->StartRangeEffectTimeline();
	}

	if (LockOnWidgetComponent)
	{
		if (UUserWidget* LockOnWidget = LockOnWidgetComponent->GetWidget())
		{
			if (UTXLockOnWidget* LockOn = Cast<UTXLockOnWidget>(LockOnWidget))
			{
				LockOn->SetOwningWidgetComponent(LockOnWidgetComponent);
			}
		}
	}
}

void UPXLockOnComponent::SetOwnerCharacter(ACombatCharacter* InOwnerCharacter)
{
	OwnerCharacter = InOwnerCharacter;
}