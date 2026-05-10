// Fill out your copyright notice in the Description page of Project Settings.


#include "TXAugmentChoiceWidget.h"
#include "GameFramework/PlayerController.h"
#include "GameSystem/Augments/Data/PXAugmentData.h"
#include "TXAugmentCardWidget.h"
#include "Algo/RandomShuffle.h"
#include "GameSystem/Augments/PXAugmentComponent.h"
#include "GameSystem/Rounds/PXRoundManager.h"
#include "GameSystem/Augments/Data/PXStatAugmentData.h"
#include "Manager/PXDataTableManager.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

void UTXAugmentChoiceWidget::ShowChoices(AActor* InOwnerActor, int32 NumChoices)
{
    OwnerActor = InOwnerActor;

    TArray<FPXAugmentData> Picks;
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

void UTXAugmentChoiceWidget::ShowChoicesFromRoundManager(APXRoundManager* InRoundManager)
{
    RoundManager = InRoundManager;
    if (!RoundManager.IsValid()) { RemoveFromParent(); return; }

	const TArray<FPXStatAugmentData> Picks = RoundManager->GetCurrentAugmentChoices();
	if (Picks.IsEmpty()) { RemoveFromParent(); return; }

	TArray<UTXAugmentCardWidget*> Cards = { Card_1, Card_2, Card_3 };
	if (!HasUsableBoundCards())
	{
		ShowFallbackStatChoices(Picks);
		return;
    }

    for (int32 i = 0; i < Cards.Num(); ++i)
    {
        if (!Cards[i]) continue;
        if (Picks.IsValidIndex(i))
        {
            Cards[i]->SetupFromStatData(Picks[i]);
            Cards[i]->OnAugmentSelected.Clear();
            Cards[i]->OnAugmentSelected.AddDynamic(this, &UTXAugmentChoiceWidget::OnStatCardSelected);
            Cards[i]->SetVisibility(ESlateVisibility::Visible);
        }
        else
        {
            Cards[i]->SetVisibility(ESlateVisibility::Collapsed);
        }
	}
}

bool UTXAugmentChoiceWidget::HasUsableBoundCards() const
{
	return Card_1 && Card_1->Btn_Select
		&& Card_2 && Card_2->Btn_Select
		&& Card_3 && Card_3->Btn_Select;
}

void UTXAugmentChoiceWidget::ShowFallbackStatChoices(const TArray<FPXStatAugmentData>& Picks)
{
	if (!WidgetTree)
	{
        return;
    }

    FallbackAugmentIDs.Reset();

	UCanvasPanel* RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("FallbackAugmentRoot"));
	WidgetTree->RootWidget = RootCanvas;
	RootCanvas->SetVisibility(ESlateVisibility::Visible);
	RootCanvas->SetRenderOpacity(1.0f);

	UBorder* Backdrop = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("FallbackAugmentBackdrop"));
	Backdrop->SetBrushColor(FLinearColor(0.01f, 0.012f, 0.02f, 0.82f));
	if (UCanvasPanelSlot* BackdropSlot = RootCanvas->AddChildToCanvas(Backdrop))
	{
		BackdropSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
		BackdropSlot->SetOffsets(FMargin(0.0f));
	}

	UVerticalBox* RootBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("FallbackAugmentContent"));
	if (UCanvasPanelSlot* ContentSlot = RootCanvas->AddChildToCanvas(RootBox))
	{
		ContentSlot->SetAnchors(FAnchors(0.5f, 0.5f));
		ContentSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		ContentSlot->SetPosition(FVector2D::ZeroVector);
		ContentSlot->SetSize(FVector2D(1120.0f, 420.0f));
	}

	UTextBlock* Title = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("FallbackAugmentTitle"));
	Title->SetText(FText::FromString(TEXT("SELECT AUGMENT")));
	Title->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.08f, 1.0f, 1.0f)));
	Title->SetJustification(ETextJustify::Center);
	if (UVerticalBoxSlot* TitleSlot = RootBox->AddChildToVerticalBox(Title))
	{
		TitleSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 28.0f));
		TitleSlot->SetHorizontalAlignment(HAlign_Fill);
	}

	UHorizontalBox* CardRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("FallbackAugmentCardRow"));
	if (UVerticalBoxSlot* RowSlot = RootBox->AddChildToVerticalBox(CardRow))
	{
		RowSlot->SetHorizontalAlignment(HAlign_Center);
	}

    for (int32 Index = 0; Index < FMath::Min(3, Picks.Num()); ++Index)
    {
        FallbackAugmentIDs.Add(Picks[Index].AugmentID);

		UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), *FString::Printf(TEXT("FallbackAugmentButton_%d"), Index + 1));
		Button->SetBackgroundColor(FLinearColor(0.12f, 0.0f, 0.16f, 0.95f));

		UBorder* CardBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), *FString::Printf(TEXT("FallbackAugmentCard_%d"), Index + 1));
		CardBorder->SetBrushColor(FLinearColor(0.035f, 0.025f, 0.055f, 0.98f));
		CardBorder->SetPadding(FMargin(24.0f));

		UVerticalBox* CardBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), *FString::Printf(TEXT("FallbackAugmentCardBox_%d"), Index + 1));
		CardBorder->SetContent(CardBox);

		UTextBlock* NameLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), *FString::Printf(TEXT("FallbackAugmentName_%d"), Index + 1));
		NameLabel->SetText(Picks[Index].Name);
		NameLabel->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.08f, 1.0f, 1.0f)));
		NameLabel->SetJustification(ETextJustify::Center);
		if (UVerticalBoxSlot* NameSlot = CardBox->AddChildToVerticalBox(NameLabel))
		{
			NameSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 24.0f));
			NameSlot->SetHorizontalAlignment(HAlign_Fill);
		}

		UTextBlock* DescLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), *FString::Printf(TEXT("FallbackAugmentDesc_%d"), Index + 1));
		DescLabel->SetText(Picks[Index].Description);
		DescLabel->SetColorAndOpacity(FSlateColor(FLinearColor(0.84f, 0.86f, 1.0f, 1.0f)));
		DescLabel->SetJustification(ETextJustify::Center);
		DescLabel->SetAutoWrapText(true);
		if (UVerticalBoxSlot* DescSlot = CardBox->AddChildToVerticalBox(DescLabel))
		{
			DescSlot->SetHorizontalAlignment(HAlign_Fill);
		}

		Button->AddChild(CardBorder);

        if (Index == 0)
        {
            Button->OnClicked.AddDynamic(this, &UTXAugmentChoiceWidget::OnFallbackCard1Selected);
        }
        else if (Index == 1)
        {
            Button->OnClicked.AddDynamic(this, &UTXAugmentChoiceWidget::OnFallbackCard2Selected);
        }
        else
        {
            Button->OnClicked.AddDynamic(this, &UTXAugmentChoiceWidget::OnFallbackCard3Selected);
        }

        if (UHorizontalBoxSlot* BoxSlot = CardRow->AddChildToHorizontalBox(Button))
        {
            BoxSlot->SetPadding(FMargin(14.0f, 0.0f));
			BoxSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
        }
    }
}

