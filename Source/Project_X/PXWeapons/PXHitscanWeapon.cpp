// Fill out your copyright notice in the Description page of Project Settings.


#include "PXWeapons/PXHitscanWeapon.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "CombatCharacter.h"
#include "Damageable.h"

APXHitscanWeapon::APXHitscanWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	SetWeaponType(EWeaponType::EWT_Hitscan);
}

void APXHitscanWeapon::Fire()
{
	APawn* OwnerPawn = Cast<APawn>(GetOwnerCharacter());
	if (nullptr == OwnerPawn)
		return;

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("Muzzle");
	if (MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();
		FHitResult FireHit;
		TraceHitTarget(FireHit);
		FVector HitTarget = FireHit.ImpactPoint;

		AController* InstigatorController = OwnerPawn->GetController();
		IDamageable* Damageable = Cast<IDamageable>(FireHit.GetActor());

		UWorld* World = GetWorld();
		if (World)
		{
			FVector End = Start + (HitTarget - Start) * 1.25f;
			FHitResult OutHit;

			World->LineTraceSingleByChannel(
				OutHit,
				Start,
				End,
				ECollisionChannel::ECC_Visibility
			);
			FVector BeamEnd = End;
			if (OutHit.bBlockingHit)
			{
				BeamEnd = OutHit.ImpactPoint;
			}

			if (BeamParticle)
			{
				UNiagaraComponent* Beam = UNiagaraFunctionLibrary::SpawnSystemAttached(
					BeamParticle,
					GetRootComponent(),
					FName(),
					GetActorLocation(),
					GetActorRotation(),
					EAttachLocation::KeepWorldPosition,
					false
				);

				if (Beam)
				{
					Beam->SetVectorParameter(FName("Beam Start"), Start);
					Beam->SetVectorParameter(FName("Beam End"), BeamEnd);
				}
			}
		}

		if (ImpactParticles)
		{
			UNiagaraFunctionLibrary::SpawnSystemAttached(
				ImpactParticles,
				GetRootComponent(),
				FName(),
				FireHit.ImpactPoint,
				FireHit.ImpactNormal.Rotation(),
				EAttachLocation::KeepWorldPosition,
				false
			);
		}
		if (MuzzleFlash)
		{
			UNiagaraFunctionLibrary::SpawnSystemAttached(
				MuzzleFlash,
				GetWeaponMesh(),
				MuzzleFlashSocket->SocketName,
				FVector::ZeroVector,
				FRotator::ZeroRotator,
				EAttachLocation::SnapToTarget,
				false
			);
		}
	}
}

void APXHitscanWeapon::SetVisibility(bool InbVisibility)
{
	Super::SetVisibility(InbVisibility);
}

void APXHitscanWeapon::TraceHitTarget(FHitResult& HitResult)
{
	if (nullptr == GetOwnerCharacter() || nullptr == GetWeaponMesh())
		return;

	UWorld* World = GetWorld();

	if (nullptr == World)
		return;

	FVector2D ViewportSize;
	if (!GEngine || !GEngine->GameViewport)
		return;
	
	GEngine->GameViewport->GetViewportSize(ViewportSize);

	FVector2D CrossHairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(GetOwnerCharacter(), 0),
		CrossHairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);

	if (!bScreenToWorld)
		return;

	const USkeletalMeshSocket* MuzzleSocket = GetWeaponMesh()->GetSocketByName("Muzzle");
	if (MuzzleSocket)
	{
		FTransform SocketTransform = MuzzleSocket->GetSocketTransform(GetWeaponMesh());

		FVector Direction = CrosshairWorldDirection;
		Direction.Normalize();

		FVector Start = SocketTransform.GetLocation();
		FVector End = Start + CrosshairWorldDirection * 3000.f;

		GetWorld()->LineTraceSingleByChannel(
			HitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility
		);
	}
}
