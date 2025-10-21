// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TXCombatGameMode.generated.h"

class UGameNetworkSubsystem;
/**
 * 
 */
UCLASS()
class PROJECT_X_API ATXCombatGameMode : public AGameModeBase
{
	GENERATED_BODY()

	ATXCombatGameMode();

public:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
};
