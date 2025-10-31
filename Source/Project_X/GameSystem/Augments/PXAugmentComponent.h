// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/EnumTypes.h"
#include "PXAugmentComponent.generated.h"

struct FPXAugmentData;
struct FPXAugmentEffect;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECT_X_API UPXAugmentComponent : public UActorComponent
{
	GENERATED_BODY()

	TArray<const FPXAugmentData*> OwnedAugments;
public:
	// Sets default values for this component's properties
	UPXAugmentComponent();

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
	void ExecuteEffect(const FPXAugmentEffect& Effect, AActor* Instigator, AActor* ExplicitTarget);
};
