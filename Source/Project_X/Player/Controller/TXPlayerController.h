// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TXPlayerController.generated.h"
class UInputMappingContext;
class UTXHUDWidget;
class UTXIndicatorHUDWidget;
class UTXMiniMapWidget;
class ACombatCharacter;
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

protected:
	virtual void BeginPlay() override;
	
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

public:
	UTXIndicatorHUDWidget* GetIndicatorHUDWidget() { return IndicatorHUDWidget; }
	UTXMiniMapWidget* GetMiniMapWidget() { return MiniMapWidget; }

};
