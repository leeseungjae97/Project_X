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
	ETXCollisionShapeType ShapeType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition ="ShapeType == ETXCollisionShapeType::Box"))
	FVector BoxExtent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition ="ShapeType == ETXCollisionShapeType::Sphere"))
	float SphereRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition ="ShapeType == ETXCollisionShapeType::Capsule"))
	float CapsuleHalfH;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition ="ShapeType == ETXCollisionShapeType::Capsule"))
	float CapsuleRadius;
	
};