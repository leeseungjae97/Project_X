#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TXLockOnTargetInterface.generated.h"

UINTERFACE(MinimalAPI)
class UTXLockOnTargetInterface : public UInterface
{
	GENERATED_BODY()
};

class PROJECT_X_API ITXLockOnTargetInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "LockOn")
	FVector GetLockOnSocketLocation() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "LockOn")
	void SetLockOnTargeted(bool bIsTargeted);
};
