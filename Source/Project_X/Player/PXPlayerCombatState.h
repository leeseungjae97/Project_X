// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "PXPlayerCombatState.generated.h"

class APXPartyComponent;
class ACombatCharacter;
class ATXPlayerController;
UCLASS()
class PROJECT_X_API APXPlayerCombatState : public APlayerState
{
	GENERATED_BODY()
	
public:
	APXPlayerCombatState();

protected:
	UPROPERTY()
	APXPartyComponent* PartyComponent;

	UPROPERTY()
	ACombatCharacter* Character;

	UPROPERTY()
	ATXPlayerController* PlayerController;

};
