// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TXEnemyMarkerWidget.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_X_API UTXEnemyMarkerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "MiniMap")
	void SetMarkerActor(AActor* InActor);

	UFUNCTION(BlueprintCallable, Category = "MiniMap")
	void SetMarkerStyle(const FLinearColor& InColor, float InScale);

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	void EnsureValidActorReference();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MiniMap")
	FLinearColor MarkerColor = FLinearColor::Red;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MiniMap")
	float MarkerScale = 1.0f;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniMap", meta = (ExposeOnSpawn = "true"))
	AActor* Actor = nullptr;
	
};
