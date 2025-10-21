// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TXMapPointComponent.generated.h"

class UTXMiniMapWidget;
class UMaterialParameterCollection;
class UTXEnemyMarkerWidget;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_X_API UTXMapPointComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UTXMapPointComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	bool bIsMapPointCreated = false;

private:
	UPROPERTY()
	ACharacter* OwnerCharacter;

private:

	void CreateMapPoint();

public:
	FORCEINLINE void SetOwnerCharacter(ACharacter* InOwner) { OwnerCharacter = InOwner; }
};
