#include "PXComponents/PXCombatComponent.h"
#include "Player/Controller/TXPlayerController.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PXWeapons/PXWeapon.h"
#include "PXWeapons/PXHitscanWeapon.h"
#include "CombatCharacter.h"
#include "TimerManager.h"

UPXCombatComponent::UPXCombatComponent(): LifeBarColor(), MaxWalkSpeed(0)
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UPXCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	ResetStat();
	StartStaminaRecovery();
}

void UPXCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ConsumeStaminaWhileRunning(DeltaTime);

	if (HealthRecoveryAmount > 0.0f && CurrentHP > 0.0f && CurrentHP < MaxHP)
	{
		CurrentHP = FMath::Min(MaxHP, CurrentHP + HealthRecoveryAmount * DeltaTime);
		UpdateHPBar();
	}
}

void UPXCombatComponent::RecoverStamina()
{
	if (CurrentStamina >= MaxStamina)
	{
		CurrentStamina = MaxStamina;
		StopStaminaRecovery();
		return;
	}

	CurrentStamina = FMath::Clamp(CurrentStamina + StaminaRecoveryAmount, 0.f, MaxStamina);
	UpdateStaminaBar();
}

void UPXCombatComponent::ConsumeStaminaWhileRunning(float DeltaTime)
{
	if (!bIsRunning || CurrentStamina <= 0.f)
		return;

	const float StaminaCost = StaminaCostPerSecondWhileRunning * DeltaTime;
	CurrentStamina = FMath::Max(0.f, CurrentStamina - StaminaCost);

	UpdateStaminaBar();
	
	if (CurrentStamina <= 0.f)
	{
		SetIsRunning(false);
		OwnerCharacter->StopDash();
	}
}

void UPXCombatComponent::ResetStat()
{
	ResetHP();
	ResetStamina();
}

void UPXCombatComponent::StartStaminaRecovery()
{
	if (bIsRecoveringStamina) return;

	UWorld* pWorld = GetWorld();
	if (!pWorld) return;

	pWorld->GetTimerManager().SetTimer(
		StaminaRecoveryTimer,
		this,
		&UPXCombatComponent::RecoverStamina,
		StaminaRecoveryInterval,
		true
	);

	bIsRecoveringStamina = true;
}

void UPXCombatComponent::StopStaminaRecovery()
{
	if (!bIsRecoveringStamina) return;

	UWorld* pWorld = GetWorld();
	if (!pWorld) return;

	pWorld->GetTimerManager().ClearTimer(StaminaRecoveryTimer);
	bIsRecoveringStamina = false;
}

bool UPXCombatComponent::ConsumeStamina(float Amount)
{
	if (CurrentStamina < Amount)
		return false;

	CurrentStamina -= Amount;
	UpdateStaminaBar();
	return true;
}

void UPXCombatComponent::UpdateStaminaBar() const
{
	float StaminaPercent = CurrentStamina / MaxStamina;
	if (OwnerCharacter == nullptr)
		return;
	if (ATXPlayerController* PC = Cast<ATXPlayerController>(OwnerCharacter->GetController()))
	{
		PC->SetStaminaBar(StaminaPercent);
	}
}

void UPXCombatComponent::EquipWeapon(APXWeapon* Weapon)
{
	if (Weapon == nullptr || nullptr == OwnerCharacter || nullptr == OwnerCharacter->GetMesh())
		return;

	if (bIsWeaponEquipped)
	{
		if (EquippedWeapon)
		{
			EquippedWeapon->SetVisibility(false);
		}
	}
	bIsWeaponEquipped = true;
	EquippedWeapon = Weapon;

	EquippedWeapon->SetVisibility(true);

	EWeaponType WeaponType = Weapon->GetWeaponType();

	FName SocketName;
	if (WeaponType == EWeaponType::EWT_Melee)
		SocketName = TEXT("MeleeHandGrip_R");
	else
		SocketName = TEXT("HitscanHandGrip_R");

	if (const USkeletalMeshSocket* Socket = OwnerCharacter->GetMesh()->GetSocketByName(SocketName))
	{
		Socket->AttachActor(EquippedWeapon, OwnerCharacter->GetMesh());
	}
}

void UPXCombatComponent::UnequipWeapon()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->SetVisibility(false);
	}

	EquippedWeapon = nullptr;
	bIsWeaponEquipped = false;
	bIsAttacking = false;
}

void UPXCombatComponent::WeaponAttack()
{
	if (EquippedWeapon == nullptr)
		return;

	WeaponAttackStart.Broadcast();
	
	if (EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Melee)
	{

	}
}
void UPXCombatComponent::Fire()
{
	if(EquippedWeapon == nullptr)
		return;

	if (EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Melee)
		return;

	if (APXHitscanWeapon* HitscanWeapon = Cast<APXHitscanWeapon>(EquippedWeapon))
	{
		HitscanWeapon->Fire();
	}
}

EWeaponType UPXCombatComponent::GetWeaponType()
{
	if (EquippedWeapon == nullptr)
		return EWeaponType::EWT_MAX;

	return EquippedWeapon->GetWeaponType();
}

bool UPXCombatComponent::CanAttack() const
{
	const UWorld* World = GetWorld();
	if (!World)
		return false;

	return World->GetTimeSeconds() - LastAttackTime >= GetAttackCooldown();
}

void UPXCombatComponent::MarkAttackUsed()
{
	if (const UWorld* World = GetWorld())
	{
		LastAttackTime = World->GetTimeSeconds();
	}
}

float UPXCombatComponent::GetAttackCooldown() const
{
	return 1.0f / FMath::Max(0.1f, AttackSpeed);
}

