// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TXAugmentCardWidget.generated.h"

struct FPXAugmentData;
class UTextBlock;
class UButton;
class UImage;
/**
 * 
 */
UCLASS()
class PROJECT_X_API UTXAugmentCardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetupFromData(const FPXAugmentData& InData);

	UPROPERTY(meta=(BindWidget)) UTextBlock* Text_Name;
	UPROPERTY(meta=(BindWidget)) UTextBlock* Text_Desc;
	UPROPERTY(meta=(BindWidget)) UImage*     Img_Icon;
	UPROPERTY(meta=(BindWidget)) UImage*     Img_Frame;
	UPROPERTY(meta=(BindWidget)) UButton*    Btn_Select;

	// 선택 시 브로드캐스트
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAugmentSelected, FName, AugmentID);
	UPROPERTY(BlueprintAssignable) FAugmentSelected OnAugmentSelected;

private:
	FName CachedAugmentID;
	UFUNCTION() void HandleClicked();
};
