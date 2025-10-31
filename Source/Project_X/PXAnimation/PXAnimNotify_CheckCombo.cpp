// Copyright Epic Games, Inc. All Rights Reserved.


#include "PXAnimNotify_CheckCombo.h"
#include "Components/SkeletalMeshComponent.h"
#include "CombatAttacker.h"

void UPXAnimNotify_CheckCombo::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	// cast the owner to the attacker interface
	if (ICombatAttacker* AttackerInterface = Cast<ICombatAttacker>(MeshComp->GetOwner()))
	{
		// tell the actor to check for combo string
		AttackerInterface->CheckCombo();
	}
}

FString UPXAnimNotify_CheckCombo::GetNotifyName_Implementation() const
{
	return FString("Check Combo String");
}
