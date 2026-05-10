// Fill out your copyright notice in the Description page of Project Settings.


#include "TXHUDWidget.h"

#include "Components/Image.h"
#include "Components/Border.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/PanelWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/Widget.h"
#include "Blueprint/WidgetTree.h"

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

	CreateFallbackBossHealthUI();
	SetBossHealthVisible(false);
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

void UTXHUDWidget::SetRoundInfo(int32 RoundIndex, int32 RemainingEnemies)
{
	if (Text_Round)
	{
		Text_Round->SetText(FText::Format(NSLOCTEXT("PXHUD", "RoundFormat", "Round {0}"), FText::AsNumber(RoundIndex)));
	}

	if (Text_RemainingEnemies)
	{
		Text_RemainingEnemies->SetText(FText::Format(NSLOCTEXT("PXHUD", "RemainingEnemiesFormat", "Enemies {0}"), FText::AsNumber(RemainingEnemies)));
	}
}

void UTXHUDWidget::SetResultText(const FText& ResultText)
{
	if (Text_Result)
	{
		Text_Result->SetText(ResultText);
		Text_Result->SetVisibility(ResultText.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	}
}

void UTXHUDWidget::CreateFallbackBossHealthUI()
{
	if (BossHealthRoot || PB_BossHPBar || !WidgetTree)
	{
		return;
	}

	UPanelWidget* RootPanel = Cast<UPanelWidget>(GetRootWidget());
	if (!RootPanel)
	{
		return;
	}

	UBorder* RootBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("BossHealthRoot_Runtime"));
	UVerticalBox* VerticalBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("BossHealthBox_Runtime"));
	Text_BossName = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Text_BossName_Runtime"));
	PB_BossHPBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("PB_BossHPBar_Runtime"));

	if (!RootBorder || !VerticalBox || !Text_BossName || !PB_BossHPBar)
	{
		return;
	}

	Text_BossName->SetText(NSLOCTEXT("PXHUD", "BossNameDefault", "BOSS"));
	Text_BossName->SetJustification(ETextJustify::Center);
	Text_BossName->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.85f, 0.75f, 1.0f)));

	PB_BossHPBar->SetPercent(1.0f);
	PB_BossHPBar->SetFillColorAndOpacity(FLinearColor(0.95f, 0.05f, 0.05f, 1.0f));

	VerticalBox->AddChildToVerticalBox(Text_BossName);
	VerticalBox->AddChildToVerticalBox(PB_BossHPBar);
	RootBorder->SetContent(VerticalBox);
	RootBorder->SetPadding(FMargin(10.0f, 6.0f));
	RootBorder->SetBrushColor(FLinearColor(0.02f, 0.0f, 0.0f, 0.65f));

	RootPanel->AddChild(RootBorder);
	BossHealthRoot = RootBorder;

	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(RootBorder->Slot))
	{
		CanvasSlot->SetAnchors(FAnchors(0.5f, 0.0f, 0.5f, 0.0f));
		CanvasSlot->SetAlignment(FVector2D(0.5f, 0.0f));
		CanvasSlot->SetPosition(FVector2D(0.0f, 36.0f));
		CanvasSlot->SetSize(FVector2D(560.0f, 58.0f));
	}
}

void UTXHUDWidget::SetBossHealthVisible(bool bVisible)
{
	if (BossHealthRoot)
	{
		BossHealthRoot->SetVisibility(bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
}

void UTXHUDWidget::SetBossHealth(float HealthPercent, const FText& BossName)
{
	CreateFallbackBossHealthUI();

	if (Text_BossName)
	{
		Text_BossName->SetText(BossName);
	}

	if (PB_BossHPBar)
	{
		PB_BossHPBar->SetPercent(FMath::Clamp(HealthPercent, 0.0f, 1.0f));
	}
}
