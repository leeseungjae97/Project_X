// Fill out your copyright notice in the Description page of Project Settings.


#include "TXPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"
#include "UI/TXHUDWidget.h"
#include "UI/TXIndicatorHUDWidget.h"
#include "UI/TXMiniMapWidget.h"
#include "Kismet/GameplayStatics.h"
#include "CombatCharacter.h"

void ATXPlayerController::BeginPlay()
{
	Super::BeginPlay();

	AddHUDWidget();
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
}
