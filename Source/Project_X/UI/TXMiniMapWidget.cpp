// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/TXMiniMapWidget.h"
#include "GameFramework/PlayerController.h"
#include "TXComponents/TXMiniMapComponent.h"
#include "TXComponents/TXLockOnComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Texture2D.h"
#include "UI/TXEnemyMarkerWidget.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "CombatCharacter.h"
#include "Blueprint/WidgetLayoutLibrary.h"
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
		MiniMapComponent = PlayerPawn->FindComponentByClass<UTXMiniMapComponent>();

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

	GetOwningPlayerPawn();
	ACharacter* Character = UGameplayStatics::GetPlayerCharacter(this, 0);

	if (!Character)
		return;

	FVector Loc = Character->GetActorLocation();
	FRotator Rot = Character->GetActorRotation();

	UKismetMaterialLibrary::SetScalarParameterValue(this, MPC_MiniMap, TEXT("X"), Loc.X);
	UKismetMaterialLibrary::SetScalarParameterValue(this, MPC_MiniMap, TEXT("Y"), Loc.Y);

	Rot.Yaw *= -1.0;

	MiniMapCanvas->SetRenderTransformAngle(Rot.Yaw);
}