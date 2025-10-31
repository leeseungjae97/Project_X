#pragma once

#include "CoreMinimal.h"
#include "PXWeapons/PXWeapon.h"
#include "PXMeleeWeapon.generated.h"

class UNiagaraSystem;

UCLASS()
class PROJECT_X_API APXMeleeWeapon : public APXWeapon
{
	GENERATED_BODY()

public:
	APXMeleeWeapon();

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
