// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PXCombatGameMode.generated.h"

class UGameNetworkSubsystem;
/**
 * 
 */
UCLASS()
class PROJECT_X_API APXCombatGameMode : public AGameModeBase
{
	GENERATED_BODY()

	APXCombatGameMode();

public:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
};
