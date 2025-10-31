// Copyright Epic Games, Inc. All Rights Reserved.


#include "PXAnimNotify_DoAttackTrace.h"
#include "Components/SkeletalMeshComponent.h"
#include "CombatAttacker.h"

void UPXAnimNotify_DoAttackTrace::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	// cast the owner to the attacker interface
	if (ICombatAttacker* AttackerInterface = Cast<ICombatAttacker>(MeshComp->GetOwner()))
	{
		AttackerInterface->DoAttackTrace(AttackBoneName);
	}
}

FString UPXAnimNotify_DoAttackTrace::GetNotifyName_Implementation() const
{
	return FString("Do Attack Trace");
}