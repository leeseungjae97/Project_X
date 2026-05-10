#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PXHealthPickup.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UNiagaraSystem;

UCLASS()
class PROJECT_X_API APXHealthPickup : public AActor
{
	GENERATED_BODY()

public:
	APXHealthPickup();

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	UPROPERTY(VisibleAnywhere, Category = "Pickup")
	USphereComponent* CollisionComponent;

	UPROPERTY(VisibleAnywhere, Category = "Pickup")
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(EditAnywhere, Category = "Pickup", meta = (ClampMin = 0))
	float HealAmount = 2.0f;

	UPROPERTY(EditAnywhere, Category = "Pickup")
	UNiagaraSystem* PickupEffect = nullptr;
};
