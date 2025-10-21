#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TXPartyRightClickWidget.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FKickDelegate, int64, ObjectId);
DECLARE_DYNAMIC_DELEGATE_OneParam(FChangeLeaderDelegate, int64, ObjectId);

class UTXPartySystem;
class UButton;

UCLASS()
class PROJECT_X_API UTXPartyRightClickWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeTick(const FGeometry& MyGeometry, float DeltaTime) override;
	virtual void NativeConstruct() override;

	UFUNCTION()
	void SetObjectId(int64 ObjectId);

private:
	UFUNCTION()
	void Kick();

	UFUNCTION()
	void ChangeLeader();
	
private:
	UPROPERTY()
	UTXPartySystem* PartySystem;

	FKickDelegate KickDelegate;
	FChangeLeaderDelegate ChangeLeaderDelegate;
	
private:
	int64 ObjectId;
	
private:
	UPROPERTY(meta = (BindWidget))
	UButton* KickButton;

	UPROPERTY(meta = (BindWidget))
	UButton* ChangeLeaderButton;
};
