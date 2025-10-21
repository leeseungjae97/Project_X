// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "TXPlayerCombatState.generated.h"

class ATXPartyComponent;
class ACombatCharacter;
class ATXPlayerController;
UCLASS()
class PROJECT_X_API ATXPlayerCombatState : public APlayerState
{
	GENERATED_BODY()
	
public:
	ATXPlayerCombatState();

protected:
	UPROPERTY()
	ATXPartyComponent* PartyComponent;

	UPROPERTY()
	ACombatCharacter* Character;

	UPROPERTY()
	ATXPlayerController* PlayerController;

};
