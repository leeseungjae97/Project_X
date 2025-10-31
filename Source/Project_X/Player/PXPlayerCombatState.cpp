// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/PXPlayerCombatState.h"
#include "PXComponents/PXPartyComponent.h"

APXPlayerCombatState::APXPlayerCombatState()
{
	PartyComponent = CreateDefaultSubobject<APXPartyComponent>(TEXT("TX Party Component"));
}
