#include "TXComponents/TXCollisionComponent.h"

#include "CombatCharacter.h"
#include "Damageable.h"
#include "Project_X.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/ShapeComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

UTXCollisionComponent::UTXCollisionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UTXCollisionComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
                                          FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	for (auto& [Actor, Elapsed] : ActorsElapsed)
	{
		Elapsed = FMath::Clamp(Elapsed + DeltaTime, 0, AttackCycleTime);
	}
}

void UTXCollisionComponent::BeginPlay()
{
	Super::BeginPlay();

	
}

void UTXCollisionComponent::BindOnBeginOverlap(UObject* UserObject, FName FunctionName)
{
	FScriptDelegate OnBeginOverlapDelegate;
	OnBeginOverlapDelegate.BindUFunction(UserObject, FunctionName);

	if (ShapeComponent)
	{
		ShapeComponent->OnComponentBeginOverlap.Add(OnBeginOverlapDelegate);
	}
}

void UTXCollisionComponent::BindOnEndOverlap(UObject* UserObject, FName FunctionName)
{
	FScriptDelegate OnEndOverlapDelegate;
	OnEndOverlapDelegate.BindUFunction(UserObject, FunctionName);

	if (ShapeComponent)
	{
		ShapeComponent->OnComponentEndOverlap.Add(OnEndOverlapDelegate);
	}
}

void UTXCollisionComponent::HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ATXWeapon* Weapon = Cast<ATXWeapon>(GetOwner());
	if (!Weapon)
		return;

	if (!Weapon->GetOwnerCharacter() || OtherActor == Weapon->GetOwnerCharacter())
		return;

	if (ACombatCharacter* Character = Cast<ACombatCharacter>(Weapon->GetOwnerCharacter()))
	{
		if (!Character->IsAttacking())
			return;
	}

	IDamageable* Damageable = Cast<IDamageable>(OtherActor);
	if (!Damageable)
		return;

	if (!ActorsHitCount.Contains(OtherActor))
	{
		ActorsHitCount.Add(OtherActor, 0);
		ActorsElapsed.Add(OtherActor, 0.0f);
	}

	if (ActorsElapsed[OtherActor] >= AttackCycleTime)
	{
		++ActorsHitCount[OtherActor];
		ActorsElapsed[OtherActor] = 0.0f;
	}
	
	if (ActorsHitCount[OtherActor] >= MaxHitCount)
	{
		return;
	}


	// UGameInstance* gi = UGameplayStatics::GetGameInstance(GetWorld());
	//
	// FTransform Trans = GetSocketTransform(FName("collision"));
	//
	// Protocol::C_USE_SKILL pkt;
	// pkt.set_skill_id(1);
	//
	// pkt.set_target_yaw(Trans.Rotator().Yaw);
	//
	// pkt.mutable_target_position()->set_x(Trans.GetLocation().X);
	// pkt.mutable_target_position()->set_y(Trans.GetLocation().Y);
	// pkt.mutable_target_position()->set_z(Trans.GetLocation().Z);
	//
	// SendBufferRef sendBuffer{ ServerPacketHandler::MakeSendBuffer(pkt) };
	//
	// gi->GetSubsystem<UGameNetworkSubsystem>()->SendPacket(sendBuffer);
}

void UTXCollisionComponent::HandleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// ActorsHitCount.Empty();
}

void UTXCollisionComponent::HandleBeginAttack()
{
	
}

void UTXCollisionComponent::InitCollisionComponent(const FCollisionInfo InCollisionInfo)
{
	CollisionInfo = InCollisionInfo;

	// 기존에 ShapeComponent가 있었다면 제거
	if (ShapeComponent)
	{
		ShapeComponent->DestroyComponent();
		ShapeComponent->UnregisterComponent();

		ShapeComponent = nullptr;
	}

	AActor* Owner = GetOwner();
	if (!Owner) return;

	switch (CollisionInfo.ShapeType)
	{
	case ETXCollisionShapeType::Box:
		{
			UBoxComponent* NewBox = NewObject<UBoxComponent>(Owner, TEXT("CollisionBox"));
			NewBox->SetBoxExtent(CollisionInfo.BoxExtent);

			ShapeComponent = NewBox;
		}
		break;
	case ETXCollisionShapeType::Sphere:
		{
			USphereComponent* NewSphere = NewObject<USphereComponent>(Owner, TEXT("CollisionSphere"));
			NewSphere->SetSphereRadius(CollisionInfo.SphereRadius);
			ShapeComponent = NewSphere;
		}
		break;
	case ETXCollisionShapeType::Capsule:
		{
			UCapsuleComponent* NewCapsule = NewObject<UCapsuleComponent>(Owner, TEXT("CollisionCapsule"));
			NewCapsule->SetCapsuleSize(CollisionInfo.CapsuleRadius, CollisionInfo.CapsuleHalfH);
			ShapeComponent = NewCapsule;
		}
		break;
	}

	if (ShapeComponent)
	{
		ShapeComponent->OnComponentBeginOverlap.AddDynamic(this, &UTXCollisionComponent::HandleBeginOverlap);
		ShapeComponent->OnComponentEndOverlap.AddDynamic(this, &UTXCollisionComponent::HandleEndOverlap);

		ShapeComponent->IgnoreActorWhenMoving(Owner, true);

		ShapeComponent->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
		ShapeComponent->RegisterComponent();

		ShapeComponent->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);

		ShapeComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

		ShapeComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

		ShapeComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	}
}
