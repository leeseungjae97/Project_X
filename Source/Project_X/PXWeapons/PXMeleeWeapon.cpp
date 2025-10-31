// Fill out your copyright notice in the Description page of Project Settings.


#include "PXWeapons/PXMeleeWeapon.h"

#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Core/Structures.h"
#include "PXComponents/PXCollisionComponent.h"

APXMeleeWeapon::APXMeleeWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	SetWeaponType(EWeaponType::EWT_Melee);

	CollisionComponent = CreateDefaultSubobject<UPXCollisionComponent>(TEXT("Weapon Collision"));
	CollisionComponent->SetupAttachment ( GetWeaponMesh(),  FName ("collision"));
	
}

void APXMeleeWeapon::BeginPlay()
{
	Super::BeginPlay();

	//SpawnAurorSystem();

	FBoxSphereBounds Box = GetWeaponMesh()->GetLocalBounds();
	
	// 무기를 손으로 집는 기준 Collision 크기
	CollisionComponent->InitCollisionComponent(FCollisionInfo{EPXCollisionShapeType::Box, FVector{Box.BoxExtent.Z, Box.BoxExtent.X, Box.BoxExtent.Y}});
	// CollisionComponent->BindOnBeginOverlap(this, FName("SendCollisionPacket"), 1);
}

void APXMeleeWeapon::SpawnAurorSystem()
{
	if (AurorSystem)
	{
		AurorSystemComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			AurorSystem,
			GetRootComponent(),
			FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition,
			false
		);
	}
}

void APXMeleeWeapon::SetVisibility(bool InbVisibility)
{
	Super::SetVisibility(InbVisibility);

	if (AurorSystemComponent)
	{
		AurorSystemComponent->SetVisibility(InbVisibility, true);
	}
	else if (InbVisibility)
	{
		SpawnAurorSystem();
	}
}
