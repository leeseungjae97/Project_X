#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TXWeapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),

	EWS_MAX UMETA(DisplayName = "DefaultMAX"),
};

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_Melee UMETA(DisplayName = "Melee Type"),
	EWT_Projectile UMETA(DisplayName = "Projectile Type"),
	EWT_Hitscan UMETA(DisplayName = "Hitscan Type"),

	EWT_MAX UMETA(DisplayName = "DefaultMAX"),
};

class ACombatCharacter;
class UTXCollisionComponent;

UCLASS()
class PROJECT_X_API ATXWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	ATXWeapon();

protected:
	virtual void BeginPlay() override;

public:
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE EWeaponState GetWeaponState() const { return WeaponState; }

	FORCEINLINE void SetWeaponState(EWeaponState NewState) { WeaponState = NewState; }
	FORCEINLINE void SetWeaponType(EWeaponType NewType) { WeaponType = NewType; }

public:	
	virtual void Tick(float DeltaTime) override;

public:
	virtual void SetVisibility(bool InbVisibility);

	UFUNCTION()
	virtual void AttackStart();
	
	UFUNCTION()
	virtual void Equipped();
	
private:
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	EWeaponState WeaponState;

protected:
	UPROPERTY()
	ACombatCharacter* OwnerCharacter;
	
public:
	UPROPERTY()
	UTXCollisionComponent* CollisionComponent;
	
	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;

public:
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
	FORCEINLINE ACombatCharacter* GetOwnerCharacter() const { return OwnerCharacter; }

	FORCEINLINE void SetOwnerCharacter(ACombatCharacter* NewOwner) { OwnerCharacter = NewOwner; }
};
