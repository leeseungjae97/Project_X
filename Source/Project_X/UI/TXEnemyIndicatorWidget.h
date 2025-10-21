// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TXEnemyIndicatorWidget.generated.h"

class UTextBlock;

UCLASS()
class PROJECT_X_API UTXEnemyIndicatorWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* EnemyIndexText;

public:
	void SetEnemyIndex(int32 Index);
};
