#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TimelineComponent.h"
#include "TXIndicatorHUDWidget.generated.h"

class UTXEnemyIndicatorWidget;
class AActor;
class UPXLockOnComponent;
class UMaterialInstance;
class UMaterialInstanceDynamic;
class UCurveFloat;
class UCurveVector;
class UDecalComponent;
class ADecalActor;

UCLASS()
class PROJECT_X_API UTXIndicatorHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	void SpawnIndicator();
	
	TArray<TPair<AActor*, float>> DetectedActorsArray;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UTXEnemyIndicatorWidget> IndicatorWidgetClass;

protected:
	UPROPERTY()
	TMap<AActor*, UTXEnemyIndicatorWidget*> ActiveIndicatorsMap;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	float IndicatorPadding = 50.0f; // Default padding

	UPROPERTY(EditAnywhere, Category = "Range Effect")
	UMaterialInstance* RangeEffectMaterial;

	FTimeline RangeEffectTimeline;

	UPROPERTY(EditAnywhere, Category = "Range Effect")
	UCurveVector* RangeEffectVectorCurve;

	UPROPERTY(EditAnywhere, Category = "Range Effect")
	UCurveFloat* InnerRadiusFloatCurve;

	UPROPERTY(EditAnywhere, Category = "Range Effect")
	UCurveFloat* IInnerRadiusFloatCurve;

	UPROPERTY(EditAnywhere, Category = "Range Effect")
	UCurveFloat* RadiusFloatCurve;

private:
	bool CalculateIndicatorPositionAndRotation(AActor* TargetActor, FVector2D& OutScreenPosition, float& OutRotationAngle);

	void SpawnRangeEffectDecal();

	UFUNCTION()
	void UpdateRangeEffectSize(FVector Size);

	UFUNCTION()
	void UpdateRangeEffectIInnerRadius(float IInnerRadius);

	UFUNCTION()
	void UpdateRangeEffectInnerRadius(float InnerRadius);
	UFUNCTION()
	void UpdateRangeEffectRadius(float Radius);

	UFUNCTION()
	void FinishUpdateRangeEffect();
	

	UPROPERTY()
	UPXLockOnComponent* LockOnComponent;

	UPROPERTY()
	ADecalActor* RangeEffectDecalActor;

	UPROPERTY()
	UDecalComponent* RangeEffectDecalComponent;

	UPROPERTY()
	UMaterialInstanceDynamic* RangeEffectMaterialDynamic;

public:
	void StartRangeEffectTimeline();

};
