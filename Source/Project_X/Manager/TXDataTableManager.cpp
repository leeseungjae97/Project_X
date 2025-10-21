// Fill out your copyright notice in the Description page of Project Settings.


#include "TXDataTableManager.h"


void UTXDataTableManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 경로 등록 예시
	// TablePaths.Add("Augment", TEXT("/Game/Data/DT_Augments.DT_Augments"));
	
	LoadAllTables();
}

void UTXDataTableManager::LoadAllTables()
{
	for (const auto& Pair : TablePaths)
	{
		const FName& TableName = Pair.Key;
		const FString& Path = Pair.Value;

		UDataTable* Loaded = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *Path));
		if (Loaded)
		{
			LoadedTables.Add(TableName, Loaded);
			UE_LOG(LogTemp, Log, TEXT("Loaded DataTable: %s"), *TableName.ToString());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to load DataTable: %s (%s)"), *TableName.ToString(), *Path);
		}
	}
}

const UDataTable* UTXDataTableManager::GetTableByName(FName TableName) const
{
	if (const UDataTable* const* FoundTable = LoadedTables.Find(TableName))
	{
		return *FoundTable;
	}
	return nullptr;
}
