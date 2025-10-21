// Fill out your copyright notice in the Description page of Project Settings.


#include "TXLevelComponent.h"

#include "CombatCharacter.h"
#include "Player/Controller/TXPlayerController.h"


// Sets default values for this component's properties
UTXLevelComponent::UTXLevelComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UTXLevelComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UTXLevelComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                      FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UTXLevelComponent::AddExperience(int32 ExpAmount)
{
	CurrentExp += ExpAmount;
	UpdateXPBar();
}

void UTXLevelComponent::UpdateXPBar()
{
	if (CurrentExp >= ExpToNextLevel)
	{
		CurrentExp -= ExpToNextLevel;
		LevelUp();
	}
	
	float NewXPPer = CurrentExp / ExpToNextLevel;
	if (ATXPlayerController* PC = Cast<ATXPlayerController>(OwnerCharacter->GetController()))
	{
		PC->SetXPBar(NewXPPer);
	}
}

void UTXLevelComponent::LevelUp()
{
	
}

