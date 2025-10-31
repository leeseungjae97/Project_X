#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "PXAnimNotify_ControlMovement.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_X_API UPXAnimNotify_ControlMovement : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
