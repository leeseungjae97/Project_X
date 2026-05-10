// Copyright Epic Games, Inc. All Rights Reserved.


#include "CombatEnemy.h"
#include "AIController.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CombatAIController.h"
#include "Components/WidgetComponent.h"
#include "Engine/DamageEvents.h"
#include "TimerManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "PXComponents/PXMapPointComponent.h"
#include "PXCombatLifeBar.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "PXEnemyProjectile.h"
#include "UObject/ConstructorHelpers.h"

ACombatEnemy::ACombatEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	// bind the attack montage ended delegate
	OnAttackMontageEnded.BindUObject(this, &ACombatEnemy::AttackMontageEnded);

	// set the AI Controller class by default
	AIControllerClass = ACombatAIController::StaticClass();

	// use an AI Controller regardless of whether we're placed or spawned
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// ignore the controller's yaw rotation
	bUseControllerRotationYaw = false;

	// create the life bar
	LifeBar = CreateDefaultSubobject<UWidgetComponent>(TEXT("LifeBar"));
	LifeBar->SetupAttachment(RootComponent);
	LifeBar->SetHiddenInGame(true);

	MapPointComponent = CreateDefaultSubobject<UPXMapPointComponent>(TEXT("TX Map Point Component"));

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> EnemyMaterialFinder(TEXT("/Game/Materials/M_EnemyTypeColor.M_EnemyTypeColor"));
	if (EnemyMaterialFinder.Succeeded())
	{
		EnemyTypeMaterial = EnemyMaterialFinder.Object;
	}

	// set the collision capsule size
	GetCapsuleComponent()->SetCapsuleSize(35.0f, 90.0f);

	// set the character movement properties
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->MaxWalkSpeed = EnemyStats.MoveSpeed;

	// reset HP to maximum
	CurrentHP = MaxHP;
}

void ACombatEnemy::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdatePrototypeChase(DeltaSeconds);
}

void ACombatEnemy::DoAIComboAttack()
{
	// ignore if we're already playing an attack animation
	if (bIsAttacking)
	{
		return;
	}

	if (!ComboAttackMontage || ComboSectionNames.IsEmpty())
	{
		OnAttackCompleted.ExecuteIfBound();
		return;
	}

	// raise the attacking flag
	bIsAttacking = true;
	bDamageAppliedThisAttack = false;

	// choose how many times we're going to attack
	TargetComboCount = FMath::RandRange(1, ComboSectionNames.Num());

	// reset the attack counter
	CurrentComboAttack = 0;

	// play the attack montage
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		const float MontageLength = AnimInstance->Montage_Play(ComboAttackMontage, 1.0f, EMontagePlayReturnType::MontageLength, 0.0f, true);

		// subscribe to montage completed and interrupted events
		if (MontageLength > 0.0f)
		{
			// set the end delegate for the montage
			AnimInstance->Montage_SetEndDelegate(OnAttackMontageEnded, ComboAttackMontage);

			const float DamageDelay = FMath::Max(0.0f, MontageLength * PrototypeAttackDamageTime);
			GetWorldTimerManager().SetTimer(PrototypeAttackDamageTimer, this, &ACombatEnemy::ApplyPrototypeAttackDamage, DamageDelay, false);
		}
		else
		{
			bIsAttacking = false;
			OnAttackCompleted.ExecuteIfBound();
		}
	}
	else
	{
		bIsAttacking = false;
		OnAttackCompleted.ExecuteIfBound();
	}
}

void ACombatEnemy::DoAIChargedAttack()
{
	// ignore if we're already playing an attack animation
	if (bIsAttacking)
	{
		return;
	}

	if (!ChargedAttackMontage)
	{
		return;
	}

	// raise the attacking flag
	bIsAttacking = true;

	// choose how many loops are we going to charge for
	TargetChargeLoops = FMath::RandRange(MinChargeLoops, MaxChargeLoops);

	// reset the charge loop counter
	CurrentChargeLoop = 0;

	// play the attack montage
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		const float MontageLength = AnimInstance->Montage_Play(ChargedAttackMontage, 1.0f, EMontagePlayReturnType::MontageLength, 0.0f, true);

		// subscribe to montage completed and interrupted events
		if (MontageLength > 0.0f)
		{
			// set the end delegate for the montage
			AnimInstance->Montage_SetEndDelegate(OnAttackMontageEnded, ChargedAttackMontage);
		}
	}
}

