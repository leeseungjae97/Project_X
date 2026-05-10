// Fill out your copyright notice in the Description page of Project Settings.


#include "TXPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"
#include "GameMode/PXWaveSurvivalGameMode.h"
#include "GameSystem/Rounds/PXRoundManager.h"
#include "UI/TXAugmentChoiceWidget.h"
#include "UI/TXHUDWidget.h"
#include "UI/TXIndicatorHUDWidget.h"
#include "UI/TXMiniMapWidget.h"
#include "Kismet/GameplayStatics.h"
#include "AI/CombatEnemy.h"
#include "CombatCharacter.h"
#include "TimerManager.h"

void ATXPlayerController::BeginPlay()
{
	Super::BeginPlay();

	PrimaryActorTick.bCanEverTick = true;

	AddHUDWidget();
	BindRoundManager();
	BindGameMode();
}

void ATXPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	EnsureAugmentChoiceWidgetVisible();
	UpdateBossHealthBar();
}

void ATXPlayerController::SetupInputComponent()
{
	// add the input mapping context
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
		{
			Subsystem->AddMappingContext(CurrentContext, 0);
		}
	}
}

void ATXPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// subscribe to the pawn's OnDestroyed delegate
	InPawn->OnDestroyed.AddDynamic(this, &ATXPlayerController::OnPawnDestroyed);
}

void ATXPlayerController::SetRespawnTransform(const FTransform& NewRespawn)
{
	// save the new respawn transform
	RespawnTransform = NewRespawn;
}

const UTXHUDWidget* ATXPlayerController::GetHUDWidget() const
{
	if (HUDWidget)
		return HUDWidget;
	
	return nullptr;
}

void ATXPlayerController::SetHpBar(float NewHPPer)
{
	if (HUDWidget)
	{
		HUDWidget->SetHP(NewHPPer);
	}
}

void ATXPlayerController::SetStaminaBar(float NewStaminaPer)
{
	if (HUDWidget)
	{
		HUDWidget->SetStamina(NewStaminaPer);
	}
}

void ATXPlayerController::SetXPBar(float NewXPPer)
{
	if (HUDWidget)
	{
		HUDWidget->SetXPRing(NewXPPer);
	}
}

void ATXPlayerController::OnPawnDestroyed(AActor* DestroyedActor)
{
	// spawn a new character at the respawn transform
	if (ACombatCharacter* RespawnedCharacter = GetWorld()->SpawnActor<ACombatCharacter>(CharacterClass, RespawnTransform))
	{
		// possess the character
		Possess(RespawnedCharacter);
	}
}

void ATXPlayerController::AddHUDWidget()
{
	if (HUDWidgetClass)
	{
		HUDWidget = CreateWidget<UTXHUDWidget>(GetWorld(), HUDWidgetClass);
		if (HUDWidget)
		{
			HUDWidget->AddToViewport();
		}
	}

	if (IndicatorHUDWidgetClass)
	{
		IndicatorHUDWidget = CreateWidget<UTXIndicatorHUDWidget>(GetWorld(), IndicatorHUDWidgetClass);
		if (IndicatorHUDWidget)
		{
			IndicatorHUDWidget->AddToViewport();
		}
	}

	if (MiniMapWidgetClass)
	{
		MiniMapWidget = CreateWidget<UTXMiniMapWidget>(GetWorld(), MiniMapWidgetClass);
		if (MiniMapWidget)
		{
			MiniMapWidget->AddToViewport();
		}
	}

	if (!AugmentChoiceWidgetClass)
	{
		AugmentChoiceWidgetClass = LoadClass<UUserWidget>(nullptr, TEXT("/Game/UI/UI_AugmentChoiceWidget.UI_AugmentChoiceWidget_C"));
		if (!AugmentChoiceWidgetClass)
		{
			AugmentChoiceWidgetClass = UTXAugmentChoiceWidget::StaticClass();
		}
	}
}

void ATXPlayerController::BindRoundManager()
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(this, APXRoundManager::StaticClass(), FoundActors);
	if (FoundActors.IsEmpty())
	{
		return;
	}

	BoundRoundManager = Cast<APXRoundManager>(FoundActors[0]);
	if (!BoundRoundManager)
	{
		return;
	}

	BoundRoundManager->OnRoundStarted.AddDynamic(this, &ATXPlayerController::HandleRoundStarted);
	BoundRoundManager->OnRoundProgressChanged.AddDynamic(this, &ATXPlayerController::HandleRoundProgressChanged);
	BoundRoundManager->OnAugmentChoiceRequested.AddDynamic(this, &ATXPlayerController::HandleAugmentChoiceRequested);
	BoundRoundManager->OnBossSpawned.AddDynamic(this, &ATXPlayerController::HandleBossSpawned);

	if (BoundRoundManager->GetRoundState() == EPXRoundState::ChoosingAugment)
	{
		HandleAugmentChoiceRequested(BoundRoundManager->GetCurrentRoundNumber());
	}
}

