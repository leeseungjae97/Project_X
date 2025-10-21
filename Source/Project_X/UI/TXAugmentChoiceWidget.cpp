// Fill out your copyright notice in the Description page of Project Settings.


#include "TXAugmentChoiceWidget.h"
#include "GameSystem/Augments/Data/TXAugmentData.h"
#include "TXAugmentCardWidget.h"
#include "Algo/RandomShuffle.h"
#include "GameSystem/Augments/TXAugmentComponent.h"
#include "Manager/TXDataTableManager.h"

void UTXAugmentChoiceWidget::ShowChoices(AActor* InOwnerActor, int32 NumChoices)
{
    OwnerActor = InOwnerActor;

    TArray<FTXAugmentData> Picks;
    if (!GetRandomAugments(NumChoices, Picks)) { RemoveFromParent(); return; }

    // 카드 세팅 & 바인딩
    TArray<UTXAugmentCardWidget*> Cards = { Card_1, Card_2, Card_3 };
    for (int32 i=0;i<Cards.Num();++i)
    {
        if (!Cards[i]) continue;
        if (Picks.IsValidIndex(i))
        {
            Cards[i]->SetupFromData(Picks[i]);
            Cards[i]->OnAugmentSelected.Clear();
            Cards[i]->OnAugmentSelected.AddDynamic(this, &UTXAugmentChoiceWidget::OnCardSelected);
            Cards[i]->SetVisibility(ESlateVisibility::Visible);
        }
        else
        {
            Cards[i]->SetVisibility(ESlateVisibility::Collapsed);
        }
    }
}

bool UTXAugmentChoiceWidget::GetRandomAugments(int32 Num, TArray<FTXAugmentData>& OutList) const
{
    if (!GetWorld()) return false;
    if (UGameInstance* GI = GetWorld()->GetGameInstance())
    {
        if (auto* TM = GI->GetSubsystem<UTXDataTableManager>())
        {
            static const FName TableName = TEXT("AugmentTable");
            if (const UDataTable* Table = TM->GetTableByName(TableName))
            {
                TArray<FName> Rows = Table->GetRowNames();
                GetRandomAugmentRows(Table, 3, OutList);
                return OutList.Num() > 0;
            }
        }
    }
    return false;
}

const FTXAugmentData* UTXAugmentChoiceWidget::GetRandomAugmentRow(const UDataTable* Table)
{
    if (!Table) return nullptr;

    TArray<FName> RowNames = Table->GetRowNames();
    if (RowNames.Num() == 0) return nullptr;

    // 한 번 섞고 첫 요소 선택
    Algo::RandomShuffle(RowNames);
    return Table->FindRow<FTXAugmentData>(RowNames[0], TEXT("GetRandomAugmentRow"));
}

void UTXAugmentChoiceWidget::GetRandomAugmentRows(const UDataTable* Table, int32 Num, TArray<FTXAugmentData>& Out)
{
    Out.Reset();
    if (!Table || Num <= 0) return;

    TArray<FName> RowNames = Table->GetRowNames();
    if (RowNames.Num() == 0) return;

    Algo::RandomShuffle(RowNames);

    const int32 Count = FMath::Min(Num, RowNames.Num());
    Out.Reserve(Count);

    for (int32 i = 0; i < Count; ++i)
    {
        if (const FTXAugmentData* Row = Table->FindRow<FTXAugmentData>(RowNames[i], TEXT("GetRandomAugmentRows")))
        {
            Out.Add(*Row);
        }
    }
}

void UTXAugmentChoiceWidget::OnCardSelected(FName AugmentID)
{
    if (!OwnerActor.IsValid()) { RemoveFromParent(); return; }

    if (auto* AugComp = OwnerActor->FindComponentByClass<UTXAugmentComponent>())
    {
        // 내부에서 TableManager 통해 실제 적용
        AugComp->ApplyAugmentByID(AugmentID, OwnerActor.Get(), nullptr);
    }

    RemoveFromParent(); // 선택 후 닫기
}
