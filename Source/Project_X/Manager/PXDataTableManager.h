// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PXDataTableManager.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_X_API UPXDataTableManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	void LoadAllTables();
	
	const UDataTable* GetTableByName(FName TableName) const;


private:
	// 이름 → 경로
	TMap<FName, FString> TablePaths;

	// 이름 → 로드된 테이블
	UPROPERTY()
	TMap<FName, UDataTable*> LoadedTables;

	// 아이템 테이블
	// UPROPERTY() TObjectPtr<UDataTable> ItemTable;
};
