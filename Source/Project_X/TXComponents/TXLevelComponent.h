// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TXLevelComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECT_X_API UTXLevelComponent : public UActorComponent
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category="Level")
	int32 CurrentLevel = 1;

	UPROPERTY(VisibleAnywhere, Category="Level")
	int32 CurrentExp = 0;

	UPROPERTY(EditDefaultsOnly, Category="Level")
	int32 ExpToNextLevel = 100;

	UPROPERTY()
	TObjectPtr<class ACombatCharacter> OwnerCharacter;
	
public:
	// Sets default values for this component's properties
	UTXLevelComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	void AddExperience(int32 ExpAmount);
	void UpdateXPBar();
	void LevelUp();
};
