// Fill out your copyright notice in the Description page of Project Settings.


#include "TXCombatGameMode.h"

ATXCombatGameMode::ATXCombatGameMode()
{
	PrimaryActorTick.bCanEverTick      = true;
}

void ATXCombatGameMode::BeginPlay()
{
	Super::BeginPlay();
}

void ATXCombatGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
