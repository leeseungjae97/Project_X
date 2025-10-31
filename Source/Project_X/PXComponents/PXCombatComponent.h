#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PXWeapons/PXWeapon.h"
#include "PXCombatComponent.generated.h"

class UTXHUDWidget;
class UPXCombatLifeBar;
class UWidgetComponent;
class ACombatCharacter;

DECLARE_MULTICAST_DELEGATE(FWeaponAttackStart);
DECLARE_MULTICAST_DELEGATE(FWeaponEquipped);
DECLARE_MULTICAST_DELEGATE(FWeaponEnterHitPoint);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECT_X_API UPXCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPXCombatComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	// 스태미너 회복 함수
	void RecoverStamina();
	void ConsumeStaminaWhileRunning(float DeltaTime);
public:
	void ResetStat();
	void ResetHP();
	void UpdateHPBar() const;
	float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser);
	void Landed(const FHitResult& Hit);
	
	// 스태미너 회복 시작 / 중지
	void ResetStamina();
	void StartStaminaRecovery();
	void StopStaminaRecovery();
	bool ConsumeStamina(float Amount);
	void UpdateStaminaBar() const;

	void EquipWeapon(APXWeapon* Weapon);
	void WeaponAttack();
	void Fire();

	EWeaponType GetWeaponType();

public:
	FORCEINLINE float GetMaxHP() { return MaxHP; }
	FORCEINLINE float GetCurrentHP() { return CurrentHP; }
	FORCEINLINE float GetMaxStamina() { return MaxStamina; }
	FORCEINLINE float GetCurrentStamina() { return CurrentStamina; }
	FORCEINLINE float GetMeleeTraceDistance() { return MeleeTraceDistance; }
	FORCEINLINE float GetMeleeTraceRadius() { return MeleeTraceRadius; }
	FORCEINLINE float GetMeleeDamage() { return MeleeDamage; }
	FORCEINLINE float GetMeleeKnockbackImpulse() { return MeleeKnockbackImpulse; }
	FORCEINLINE float GetMeleeLaunchImpulse() { return MeleeLaunchImpulse; }
	FORCEINLINE float GetMaxWalkSpeed() { return MaxWalkSpeed; }
	FORCEINLINE bool IsAttacking() { return bIsAttacking; }
	FORCEINLINE bool IsInventoryOpen() { return bIsInventoryOpen; }
	FORCEINLINE bool IsRunning() { return bIsRunning; }
	FORCEINLINE bool IsDodging() { return bIsDodging; }
	FORCEINLINE bool IsWeaponEquipped() { return bIsWeaponEquipped; }

	FORCEINLINE void SetMaxHP(float InMaxHP) { MaxHP = InMaxHP; }
	FORCEINLINE void SetCurrentHP(float InCurrentHP) { CurrentHP = InCurrentHP; }
	FORCEINLINE void SetMaxStamina(float InMaxStamina) { MaxStamina = InMaxStamina; }
	FORCEINLINE void SetCurrentStamina(float InCurrentStamina) { CurrentStamina = InCurrentStamina; }
	FORCEINLINE void SetMeleeTraceDistance(float InMeleeTraceDistance) { MeleeTraceDistance = InMeleeTraceDistance; }
	FORCEINLINE void SetMeleeTraceRadius(float InMeleeTraceRadius) { MeleeTraceRadius = InMeleeTraceRadius; }
	FORCEINLINE void SetMeleeDamage(float InMeleeDamage) { MeleeDamage = InMeleeDamage; }
	FORCEINLINE void SetMeleeKnockbackImpulse(float InMeleeKnockbackImpulse) { MeleeKnockbackImpulse = InMeleeKnockbackImpulse; }
	FORCEINLINE void SetMeleeLaunchImpulse(float InMeleeLaunchImpulse) { MeleeLaunchImpulse = InMeleeLaunchImpulse; }
	FORCEINLINE void SetMaxWalkSpeed(float InMaxWalkSpeed) { MaxWalkSpeed = InMaxWalkSpeed; }
	FORCEINLINE void SetIsInventoryOpen(bool InbIsInventoryOpen) { bIsInventoryOpen = InbIsInventoryOpen; }
	FORCEINLINE void SetIsRunning(bool InbIsRunning) { bIsRunning = InbIsRunning; }
	FORCEINLINE void SetIsDodging(bool InbIsDashing) { bIsDodging = InbIsDashing; }
	FORCEINLINE void SetIsWeaponEquipped(bool InbIsWeaponEquipped) { bIsWeaponEquipped = InbIsWeaponEquipped; }
	void SetIsAttacking(bool InbIsAttacking);

