#include "PXAnimation/PXPlayerAnimInstance.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CombatCharacter.h"

void UPXPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	ACharacter* pCharacter =  Cast<ACharacter>(TryGetPawnOwner());
	if (pCharacter)
	{
		MaxWalkSpeed = pCharacter->GetCharacterMovement()->MaxWalkSpeed * 1.5f;
	}
}

void UPXPlayerAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	APawn* OwnerPawn = TryGetPawnOwner();
	if (!OwnerPawn) return;

	if (ACharacter* OwnerCharacter = Cast<ACharacter>(OwnerPawn))
	{
		if (UCharacterMovementComponent* CharacterMovement = OwnerCharacter->GetCharacterMovement())
		{
			MaxWalkSpeed = CharacterMovement->MaxWalkSpeed;
			const float GroundSpeed = CharacterMovement->Velocity.Size2D();
			bIsRunning = GroundSpeed > MaxWalkSpeed * 0.65f;
		}
	}

	if (!TXCharacter || TXCharacter != OwnerPawn)
	{
		TXCharacter = Cast<ACombatCharacter>(OwnerPawn);
		if (!TXCharacter) return;
	}

	// 플레이어는 입력 상태를 기준으로 달리기 애니메이션을 결정한다.
	bIsRunning = TXCharacter->IsRunning();
	bIsAttacking  = TXCharacter->IsAttacking();
}

void UPXPlayerAnimInstance::CheckCombo(bool IsAttacking, const UAnimMontage* Montage)
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

void UPXPlayerAnimInstance::ChargedAttack(FOnMontageEnded& OnAttackMontageEnded, UAnimMontage* ChargedAttackMontage)
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

void UPXPlayerAnimInstance::ComboAttack(FOnMontageEnded& OnAttackMontageEnded, UAnimMontage* ComboAttackMontage)
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

void UPXPlayerAnimInstance::WeaponAttack(FOnMontageEnded& OnAttackMontageEnded, UAnimMontage* MeleeAttackMontage)
{
	const float MontageLength = Montage_Play(MeleeAttackMontage, 1.0f, EMontagePlayReturnType::MontageLength, 0.0f, true);

	if (MontageLength > 0.0f)
	{
		// set the end delegate for the montage
		Montage_SetEndDelegate(OnAttackMontageEnded, MeleeAttackMontage);
	}
}

void UPXPlayerAnimInstance::CheckChargedAttack(UAnimMontage* ChargedAttackMontage)
{
	// raise the looped charged attack flag
	bHasLoopedChargedAttack = true;

	// jump to either the loop or the attack section depending on whether we're still holding the charge button
	Montage_JumpToSection(bIsChargingAttack ? ChargeLoopSection : ChargeAttackSection, ChargedAttackMontage);
}

float UPXPlayerAnimInstance::GetNonStaleCachedInput()
{
	return GetWorld()->GetTimeSeconds() - CachedAttackInputTime <= AttackInputCacheTimeTolerance;
}

void UPXPlayerAnimInstance::PlayDodgeAnimation(FOnMontageEnded InOnMontageEndedDelegate)
{
	if (DodgeMontage)
	{
		Montage_Play(DodgeMontage);
		Montage_SetEndDelegate(InOnMontageEndedDelegate, DodgeMontage);
	}
}