void ACombatEnemy::AttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	GetWorldTimerManager().ClearTimer(PrototypeAttackDamageTimer);

	// reset the attacking flag
	bIsAttacking = false;

	// call the attack completed delegate so the StateTree can continue execution
	OnAttackCompleted.ExecuteIfBound();
}

void ACombatEnemy::DoAttackTrace(FName DamageSourceBone)
{
	if (IsRangedEnemy())
	{
		FireProjectileAtTarget(GetTargetPlayerCharacter());
		return;
	}

	bDamageAppliedThisAttack = true;

	// sweep for objects in front of the character to be hit by the attack
	TArray<FHitResult> OutHits;

	// start at the provided socket location, sweep forward
	const FVector TraceStart = GetMesh()->GetSocketLocation(DamageSourceBone);
	const FVector TraceEnd = TraceStart + (GetActorForwardVector() * MeleeTraceDistance);

	// enemies only affect Pawn collision objects; they don't knock back boxes
	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);

	// use a sphere shape for the sweep
	FCollisionShape CollisionShape;
	CollisionShape.SetSphere(MeleeTraceRadius);

	// ignore self
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	if (GetWorld()->SweepMultiByObjectType(OutHits, TraceStart, TraceEnd, FQuat::Identity, ObjectParams, CollisionShape, QueryParams))
	{
		// iterate over each object hit
		for (const FHitResult& CurrentHit : OutHits)
		{
			/** does the actor have the player tag? */
			if (CurrentHit.GetActor()->ActorHasTag(FName("Player")))
			{
				// check if the actor is damageable
				IDamageable* Damageable = Cast<IDamageable>(CurrentHit.GetActor());

				if (Damageable)
				{
					// knock upwards and away from the impact normal
					const FVector Impulse = (CurrentHit.ImpactNormal * -MeleeKnockbackImpulse) + (FVector::UpVector * MeleeLaunchImpulse);

					// pass the damage event to the actor
					Damageable->ApplyDamage(MeleeDamage, this, CurrentHit.ImpactPoint, Impulse);

				}
			}
		}
	}
}

void ACombatEnemy::CheckCombo()
{
	// increase the combo counter
	++CurrentComboAttack;

	// do we still have attacks to play in this string?
	if (CurrentComboAttack < TargetComboCount)
	{
		// jump to the next attack section
		if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
		{
			AnimInstance->Montage_JumpToSection(ComboSectionNames[CurrentComboAttack], ComboAttackMontage);
		}
	}
}

void ACombatEnemy::CheckChargedAttack()
{
	// increase the charge loop counter
	++CurrentChargeLoop;

	// jump to either the loop or attack section of the montage depending on whether we hit the loop target
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->Montage_JumpToSection(CurrentChargeLoop >= TargetChargeLoops ? ChargeAttackSection : ChargeLoopSection, ChargedAttackMontage);
	}
}

void ACombatEnemy::ApplyDamage(float Damage, AActor* DamageCauser, const FVector& DamageLocation, const FVector& DamageImpulse)
{

	// pass the damage event to the actor
	FDamageEvent DamageEvent;
	const float ActualDamage = TakeDamage(Damage, DamageEvent, nullptr, DamageCauser);

	// only process knockback and effects if we received nonzero damage
	if (ActualDamage > 0.0f)
	{
		// apply the knockback impulse
		GetCharacterMovement()->AddImpulse(DamageImpulse, true);

		// is the character ragdolling?
		if (GetMesh()->IsSimulatingPhysics())
		{
			// apply an impulse to the ragdoll
			GetMesh()->AddImpulseAtLocation(DamageImpulse * GetMesh()->GetMass(), DamageLocation);
		}

		// stop the attack montages to interrupt the attack
		if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
		{
			AnimInstance->Montage_Stop(0.1f, ComboAttackMontage);
			AnimInstance->Montage_Stop(0.1f, ChargedAttackMontage);
		}

		// pass control to BP to play effects, etc.
		ReceivedDamage(ActualDamage, DamageLocation, DamageImpulse.GetSafeNormal());
	}
}

