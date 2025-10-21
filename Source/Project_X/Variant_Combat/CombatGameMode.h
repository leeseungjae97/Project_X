// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CombatGameMode.generated.h"

/**
 *  Simple GameMode for a third person combat game
 */
UCLASS(abstract)
class PROJECT_X_API ACombatGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:

	ACombatGameMode();

	virtual void BeginPlay() override;
};
