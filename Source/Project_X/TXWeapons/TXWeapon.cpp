#include "TXWeapons/TXWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "TXComponents/TXCollisionComponent.h"

ATXWeapon::ATXWeapon()
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

void ATXWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

void ATXWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ATXWeapon::SetVisibility(bool InbVisibility)
{
	if (WeaponMesh)
	{
		WeaponMesh->SetVisibility(InbVisibility, true);
	}
}

void ATXWeapon::AttackStart()
{
	
}

void ATXWeapon::Equipped()
{
	
}
