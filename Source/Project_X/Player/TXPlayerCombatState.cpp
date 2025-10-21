// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/TXPlayerCombatState.h"
#include "TXComponents/TXPartyComponent.h"

ATXPlayerCombatState::ATXPlayerCombatState()
{
	PartyComponent = CreateDefaultSubobject<ATXPartyComponent>(TEXT("TX Party Component"));
}
