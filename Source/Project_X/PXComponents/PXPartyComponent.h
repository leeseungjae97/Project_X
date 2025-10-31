#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PXPartyComponent.generated.h"

UCLASS()
class PROJECT_X_API APXPartyComponent : public AActor
{
	GENERATED_BODY()
	
public:	
	APXPartyComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void SetReady(bool bReady);

public:
	int64 CurrentPartyId;

	UPROPERTY()
	bool IsReady;
};