void ACombatEnemy::HandleDeath()
{
	if (CurrentHP <= 0.0f)
	{
		CurrentHP = 0.0f;
	}

	// hide the life bar
	LifeBar->SetHiddenInGame(true);

	if (MapPointComponent)
	{
		MapPointComponent->RemoveMapPoint();
	}

	// disable the collision capsule to avoid being hit again while dead
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// disable character movement
	GetCharacterMovement()->DisableMovement();

	// enable full ragdoll physics
	GetMesh()->SetSimulatePhysics(true);

	// call the died delegate to notify any subscribers
	OnEnemyDied.Broadcast();

	// set up the death timer
	GetWorld()->GetTimerManager().SetTimer(DeathTimer, this, &ACombatEnemy::RemoveFromLevel, FMath::Min(DeathRemovalTime, 0.25f), false);
}

void ACombatEnemy::ApplyHealing(float Healing, AActor* Healer)
{
	// stub
}

void ACombatEnemy::ApplyEnemyStats()
{
	float HealthMultiplier = 1.0f;
	float SpeedMultiplier = 1.0f;
	float DamageMultiplier = 1.0f;
	float RangeMultiplier = 1.0f;
	float RadiusMultiplier = 1.0f;

	switch (GetEffectiveEnemyType())
	{
	case EPXEnemyType::Fast:
		HealthMultiplier = 0.7f;
		SpeedMultiplier = 1.45f;
		DamageMultiplier = 0.85f;
		RangeMultiplier = 0.9f;
		PrototypeAttackCooldown = 0.75f;
		break;
	case EPXEnemyType::Tank:
		HealthMultiplier = 3.5f;
		SpeedMultiplier = 0.65f;
		DamageMultiplier = 1.7f;
		RangeMultiplier = 1.25f;
		RadiusMultiplier = 1.35f;
		PrototypeAttackCooldown = 1.35f;
		break;
	case EPXEnemyType::Ranged:
		HealthMultiplier = 1.0f;
		SpeedMultiplier = 0.8f;
		DamageMultiplier = 1.0f;
		PrototypeAttackCooldown = 1.6f;
		RangedAttackDistance = FMath::Max(RangedAttackDistance, 900.0f);
		break;
	case EPXEnemyType::Boss:
		HealthMultiplier = 12.0f;
		SpeedMultiplier = 0.75f;
		DamageMultiplier = 2.5f;
		RangeMultiplier = 1.6f;
		RadiusMultiplier = 1.8f;
		PrototypeAttackCooldown = 1.2f;
		break;
	case EPXEnemyType::Normal:
	default:
		break;
	}

	MaxHP = FMath::Max(1.0f, EnemyStats.MaxHealth * HealthMultiplier);
	CurrentHP = MaxHP;
	MeleeDamage = FMath::Max(0.0f, EnemyStats.AttackDamage * DamageMultiplier);
	MeleeTraceDistance = FMath::Max(0.0f, EnemyStats.AttackRange * RangeMultiplier);
	MeleeTraceRadius = FMath::Max(0.0f, EnemyStats.AttackRadius * RadiusMultiplier);

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = FMath::Max(0.0f, EnemyStats.MoveSpeed * SpeedMultiplier);
	}
}

void ACombatEnemy::ApplyEnemyTypeVisuals()
{
	if (!bApplyTypeColor)
	{
		return;
	}

	const FLinearColor TypeColor = GetEnemyTypeColor();
	USkeletalMeshComponent* EnemyMesh = GetMesh();
	if (!EnemyMesh)
	{
		return;
	}

	const int32 MaterialCount = FMath::Max(1, EnemyMesh->GetNumMaterials());
	for (int32 MaterialIndex = 0; MaterialIndex < MaterialCount; ++MaterialIndex)
	{
		UMaterialInterface* BaseMaterial = EnemyTypeMaterial ? EnemyTypeMaterial : EnemyMesh->GetMaterial(MaterialIndex);
		if (!BaseMaterial)
		{
			continue;
		}

		if (UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this))
		{
			DynamicMaterial->SetVectorParameterValue(TEXT("Color"), TypeColor);
			DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), TypeColor);
			DynamicMaterial->SetVectorParameterValue(TEXT("Tint"), TypeColor);
			DynamicMaterial->SetVectorParameterValue(TEXT("BodyColor"), TypeColor);
			EnemyMesh->SetMaterial(MaterialIndex, DynamicMaterial);
		}
	}
}

