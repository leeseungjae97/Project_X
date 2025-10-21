#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Blueprint/UserWidget.h"
#include "TXPartyMemberListEntryWidget.generated.h"

class UTextBlock;
class UImage;
class UProgressBar;

UCLASS()
class PROJECT_X_API UTXPartyMemberListEntryWidget : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
private:
	void SetData(UObject* ListItemObject);

private:
	FString PlayerName;
	float PlayerHPRatio;
	float PlayerUltTimeRatio;

	bool bRefresh;
	
public:
	FORCEINLINE FString GetPlayerName() {return PlayerName;}
	FORCEINLINE float GetPlayerHPRatio() const {return PlayerHPRatio;}
	FORCEINLINE float GetPlayerUltTimeRatio() const {return PlayerUltTimeRatio;}
	FORCEINLINE bool IsRefresh() const {return bRefresh;}

	FORCEINLINE void SetPlayerName(FString Name) {PlayerName = Name;}
	FORCEINLINE void SetPlayerHPRatio(float Ratio) {PlayerHPRatio = Ratio;}
	FORCEINLINE void SetPlayerUltTimeRatio(float Ratio) {PlayerUltTimeRatio = Ratio;}
	FORCEINLINE void Refresh(bool InbRefresh) { bRefresh = InbRefresh; }
	
private:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* PlayerNameText;

	UPROPERTY(meta = (BindWidget))
	UImage* PlayerUltTimer;
	
	UPROPERTY(meta = (BindWidget))
	UProgressBar* PlayerHPBar;
};
