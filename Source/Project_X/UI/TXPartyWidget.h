#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameSystem/Party/TXPartyData.h"
#include "TXPartyWidget.generated.h"

class UListView;
class UTXPartyListEntryWidget;
class UTXPartySystem;

UCLASS()
class PROJECT_X_API UTXPartyWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
	virtual void NativePreConstruct() override;

	virtual void NativeDestruct() override;

	
public:
	UFUNCTION()
	void RefreshPartyList(const TArray<FPartyMemberInfo>& Members);

	UFUNCTION()
	void PartyLeaderChange(int64 ObjectId);
	
	void MakeList();

public:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UUserWidget> PartyEntryClass;

	UPROPERTY(meta = (BindWidget))
	UListView* PartyListView;

	UPROPERTY(meta = (BindWidget))
	UUserWidget* UI_PartyRightClickWidget;

	UPROPERTY()
	TMap<int32, UTXPartyListEntryWidget*> ListEntryMap;

	UPROPERTY()
	UTXPartySystem* PartySystem;

private:
	FGeometry Geometry;

private:
	TMap<int64, int32> PartyMap;
private:
	UFUNCTION()
	void SetSelectInfo(int32 index, int64 ObjectId, FVector2f MousePosition);
	
	UFUNCTION()
	FORCEINLINE int32 GetSelectIndex() {return SelectIndex;}
	
	int32 SelectIndex;
	int64 SelectObjectId;
private:
	int32 CurrentPartyCapacity = 0;

	int32 MaxPartyMemberCount = 3;
};
