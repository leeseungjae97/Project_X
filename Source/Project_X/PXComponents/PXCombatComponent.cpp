#include "PXComponents/PXCombatComponent.h"
#include "Player/Controller/TXPlayerController.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "PXWeapons/PXWeapon.h"
#include "PXWeapons/PXHitscanWeapon.h"
#include "CombatCharacter.h"

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
			// enable partial ragdoll physics, but keep the pelvis vertical
			OwnerCharacter->GetMesh()->SetPhysicsBlendWeight(0.5f);
			OwnerCharacter->GetMesh()->SetBodySimulatePhysics(OwnerCharacter->PelvisBoneName, false);
		}
	}
	return Damage;
}

void UPXCombatComponent::Landed(const FHitResult& Hit)
{
	if (nullptr == OwnerCharacter)
		return;

	if (CurrentHP >= 0.0f)
	{
		// disable ragdoll physics
		OwnerCharacter->GetMesh()->SetPhysicsBlendWeight(0.0f);
	}
}

void UPXCombatComponent::ResetStamina()
{
	CurrentStamina = MaxStamina;
	UpdateStaminaBar();
}
