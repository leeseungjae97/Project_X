#pragma once
#include "GameplayTagContainer.h"
#include "Core/EnumTypes.h"

#include "TXAugmentEffect.generated.h"

USTRUCT(BlueprintType)
struct FTXAugmentEffect
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	EAugmentEffectType EffectType;

	UPROPERTY(EditAnywhere)
	float Value;

	UPROPERTY(EditAnywhere)
	float Duration = 0.0f; // 지속 효과 시간(있다면)

	UPROPERTY(EditAnywhere)
	EAugmentTarget Target;

	UPROPERTY(EditAnywhere)
	FGameplayTag TagToApply;

};
