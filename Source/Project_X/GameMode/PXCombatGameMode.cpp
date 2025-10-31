// Fill out your copyright notice in the Description page of Project Settings.


#include "PXCombatGameMode.h"

APXCombatGameMode::APXCombatGameMode()
{
	PrimaryActorTick.bCanEverTick      = true;
}

void APXCombatGameMode::BeginPlay()
{
	Super::BeginPlay();
}

void APXCombatGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
