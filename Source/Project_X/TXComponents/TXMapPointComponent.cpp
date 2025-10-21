#include "TXComponents/TXMapPointComponent.h"
#include "UI/TXEnemyMarkerWidget.h"
#include "UI/TXMiniMapWidget.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Player/Controller/TXPlayerController.h"
#include "GameFramework/Character.h"

UTXMapPointComponent::UTXMapPointComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UTXMapPointComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UTXMapPointComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	CreateMapPoint();
}

void UTXMapPointComponent::CreateMapPoint()
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
