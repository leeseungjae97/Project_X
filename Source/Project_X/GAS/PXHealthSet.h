// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/PXBaseSet.h"
#include "AbilitySystemComponent.h"
#include "PXHealthSet.generated.h"


DECLARE_MULTICAST_DELEGATE_FourParams(FDamageTakenEvent, AActor*, AActor*, const FGameplayTagContainer&, float);

/**
 * 
 */
UCLASS()
class PROJECT_X_API UPXHealthSet : public UPXBaseSet
{
	GENERATED_BODY()
public:
	UPXHealthSet();
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Attributes")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS_BASIC(UPXHealthSet, Health);

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Attributes")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS_BASIC(UPXHealthSet, MaxHealth);

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Attributes")
	FGameplayAttributeData Stemina;
	ATTRIBUTE_ACCESSORS_BASIC(UPXHealthSet, Stemina);

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Attributes")
	FGameplayAttributeData MaxStemina;
	ATTRIBUTE_ACCESSORS_BASIC(UPXHealthSet, MaxStemina);

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Attributes")
	FGameplayAttributeData InDamage;
	ATTRIBUTE_ACCESSORS_BASIC(UPXHealthSet, InDamage);

	FDamageTakenEvent OnDamageTaken;
	
public:
	virtual void ClampAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;
};
