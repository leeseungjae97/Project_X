// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CombatAttacker.h"
#include "Damageable.h"
#include "TXWeapons/TXWeapon.h"
#include "TXWeapons/TXMeleeWeapon.h"
#include "TXWeapons/TXHitscanWeapon.h"
#include "CombatCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;
class UCombatLifeBar;
class UWidgetComponent;
class UTXCombatComponent;
class UTXPlayerAnimInstance;
class UTXLockOnComponent;
class UTXLockOnWidget;
class UTXMiniMapComponent;
//class ATXWeapon;

DECLARE_LOG_CATEGORY_EXTERN(LogCombatCharacter, Log, All);

/**
 *  An enhanced Third Person Character with melee combat capabilities:
 *  - Combo attack string
 *  - Press and hold charged attack
 *  - Damage dealing and reaction
 *  - Death
 *  - Respawning
 */
UCLASS(abstract)
class PROJECT_X_API ACombatCharacter : public ACharacter, public ICombatAttacker, public IDamageable
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	/** Life bar widget component */
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = UI, meta = (AllowPrivateAccess = "true"))
	//UWidgetComponent* LifeBar;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UTXCombatComponent* TXCombatComponent;

	UPROPERTY()
	UTXPlayerAnimInstance* TXPlayerAnimInstance;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UTXLockOnComponent* TXLockOnComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = MiniMap, meta = (AllowPrivateAccess = "true"))
	UTXMiniMapComponent* TXMiniMapComponent;

private:
	UPROPERTY()
	FString PlayerName;

public:
	FORCEINLINE const FString GetPlayerName() { return PlayerName; }
	FORCEINLINE void SetPlayerName(FString InPlayerName) { PlayerName = InPlayerName; }

protected:
	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* MouseLookAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* DashAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* LockOnAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* ESCAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* InventoryAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* InteractAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* ChatAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* SkillEAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* SkillQAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* Use1Action;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* Use2Action;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* Use3Action;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* Use4Action;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* Use5Action;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* LockChangeAction;


	/** Combo Attack Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* ComboAttackAction;

	/** Charged Attack Input Action */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* ChargedAttackAction;

	/** AnimMontage that will play for combo attacks */
	UPROPERTY(EditAnywhere, Category = "Melee Attack|Combo")
	UAnimMontage* ComboAttackMontage;

	UPROPERTY(EditAnywhere, Category = "Melee Attack")
	UAnimMontage* MeleeAttackMontage;

	UPROPERTY(EditAnywhere, Category = "Range Attack")
	UAnimMontage* HitscanAttackMontage;

	/** AnimMontage that will play for charged attacks */
	UPROPERTY(EditAnywhere, Category = "Melee Attack|Charged")
	UAnimMontage* ChargedAttackMontage;

	/*
	* Camera
	*/
	/** Camera boom length while the character is dead */
	UPROPERTY(EditAnywhere, Category = "Camera", meta = (ClampMin = 0, ClampMax = 1000, Units = "cm"))
	float DeathCameraDistance = 400.0f;

	/** Camera boom length when the character respawns */
	UPROPERTY(EditAnywhere, Category = "Camera", meta = (ClampMin = 0, ClampMax = 1000, Units = "cm"), meta = (AllowPrivateAccess = "true"))
	float DefaultCameraDistance = 700.0f;

	/*
	* UI Var
	*
	*/
	/** Life bar widget fill color */
	UPROPERTY(EditAnywhere, Category = "Damage")
	FLinearColor LifeBarColor;

	/** Character respawn timer */
	FTimerHandle RespawnTimer;

	/** Time to wait before respawning the character */
	UPROPERTY(EditAnywhere, Category = "Respawn", meta = (ClampMin = 0, ClampMax = 10))
	float RespawnTime = 3.0f;

	/** Attack montage ended delegate */
	FOnMontageEnded OnAttackMontageEnded;

	/** Copy of the mesh's transform so we can reset it after ragdoll animations */
	FTransform MeshStartingTransform;

public:
	/** Pointer to the life bar widget */
	//UPROPERTY(EditAnywhere, Category="Damage")
	//TObjectPtr<UCombatLifeBar> LifeBarWidget;

	/** Name of the pelvis bone, for damage ragdoll physics */
	UPROPERTY(EditAnywhere, Category = "Damage")
	FName PelvisBoneName;

public:

	/** Constructor */
	ACombatCharacter();

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	void StartDash() const;
	void DoDodge();

	void HandleInventory();

	void OpenInventory();

	void CloseInventory();

	void Interaction();

	void ESC();

	void Chat(const FInputActionValue& Value);

	void SkillE(const FInputActionValue& Value);

	void SkillQ(const FInputActionValue& Value);

	void LockOn();
	void LockOnChange();

	/** Called for combo attack input */
	void ComboAttackPressed();

	/** Called for combo attack input pressed */
	void ChargedAttackPressed();

	/** Called for combo attack input released */
	void ChargedAttackReleased();

	void Use1ActionPressed();
	void Use2ActionPressed();
	void Use3ActionPressed();
	void Use4ActionPressed();
	void Use5ActionPressed();

