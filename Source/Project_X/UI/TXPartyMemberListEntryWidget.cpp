#include "UI/TXPartyMemberListEntryWidget.h"

#include "Components/TextBlock.h"

void UTXPartyMemberListEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetVisibility(ESlateVisibility::Hidden);
}

void UTXPartyMemberListEntryWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	SetVisibility(ESlateVisibility::Visible);

	SetData(ListItemObject);
}

void UTXPartyMemberListEntryWidget::SetData(UObject* ListItemObject)
{
	UTXPartyMemberListEntryWidget* Widget = Cast<UTXPartyMemberListEntryWidget>(ListItemObject);
	
	if (!Widget || !Widget->IsRefresh())
	{
		SetVisibility(ESlateVisibility::Hidden);
		return;
	}
	
	PlayerName = Widget->GetPlayerName();
	PlayerHPRatio = Widget->GetPlayerHPRatio();
	PlayerUltTimeRatio = Widget->GetPlayerUltTimeRatio();
}
