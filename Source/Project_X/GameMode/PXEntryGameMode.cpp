// Fill out your copyright notice in the Description page of Project Settings.


#include "PXEntryGameMode.h"

#include "Manager/AssetManager/PXAssetManager.h"

void APXEntryGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	UPXAssetManager::Get().StartAssetPreLoading(this);
}
