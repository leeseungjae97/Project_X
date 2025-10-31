#pragma once
#include "CoreMinimal.h"
#include "GameSystem/Augments/AugmentEffect/PXAugmentEffect.h"
#include "Core/EnumTypes.h"
#include "PXAugmentData.generated.h"

USTRUCT(BlueprintType)
struct FPXAugmentData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FName AugmentID;

	UPROPERTY(EditAnywhere)
	FText Name;

	UPROPERTY(EditAnywhere)
	FText Description;

	UPROPERTY(EditAnywhere)
	EAugmentTriggerCondition TriggerCondition;

	UPROPERTY(EditAnywhere)
	UTexture2D* Icon;

	UPROPERTY(EditAnywhere)
	TArray<FPXAugmentEffect> Effects;
};