void ACombatEnemy::UpdatePrototypeChase(float DeltaSeconds)
{
	if (!bEnablePrototypeChaseFallback || !HasAuthority() || CurrentHP <= 0.0f || bIsAttacking)
	{
		return;
	}

	AAIController* AIController = Cast<AAIController>(GetController());
	ACharacter* TargetCharacter = GetTargetPlayerCharacter();
	if (!AIController || !TargetCharacter)
	{
		return;
	}

	const FVector ToTarget = TargetCharacter->GetActorLocation() - GetActorLocation();
	const float DistanceToTarget = FVector::Dist2D(GetActorLocation(), TargetCharacter->GetActorLocation());
	const float AttackDistance = GetPrototypeAttackDistance();
	if (DistanceToTarget <= AttackDistance)
	{
		AIController->StopMovement();
		AIController->SetFocus(TargetCharacter);
		TryPrototypeAttack(TargetCharacter);
		return;
	}

	const FVector MoveDirection = FVector(ToTarget.X, ToTarget.Y, 0.0f).GetSafeNormal();
	if (!MoveDirection.IsNearlyZero())
	{
		AddMovementInput(MoveDirection, 1.0f);
		SetActorRotation(MoveDirection.Rotation());
	}

	ChaseUpdateElapsed += DeltaSeconds;
	if (ChaseUpdateElapsed < ChaseUpdateInterval)
	{
		return;
	}
	ChaseUpdateElapsed = 0.0f;

	AIController->SetFocus(TargetCharacter);
	AIController->MoveToActor(TargetCharacter, AttackDistance, true, true, true);
}

void ACombatEnemy::TryPrototypeAttack(ACharacter* TargetCharacter)
{
	if (!TargetCharacter || CurrentHP <= 0.0f)
	{
		return;
	}

	const float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	if (CurrentTime - LastPrototypeAttackTime < PrototypeAttackCooldown)
	{
		return;
	}
	LastPrototypeAttackTime = CurrentTime;

	DoAIComboAttack();
}

void ACombatEnemy::ApplyPrototypeAttackDamage()
{
	if (CurrentHP <= 0.0f || bDamageAppliedThisAttack)
	{
		return;
	}

	if (IsRangedEnemy())
	{
		FireProjectileAtTarget(GetTargetPlayerCharacter());
		return;
	}

	DoAttackTrace(PrototypeAttackDamageBone);
}

void ACombatEnemy::FireProjectileAtTarget(ACharacter* TargetCharacter)
{
	if (!TargetCharacter || !GetWorld() || bDamageAppliedThisAttack)
	{
		return;
	}

	bDamageAppliedThisAttack = true;

	const FVector SpawnLocation = GetActorLocation() + (GetActorForwardVector() * ProjectileSpawnForwardOffset) + (FVector::UpVector * ProjectileSpawnUpOffset);
	const FVector TargetLocation = TargetCharacter->GetActorLocation() + FVector(0.0f, 0.0f, 55.0f);
	const FVector FireDirection = (TargetLocation - SpawnLocation).GetSafeNormal();
	if (FireDirection.IsNearlyZero())
	{
		return;
	}

	UClass* ProjectileToSpawn = ProjectileClass ? ProjectileClass.Get() : APXEnemyProjectile::StaticClass();
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.Instigator = this;

	if (APXEnemyProjectile* Projectile = GetWorld()->SpawnActor<APXEnemyProjectile>(ProjectileToSpawn, SpawnLocation, FireDirection.Rotation(), SpawnParameters))
	{
		Projectile->InitializeProjectile(this, MeleeDamage, MeleeKnockbackImpulse, MeleeLaunchImpulse);
	}
}

ACharacter* ACombatEnemy::GetTargetPlayerCharacter() const
{
	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(this, 0);
	if (PlayerCharacter && PlayerCharacter->Tags.Contains(FName("Player")))
	{
		return PlayerCharacter;
	}

	return PlayerCharacter;
}

EPXEnemyType ACombatEnemy::GetEffectiveEnemyType() const
{
	const FString ClassName = GetClass() ? GetClass()->GetName() : FString();
	if (ClassName.Contains(TEXT("Boss")))
	{
		return EPXEnemyType::Boss;
	}
	if (ClassName.Contains(TEXT("Range")) || ClassName.Contains(TEXT("Ranged")))
	{
		return EPXEnemyType::Ranged;
	}
	if (ClassName.Contains(TEXT("Tank")))
	{
		return EPXEnemyType::Tank;
	}
	if (ClassName.Contains(TEXT("Fast")))
	{
		return EPXEnemyType::Fast;
	}

	return EnemyStats.Type;
}

