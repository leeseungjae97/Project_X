// Copyright Epic Games, Inc. All Rights Reserved.


#include "CombatGameMode.h"

#include "AI/NavDataGenerator.h"
#include "AI/NavigationSystemBase.h"
#include "NavMesh/RecastNavMesh.h"

ACombatGameMode::ACombatGameMode()
{

}

void ACombatGameMode::BeginPlay()
{
	Super::BeginPlay();

	UNavigationSystemBase* NavSys = GetWorld()->GetNavigationSystem();
	INavigationDataInterface* NavData = NavSys->GetMainNavData();
	ARecastNavMesh* NavMesh = Cast<ARecastNavMesh>(NavData);

	NavMesh->GetGenerator()->ExportNavigationData(FString(TEXT("G:\\TestNav.obj")));
	NavMesh->GetGenerator()->ExportNavigationData(FString(TEXT("G:\\TestNav.bin")));
}
