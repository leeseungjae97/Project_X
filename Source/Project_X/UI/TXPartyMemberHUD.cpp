#include "UI/TXPartyMemberHUD.h"

#include "TXPartyMemberListEntryWidget.h"
#include "GameSystem/Party/PXPartySystem.h"
#include "Components/ListView.h"

void UTXPartyMemberHUD::NativeConstruct()
{
	Super::NativeConstruct();
	
	PartySystem = GetGameInstance()->GetSubsystem<UPXPartySystem>();
	if (PartySystem)
	{
		PartySystem->OnPartyDataChanged.AddDynamic(this, &UTXPartyMemberHUD::RefreshPartyMemberList);
		PartySystem->OnPartyLeaderChanged.AddDynamic(this, &UTXPartyMemberHUD::PartyLeaderChange);
	}
	RefreshPartyMemberList({{1, FString(TEXT("Name1")), 10, true}, {2, FString(TEXT("Name2")), 99, false}});
}

void UTXPartyMemberHUD::NativeDestruct()
{
	Super::NativeDestruct();

	if (PartySystem)
	{
		PartySystem->OnPartyDataChanged.RemoveAll(this);
	}
}

void UTXPartyMemberHUD::RefreshPartyMemberList(const TArray<FPartyMemberInfo>& Members)
{
	for (int i = 0 ; i < PartyMemberList->GetListItems().Num(); ++i)
	{
		UObject* ListEntry = PartyMemberList->GetListItems()[i];
		
		UTXPartyMemberListEntryWidget* MemberEntry = Cast<UTXPartyMemberListEntryWidget>(ListEntry);
		if (!MemberEntry) continue;
		if (Members.Num() <= i)
		{
			MemberEntry->Refresh(false);
			continue;
		}
		FPartyMemberInfo MemberInfo = Members[i];
			
		MemberEntry->SetPlayerName(MemberInfo.Name);
		MemberEntry->SetPlayerHPRatio(0.5);
		MemberEntry->SetPlayerUltTimeRatio(0.8);
		MemberEntry->Refresh(true);
	}
}

void UTXPartyMemberHUD::PartyLeaderChange(int64 ObjectId)
{
	
}
