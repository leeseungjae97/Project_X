#include "TXAnimation/TXPlayerAnimInstance.h"
#include "CombatCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PawnMovementComponent.h"

void UTXPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	ACharacter* pCharacter =  Cast<ACharacter>(TryGetPawnOwner());
	if (pCharacter)
	{
		MaxWalkSpeed = pCharacter->GetCharacterMovement()->MaxWalkSpeed * 1.5f;
	}
}

void UTXPlayerAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	APawn* OwnerPawn = TryGetPawnOwner();
	if (!OwnerPawn) return;

	if (!TXCharacter || TXCharacter != OwnerPawn)
	{
		TXCharacter = Cast<ACombatCharacter>(OwnerPawn);
		if (!TXCharacter) return;
	}

	// 최종 보장된 상태에서만 호출
	bIsRunning = TXCharacter->IsRunning();
	bIsAttacking  = TXCharacter->IsAttacking();
}

void UTXPlayerAnimInstance::CheckCombo(bool IsAttacking, const UAnimMontage* Montage)
{
	// are we playing a non-charge attack animation?
	if (!IsAttacking || bIsChargingAttack)
		return;
	
	// is the last attack input not stale?
	if (GetNonStaleCachedInput())
	{
		// consume the attack input so we don't accidentally trigger it twice
		CachedAttackInputTime = 0.0f;

		 
		// increase the combo counter
		++ComboCount;

		// do we still have a combo section to play?
		if (ComboCount < ComboSectionNames.Num())
		{
			// jump to the next combo section
			Montage_JumpToSection(ComboSectionNames[ComboCount], Montage);
		}
	}
}

void UTXPlayerAnimInstance::ChargedAttack(FOnMontageEnded& OnAttackMontageEnded, UAnimMontage* ChargedAttackMontage)
{
	// reset the charge loop flag
	bHasLoopedChargedAttack = false;

	// play the charged attack montage
	const float MontageLength = Montage_Play(ChargedAttackMontage, 1.0f, EMontagePlayReturnType::MontageLength, 0.0f, true);

	// subscribe to montage completed and interrupted events
	if (MontageLength > 0.0f)
	{
		// set the end delegate for the montage
		Montage_SetEndDelegate(OnAttackMontageEnded, ChargedAttackMontage);
	}
}

void UTXPlayerAnimInstance::ComboAttack(FOnMontageEnded& OnAttackMontageEnded, UAnimMontage* ComboAttackMontage)
{
	// reset the combo count
	ComboCount = 0;

	// play the attack montage
	const float MontageLength = Montage_Play(ComboAttackMontage, 1.0f, EMontagePlayReturnType::MontageLength, 0.0f, true);

	// subscribe to montage completed and interrupted events
	if (MontageLength > 0.0f)
	{
		// set the end delegate for the montage
		Montage_SetEndDelegate(OnAttackMontageEnded, ComboAttackMontage);
	}
}

void UTXPlayerAnimInstance::WeaponAttack(FOnMontageEnded& OnAttackMontageEnded, UAnimMontage* MeleeAttackMontage)
{
	const float MontageLength = Montage_Play(MeleeAttackMontage, 1.0f, EMontagePlayReturnType::MontageLength, 0.0f, true);

	if (MontageLength > 0.0f)
	{
		// set the end delegate for the montage
		Montage_SetEndDelegate(OnAttackMontageEnded, MeleeAttackMontage);
	}
}

void UTXPlayerAnimInstance::CheckChargedAttack(UAnimMontage* ChargedAttackMontage)
{
	// raise the looped charged attack flag
	bHasLoopedChargedAttack = true;

	// jump to either the loop or the attack section depending on whether we're still holding the charge button
	Montage_JumpToSection(bIsChargingAttack ? ChargeLoopSection : ChargeAttackSection, ChargedAttackMontage);
}

float UTXPlayerAnimInstance::GetNonStaleCachedInput()
{
	return GetWorld()->GetTimeSeconds() - CachedAttackInputTime <= AttackInputCacheTimeTolerance;
}

void UTXPlayerAnimInstance::PlayDodgeAnimation(FOnMontageEnded InOnMontageEndedDelegate)
{
	if (DodgeMontage)
	{
		Montage_Play(DodgeMontage);
		Montage_SetEndDelegate(InOnMontageEndedDelegate, DodgeMontage);
	}
}
