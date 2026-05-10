// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/PXHealthSet.h"
#include "GameplayEffectExtension.h"

UPXHealthSet::UPXHealthSet()
{
	InitHealth(100.f);
	InitMaxHealth(100.f);
	InitStemina(100.f);
	InitMaxStemina(100.f);
}

void UPXHealthSet::ClampAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::ClampAttributeChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	else if (Attribute == GetMaxSteminaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxStemina());
	}
}

void UPXHealthSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetInDamageAttribute())
	{
		float InDamageDone = GetInDamage();
		SetInDamage(0.f);
		if (InDamageDone > 0.0f)
		{
			if (OnDamageTaken.IsBound())
			{
				const FGameplayEffectContextHandle& EffectContextHandle = Data.EffectSpec.GetEffectContext();
				AActor* Instigator = EffectContextHandle.GetOriginalInstigator();
				AActor* Causer = EffectContextHandle.GetEffectCauser();

				OnDamageTaken.Broadcast(Instigator, Causer, Data.EffectSpec.CapturedSourceTags.GetActorTags(), Data.EvaluatedData.Magnitude);
			}

			if (InDamageDone > 0.0f && GetHealth() > 0.0f)
			{
				const float NewHealth = GetHealth() - InDamageDone;
				SetHealth(FMath::Clamp(NewHealth, 0.0f, GetMaxHealth()));
			}
		}
	}
}
