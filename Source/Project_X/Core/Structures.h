#pragma once

#include "CoreMinimal.h"
#include "EnumTypes.h"
#include "Structures.generated.h"

USTRUCT(BlueprintType)
struct FCollisionInfo
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	EPXCollisionShapeType ShapeType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition ="ShapeType == EPXCollisionShapeType::Box"))
	FVector BoxExtent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition ="ShapeType == EPXCollisionShapeType::Sphere"))
	float SphereRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition ="ShapeType == EPXCollisionShapeType::Capsule"))
	float CapsuleHalfH;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition ="ShapeType == EPXCollisionShapeType::Capsule"))
	float CapsuleRadius;
	
};