public:
	bool IsRunning();

	bool IsDashing();

	bool IsAttacking();
	
	void StopDash() const;
public:

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles look inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoLook(float Yaw, float Pitch);

	/** Handles combo attack pressed from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoComboAttackStart();

	/** Handles combo attack released from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoComboAttackEnd();

	/** Handles charged attack pressed from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoChargedAttackStart();

	/** Handles charged attack released from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoChargedAttackEnd();

protected:

	/** Resets the character's current HP to maximum */
	void ResetHP();

	/** Performs a combo attack */
	void ComboAttack();

	/** Performs a charged attack */
	void ChargedAttack();

	/** Called from a delegate when the attack montage ends */
	void AttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);


public:

	// ~begin CombatAttacker interface

	/** Performs the collision check for an attack */
	virtual void DoAttackTrace(FName DamageSourceBone) override;

	/** Performs the combo string check */
	virtual void CheckCombo() override;

	/** Performs the charged attack hold check */
	virtual void CheckChargedAttack() override;

	// ~end CombatAttacker interface

	// ~begin CombatDamageable interface

	/** Handles damage and knockback events */
	virtual void ApplyDamage(float Damage, AActor* DamageCauser, const FVector& DamageLocation, const FVector& DamageImpulse) override;

	/** Handles death events */
	virtual void HandleDeath() override;

	/** Handles healing events */
	virtual void ApplyHealing(float Healing, AActor* Healer) override;

	// ~end CombatDamageable interface

	/** Called from the respawn timer to destroy and re-create the character */
	void RespawnCharacter();

	void SetLifePercentage(float Percent);

public:

	/** Overrides the default TakeDamage functionality */
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	/** Overrides landing to reset damage ragdoll physics */
	virtual void Landed(const FHitResult& Hit) override;

protected:

	/** Blueprint handler to play damage dealt effects */
	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void DealtDamage(float Damage, const FVector& ImpactPoint);

	/** Blueprint handler to play damage received effects */
	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void ReceivedDamage(float Damage, const FVector& ImpactPoint, const FVector& DamageDirection);

protected:

	/** Initialization */
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	/** Cleanup */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Handles input bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Handles possessed initialization */
	virtual void NotifyControllerChanged() override;

public:

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

public:
	UPROPERTY(VisibleAnywhere)
	int64 MyObjectID;

	void SetObjectID(int64 ObjectID) { MyObjectID = ObjectID; }
	int64 GetObjectID() { return MyObjectID; }

	UFUNCTION(BlueprintCallable, Category = "Combat")
	FVector GetSimulatedVelocity() const { return SimulatedVelocity; }
	UFUNCTION(BlueprintCallable, Category = "Combat")
	FVector GetSimulatedAcceleration() const { return SimulatedAcceleration; }

private:
	FVector LastFrameLocation = FVector::ZeroVector;
	FVector SimulatedVelocity = FVector::ZeroVector;
	FVector LastSimulatedVelocity = FVector::ZeroVector;
	FVector SimulatedAcceleration = FVector::ZeroVector;

	FVector TargetLocation;
	float TargetYaw = 0.f;

	int64 TimeStampAtReceiveMs = 0;      // 클라가 수신한 시간 (ms)
	int64 EstimatedArrivalTimeMs = 0;    // 이때까지 도착해야 함 (ms)

	float InterpSpeed = 10.f; // 회전 보간 속도
public:
	double DashHoldStartTime = 0.0;

	UFUNCTION()
	void OnDashStarted(const FInputActionInstance& Instance);

	UFUNCTION()
	void OnDashTriggered(const FInputActionInstance& Instance);

	UFUNCTION()
	void OnDashCompleted(const FInputActionInstance& Instance);

	UFUNCTION()
	void OnDashCanceled(const FInputActionInstance& Instance);
	
public:
	void SetIsAttacking(bool bIsAttack);
	
public:
	void SpawnWeapon(TSubclassOf<ATXWeapon> SpawnWeaponClass);
	// void SwapWeapon();
	
	//void EquipWeapon(EWeaponType WeaponType);

	UPROPERTY()
	ATXWeapon* EquippedWeapon;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<ATXWeapon> MeleeWeaponClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ATXWeapon> HitscanWeaponClass;

	float HP = 100.f;
public:
	void WeaponAttack();
	void FireWeapon();
	void PlayWeaponMontage();
};
