#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TXMiniMapComponent.generated.h"

class ACombatCharacter;
class UTextureRenderTarget2D;
class USceneCaptureComponent2D;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_X_API UTXMiniMapComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UTXMiniMapComponent();

protected:
	virtual void BeginPlay() override;

	void MakeRenderTarget();

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY()
	ACombatCharacter* OwnerCharacter;

	bool bInit = false;

public:
	FORCEINLINE void SetOwnerCharacter(ACombatCharacter* InOwnerCharacter) { OwnerCharacter = InOwnerCharacter; }

public:
	// The Scene Capture Component used to capture the minimap view.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Minimap")
	USceneCaptureComponent2D* SceneCaptureComponent;

	// The Render Target to draw the minimap to.
	// If left empty, a new one will be created dynamically.
	UPROPERTY(VisibleAnywhere, Category = "Minimap")
	UTextureRenderTarget2D* RenderTarget;

	// Size of the dynamically created Render Target if one is not provided.
	UPROPERTY(EditDefaultsOnly, Category = "Minimap", meta = (EditCondition = "RenderTarget == nullptr"))
	int32 MinimapResolutionX = 512;

	UPROPERTY(EditDefaultsOnly, Category = "Minimap", meta = (EditCondition = "RenderTarget == nullptr"))
	int32 MinimapResolutionY = 512;

	UPROPERTY(EditDefaultsOnly, Category = "Minimap")
	float CameraHeight = 1000.0f;

	// The orthographic width of the camera, effectively the zoom level.
	UPROPERTY(EditDefaultsOnly, Category = "Minimap")
	float CameraOrthoWidth = 4096.0f;
};