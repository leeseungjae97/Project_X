#include "UI/TXPartyListEntryWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void UTXPartyListEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	ReadyText->SetVisibility(ESlateVisibility::Hidden);
	LevelText->SetVisibility(ESlateVisibility::Hidden);
	ClassIconImage->SetVisibility(ESlateVisibility::Hidden);
	PlayerNameText->SetVisibility(ESlateVisibility::Hidden); 
	PartyLeaderIcon->SetVisibility(ESlateVisibility::Hidden);

	if (DefaultPreviewMI)
	{
		DefaultPreviewBrush.SetResourceObject(DefaultPreviewMI);
		DefaultPreviewBrush.DrawAs = ESlateBrushDrawType::Image;
	}
}

FReply UTXPartyListEntryWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	FVector2f AbsoluteMousePosition = InMouseEvent.GetScreenSpacePosition();
	FKey Key = InMouseEvent.GetEffectingButton();

	if (Key == EKeys::RightMouseButton)
	{
		DelegateObject->OnMouseEnterEvent.ExecuteIfBound(Index, ObjectId, AbsoluteMousePosition);
	}

	Reply.Handled();
	
	return Reply; 
}

void UTXPartyListEntryWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	DelegateObject = Cast<UTXPartyListEntryWidget>(ListItemObject);
	if (!DelegateObject || !DelegateObject->IsRefresh())
	{
		InActive();
		return;
	}
	
	SetData(DelegateObject);

	Active();
}

void UTXPartyListEntryWidget::SetData(UObject* ListItemObject)
{
	UTXPartyListEntryWidget* NewWidget = Cast<UTXPartyListEntryWidget>(ListItemObject);
	if (!NewWidget)
		return;

	PlayerName = NewWidget->GetPlayerName();
	bIsPlayerReady = NewWidget->IsPlayerReady();
	bIsLeader = NewWidget->IsLeader();
	Level = NewWidget->GetLevel();
	Index = NewWidget->GetIndex();
	ObjectId = NewWidget->GetObjectId();

	PlayerNameText->SetText(FText::FromString(PlayerName));
	PartyLeaderIcon->SetVisibility(bIsLeader ? ESlateVisibility::Visible : ESlateVisibility::Hidden);

	FString LevelString = FString::Printf(TEXT("Lvl %lld"), Level);
	LevelText->SetText(FText::FromString(LevelString));
	
	ReadyText->SetVisibility(bIsPlayerReady ? ESlateVisibility::Visible : ESlateVisibility::Hidden);

	// DefaultPreviewBrush.SetResourceObject(DefaultPreviewMI);
	// DefaultPreviewBrush.DrawAs = ESlateBrushDrawType::Image;
	// PlayerPreviewImage->SetBrush(DefaultPreviewBrush);
}

void UTXPartyListEntryWidget::Active()
{
	LevelText->SetVisibility(ESlateVisibility::Visible);
	ClassIconImage->SetVisibility(ESlateVisibility::Visible);
	PlayerNameText->SetVisibility(ESlateVisibility::Visible);
}

void UTXPartyListEntryWidget::InActive()
{
	ObjectId = -1;

	PlayerPreviewImage->SetBrush(DefaultPreviewBrush);

	ReadyText->SetVisibility(ESlateVisibility::Hidden);
	LevelText->SetVisibility(ESlateVisibility::Hidden);
	ClassIconImage->SetVisibility(ESlateVisibility::Hidden);
	PlayerNameText->SetVisibility(ESlateVisibility::Hidden);
}

void UTXPartyListEntryWidget::SetReady(bool bReady)
{
	bIsPlayerReady = bReady;
}

void UTXPartyListEntryWidget::SetPlayerPreviewImage() { if (!PlayerPreviewImage) return; }

void UTXPartyListEntryWidget::SetLevel(int64 InLevel)
{
	Level = InLevel;
}

void UTXPartyListEntryWidget::SetClassIconImage()
{
	if (!ClassIconImage) return;
}
