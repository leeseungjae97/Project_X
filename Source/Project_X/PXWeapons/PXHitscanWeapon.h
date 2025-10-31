// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PXWeapons/PXWeapon.h"
#include "PXHitscanWeapon.generated.h"

class UNiagaraSystem;

UCLASS()
class PROJECT_X_API APXHitscanWeapon : public APXWeapon
{
	GENERATED_BODY()
	
public:
	APXHitscanWeapon();

	virtual void Fire();

	virtual void SetVisibility(bool InbVisibility) override;

public:
	UPROPERTY(EditAnywhere)
	UNiagaraSystem* ImpactParticles;

	UPROPERTY(EditAnywhere)
	UNiagaraSystem* BeamParticle;

	UPROPERTY(EditAnywhere)
	UNiagaraSystem* MuzzleFlash;

private:
	void TraceHitTarget(FHitResult& HitResult);
};
