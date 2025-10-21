// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/TXLockOnWidget.h"
#include "TXLockOnWidget.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Components/SceneComponent.h"
#include "Components/WidgetComponent.h"

void UTXLockOnWidget::PlayLockOnAnimation()
{
	if (LockOn)
	{
		PlayAnimation(LockOn, 0.f, 1);
	}
}

void UTXLockOnWidget::AttachToTargetAndPlay(ACharacter* TargetActor)
{
	if (USkeletalMeshComponent* SkeletalMesh = TargetActor->GetMesh())
	{
		if (OwningWidgetComponent)
		{
			//WidgetComponent->AttachToComponent(SkeletalMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName(TEXT("LockOnSocket")));
			OwningWidgetComponent->AttachToComponent(SkeletalMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName(TEXT("LockOnSocket")));
		}
	}

	PlayLockOnAnimation();
}

void UTXLockOnWidget::SetOwningWidgetComponent(UWidgetComponent* InComponent)
{
	if (OwningWidgetComponent)
		return;
	OwningWidgetComponent = InComponent;
	
}