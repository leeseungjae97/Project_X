// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/TXMiniMapWidget.h"
#include "GameFramework/PlayerController.h"
#include "PXComponents/PXMiniMapComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Texture2D.h"
#include "UI/TXEnemyMarkerWidget.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Components/Overlay.h"
#include "Blueprint/WidgetTree.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMaterialLibrary.h"


void UTXMiniMapWidget::NativeConstruct()
{
	Super::NativeConstruct();
	IsReady = true;
}

void UTXMiniMapWidget::SetMiniMapImage(UTextureRenderTarget2D* RenderTarget)
{
	if (!MiniMapImage)
		return;

	FSlateBrush NewBrush;
	NewBrush.SetResourceObject(RenderTarget);
	NewBrush.ImageSize = FVector2D(RenderTarget->SizeX, RenderTarget->SizeY);
	MiniMapImage->SetBrush(NewBrush);
}

void UTXMiniMapWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	SetMiniMapImage();
	SetPanelRotate();
}

void UTXMiniMapWidget::AddMapPoint_Implementation(AActor* Actor)
{
	UOverlay* MapPointsOverlay = FindMapPointsOverlay();
	if (!Actor || !MapPointsOverlay)
	{
		return;
	}

	TSubclassOf<UTXEnemyMarkerWidget> WidgetClass = MarkerWidgetClass;
	if (!WidgetClass)
	{
		WidgetClass = UTXEnemyMarkerWidget::StaticClass();
	}
	UTXEnemyMarkerWidget* MarkerWidget = CreateWidget<UTXEnemyMarkerWidget>(GetOwningPlayer(), WidgetClass);
	if (!MarkerWidget)
	{
		return;
	}

	const FString ActorClassName = Actor->GetClass() ? Actor->GetClass()->GetName() : FString();
	const bool bBoss = ActorClassName.Contains(TEXT("Boss"));
	const bool bRanged = ActorClassName.Contains(TEXT("Range")) || ActorClassName.Contains(TEXT("Ranged"));

	MarkerWidget->SetMarkerActor(Actor);
	if (bBoss)
	{
		MarkerWidget->SetMarkerStyle(FLinearColor(1.0f, 0.05f, 0.05f, 1.0f), 1.6f);
	}
	else if (bRanged)
	{
		MarkerWidget->SetMarkerStyle(FLinearColor(0.65f, 0.25f, 1.0f, 1.0f), 1.15f);
	}
	else
	{
		MarkerWidget->SetMarkerStyle(FLinearColor(1.0f, 0.25f, 0.1f, 1.0f), 1.0f);
	}

	MapPointsOverlay->AddChild(MarkerWidget);
}

void UTXMiniMapWidget::RemoveMapPoint_Implementation(AActor* Actor)
{
	UOverlay* MapPointsOverlay = FindMapPointsOverlay();
	if (!Actor || !MapPointsOverlay)
	{
		return;
	}

	for (int32 ChildIndex = MapPointsOverlay->GetChildrenCount() - 1; ChildIndex >= 0; --ChildIndex)
	{
		UWidget* ChildWidget = MapPointsOverlay->GetChildAt(ChildIndex);
		if (!ChildWidget)
		{
			continue;
		}

		FObjectPropertyBase* ActorProperty = FindFProperty<FObjectPropertyBase>(ChildWidget->GetClass(), TEXT("Actor"));
		if (!ActorProperty)
		{
			continue;
		}

		UObject* ReferencedObject = ActorProperty->GetObjectPropertyValue_InContainer(ChildWidget);
		if (ReferencedObject == Actor)
		{
			MapPointsOverlay->RemoveChildAt(ChildIndex);
		}
	}
}

void UTXMiniMapWidget::SetMiniMapImage()
{
	if (MiniMapImage && MiniMapImage->GetBrush().GetResourceObject() != nullptr)
	{
		return;
	}

	APlayerController* PlayerController = GetOwningPlayer();
	if (!PlayerController) return;

	APawn* PlayerPawn = PlayerController->GetPawn();
	if (!PlayerPawn) return;

	if (!MiniMapComponent)
		MiniMapComponent = PlayerPawn->FindComponentByClass<UPXMiniMapComponent>();

	if (!MiniMapComponent || !MiniMapComponent->SceneCaptureComponent)
		return;


	if (MiniMapCanvas)
	{
		CameraLoc = MiniMapComponent->SceneCaptureComponent->GetComponentLocation();
		OrthoWidth = MiniMapComponent->SceneCaptureComponent->OrthoWidth;
		MiniMapSize = MiniMapCanvas->GetDesiredSize().X;
		WorldUnitsPerPixel = MiniMapSize / OrthoWidth;
	}

	if (MiniMapComponent && MiniMapComponent->RenderTarget)
	{
		if (MiniMapImage)
		{
			FSlateBrush NewBrush;
			NewBrush.SetResourceObject(MiniMapComponent->RenderTarget);
			NewBrush.DrawAs = ESlateBrushDrawType::Image;

			MiniMapImage->SetBrush(NewBrush);
			UE_LOG(LogTemp, Log, TEXT("Brush Set"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("MiniMapImage Widget x."));
		}
	}
}

void UTXMiniMapWidget::SetPanelRotate()
{
	if (!GetOwningPlayerPawn())
		return;


	// ACharacter* Character = UGameplayStatics::GetPlayerCharacter(this, 0);
	APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!IsValid(Pawn))
		return;

	FVector Loc = Pawn->GetActorLocation();

	UKismetMaterialLibrary::SetScalarParameterValue(this, MPC_MiniMap, TEXT("X"), Loc.X);
	UKismetMaterialLibrary::SetScalarParameterValue(this, MPC_MiniMap, TEXT("Y"), Loc.Y);
}

UOverlay* UTXMiniMapWidget::FindMapPointsOverlay() const
{
	// 디버깅 메세지 출력


	if (!WidgetTree)
	{
		return nullptr;
	}

	if (UOverlay* Overlay = WidgetTree->FindWidget<UOverlay>(TEXT("MapPoints")))
	{
		return Overlay;
	}

	return WidgetTree->FindWidget<UOverlay>(TEXT("Map Points"));
}
