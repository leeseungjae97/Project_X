#include "TXAnimation/TXAnimNotify_ControlMovement.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

void UTXAnimNotify_ControlMovement::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (MeshComp && MeshComp->GetOwner())
	{
		if (ACharacter* Character = Cast<ACharacter>(MeshComp->GetOwner()))
		{
			if (Character->GetCharacterMovement()->MovementMode != EMovementMode::MOVE_None)
				Character->GetCharacterMovement()->DisableMovement();
			else
				Character->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
		}
	}
}
