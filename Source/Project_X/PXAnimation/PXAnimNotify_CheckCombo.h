// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "PXAnimNotify_CheckCombo.generated.h"

/**
 *  AnimNotify to perform a combo string check.
 */
UCLASS()
class UPXAnimNotify_CheckCombo : public UAnimNotify
{
	GENERATED_BODY()
	
public:

	/** Perform the Anim Notify */
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	/** Get the notify name */
	virtual FString GetNotifyName_Implementation() const override;
};
