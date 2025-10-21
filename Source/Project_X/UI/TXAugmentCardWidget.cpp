// Fill out your copyright notice in the Description page of Project Settings.

#include "TXAugmentCardWidget.h"
#include "GameSystem/Augments/Data/TXAugmentData.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Button.h"

void UTXAugmentCardWidget::SetupFromData(const FTXAugmentData& InData)
{
	CachedAugmentID = InData.AugmentID;
	if (Text_Name)  Text_Name->SetText(InData.Name);
	if (Text_Desc)  Text_Desc->SetText(InData.Description);
	if (Img_Icon && InData.Icon) Img_Icon->SetBrushFromTexture(InData.Icon);
	if (Btn_Select) { Btn_Select->OnClicked.Clear(); Btn_Select->OnClicked.AddDynamic(this, &UTXAugmentCardWidget::HandleClicked); }
}

void UTXAugmentCardWidget::HandleClicked()
{
	OnAugmentSelected.Broadcast(CachedAugmentID);
}
