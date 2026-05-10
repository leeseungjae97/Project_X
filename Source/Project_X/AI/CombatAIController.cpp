// Copyright Epic Games, Inc. All Rights Reserved.


#include "CombatAIController.h"
#include "Components/StateTreeAIComponent.h"

ACombatAIController::ACombatAIController()
{
	// create the StateTree AI Component
	StateTreeAI = CreateDefaultSubobject<UStateTreeAIComponent>(TEXT("StateTreeAI"));
	check(StateTreeAI);
	BrainComponent = StateTreeAI;

	// ensure we start the StateTree when we possess the pawn
	bStartAILogicOnPossess = true;

	// ensure we're attached to the possessed character.
	// this is necessary for EnvQueries to work correctly
	bAttachToPawn = true;
}

void ACombatAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (StateTreeAI && !StateTreeAI->IsRunning())
	{
		StateTreeAI->StartLogic();
	}
}
