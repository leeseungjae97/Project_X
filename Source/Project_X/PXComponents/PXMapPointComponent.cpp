#include "PXComponents/PXMapPointComponent.h"
#include "UI/TXEnemyMarkerWidget.h"
#include "UI/TXMiniMapWidget.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Player/Controller/TXPlayerController.h"
#include "GameFramework/Character.h"

UPXMapPointComponent::UPXMapPointComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UPXMapPointComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UPXMapPointComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	CreateMapPoint();
}

void UPXMapPointComponent::CreateMapPoint()
{
	if (bIsMapPointCreated)
		return;
	
	if (OwnerCharacter && GetWorld() && GetWorld()->GetFirstPlayerController())
	{
		if (ATXPlayerController* PC = Cast<ATXPlayerController>(GetWorld()->GetFirstPlayerController()))
		{
			if (UTXMiniMapWidget* Widget = PC->GetMiniMapWidget())
			{
				Widget->AddMapPoint(OwnerCharacter);
				bIsMapPointCreated = true;
			}
		}
	}
}
