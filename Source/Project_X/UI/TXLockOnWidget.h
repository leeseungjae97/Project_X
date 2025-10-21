// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TXLockOnWidget.generated.h"

/**
 * 
 */
class ACharacter;
class UWidgetComponent;

UCLASS()
class PROJECT_X_API UTXLockOnWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* LockOn;

	UPROPERTY()
	UWidgetComponent* OwningWidgetComponent;

public:
	void SetOwningWidgetComponent(UWidgetComponent* InComponent);

	void PlayLockOnAnimation();
	void AttachToTargetAndPlay(ACharacter* TargetActor);
};
