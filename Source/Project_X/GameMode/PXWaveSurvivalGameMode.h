#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PXWaveSurvivalGameMode.generated.h"

class APXRoundManager;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPXGameEnded);

UCLASS()
class PROJECT_X_API APXWaveSurvivalGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	APXWaveSurvivalGameMode();

protected:
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable, Category = "Wave Survival")
	void HandlePlayerDied();

	UFUNCTION(BlueprintCallable, Category = "Wave Survival")
	void HandleAllRoundsCleared();

	UFUNCTION(BlueprintPure, Category = "Wave Survival")
	bool IsGameEnded() const;

	UFUNCTION(BlueprintPure, Category = "Wave Survival")
	bool DidPlayerWin() const;

public:
	UPROPERTY(BlueprintAssignable, Category = "Wave Survival")
	FPXGameEnded OnVictory;

	UPROPERTY(BlueprintAssignable, Category = "Wave Survival")
	FPXGameEnded OnDefeat;

private:
	UFUNCTION()
	void BindRoundManager();

private:
	UPROPERTY(EditAnywhere, Category = "Wave Survival")
	APXRoundManager* RoundManager = nullptr;

	bool bGameEnded = false;
	bool bPlayerWon = false;
};
