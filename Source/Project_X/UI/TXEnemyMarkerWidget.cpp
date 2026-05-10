// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/TXEnemyMarkerWidget.h"
#include "GameFramework/PlayerController.h"

void UTXEnemyMarkerWidget::NativePreConstruct()
{
	EnsureValidActorReference();
	Super::NativePreConstruct();
}

void UTXEnemyMarkerWidget::NativeConstruct()
{
	EnsureValidActorReference();
	Super::NativeConstruct();
}

void UTXEnemyMarkerWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	EnsureValidActorReference();
	Super::NativeTick(MyGeometry, InDeltaTime);
}

void UTXEnemyMarkerWidget::EnsureValidActorReference()
{
	if (IsValid(Actor))
	{
		return;
	}

	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		Actor = PlayerController->GetPawn();
	}

	SetVisibility(IsValid(Actor) ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}

void UTXEnemyMarkerWidget::SetMarkerActor(AActor* InActor)
{
	Actor = InActor;
	SetVisibility(IsValid(Actor) ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}

void UTXEnemyMarkerWidget::SetMarkerStyle(const FLinearColor& InColor, float InScale)
{
	MarkerColor = InColor;
	MarkerScale = FMath::Max(0.1f, InScale);
	SetRenderScale(FVector2D(MarkerScale, MarkerScale));
	SetColorAndOpacity(MarkerColor);
}