bool UTXAugmentChoiceWidget::GetRandomAugments(int32 Num, TArray<FPXAugmentData>& OutList) const
{
    if (!GetWorld()) return false;
    if (UGameInstance* GI = GetWorld()->GetGameInstance())
    {
        if (auto* TM = GI->GetSubsystem<UPXDataTableManager>())
        {
            static const FName TableName = TEXT("AugmentTable");
            if (const UDataTable* Table = TM->GetTableByName(TableName))
            {
                TArray<FName> Rows = Table->GetRowNames();
                GetRandomAugmentRows(Table, Num, OutList);
                return OutList.Num() > 0;
            }
        }
    }
    return false;
}

const FPXAugmentData* UTXAugmentChoiceWidget::GetRandomAugmentRow(const UDataTable* Table)
{
    if (!Table) return nullptr;

    TArray<FName> RowNames = Table->GetRowNames();
    if (RowNames.Num() == 0) return nullptr;

    // 한 번 섞고 첫 요소 선택
    Algo::RandomShuffle(RowNames);
    return Table->FindRow<FPXAugmentData>(RowNames[0], TEXT("GetRandomAugmentRow"));
}

void UTXAugmentChoiceWidget::GetRandomAugmentRows(const UDataTable* Table, int32 Num, TArray<FPXAugmentData>& Out)
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
        if (const FPXAugmentData* Row = Table->FindRow<FPXAugmentData>(RowNames[i], TEXT("GetRandomAugmentRows")))
        {
            Out.Add(*Row);
        }
    }
}

void UTXAugmentChoiceWidget::OnCardSelected(FName AugmentID)
{
    if (!OwnerActor.IsValid()) { RemoveFromParent(); return; }

    if (auto* AugComp = OwnerActor->FindComponentByClass<UPXAugmentComponent>())
    {
        // 내부에서 TableManager 통해 실제 적용
        AugComp->ApplyAugmentByID(AugmentID, OwnerActor.Get(), nullptr);
    }

	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		PlayerController->FlushPressedKeys();
		PlayerController->SetIgnoreMoveInput(false);
		PlayerController->SetIgnoreLookInput(false);
		PlayerController->bShowMouseCursor = false;
		PlayerController->SetInputMode(FInputModeGameOnly());
	}

    RemoveFromParent(); // 선택 후 닫기
}

void UTXAugmentChoiceWidget::OnStatCardSelected(FName AugmentID)
{
	if (RoundManager.IsValid())
	{
		RoundManager->NotifyAugmentSelected(AugmentID);
	}

	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		PlayerController->FlushPressedKeys();
		PlayerController->SetIgnoreMoveInput(false);
		PlayerController->SetIgnoreLookInput(false);
		PlayerController->bShowMouseCursor = false;
		PlayerController->SetInputMode(FInputModeGameOnly());
	}

	RemoveFromParent();
}

void UTXAugmentChoiceWidget::OnFallbackCard1Selected()
{
    SelectFallbackCard(0);
}

void UTXAugmentChoiceWidget::OnFallbackCard2Selected()
{
    SelectFallbackCard(1);
}

void UTXAugmentChoiceWidget::OnFallbackCard3Selected()
{
    SelectFallbackCard(2);
}

void UTXAugmentChoiceWidget::SelectFallbackCard(int32 CardIndex)
{
    if (FallbackAugmentIDs.IsValidIndex(CardIndex))
    {
        OnStatCardSelected(FallbackAugmentIDs[CardIndex]);
    }
}
