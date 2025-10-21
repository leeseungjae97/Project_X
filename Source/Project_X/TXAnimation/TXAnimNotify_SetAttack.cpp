
#include "TXAnimation/TXAnimNotify_SetAttack.h"

#include "CombatCharacter.h"

void UTXAnimNotify_SetAttack::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                     const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (ACombatCharacter* Character = Cast<ACombatCharacter>(MeshComp->GetOwner()))
	{
		Character->SetIsAttacking(bIsAttacking);
	}
}
