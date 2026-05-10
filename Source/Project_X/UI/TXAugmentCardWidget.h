// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TXAugmentCardWidget.generated.h"

struct FPXAugmentData;
struct FPXStatAugmentData;
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

	UFUNCTION(BlueprintCallable)
	void SetupFromStatData(const FPXStatAugmentData& InData);

	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* Text_Name;
	UPROPERTY(meta=(BindWidgetOptional)) UTextBlock* Text_Desc;
	UPROPERTY(meta=(BindWidgetOptional)) UImage*     Img_Icon;
	UPROPERTY(meta=(BindWidgetOptional)) UImage*     Img_Frame;
	UPROPERTY(meta=(BindWidgetOptional)) UButton*    Btn_Select;

	// 선택 시 브로드캐스트
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAugmentSelected, FName, AugmentID);
	UPROPERTY(BlueprintAssignable) FAugmentSelected OnAugmentSelected;

private:
	FName CachedAugmentID;
	UFUNCTION() void HandleClicked();
};
