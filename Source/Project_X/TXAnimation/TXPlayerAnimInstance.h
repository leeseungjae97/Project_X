#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "TXPlayerAnimInstance.generated.h"


class ACombatCharacter;

UCLASS()
class PROJECT_X_API UTXPlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement|Dodge", meta=(AllowPrivateAccess="true"))
	UAnimMontage* DodgeMontage;

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaTime) override;

public:
	void CheckCombo(bool IsAttacking, const UAnimMontage* Montage);
	void ChargedAttack(FOnMontageEnded& OnAttackMontageEnded, UAnimMontage* ChargedAttackMontage);
	void ComboAttack(FOnMontageEnded& OnAttackMontageEnded, UAnimMontage* ComboAttackMontage);
	void WeaponAttack(FOnMontageEnded& OnAttackMontageEnded, UAnimMontage* MeleeAttackMontage);
	void CheckChargedAttack(UAnimMontage* ChargedAttackMontage);
	float GetNonStaleCachedInput();

public:
	/** Time at which an attack button was last pressed */
	float CachedAttackInputTime = 0.0f;

	/** Flag that determines if the player is currently holding the charged attack input */
	bool bIsChargingAttack = false;

	/** If true, the charged attack hold check has been tested at least once */
	bool bHasLoopedChargedAttack = false;

	bool bIsRunning = false;

	bool bIsAttacking = false;

	UPROPERTY(BlueprintReadOnly)
	float MaxWalkSpeed;

private:
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	ACombatCharacter* TXCharacter;

private:
	/** Max amount of time that may elapse for a non-combo attack input to not be considered stale */
	UPROPERTY(EditAnywhere, Category="Melee Attack", meta = (ClampMin = 0, ClampMax = 5))
	float AttackInputCacheTimeTolerance = 1.0f;

	/** Names of the AnimMontage sections that correspond to each stage of the combo attack */
	UPROPERTY(EditAnywhere, Category="Melee Attack|Combo")
	TArray<FName> ComboSectionNames;

	UPROPERTY(EditAnywhere, Category="Melee Attack|Combo", meta = (ClampMin = 0, ClampMax = 5))
	float ComboInputCacheTimeTolerance = 0.45f;

	/** Index of the current stage of the melee attack combo */
	int32 ComboCount = 0;

	/** Name of the AnimMontage section that corresponds to the charge loop */
	UPROPERTY(EditAnywhere, Category="Melee Attack|Charged")
	FName ChargeLoopSection;

	/** Name of the AnimMontage section that corresponds to the attack */
	UPROPERTY(EditAnywhere, Category="Melee Attack|Charged")
	FName ChargeAttackSection;

public:
	void PlayDodgeAnimation(FOnMontageEnded InOnMontageEndedDelegate);
};
