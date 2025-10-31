#include "UI/TXPartyRightClickWidget.h"

#include "Components/Button.h"
#include "GameSystem/Party/PXPartySystem.h"

void UTXPartyRightClickWidget::NativeTick(const FGeometry& MyGeometry, float DeltaTime)
{
	Super::NativeTick(MyGeometry, DeltaTime);
}

void UTXPartyRightClickWidget::NativeConstruct()
{
	Super::NativeConstruct();

	PartySystem = GetGameInstance()->GetSubsystem<UPXPartySystem>();
	KickButton->OnClicked.AddDynamic(this, &UTXPartyRightClickWidget::Kick);
	ChangeLeaderButton->OnClicked.AddDynamic(this, &UTXPartyRightClickWidget::ChangeLeader);
}

void UTXPartyRightClickWidget::SetObjectId(int64 InObjectId)
{
	ObjectId = InObjectId;
	if (!PartySystem)
		return;
	
	KickDelegate.Clear();
	KickDelegate.BindDynamic(PartySystem, &UPXPartySystem::MemberRemove);

	ChangeLeaderDelegate.Clear();
	ChangeLeaderDelegate.BindDynamic(PartySystem, &UPXPartySystem::ChangePartyLeader);
}

void UTXPartyRightClickWidget::Kick()
{
	KickDelegate.ExecuteIfBound(ObjectId);
}

void UTXPartyRightClickWidget::ChangeLeader()
{
	ChangeLeaderDelegate.ExecuteIfBound(ObjectId);
}
