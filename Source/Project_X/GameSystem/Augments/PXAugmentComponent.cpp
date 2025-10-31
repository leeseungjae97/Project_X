// Fill out your copyright notice in the Description page of Project Settings.


#include "PXAugmentComponent.h"

#include "Core/EnumTypes.h"
#include "Data/PXAugmentData.h"
#include "GameSystem/Augments/AugmentEffect/PXAugmentEffect.h"
#include "Manager/PXDataTableManager.h"


// Sets default values for this component's properties
UPXAugmentComponent::UPXAugmentComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UPXAugmentComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UPXAugmentComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                        FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UPXAugmentComponent::ApplyAugmentByID(FName AugmentID, AActor* Instigator, AActor* Target)
{
	if (!Target || AugmentID.IsNone()) return;

	// GameInstance로부터 TableManager 가져오기
	if (UGameInstance* GI = GetWorld()->GetGameInstance())
	{
		UPXDataTableManager* TableManager = GI->GetSubsystem<UPXDataTableManager>();
		if (!TableManager) return;

		// 테이블 가져오기
		static const FName AugmentTableName = TEXT("AugmentTable");
		const UDataTable* FoundedTable = TableManager->GetTableByName(AugmentTableName);
		if (!FoundedTable) return;

		// Row 찾기
		const FPXAugmentData* AugmentRow = FoundedTable->FindRow<FPXAugmentData>(AugmentID, AugmentID.ToString());
		if (!AugmentRow) return;
		
		// 효과 적용
		OwnedAugments.Add(AugmentRow);
		TriggerAugment(AugmentRow->TriggerCondition, Instigator, Target);
	}
}

void UPXAugmentComponent::TriggerAugment(EAugmentTriggerCondition TriggerType, AActor* Instigator, AActor* Target)
{
	for (const auto& Augment : OwnedAugments)
	{
		if (Augment->TriggerCondition == TriggerType)
		{
			for (const FPXAugmentEffect& Effect : Augment->Effects)
			{
				ExecuteEffect(Effect, Instigator, Target);
			}
		}
	}
}

void UPXAugmentComponent::ExecuteEffect(const FPXAugmentEffect& Effect, AActor* Instigator, AActor* ExplicitTarget)
{
	AActor* FinalTarget = nullptr;
	switch (Effect.Target)
	{
	case EAugmentTarget::Self:
		FinalTarget = GetOwner();
		break;
	case EAugmentTarget::Enemy:
	case EAugmentTarget::Ally:
		FinalTarget = ExplicitTarget;
		break;
	case EAugmentTarget::Area:
		// 범위 내 여러 대상 검색
		return;
	}

	if (!FinalTarget) return;

	// 실제 효과 적용
	switch (Effect.EffectType)
	{
	case EAugmentEffectType::Heal:
		{
			// 예시
			//FinalTarget->TakeHealing(Effect.Value); 
			break;
		}
	case EAugmentEffectType::DamageBoost:
		// 예시
		//FinalTarget->DamageBoost(Effect.Value);
		break;
		
	default:
		break;
	}
}

