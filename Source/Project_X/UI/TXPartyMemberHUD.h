#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameSystem/Party/PXPartyData.h"
#include "TXPartyMemberHUD.generated.h"

class UListView;
class UPXPartySystem;

UCLASS()
class PROJECT_X_API UTXPartyMemberHUD : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	virtual void NativeDestruct() override;
	
	UFUNCTION()
	void RefreshPartyMemberList(const TArray<FPartyMemberInfo>& Members);

	UFUNCTION()
	void PartyLeaderChange(int64 ObjectId);

public:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UUserWidget> PartyMemberEntryClass;

	UPROPERTY(meta = (BindWidget))
	UListView* PartyMemberList;
	
	UPROPERTY()
	UPXPartySystem* PartySystem;

};
