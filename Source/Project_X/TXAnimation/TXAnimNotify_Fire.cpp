#include "TXAnimation/TXAnimNotify_Fire.h"
#include "CombatCharacter.h"
#include "TXWeapons/TXHitscanWeapon.h"
#include "Kismet/GameplayStatics.h"

void UTXAnimNotify_Fire::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
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
