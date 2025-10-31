#include "PXComponents/PXMiniMapComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"

/**
 * 미니맵 Texture 생성 전 까지 임시로 사용하는 MiniMap Component
 * Scene Capture를 통해 이미지를 생성함.
 * 미니맵 Textrue가 생기면 MiniMapComponent 삭제
 * @deprecated
 */

/**@deprecated*/
UPXMiniMapComponent::UPXMiniMapComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	SceneCaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("MinimapSceneCapture"));
	SceneCaptureComponent->SetupAttachment(nullptr);
	SceneCaptureComponent->bCaptureEveryFrame = true;
}

/**@deprecated*/
void UPXMiniMapComponent::BeginPlay()
{
	Super::BeginPlay();

	MakeRenderTarget();
}

/**@deprecated*/
void UPXMiniMapComponent::MakeRenderTarget()
{
	AActor* Owner = GetOwner();
	if (!Owner || !SceneCaptureComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("UPXMiniMapComponent has no owner or SceneCaptureComponent is invalid!"));
		return;
	}
	USceneComponent* OwnerRoot = Owner->GetRootComponent();
	if (OwnerRoot)
	{
		SceneCaptureComponent->AttachToComponent(OwnerRoot, FAttachmentTransformRules::KeepRelativeTransform);
	}

	if (RenderTarget == nullptr)
	{
		RenderTarget = NewObject<UTextureRenderTarget2D>(this, TEXT("MinimapRenderTarget"));
		if (RenderTarget)
		{
			RenderTarget->InitCustomFormat(MinimapResolutionX, MinimapResolutionY, PF_B8G8R8A8, true);
			RenderTarget->UpdateResource();
			UE_LOG(LogTemp, Log, TEXT("Dynamically created RenderTarget for Minimap."));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to dynamically create RenderTarget for Minimap."));
			return;
		}
	}

	if (RenderTarget)
	{
		SceneCaptureComponent->TextureTarget = RenderTarget;
	}
	FEngineShowFlags& ShowFlags = SceneCaptureComponent->ShowFlags;
	ShowFlags.SetAtmosphere(true);
	ShowFlags.SetBSP(true);
	ShowFlags.SetFog(true);
	ShowFlags.SetLandscape(true);
	ShowFlags.SetSkeletalMeshes(false);
	ShowFlags.SetStaticMeshes(true);
	ShowFlags.SetLighting(false);
	ShowFlags.SetTranslucency(false);
	
	SceneCaptureComponent->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	SceneCaptureComponent->ProjectionType = ECameraProjectionMode::Orthographic;
	SceneCaptureComponent->OrthoWidth = CameraOrthoWidth;

	SceneCaptureComponent->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));
	SceneCaptureComponent->SetRelativeLocation(FVector(0.f, 0.f, CameraHeight));
}

/**@deprecated*/
void UPXMiniMapComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
