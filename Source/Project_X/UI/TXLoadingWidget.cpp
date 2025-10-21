// Fill out your copyright notice in the Description page of Project Settings.


#include "TXLoadingWidget.h"

#include "Components/Button.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"


void UTXLoadingWidget::NativeOnInitialized()
{
	if(PB_LoadingBar != nullptr)
	{
		PB_LoadingBar->SetPercent(0.f);
	}
}

void UTXLoadingWidget::SetPercent(float Percent) const
{
	if(PB_LoadingBar)
		PB_LoadingBar->SetPercent(Percent);
}

void UTXLoadingWidget::ReadyToStart() const
{
	if(PB_LoadingBar)
		PB_LoadingBar->SetPercent(1.f);
	if(TB_Text)
		TB_Text->SetVisibility(ESlateVisibility::HitTestInvisible);
	if(BTN_Click)
		BTN_Click->SetVisibility(ESlateVisibility::Visible);
	if(TB_ObjectName)
		TB_ObjectName->SetVisibility(ESlateVisibility::Collapsed);
}

bool UTXLoadingWidget::BindOnButtonClicked(TFunction<void()> InCallback)
{
	if (!BTN_Click) return false;

	StoredCallback = MoveTemp(InCallback);

	BTN_Click->OnClicked.RemoveAll(this);// 중복 방지
	BTN_Click->OnClicked.AddDynamic(this, &UTXLoadingWidget::HandleButtonClicked);
	return true;
}

void UTXLoadingWidget::SetObjectName(const FString& ObjectName, const int32& CompleteNum, const int32& TotalNum) const
{
	if(TB_ObjectName)
	{
		const FText Template = FText::FromString(TEXT("{0} ({1} / {2})"));

		const FText Display = FText::Format(
			Template,
			FText::FromString(ObjectName),
			FText::AsNumber(CompleteNum),
			FText::AsNumber(TotalNum));

		TB_ObjectName->SetVisibility(ESlateVisibility::Visible);
		TB_ObjectName->SetText(Display);
	}
}

void UTXLoadingWidget::HandleButtonClicked()
{
	if (StoredCallback.IsSet())
	{
		StoredCallback.GetValue()();
	}
}
