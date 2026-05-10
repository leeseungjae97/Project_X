// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TXHUDWidget.generated.h"

class UProgressBar;
class UImage;
class UTextBlock;
class UWidget;
/**
 * 
 */
UCLASS()
class PROJECT_X_API UTXHUDWidget : public UUserWidget
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true", BindWidget))
	UImage* IMG_HPBar;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true", BindWidget))
	UImage* IMG_XPRing;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true", BindWidget))
	UProgressBar* PB_StaminaBar;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true", BindWidgetOptional))
	UTextBlock* Text_Round;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true", BindWidgetOptional))
	UTextBlock* Text_RemainingEnemies;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true", BindWidgetOptional))
	UTextBlock* Text_Result;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true", BindWidgetOptional))
	UWidget* BossHealthRoot;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true", BindWidgetOptional))
	UProgressBar* PB_BossHPBar;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true", BindWidgetOptional))
	UTextBlock* Text_BossName;

	UPROPERTY()
	UMaterialInstanceDynamic* MID_HpBar = nullptr;

	UPROPERTY()
	UMaterialInstanceDynamic* MID_XpRing = nullptr;

	// 체력
	float CurrentHPBackPercent = 1.f;
	float CurrentHPFrontPercent = 1.f;
	float TargetHPPercent = 1.f;
	float HPInterpSpeed = 5.0f;

	// 스테미나
	float CurrentStaminaPercent = 1.0f; 
	float TargetStaminaPercent = 1.0f;  
	float StaminaInterpSpeed = 5.0f;   
	
	// 경험치
	float CurrentXPPercent = 0.f;
	float TargetXPPercent = 0.f;
	float XPInterpSpeed = 5.0f;
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	void UpdateHpBar(float DeltaTime);
	void UpdateStaminaBar(float DeltaTime);
	void UpdateXPBar(float DeltaTime);
	void CreateFallbackBossHealthUI();
	
public:
	void SetHP(float HPPercent);
	void SetStamina(float StaminaPercent);
	void SetXPRing(float XPRingPercent);
	void SetRoundInfo(int32 RoundIndex, int32 RemainingEnemies);
	void SetResultText(const FText& ResultText);
	void SetBossHealthVisible(bool bVisible);
	void SetBossHealth(float HealthPercent, const FText& BossName);
};
