// Fill out your copyright notice in the Description page of Project Settings.


#include "TXEntryGameMode.h"

#include "Manager/AssetManager/TXAssetManager.h"

void ATXEntryGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	UTXAssetManager::Get().StartAssetPreLoading(this);
}
