// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TXPlayerController.generated.h"
class UInputMappingContext;
class UTXHUDWidget;
class UTXIndicatorHUDWidget;
class UTXMiniMapWidget;
class UTXAugmentChoiceWidget;
class ACombatCharacter;
class ACombatEnemy;
class APXRoundManager;
class APXWaveSurvivalGameMode;
/**
 * 
 */
UCLASS()
class PROJECT_X_API ATXPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:

	/** Input mapping context for this player */
	UPROPERTY(EditAnywhere, Category="Input")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Character class to respawn when the possessed pawn is destroyed */
	UPROPERTY(EditAnywhere, Category="Respawn")
	TSubclassOf<ACombatCharacter> CharacterClass;

	/** Transform to respawn the character at. Can be set to create checkpoints */
	FTransform RespawnTransform;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> HUDWidgetClass;
	
	UPROPERTY()
	UTXHUDWidget* HUDWidget;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> IndicatorHUDWidgetClass;

	UPROPERTY()
	UTXIndicatorHUDWidget* IndicatorHUDWidget;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> MiniMapWidgetClass;

	UPROPERTY()
	UTXMiniMapWidget* MiniMapWidget;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> AugmentChoiceWidgetClass;

	UPROPERTY()
	UTXAugmentChoiceWidget* AugmentChoiceWidget;

	UPROPERTY()
	APXRoundManager* BoundRoundManager;

	UPROPERTY()
	ACombatEnemy* ActiveBossEnemy;

	FTimerHandle ResultTextClearTimer;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	
	/** Initialize input bindings */
	virtual void SetupInputComponent() override;

	/** Pawn initialization */
	virtual void OnPossess(APawn* InPawn) override;

public:

	/** Updates the character respawn transform */
	void SetRespawnTransform(const FTransform& NewRespawn);

	const UTXHUDWidget* GetHUDWidget() const;

	void SetHpBar(float NewHPPer);
	void SetStaminaBar(float NewStaminaPer);
	void SetXPBar(float NewXPPer);

protected:

	/** Called if the possessed pawn is destroyed */
	UFUNCTION()
	void OnPawnDestroyed(AActor* DestroyedActor);

private:
	void AddHUDWidget();
	void BindRoundManager();
	void BindGameMode();
	void EnsureAugmentChoiceWidgetVisible();
	void ClearTransientResultText();
	void UpdateBossHealthBar();

	UFUNCTION()
	void HandleRoundStarted(int32 RoundIndex, int32 TotalEnemies);

	UFUNCTION()
	void HandleRoundProgressChanged(int32 RoundIndex, int32 RemainingEnemies, int32 AliveEnemies);

	UFUNCTION()
	void HandleAugmentChoiceRequested(int32 CompletedRoundIndex);

	UFUNCTION()
	void HandleBossSpawned(ACombatEnemy* BossEnemy);

	UFUNCTION()
	void HandleVictory();

	UFUNCTION()
	void HandleDefeat();

public:
	UTXIndicatorHUDWidget* GetIndicatorHUDWidget() { return IndicatorHUDWidget; }
	UTXMiniMapWidget* GetMiniMapWidget() { return MiniMapWidget; }

};
