// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Blueprint/UserWidget.h"
#include "TXPartyListEntryWidget.generated.h"

class UTextBlock;
class UImage;
class UMaterialInterface;
class UMaterialInstanceDynamic;

DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnMouseEnterEvent, int32, SelectIndex, int64, ObjectId, FVector2f, MousePosition);

UCLASS()
class PROJECT_X_API UTXPartyListEntryWidget : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

	FOnMouseEnterEvent OnMouseEnterEvent;
	
private:
	UPROPERTY(meta = (BindWidget, AllowPrivateAccess = "true")) UTextBlock* ReadyText;
	UPROPERTY(meta = (BindWidget, AllowPrivateAccess = "true")) UImage* PlayerPreviewImage;
	UPROPERTY(meta = (BindWidget, AllowPrivateAccess = "true")) UTextBlock* LevelText;
	UPROPERTY(meta = (BindWidget, AllowPrivateAccess = "true")) UImage* ClassIconImage;
	UPROPERTY(meta = (BindWidget, AllowPrivateAccess = "true")) UTextBlock* PlayerNameText;
	UPROPERTY(meta = (BindWidget, AllowPrivateAccess = "true")) UImage* PartyLeaderIcon;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	UMaterialInterface* DefaultPreviewMI;

	UPROPERTY()
	UMaterialInterface* PreviewMI;
	
	FSlateBrush DefaultPreviewBrush;

private:
	bool bIsPlayerReady = false;
	bool bIsLeader = false;
	int64 Level;
	FString PlayerName;
	
	int64 ObjectId;

	int32 Index;

private:
	bool bIsRefresh = false;

public:
	FORCEINLINE void SetObjectId(int64 InObjectId) { ObjectId = InObjectId; }
	FORCEINLINE int64 GetObjectId() { return ObjectId; }

	FORCEINLINE void SetIndex(int32 InIndex) { Index = InIndex; }
	FORCEINLINE int32 GetIndex() { return Index; }

	FORCEINLINE int64 GetLevel() { return Level; }
	
	FORCEINLINE FString GetPlayerName() { return PlayerName; }
	
	FORCEINLINE bool IsPlayerReady() { return bIsPlayerReady; }
	
	FORCEINLINE bool IsLeader() { return bIsLeader; }

	FORCEINLINE bool IsRefresh() { return bIsRefresh; }
	FORCEINLINE void Refresh(bool InValue) { bIsRefresh = InValue; }
private:
	UPROPERTY()
	UTXPartyListEntryWidget* DelegateObject;
	
private:
	void SetData(UObject* ListItemObject);
public:
	void Active();
	void InActive();

	void SetReady(bool bReady);
	void SetPlayerPreviewImage();
	void SetLevel(int64 Level);
	void SetClassIconImage();
	void SetPlayerName(const FString& Name) {PlayerName = Name;}
	void SetPartyLeader(bool InbIsLeader) {bIsLeader = InbIsLeader;}

};
