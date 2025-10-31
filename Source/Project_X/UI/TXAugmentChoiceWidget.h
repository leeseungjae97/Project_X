// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TXAugmentChoiceWidget.generated.h"
class UTXAugmentCardWidget;
struct FPXAugmentData;
/**
 * 
 */
UCLASS()
class PROJECT_X_API UTXAugmentChoiceWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable) void ShowChoices(AActor* InOwnerActor, int32 NumChoices = 3);

	// 카드 3개 바인딩
	UPROPERTY(meta=(BindWidget)) UTXAugmentCardWidget* Card_1;
	UPROPERTY(meta=(BindWidget)) UTXAugmentCardWidget* Card_2;
	UPROPERTY(meta=(BindWidget)) UTXAugmentCardWidget* Card_3;

protected:
	UFUNCTION() void OnCardSelected(FName AugmentID);

private:
	TWeakObjectPtr<AActor> OwnerActor;

	// 테이블에서 랜덤 뽑기
	bool GetRandomAugments(int32 Num, TArray<FPXAugmentData>& OutList) const;

public:
	// 1개 랜덤
	static const FPXAugmentData* GetRandomAugmentRow(const UDataTable* Table);

	// N개 랜덤 (중복X)
	static void GetRandomAugmentRows(const UDataTable* Table, int32 Num, TArray<FPXAugmentData>& Out);
};