public:
	UPROPERTY()
	ACombatCharacter* OwnerCharacter = nullptr;

public:
	FWeaponAttackStart WeaponAttackStart;
	FWeaponEquipped WeaponEquipped;

	FWeaponEnterHitPoint WeaponEnterHitPoint;

private:
	/** Max amount of HP the character will have on respawn */
	UPROPERTY(EditAnywhere, Category = "Damage", meta = (ClampMin = 0, ClampMax = 100))
	float MaxHP = 5.0f;

	/** Current amount of HP the character has */
	UPROPERTY(VisibleAnywhere, Category = "Damage")
	float CurrentHP = 0.0f;

	/** Max amount of Stamina the character will have on respawn */
	UPROPERTY(EditAnywhere, Category = "Stamina", meta = (ClampMin = 0, ClampMax = 100))
	float MaxStamina = 100.0f;

	/** Current amount of Stamina the character has */
	UPROPERTY(VisibleAnywhere, Category = "Stamina")
	float CurrentStamina = 0.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Stamina")
	float StaminaRecoveryAmount = 20.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Stamina")
	float StaminaRecoveryInterval = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Stamina", meta = (ClampMin = 0))
	float StaminaCostPerSecondWhileRunning = 8.f;

	UPROPERTY(EditAnywhere, Category = "Stamina", meta = (ClampMin = 0))
	float XP = 0.0f;

	/** Life bar widget fill color */
	UPROPERTY(EditAnywhere, Category = "Damage")
	FLinearColor LifeBarColor;

	/** Pointer to the life bar widget */
	UPROPERTY(EditAnywhere, Category = "Damage")
	TObjectPtr<UPXCombatLifeBar> LifeBarWidget;

	/** Max amount of time that may elapse for a non-combo attack input to not be considered stale */
	UPROPERTY(EditAnywhere, Category = "Melee Attack", meta = (ClampMin = 0, ClampMax = 5))
	float AttackInputCacheTimeTolerance = 1.0f;

	/** Time at which an attack button was last pressed */
	float CachedAttackInputTime = 0.0f;

	/** If true, the character is currently playing an attack animation */
	bool bIsAttacking = false;

	/** Distance ahead of the character that melee attack sphere collision traces will extend */
	UPROPERTY(EditAnywhere, Category = "Melee Attack|Trace", meta = (ClampMin = 0, ClampMax = 500, Units = "cm"))
	float MeleeTraceDistance = 75.0f;

	/** Radius of the sphere trace for melee attacks */
	UPROPERTY(EditAnywhere, Category = "Melee Attack|Trace", meta = (ClampMin = 0, ClampMax = 200, Units = "cm"))
	float MeleeTraceRadius = 75.0f;

	/** Amount of damage a melee attack will deal */
	UPROPERTY(EditAnywhere, Category = "Melee Attack|Damage", meta = (ClampMin = 0, ClampMax = 100))
	float MeleeDamage = 1.0f;

	/** Amount of knockback impulse a melee attack will apply */
	UPROPERTY(EditAnywhere, Category = "Melee Attack|Damage", meta = (ClampMin = 0, ClampMax = 1000, Units = "cm/s"))
	float MeleeKnockbackImpulse = 250.0f;

	/** Amount of upwards impulse a melee attack will apply */
	UPROPERTY(EditAnywhere, Category = "Melee Attack|Damage", meta = (ClampMin = 0, ClampMax = 1000, Units = "cm/s"))
	float MeleeLaunchImpulse = 300.0f;

	UPROPERTY(EditAnywhere, Category = "Movement|WalkSpeed", meta = (ClampMin = 0, ClampMax = 1000, Units = "cm/s"))
	float MaxWalkSpeed;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> HUDWidgetClass;
	
	bool bIsInventoryOpen = false;

	bool bIsRunning = false;

	bool bIsDodging = false;
	
	bool bIsRecoveringStamina = false;
	
	FTimerHandle StaminaRecoveryTimer;

	// Weapon
	bool bIsWeaponEquipped = false;

	UPROPERTY()
	APXWeapon* EquippedWeapon;
};
