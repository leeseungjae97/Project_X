#include "PXComponents/PXPartyComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameSystem/Party/PXPartySystem.h"
#include "UI/TXPartyListEntryWidget.h"

APXPartyComponent::APXPartyComponent()
{
	PrimaryActorTick.bCanEverTick = true;
}

void APXPartyComponent::BeginPlay()
{
	Super::BeginPlay();
}

void APXPartyComponent::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APXPartyComponent::SetReady(bool bReady)
{
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(GetWorld());
	if (!GameInstance)
		return;

	UPXPartySystem* PartySystem = GameInstance->GetSubsystem<UPXPartySystem>();
	if (!PartySystem)
		return;

	PartySystem->PlayerReady(bReady);
}