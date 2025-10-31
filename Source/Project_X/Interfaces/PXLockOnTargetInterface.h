#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PXLockOnTargetInterface.generated.h"

UINTERFACE(MinimalAPI)
class UPXLockOnTargetInterface : public UInterface
{
	GENERATED_BODY()
};

class PROJECT_X_API IPXLockOnTargetInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "LockOn")
	FVector GetLockOnSocketLocation() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "LockOn")
	void SetLockOnTargeted(bool bIsTargeted);
};
