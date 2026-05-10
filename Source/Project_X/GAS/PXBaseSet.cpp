// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/PXBaseSet.h"

void UPXBaseSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);
}

void UPXBaseSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
}

void UPXBaseSet::ClampAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
}
