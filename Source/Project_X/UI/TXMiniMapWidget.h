// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TXMiniMapWidget.generated.h"

class UImage;
class UCanvasPanel;
class UOverlay;
class APlayerController;
class UTextureRenderTarget2D;
class UTXEnemyMarkerWidget;
class UPXLockOnComponent;
class UPXMiniMapComponent;
class UMaterialParameterCollection;
class UOverlay;

UCLASS()
class PROJECT_X_API UTXMiniMapWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION(BlueprintNativeEvent)
	void AddMapPoint(AActor* Actor);

	UFUNCTION(BlueprintNativeEvent)
	void RemoveMapPoint(AActor* Actor);

private:
	void SetMiniMapImage();
	void SetPanelRotate();

public:
	bool IsReady = false;
	UPROPERTY(meta = (BindWidget))
	UImage* MiniMapImage;

	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* MiniMapCanvas;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UTXEnemyMarkerWidget> MarkerWidgetClass;

	UPROPERTY(EditAnywhere, Category = "UI")
	UMaterialParameterCollection* MPC_MiniMap;

private:
	void SetMiniMapImage(UTextureRenderTarget2D* RenderTarget);
	UOverlay* FindMapPointsOverlay() const;

private:

	UPROPERTY()
	UPXMiniMapComponent* MiniMapComponent;

	FVector CameraLoc;
	float WorldUnitsPerPixel;
	float MiniMapSize = 32.f;
	float OrthoWidth = 0.f;
};
