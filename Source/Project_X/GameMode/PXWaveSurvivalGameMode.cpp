#include "GameMode/PXWaveSurvivalGameMode.h"

#include "GameSystem/Rounds/PXRoundManager.h"
#include "Player/PXPlayerCombatState.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"

APXWaveSurvivalGameMode::APXWaveSurvivalGameMode()
{
	PrimaryActorTick.bCanEverTick = false;

	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/Blueprints/BP_CombatCharacter"));
	if (PlayerPawnClassFinder.Succeeded())
	{
		DefaultPawnClass = PlayerPawnClassFinder.Class;
	}

	static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerClassFinder(TEXT("/Game/Blueprints/BP_TXPlayerController"));
	if (PlayerControllerClassFinder.Succeeded())
	{
		PlayerControllerClass = PlayerControllerClassFinder.Class;
	}

	PlayerStateClass = APXPlayerCombatState::StaticClass();
}

void APXWaveSurvivalGameMode::BeginPlay()
{
	Super::BeginPlay();

	BindRoundManager();
}

void APXWaveSurvivalGameMode::HandlePlayerDied()
{
	if (bGameEnded)
	{
		return;
	}

	bGameEnded = true;
	bPlayerWon = false;
	OnDefeat.Broadcast();
}

void APXWaveSurvivalGameMode::HandleAllRoundsCleared()
{
	if (bGameEnded)
	{
		return;
	}

	bGameEnded = true;
	bPlayerWon = true;
	OnVictory.Broadcast();
}

bool APXWaveSurvivalGameMode::IsGameEnded() const
{
	return bGameEnded;
}

bool APXWaveSurvivalGameMode::DidPlayerWin() const
{
	return bGameEnded && bPlayerWon;
}

void APXWaveSurvivalGameMode::BindRoundManager()
{
	if (!RoundManager)
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(this, APXRoundManager::StaticClass(), FoundActors);

		if (!FoundActors.IsEmpty())
		{
			RoundManager = Cast<APXRoundManager>(FoundActors[0]);
		}
	}

	if (RoundManager)
	{
		RoundManager->OnRunCleared.AddDynamic(this, &APXWaveSurvivalGameMode::HandleAllRoundsCleared);
	}
}
