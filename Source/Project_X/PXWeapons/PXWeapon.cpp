#include "PXWeapons/PXWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "PXComponents/PXCollisionComponent.h"

APXWeapon::APXWeapon()
{
	PrimaryActorTick.bCanEverTick = true;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(RootComponent);
	WeaponMesh->SetMobility(EComponentMobility::Movable);
	SetRootComponent(WeaponMesh);

	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	WeaponMesh->MarkRenderStateDirty();
}

void APXWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

void APXWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APXWeapon::SetVisibility(bool InbVisibility)
{
	if (WeaponMesh)
	{
		WeaponMesh->SetVisibility(InbVisibility, true);
	}
}

void APXWeapon::AttackStart()
{
	
}

void APXWeapon::Equipped()
{
	
}
