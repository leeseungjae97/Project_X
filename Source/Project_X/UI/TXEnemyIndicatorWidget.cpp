// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/TXEnemyIndicatorWidget.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/PlayerCameraManager.h"

void UTXEnemyIndicatorWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

void UTXEnemyIndicatorWidget::SetEnemyIndex(int32 Index)
{
	if (EnemyIndexText)
	{
		EnemyIndexText->SetText(FText::AsNumber(Index));
	}
}