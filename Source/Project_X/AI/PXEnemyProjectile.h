// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PXEnemyProjectile.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;

UCLASS()
class PROJECT_X_API APXEnemyProjectile : public AActor
{
	GENERATED_BODY()

public:
	APXEnemyProjectile();

	void InitializeProjectile(AActor* InDamageCauser, float InDamage, float InKnockbackImpulse, float InLaunchImpulse);

protected:
	UPROPERTY(VisibleAnywhere, Category = "Projectile")
	USphereComponent* CollisionComponent;

	UPROPERTY(VisibleAnywhere, Category = "Projectile")
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere, Category = "Projectile")
	UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(EditAnywhere, Category = "Projectile|Damage", meta = (ClampMin = 0))
	float Damage = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Projectile|Damage", meta = (ClampMin = 0))
	float KnockbackImpulse = 180.0f;

	UPROPERTY(EditAnywhere, Category = "Projectile|Damage", meta = (ClampMin = 0))
	float LaunchImpulse = 250.0f;

	UPROPERTY(EditAnywhere, Category = "Projectile", meta = (ClampMin = 0.1))
	float LifeSeconds = 4.0f;

	UPROPERTY()
	AActor* DamageCauser;

	virtual void BeginPlay() override;

	UFUNCTION()
	void OnProjectileOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
