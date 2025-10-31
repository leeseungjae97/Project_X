#include "PXAnimation/PXAnimNotify_Fire.h"
#include "CombatCharacter.h"

void UPXAnimNotify_Fire::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (MeshComp == nullptr || MeshComp->GetOwner() == nullptr)
	{
		return;
	}

	if (ACombatCharacter* Character = Cast<ACombatCharacter>(MeshComp->GetOwner()))
	{
		Character->FireWeapon();
	}
}
