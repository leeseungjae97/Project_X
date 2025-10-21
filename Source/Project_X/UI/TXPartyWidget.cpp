#include "UI/TXPartyWidget.h"

#include "TXPartyRightClickWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h"
#include "UI/TXPartyListEntryWidget.h"
#include "GameSystem/Party/TXPartySystem.h"
#include "Components/ListView.h"
#include "Spatial/GeometrySet3.h"

void UTXPartyWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UI_PartyRightClickWidget->SetVisibility(ESlateVisibility::Hidden);
	PartySystem = GetGameInstance()->GetSubsystem<UTXPartySystem>();
	if (PartySystem)
	{
		PartySystem->OnPartyDataChanged.AddDynamic(this, &UTXPartyWidget::RefreshPartyList);
		PartySystem->OnPartyLeaderChanged.AddDynamic(this, &UTXPartyWidget::PartyLeaderChange);
	}
	// MakeList();
	RefreshPartyList({{1, FString(TEXT("Name1")), 10, true}, {2, FString(TEXT("Name2")), 99, false}});
}

void UTXPartyWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	Geometry = MyGeometry;
}

void UTXPartyWidget::NativePreConstruct() 
{ 
	Super::NativePreConstruct();

	MakeList();
}

void UTXPartyWidget::NativeDestruct()
{
	Super::NativeDestruct();
	if (PartySystem)
	{
		PartySystem->OnPartyDataChanged.RemoveAll(this);
	}
}

void UTXPartyWidget::RefreshPartyList(const TArray<FPartyMemberInfo>& Members) 
{
	PartyMap.Empty();
	
	for (int i = 0 ; i < PartyListView->GetListItems().Num(); ++i)
	{
		UObject* ListEntry = PartyListView->GetListItems()[i];
		
		UTXPartyListEntryWidget* PartyEntry = Cast<UTXPartyListEntryWidget>(ListEntry);
		if (!PartyEntry) continue;
		if (Members.Num() <= i)
		{
			PartyEntry->Refresh(false);
			continue;
		}
		FPartyMemberInfo MemberInfo = Members[i];
		
		PartyEntry->OnMouseEnterEvent.BindDynamic(this, &UTXPartyWidget::SetSelectInfo);

		PartyEntry->SetReady(MemberInfo.IsReady);
		PartyEntry->SetLevel(MemberInfo.Level);
		PartyEntry->SetPlayerName(MemberInfo.Name);
		PartyEntry->Refresh(true);
		PartyEntry->SetObjectId(MemberInfo.ObjectId);

		if (PartyMap.Contains(MemberInfo.ObjectId))
		{
			PartyMap[MemberInfo.ObjectId] = i;	
		}
		else
		{
			PartyMap.Add(MemberInfo.ObjectId, i);
		}
	}
}

void UTXPartyWidget::PartyLeaderChange(int64 ObjectId)
{
	if (!PartyMap.Contains(ObjectId))
		return;

	int32 index = PartyMap[ObjectId];
	if (!PartyListView->GetListItems().IsValidIndex(index))
		return;
	
	UObject* ListEntry = PartyListView->GetListItems()[index];
	if (UTXPartyListEntryWidget* PartyEntry = Cast<UTXPartyListEntryWidget>(ListEntry))
	{
		PartyEntry->SetPartyLeader(true);
	}
}

void UTXPartyWidget::MakeList()
{
	for (int i = 0; i < MaxPartyMemberCount; ++i)
	{
		UTXPartyListEntryWidget* PartyEntry = CreateWidget<UTXPartyListEntryWidget>(GetWorld(), PartyEntryClass);
		PartyEntry->SetIndex(i);
		PartyEntry->InActive();
		PartyListView->AddItem(PartyEntry);
	}
}

void UTXPartyWidget::SetSelectInfo(int32 index, int64 ObjectId, FVector2f MousePosition)
{
	SelectIndex = index;
	SelectObjectId = ObjectId;

	if (!UI_PartyRightClickWidget)
		return;
	
	const FVector2D MousePos = Geometry.AbsoluteToLocal(MousePosition);
	
	if (UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(UI_PartyRightClickWidget))
	{
		CanvasSlot->SetPosition(MousePos);

		if (UTXPartyRightClickWidget* RightClickWidget =Cast<UTXPartyRightClickWidget>(UI_PartyRightClickWidget))
		{
			RightClickWidget->SetObjectId(ObjectId);
		}
	}
	UI_PartyRightClickWidget->SetVisibility(ESlateVisibility::Visible);
}
