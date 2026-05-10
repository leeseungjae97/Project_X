// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "PXBaseSet.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_X_API UPXBaseSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	virtual void ClampAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) const;
};