void UPXCombatComponent::ApplyStatModifier(EPXPlayerStatType StatType, float AddValue, float MultiplyValue)
{
	const float SafeMultiplier = FMath::Max(0.0f, MultiplyValue);

	switch (StatType)
	{
	case EPXPlayerStatType::MaxHealth:
		MaxHP = FMath::Max(1.0f, (MaxHP + AddValue) * SafeMultiplier);
		CurrentHP = FMath::Min(CurrentHP + AddValue, MaxHP);
		UpdateHPBar();
		break;
	case EPXPlayerStatType::MoveSpeed:
		MaxWalkSpeed = FMath::Max(0.0f, (MaxWalkSpeed + AddValue) * SafeMultiplier);
		if (OwnerCharacter)
		{
			OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed = MaxWalkSpeed;
		}
		break;
	case EPXPlayerStatType::AttackSpeed:
		AttackSpeed = FMath::Max(0.1f, (AttackSpeed + AddValue) * SafeMultiplier);
		break;
	case EPXPlayerStatType::AttackRange:
		MeleeTraceDistance = FMath::Max(0.0f, (MeleeTraceDistance + AddValue) * SafeMultiplier);
		MeleeTraceRadius = FMath::Max(0.0f, (MeleeTraceRadius + AddValue) * SafeMultiplier);
		break;
	case EPXPlayerStatType::MaxStamina:
		MaxStamina = FMath::Max(1.0f, (MaxStamina + AddValue) * SafeMultiplier);
		CurrentStamina = FMath::Min(CurrentStamina + AddValue, MaxStamina);
		UpdateStaminaBar();
		break;
	case EPXPlayerStatType::AttackDamage:
		MeleeDamage = FMath::Max(0.0f, (MeleeDamage + AddValue) * SafeMultiplier);
		break;
	case EPXPlayerStatType::HealthRegen:
		HealthRecoveryAmount = FMath::Max(0.0f, (HealthRecoveryAmount + AddValue) * SafeMultiplier);
		break;
	case EPXPlayerStatType::StaminaRegen:
		StaminaRecoveryAmount = FMath::Max(0.0f, (StaminaRecoveryAmount + AddValue) * SafeMultiplier);
		break;
	default:
		break;
	}
}

void UPXCombatComponent::SetIsAttacking(bool InbIsAttacking)
{
	bIsAttacking = InbIsAttacking;

	if (!EquippedWeapon)
		return;

	if (!EquippedWeapon->CollisionComponent)
		return;
	
	WeaponEnterHitPoint.Broadcast();
}

void UPXCombatComponent::ResetHP()
{
	CurrentHP = MaxHP;
	UpdateHPBar();
}

void UPXCombatComponent::UpdateHPBar() const
{
	float NewHP = CurrentHP / MaxHP;
	if (IsValid(OwnerCharacter) == false)
		return;
	if (ATXPlayerController* PC = Cast<ATXPlayerController>(OwnerCharacter->GetController()))
	{
		PC->SetHpBar(NewHP);
	}
}

float UPXCombatComponent::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	// only process damage if the character is still alive
	if (CurrentHP <= 0.0f)
	{
		return 0.0f;
	}

	if (nullptr == OwnerCharacter)
		return 0.0f;

	// reduce the current HP
	CurrentHP -= Damage;

	//have we run out of HP?
	if (CurrentHP <= 0.0f)
	{
		// die
		OwnerCharacter->HandleDeath();
	}
	else
	{
		// update the life bar
		UpdateHPBar();

		if (OwnerCharacter->GetMesh())
		{
			// Brief physics blend gives a hit reaction without leaving the player stuck in physics.
			OwnerCharacter->GetMesh()->SetPhysicsBlendWeight(HitReactionPhysicsBlendWeight);
			OwnerCharacter->GetMesh()->SetBodySimulatePhysics(OwnerCharacter->PelvisBoneName, false);
		}

		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(HitReactionPhysicsTimer);
			World->GetTimerManager().SetTimer(HitReactionPhysicsTimer, this, &UPXCombatComponent::ResetHitReactionPhysics, HitReactionPhysicsDuration, false);
		}
	}
	return Damage;
}

void UPXCombatComponent::ResetHitReactionPhysics()
{
	if (OwnerCharacter && OwnerCharacter->GetMesh() && CurrentHP > 0.0f)
	{
		OwnerCharacter->GetMesh()->SetPhysicsBlendWeight(0.0f);
		OwnerCharacter->GetMesh()->SetSimulatePhysics(false);
		RestoreOwnerMovementIfNeeded();
	}
}

void UPXCombatComponent::RestoreOwnerMovementIfNeeded()
{
	if (!OwnerCharacter || CurrentHP <= 0.0f)
	{
		return;
	}

	UCharacterMovementComponent* MovementComponent = OwnerCharacter->GetCharacterMovement();
	if (!MovementComponent)
	{
		return;
	}

	if (MovementComponent->MovementMode == MOVE_None)
	{
		MovementComponent->SetMovementMode(MOVE_Walking);
	}

	MovementComponent->MaxWalkSpeed = MaxWalkSpeed;
}

void UPXCombatComponent::Landed(const FHitResult& Hit)
{
	if (nullptr == OwnerCharacter)
		return;

	if (CurrentHP > 0.0f)
	{
		ResetHitReactionPhysics();
		RestoreOwnerMovementIfNeeded();
	}
}

void UPXCombatComponent::ResetStamina()
{
	CurrentStamina = MaxStamina;
	UpdateStaminaBar();
}
