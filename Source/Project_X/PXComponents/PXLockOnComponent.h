#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/TimelineComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Interfaces/PXLockOnTargetInterface.h"
#include "PXLockOnComponent.generated.h"

class ACombatCharacter;
class UWidgetComponent;
class UTXIndicatorHUDWidget;
class UCurveFloat;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_X_API UPXLockOnComponent : public UActorComponent
{
	GENERATED_BODY()

private:	
	UPXLockOnComponent();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lock On", meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* LockOnWidgetComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lock On", meta = (AllowPrivateAccess = "true"))
	FName LockOnSocketName = FName("LockOnSocket");

	UPROPERTY()
	ACombatCharacter* OwnerCharacter;

protected:
	virtual void BeginPlay() override;

	virtual void DestroyComponent(bool bPromoteChildren = false) override;
private:
	void TraceEnemy(float DeltaTime);

	void HandleLockOnTarget(float DeltaTime);

	void SetVisibilityLockOn(bool InbVisibility);

	void TraceLockOnTarget();

	bool IntervalTargetTrace(FHitResult& HitResult, float DeltaTime);

	void ResetTarget();

	void ChangeLockOnTarget(FHitResult& HitResult);
	void ChangeLockOnTarget(AActor* Actor);

	UFUNCTION()
	bool ConeTraceMulti(const FVector Start, const FRotator Direction, float ConeHeight, float ConeHalfAngle, const TArray<TEnumAsByte<ECollisionChannel>>& QueryObjectTypes, bool bTraceComplex, const TArray<AActor*>& ActorsToIgnore = {}, bool bIgnoreSelf = true, float DrawTime = 0.1f);

	UFUNCTION()
	bool LineTrace(const FVector Start, const FRotator Direction, float Distance, FHitResult& HitResult, const TArray<AActor*>& ActorsToIgnore = {}, const TArray<TEnumAsByte<ECollisionChannel>>& QueryObjectTypes = {}, FLinearColor TraceColor = FLinearColor::Red, FLinearColor TraceHitColor = FLinearColor::Green, float DrawTime = 0.1f);


public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
public:
	void LockOn();

	void UpdateDetectedActorsInRange();

	void MakeIndicatorArray();

	void PickDetectedRangeActors();

	UFUNCTION()
	void UpdateDetectDistance(float Distance);

public:
	UPROPERTY(EditAnywhere, Category = "Lock On|Range")
	UCurveFloat* DetectAreaRadiusCurve;

private:
	FTimeline DectedTimeline;

	float InGameLockOnTraceDistance = 0.f;

	float UpdateLockOnTraceDistance = 0.f;

	UPROPERTY(EditAnywhere, Category = "Lock On|Range", meta = (ClampMin = 0, ClampMax = 360, Units = "Degrees"))
	float LockOnDegree = 15.0f;

	UPROPERTY(EditAnywhere, Category = "Lock On|Range", meta = (ClampMin = 0, ClampMax = 1000000, Units = "cm"))
	float LockOnTraceDistance = 1500.0f;

	UPROPERTY(EditAnywhere, Category = "Lock On", meta = (ClampMin = 0, ClampMax = 1000, Units = "ns"))
	float TraceIntervalTime = 1.f;

	float TraceIntervalElpsedTime = 0.f;

	bool bIsLockOn = false;

	bool bBeforeLockOn = false;

	UPROPERTY()
	AActor* TargetActor = nullptr;

	FVector EyeLoc;
	FRotator EyeRot;

	int DetectedActorsIndex = 0;

	TMap<AActor*, float> DetectedActorsMap;
	TArray<TPair<AActor*, float>> DetectedActorsArray;

public:
	FORCEINLINE bool IsLockOn() const { return bIsLockOn; }
	void SetOwnerCharacter(ACombatCharacter* InOwnerCharacter);

public:
	UPROPERTY()
	UTXIndicatorHUDWidget* IndicatorHUDWidget;

	FORCEINLINE AActor* GetTargetActor() const { return TargetActor; }
	FORCEINLINE ACombatCharacter* GetOwnerCharacter() const { return OwnerCharacter; }
	FORCEINLINE float GetLockOnDistance() const { return LockOnTraceDistance; }
	TArray<TPair<AActor*, float>> GetDetectedActorsArray() const { return DetectedActorsArray; }
};
