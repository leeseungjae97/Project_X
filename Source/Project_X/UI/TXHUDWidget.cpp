// Fill out your copyright notice in the Description page of Project Settings.


#include "TXHUDWidget.h"

#include "Components/Image.h"
#include "Components/ProgressBar.h"

void UTXHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (IMG_HPBar)
	{
		if (UMaterialInterface* BaseMat = IMG_HPBar->GetDynamicMaterial())
		{
			MID_HpBar = IMG_HPBar->GetDynamicMaterial(); // 만약 SetBrushFromMaterial을 사용했다면 직접 GetBrush().GetResourceObject()에서 캐스팅 필요'
			MID_HpBar->SetScalarParameterValue("TickDownValue",1.f);
			MID_HpBar->SetScalarParameterValue("Value",1.f);
		}
	}

	if (IMG_XPRing)
	{
		if (UMaterialInterface* BaseMat = IMG_XPRing->GetDynamicMaterial())
		{
			MID_XpRing = IMG_XPRing->GetDynamicMaterial();
		}
	}
}

void UTXHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	UpdateHpBar(InDeltaTime);
	UpdateStaminaBar(InDeltaTime);
	UpdateXPBar(InDeltaTime);
}

void UTXHUDWidget::UpdateHpBar(float DeltaTime)
{
	CurrentHPFrontPercent = FMath::FInterpTo(CurrentHPFrontPercent, TargetHPPercent, DeltaTime, HPInterpSpeed);
	CurrentHPBackPercent = FMath::FInterpTo(CurrentHPBackPercent, TargetHPPercent, DeltaTime, HPInterpSpeed * 0.25f);
	if (MID_HpBar)
	{
		MID_HpBar->SetScalarParameterValue("Value",CurrentHPFrontPercent);
		MID_HpBar->SetScalarParameterValue("TickDownValue",CurrentHPBackPercent);
	}
}

void UTXHUDWidget::UpdateXPBar(float DeltaTime)
{
	CurrentXPPercent = FMath::FInterpTo(CurrentXPPercent, TargetXPPercent, DeltaTime, XPInterpSpeed);
	if (MID_XpRing)
	{
		MID_XpRing->SetScalarParameterValue("Gap",CurrentXPPercent);
	}
}

void UTXHUDWidget::UpdateStaminaBar(float DeltaTime)
{
	CurrentStaminaPercent = FMath::FInterpTo(CurrentStaminaPercent, TargetStaminaPercent, DeltaTime, StaminaInterpSpeed);

	if (PB_StaminaBar)
	{
		PB_StaminaBar->SetPercent(CurrentStaminaPercent);
	}
}

void UTXHUDWidget::SetHP(float HPPercent)
{
	TargetHPPercent = FMath::Clamp(HPPercent, 0.0f, 1.0f);
}

void UTXHUDWidget::SetStamina(float StaminaPercent)
{
	TargetStaminaPercent = FMath::Clamp(StaminaPercent, 0.0f, 1.0f);
}

void UTXHUDWidget::SetXPRing(float XPRingPercent)
{
	TargetXPPercent = FMath::Clamp(XPRingPercent, 0.0f, 1.0f);
}
