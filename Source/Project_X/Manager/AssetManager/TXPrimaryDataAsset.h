// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TXPrimaryDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_X_API UTXPrimaryDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FSoftObjectPath> AssetsToLoad;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FDirectoryPath> DirectoriesToScan;
};
