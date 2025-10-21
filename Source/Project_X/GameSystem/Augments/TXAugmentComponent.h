// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/EnumTypes.h"
#include "TXAugmentComponent.generated.h"

struct FTXAugmentData;
struct FTXAugmentEffect;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECT_X_API UTXAugmentComponent : public UActorComponent
{
	GENERATED_BODY()

	TArray<const FTXAugmentData*> OwnedAugments;
public:
	// Sets default values for this component's properties
	UTXAugmentComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
public:
	void ApplyAugmentByID(FName AugmentID, AActor* Instigator, AActor* Target = nullptr);
	void TriggerAugment(EAugmentTriggerCondition TriggerType, AActor* Instigator = nullptr, AActor* Target = nullptr);

private:
	void ExecuteEffect(const FTXAugmentEffect& Effect, AActor* Instigator, AActor* ExplicitTarget);
};
