#include "TXComponents/TXPartyComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameSystem/Party/TXPartySystem.h"
#include "UI/TXPartyListEntryWidget.h"

ATXPartyComponent::ATXPartyComponent()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ATXPartyComponent::BeginPlay()
{
	Super::BeginPlay();
}

void ATXPartyComponent::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ATXPartyComponent::SetReady(bool bReady)
{
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(GetWorld());
	if (!GameInstance)
		return;

	UTXPartySystem* PartySystem = GameInstance->GetSubsystem<UTXPartySystem>();
	if (!PartySystem)
		return;

	PartySystem->PlayerReady(bReady);
}