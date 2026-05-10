#pragma once

#include "CoreMinimal.h"
#include "Core/EnumTypes.h"
#include "Engine/DataTable.h"
#include "PXStatAugmentData.generated.h"

USTRUCT(BlueprintType)
struct FPXStatAugmentData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Augment")
	FName AugmentID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Augment")
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Augment")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Augment")
	EPXAugmentRarity Rarity = EPXAugmentRarity::Common;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Augment")
	EPXPlayerStatType StatType = EPXPlayerStatType::AttackDamage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Augment")
	float AddValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Augment")
	float MultiplyValue = 1.0f;
};