FLinearColor ACombatEnemy::GetEnemyTypeColor() const
{
	switch (GetEffectiveEnemyType())
	{
	case EPXEnemyType::Fast:
		return FLinearColor(0.1f, 0.65f, 1.0f, 1.0f);
	case EPXEnemyType::Tank:
		return FLinearColor(1.0f, 0.35f, 0.05f, 1.0f);
	case EPXEnemyType::Ranged:
		return FLinearColor(0.65f, 0.25f, 1.0f, 1.0f);
	case EPXEnemyType::Boss:
		return FLinearColor(1.0f, 0.05f, 0.1f, 1.0f);
	case EPXEnemyType::Normal:
	default:
		return FLinearColor(0.15f, 1.0f, 0.25f, 1.0f);
	}
}

bool ACombatEnemy::IsRangedEnemy() const
{
	return GetEffectiveEnemyType() == EPXEnemyType::Ranged;
}

float ACombatEnemy::GetPrototypeAttackDistance() const
{
	return IsRangedEnemy() ? RangedAttackDistance : MeleeTraceDistance + AttackAcceptancePadding;
}

void ACombatEnemy::RemoveFromLevel()
{
	// destroy this actor
	Destroy();
}

float ACombatEnemy::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	// only process damage if the character is still alive
	if (CurrentHP <= 0.0f)
	{
		return 0.0f;
	}

	// reduce the current HP
	CurrentHP -= Damage;

	// have we run out of HP?
	if (CurrentHP <= 0.0f)
	{
		// die
		HandleDeath();
	}
	else
	{
		// update the life bar
		LifeBarWidget->SetLifePercentage(CurrentHP / MaxHP);

		// enable partial ragdoll physics, but keep the pelvis vertical
		GetMesh()->SetPhysicsBlendWeight(0.5f);
		GetMesh()->SetBodySimulatePhysics(PelvisBoneName, false);
	}

	// return the received damage amount
	return Damage;
}

void ACombatEnemy::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	// is the character still alive?
	if (CurrentHP >= 0.0f)
	{
		// disable ragdoll physics
		GetMesh()->SetPhysicsBlendWeight(0.0f);
	}

	// call the landed Delegate for StateTree
	OnEnemyLanded.ExecuteIfBound();
}

void ACombatEnemy::BeginPlay()
{
	ApplyEnemyStats();
	ApplyEnemyTypeVisuals();

	// we top the HP before BeginPlay so StateTree picks it up at the right value
	Super::BeginPlay();

	if (MapPointComponent)
	{
		MapPointComponent->SetOwnerCharacter(this);
	}

	// get the life bar widget from the widget comp
	LifeBarWidget = Cast<UPXCombatLifeBar>(LifeBar->GetUserWidgetObject());
	check(LifeBarWidget);

	// fill the life bar
	LifeBarWidget->SetLifePercentage(1.0f);
}

void ACombatEnemy::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// clear the death timer
	GetWorld()->GetTimerManager().ClearTimer(DeathTimer);
	GetWorld()->GetTimerManager().ClearTimer(PrototypeAttackDamageTimer);
}

void ACombatEnemy::LifeBarExposure(bool bExpose)
{
	LifeBar->SetHiddenInGame(!bExpose);
}

float ACombatEnemy::GetHealthPercent() const
{
	return MaxHP > 0.0f ? FMath::Clamp(CurrentHP / MaxHP, 0.0f, 1.0f) : 0.0f;
}

bool ACombatEnemy::IsBossEnemy() const
{
	return GetEffectiveEnemyType() == EPXEnemyType::Boss;
}

FVector ACombatEnemy::GetLockOnSocketLocation_Implementation() const
{
	if (GetMesh())
	{
		return GetMesh()->GetSocketLocation(FName("LockOnSocket"));
	}
	return GetActorLocation();
}

void ACombatEnemy::SetLockOnTargeted_Implementation(bool bIsTargeted)
{
	LifeBarExposure(bIsTargeted);
}
