#pragma once

#include "CoreMinimal.h"
#include "TXWeapons/TXWeapon.h"
#include "TXMeleeWeapon.generated.h"

class UNiagaraSystem;

UCLASS()
class PROJECT_X_API ATXMeleeWeapon : public ATXWeapon
{
	GENERATED_BODY()

public:
	ATXMeleeWeapon();

public:
	virtual void SetVisibility(bool InbVisibility) override;

protected:
	virtual void BeginPlay() override;

	void SpawnAurorSystem();
	
public:
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	UNiagaraSystem* AurorSystem;

	UPROPERTY()
	class UNiagaraComponent* AurorSystemComponent;
};