void ATXPlayerController::BindGameMode()
{
	APXWaveSurvivalGameMode* WaveGameMode = GetWorld() ? GetWorld()->GetAuthGameMode<APXWaveSurvivalGameMode>() : nullptr;
	if (!WaveGameMode)
	{
		return;
	}

	WaveGameMode->OnVictory.AddDynamic(this, &ATXPlayerController::HandleVictory);
	WaveGameMode->OnDefeat.AddDynamic(this, &ATXPlayerController::HandleDefeat);
}

void ATXPlayerController::HandleRoundStarted(int32 RoundIndex, int32 TotalEnemies)
{
	if (HUDWidget)
	{
		HUDWidget->SetRoundInfo(RoundIndex, TotalEnemies);
		HUDWidget->SetBossHealthVisible(false);

		if (RoundIndex == 5)
		{
			HUDWidget->SetResultText(NSLOCTEXT("PXHUD", "BossWarning", "BOSS ROUND"));
			GetWorldTimerManager().ClearTimer(ResultTextClearTimer);
			GetWorldTimerManager().SetTimer(ResultTextClearTimer, this, &ATXPlayerController::ClearTransientResultText, 3.0f, false);
		}
	}
}

void ATXPlayerController::HandleRoundProgressChanged(int32 RoundIndex, int32 RemainingEnemies, int32 AliveEnemies)
{
	if (HUDWidget)
	{
		HUDWidget->SetRoundInfo(RoundIndex, RemainingEnemies);
	}
}

void ATXPlayerController::HandleAugmentChoiceRequested(int32 CompletedRoundIndex)
{
	if (!AugmentChoiceWidgetClass || !BoundRoundManager)
	{
		return;
	}

	if (AugmentChoiceWidget)
	{
		AugmentChoiceWidget->RemoveFromParent();
		AugmentChoiceWidget = nullptr;
	}

	AugmentChoiceWidget = CreateWidget<UTXAugmentChoiceWidget>(this, AugmentChoiceWidgetClass);
	if (AugmentChoiceWidget)
	{
		AugmentChoiceWidget->ShowChoicesFromRoundManager(BoundRoundManager);
		AugmentChoiceWidget->AddToViewport(100);
		AugmentChoiceWidget->SetVisibility(ESlateVisibility::Visible);

		FlushPressedKeys();
		SetIgnoreMoveInput(true);
		SetIgnoreLookInput(true);
		bShowMouseCursor = true;
		FInputModeUIOnly InputMode;
		SetInputMode(InputMode);
	}
}

void ATXPlayerController::HandleBossSpawned(ACombatEnemy* BossEnemy)
{
	ActiveBossEnemy = BossEnemy;

	if (HUDWidget && ActiveBossEnemy)
	{
		HUDWidget->SetBossHealth(ActiveBossEnemy->GetHealthPercent(), FText::FromString(GetNameSafe(ActiveBossEnemy)));
		HUDWidget->SetBossHealthVisible(true);
	}
}

void ATXPlayerController::EnsureAugmentChoiceWidgetVisible()
{
	if (!BoundRoundManager || BoundRoundManager->GetRoundState() != EPXRoundState::ChoosingAugment)
	{
		return;
	}

	if (AugmentChoiceWidget && AugmentChoiceWidget->IsInViewport())
	{
		return;
	}

	HandleAugmentChoiceRequested(BoundRoundManager->GetCurrentRoundNumber());
}

void ATXPlayerController::HandleVictory()
{
	GetWorldTimerManager().ClearTimer(ResultTextClearTimer);
	if (HUDWidget)
	{
		HUDWidget->SetBossHealthVisible(false);
		HUDWidget->SetResultText(NSLOCTEXT("PXHUD", "Victory", "VICTORY"));
	}

	FlushPressedKeys();
	SetIgnoreMoveInput(true);
	SetIgnoreLookInput(true);
	bShowMouseCursor = true;
}

void ATXPlayerController::HandleDefeat()
{
	GetWorldTimerManager().ClearTimer(ResultTextClearTimer);
	if (HUDWidget)
	{
		HUDWidget->SetBossHealthVisible(false);
		HUDWidget->SetResultText(NSLOCTEXT("PXHUD", "Defeat", "DEFEAT"));
	}

	FlushPressedKeys();
	SetIgnoreMoveInput(true);
	SetIgnoreLookInput(true);
	bShowMouseCursor = true;
}

void ATXPlayerController::ClearTransientResultText()
{
	if (HUDWidget)
	{
		HUDWidget->SetResultText(FText::GetEmpty());
	}
}

void ATXPlayerController::UpdateBossHealthBar()
{
	if (!HUDWidget)
	{
		return;
	}

	if (!IsValid(ActiveBossEnemy) || ActiveBossEnemy->CurrentHP <= 0.0f)
	{
		HUDWidget->SetBossHealthVisible(false);
		ActiveBossEnemy = nullptr;
		return;
	}

	HUDWidget->SetBossHealth(ActiveBossEnemy->GetHealthPercent(), FText::FromString(GetNameSafe(ActiveBossEnemy)));
}
