// Fill out your copyright notice in the Description page of Project Settings.


#include "TXAugmentComponent.h"

#include "Core/EnumTypes.h"
#include "Data/TXAugmentData.h"
#include "GameSystem/Augments/AugmentEffect/TXAugmentEffect.h"
#include "Manager/TXDataTableManager.h"


// Sets default values for this component's properties
UTXAugmentComponent::UTXAugmentComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UTXAugmentComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UTXAugmentComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                        FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UTXAugmentComponent::ApplyAugmentByID(FName AugmentID, AActor* Instigator, AActor* Target)
{
	if (!Target || AugmentID.IsNone()) return;

	// GameInstance로부터 TableManager 가져오기
	if (UGameInstance* GI = GetWorld()->GetGameInstance())
	{
		UTXDataTableManager* TableManager = GI->GetSubsystem<UTXDataTableManager>();
		if (!TableManager) return;

		// 테이블 가져오기
		static const FName AugmentTableName = TEXT("AugmentTable");
		const UDataTable* FoundedTable = TableManager->GetTableByName(AugmentTableName);
		if (!FoundedTable) return;

		// Row 찾기
		const FTXAugmentData* AugmentRow = FoundedTable->FindRow<FTXAugmentData>(AugmentID, AugmentID.ToString());
		if (!AugmentRow) return;
		
		// 효과 적용
		OwnedAugments.Add(AugmentRow);
		TriggerAugment(AugmentRow->TriggerCondition, Instigator, Target);
	}
}

void UTXAugmentComponent::TriggerAugment(EAugmentTriggerCondition TriggerType, AActor* Instigator, AActor* Target)
{
	for (const auto& Augment : OwnedAugments)
	{
		if (Augment->TriggerCondition == TriggerType)
		{
			for (const FTXAugmentEffect& Effect : Augment->Effects)
			{
				ExecuteEffect(Effect, Instigator, Target);
			}
		}
	}
}

void UTXAugmentComponent::ExecuteEffect(const FTXAugmentEffect& Effect, AActor* Instigator, AActor* ExplicitTarget)
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

