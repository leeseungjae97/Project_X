#pragma once

#include "CoreMinimal.h"
// #include "Components/ActorComponent.h"
#include "Components/SceneComponent.h"
#include "Core/Structures.h"
#include "TXCollisionComponent.generated.h"

class UPrimitiveComponent;

UCLASS()
class PROJECT_X_API UTXCollisionComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UTXCollisionComponent();
	
protected:
	// 실제 충돌 처리를 담당할 내부 Shape Component
	UPROPERTY()
	TObjectPtr<UShapeComponent> ShapeComponent;

	void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void BeginPlay() override;
	
public:
	UFUNCTION()
	void BindOnBeginOverlap(UObject* UserObject, FName FunctionName);

	UFUNCTION()
	void BindOnEndOverlap(UObject* UserObject, FName FunctionName);

	UFUNCTION()
	void HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION ()
	void HandleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) ;

	UFUNCTION()
	void HandleBeginAttack();
	
private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Info", meta = (AllowPrivateAccess = "true"))
	FCollisionInfo CollisionInfo;

	UPROPERTY()
	int64 MaxHitCount = 1;
	
private:
	TMap<UObject*, int64> ActorsHitCount;
	TMap<UObject*, float> ActorsElapsed;

	float AttackCycleTime = 1.f;
	
public:
	void InitCollisionComponent(const FCollisionInfo InCollisionInfo);
	
};
