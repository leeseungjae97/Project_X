// Copyright Epic Games, Inc. All Rights Reserved.

#include "PXEnemyProjectile.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Damageable.h"
#include "UObject/ConstructorHelpers.h"

APXEnemyProjectile::APXEnemyProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	SetRootComponent(CollisionComponent);
	CollisionComponent->InitSphereRadius(18.0f);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComponent->SetRelativeScale3D(FVector(0.25f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh.Succeeded())
	{
		MeshComponent->SetStaticMesh(SphereMesh.Object);
	}

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = 1200.0f;
	ProjectileMovement->MaxSpeed = 1200.0f;
	ProjectileMovement->ProjectileGravityScale = 0.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
}

void APXEnemyProjectile::InitializeProjectile(AActor* InDamageCauser, float InDamage, float InKnockbackImpulse, float InLaunchImpulse)
{
	DamageCauser = InDamageCauser;
	Damage = FMath::Max(0.0f, InDamage);
	KnockbackImpulse = FMath::Max(0.0f, InKnockbackImpulse);
	LaunchImpulse = FMath::Max(0.0f, InLaunchImpulse);
}

void APXEnemyProjectile::BeginPlay()
{
	Super::BeginPlay();

	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &APXEnemyProjectile::OnProjectileOverlap);
	SetLifeSpan(LifeSeconds);
}

void APXEnemyProjectile::OnProjectileOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this || OtherActor == GetOwner())
	{
		return;
	}

	if (!OtherActor->ActorHasTag(FName("Player")))
	{
		return;
	}

	if (IDamageable* Damageable = Cast<IDamageable>(OtherActor))
	{
		const FVector Direction = GetVelocity().GetSafeNormal();
		const FVector Impulse = (Direction * KnockbackImpulse) + (FVector::UpVector * LaunchImpulse);
		Damageable->ApplyDamage(Damage, DamageCauser ? DamageCauser : this, GetActorLocation(), Impulse);
		Destroy();
	}
}
