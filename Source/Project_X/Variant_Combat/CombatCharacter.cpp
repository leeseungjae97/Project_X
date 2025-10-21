// Copyright Epic Games, Inc. All Rights Reserved.


#include "CombatCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "TXComponents/TXCombatComponent.h"
#include "TXComponents/TXLockOnComponent.h"
#include "TXComponents/TXMiniMapComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "CombatLifeBar.h"
#include "Engine/DamageEvents.h"
#include "TimerManager.h"
#include "Engine/LocalPlayer.h"
#include "CombatPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "TXAnimation/TXPlayerAnimInstance.h"
#include "UI/TXLockOnWidget.h"
#include "CombatGameMode.h"
#include "TXWeapons/TXWeapon.h"
#include "TXWeapons/TXMeleeWeapon.h"
#include "TXWeapons/TXHitScanWeapon.h"
#include "Player/Controller/TXPlayerController.h"
#include "TXComponents/TXCollisionComponent.h"

DEFINE_LOG_CATEGORY(LogCombatCharacter);

ACombatCharacter::ACombatCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// bind the attack montage ended delegate
	OnAttackMontageEnded.BindUObject(this, &ACombatCharacter::AttackMontageEnded);

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(35.0f, 90.0f);

	// Configure character movement
	GetCharacterMovement()->MaxWalkSpeed = 400.0f;

	// create the camera boom
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);

	CameraBoom->TargetArmLength = DefaultCameraDistance;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->bEnableCameraRotationLag = true;
	
	// create the orbiting camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// create the life bar widget component
	//LifeBar = CreateDefaultSubobject<UWidgetComponent>(TEXT("LifeBar"));
	//LifeBar->SetupAttachment(RootComponent);

	TXCombatComponent = CreateDefaultSubobject<UTXCombatComponent>(TEXT("TX Combat Component"));
	TXCombatComponent->SetMaxWalkSpeed(GetCharacterMovement()->MaxWalkSpeed);

	TXLockOnComponent = CreateDefaultSubobject<UTXLockOnComponent>(TEXT("TX LockOn Component"));

	TXMiniMapComponent = CreateDefaultSubobject<UTXMiniMapComponent>(TEXT("TX MiniMap Component"));

	// set the player tag
	Tags.Add(FName("Player"));
}

void ACombatCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 로컬 플레이어는 보간하지 않음 (안전한 방식)
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (PC->IsLocalController())
		{
			return;
		}
	}

	if (TargetLocation.IsNearlyZero()) return;

	int64 NowMs = FDateTime::UtcNow().ToUnixTimestamp() * 1000;
	FTimespan SinceEpoch = FDateTime::UtcNow() - FDateTime(1970, 1, 1);
	NowMs = static_cast<int64>(SinceEpoch.GetTotalMilliseconds());

	int64 TotalTimeMs = EstimatedArrivalTimeMs - TimeStampAtReceiveMs;
	int64 ElapsedMs = NowMs - TimeStampAtReceiveMs;
	if (ElapsedMs < 0) ElapsedMs = 0;

	float Alpha = TotalTimeMs > 0 ? FMath::Clamp(static_cast<float>(ElapsedMs) / static_cast<float>(TotalTimeMs), 0.f, 1.f) : 1.f;

	// 오프셋이 너무 크면 그냥 스냅
	if (TotalTimeMs > 5000)
	{
		SetActorLocation(TargetLocation);
		FRotator SnapRot = GetActorRotation();
		SnapRot.Yaw = TargetYaw;
		SetActorRotation(SnapRot);
		UE_LOG(LogCombatCharacter, Warning, TEXT("TotalTimeMS is too big"));
		return;
	}

	// 위치 보간
	FVector CurrentLoc = GetActorLocation();
	FVector NewLoc = FMath::Lerp(CurrentLoc, TargetLocation, Alpha);
	SetActorLocation(NewLoc);

	// 회전 보간
	FRotator CurrentRot = GetActorRotation();
	FRotator TargetRot = CurrentRot;
	TargetRot.Yaw = TargetYaw;
	FRotator NewRot = FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, InterpSpeed);
	SetActorRotation(NewRot);

	// 이동 속도 계산
	SimulatedVelocity = (GetActorLocation() - LastFrameLocation) / DeltaTime;
	LastFrameLocation = GetActorLocation();
	// 가속도 계산
	SimulatedAcceleration = (SimulatedVelocity - LastSimulatedVelocity) / DeltaTime;
	LastSimulatedVelocity = SimulatedVelocity;
}

void ACombatCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void ACombatCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void ACombatCharacter::StartDash() const
{
	if (TXCombatComponent)
	{
		GetCharacterMovement()->MaxWalkSpeed = TXCombatComponent->GetMaxWalkSpeed() * 1.5f;
		TXCombatComponent->SetIsRunning(true);
		TXCombatComponent->StopStaminaRecovery();
	}
}

void ACombatCharacter::StopDash() const
{
	if (TXCombatComponent)
	{
		GetCharacterMovement()->MaxWalkSpeed = TXCombatComponent->GetMaxWalkSpeed();
		TXCombatComponent->SetIsRunning(false);
		TXCombatComponent->StartStaminaRecovery();
	}
}

void ACombatCharacter::DoDodge()
{
	if (TXPlayerAnimInstance)
	{
		// 방향 계산
		TXCombatComponent->ConsumeStamina(20.f);
		TXCombatComponent->StopStaminaRecovery();
		FVector InputDir = GetLastMovementInputVector();
		if (InputDir.IsNearlyZero())
		{
			InputDir = GetActorForwardVector(); // 정지 상태면 전방
		}
		InputDir.Z = 0;
		InputDir.Normalize();

		// 캐릭터를 InputDir 방향으로 회전
		FRotator DesiredRotation = InputDir.Rotation();
		bUseControllerRotationYaw = false;
		SetActorRotation(DesiredRotation);

		FOnMontageEnded OnMontageEndedDelegate;
		OnMontageEndedDelegate.BindLambda([this](UAnimMontage* Montage, bool bInterrupted)
			{
				bUseControllerRotationYaw = true;
				TXCombatComponent->SetIsDodging(false);
				TXCombatComponent->StartStaminaRecovery();
			});
		TXPlayerAnimInstance->PlayDodgeAnimation(OnMontageEndedDelegate);
		TXCombatComponent->SetIsDodging(true);
	}
}

void ACombatCharacter::HandleInventory()
{
	if (nullptr == TXCombatComponent)
		return;
	if (TXCombatComponent->IsInventoryOpen())
	{
		CloseInventory();
	}
	else
	{
		OpenInventory();
	}

	TXCombatComponent->SetIsInventoryOpen(!TXCombatComponent->IsInventoryOpen());
}

void ACombatCharacter::OpenInventory()
{
}

void ACombatCharacter::CloseInventory()
{
}

void ACombatCharacter::Interaction()
{
	if (nullptr == TXCombatComponent)
		return;

	//TXCombatComponent->EquipWeapon(new AWeapon());
}

void ACombatCharacter::ESC()
{
}

void ACombatCharacter::Chat(const FInputActionValue& Value)
{
}

void ACombatCharacter::SkillE(const FInputActionValue& Value)
{
	if (nullptr == TXCombatComponent)
		return;
}

void ACombatCharacter::SkillQ(const FInputActionValue& Value)
{
	if (nullptr == TXCombatComponent)
		return;
}

void ACombatCharacter::LockOn()
{
	if (nullptr == TXLockOnComponent)
		return;

	GEngine->AddOnScreenDebugMessage(
		-1,
		5.f,
		FColor::Red,
		TEXT("LockOn")
	);
	TXLockOnComponent->LockOn();
}

void ACombatCharacter::LockOnChange()
{
	if (nullptr == TXLockOnComponent || !TXLockOnComponent->IsLockOn())
		return;

	TXLockOnComponent->PickDetectedRangeActors();
}

void ACombatCharacter::ComboAttackPressed()
{
	if (TXCombatComponent && TXCombatComponent->IsWeaponEquipped())
	{
		WeaponAttack();
	}
	else
	{
		// route the input
		DoComboAttackStart();
	}
}

void ACombatCharacter::ChargedAttackPressed()
{
	// route the input
	DoChargedAttackStart();
}

void ACombatCharacter::ChargedAttackReleased()
{
	// route the input
	DoChargedAttackEnd();
}

void ACombatCharacter::Use1ActionPressed()
{
	if (nullptr == TXCombatComponent)
		return;

	SpawnWeapon(HitscanWeaponClass);
}

void ACombatCharacter::Use2ActionPressed()
{
	if (nullptr == TXCombatComponent)
		return;

	SpawnWeapon(MeleeWeaponClass);
}

void ACombatCharacter::Use3ActionPressed()
{
	if (nullptr == TXCombatComponent)
		return;

}

void ACombatCharacter::Use4ActionPressed()
{
	if (nullptr == TXCombatComponent)
		return;

}

void ACombatCharacter::Use5ActionPressed()
{
	if (nullptr == TXCombatComponent)
		return;

}

bool ACombatCharacter::IsRunning()
{
	if (nullptr == TXCombatComponent)
		return false;

	return TXCombatComponent->IsRunning();
}

bool ACombatCharacter::IsDashing()
{
	if (nullptr == TXCombatComponent)
		return false;

	return TXCombatComponent->IsDodging();
}

bool ACombatCharacter::IsAttacking()
{
	if (nullptr == TXCombatComponent)
		return false;

	return TXCombatComponent->IsAttacking();
}

void ACombatCharacter::DoMove(float Right, float Forward)
{
	if (IsAttacking())
		return;
	
	if (GetController() != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void ACombatCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void ACombatCharacter::DoComboAttackStart()
{
	// are we already playing an attack animation?
	if (TXCombatComponent && TXCombatComponent->IsAttacking())
	{
		// cache the input time so we can check it later
		if (TXPlayerAnimInstance)
			TXPlayerAnimInstance->CachedAttackInputTime = GetWorld()->GetTimeSeconds();

		return;
	}

	// perform a combo attack
	ComboAttack();
}

void ACombatCharacter::DoComboAttackEnd()
{
	// stub
}

void ACombatCharacter::DoChargedAttackStart()
{
	if (nullptr == TXPlayerAnimInstance)
		return;

	// raise the charging attack flag
	TXPlayerAnimInstance->bIsChargingAttack = true;

	if (TXCombatComponent && TXCombatComponent->IsAttacking())
	{
		// cache the input time so we can check it later
		TXPlayerAnimInstance->CachedAttackInputTime = GetWorld()->GetTimeSeconds();

		return;
	}

	ChargedAttack();
}

void ACombatCharacter::DoChargedAttackEnd()
{
	if (nullptr == TXPlayerAnimInstance)
		return;

	// lower the charging attack flag
	TXPlayerAnimInstance->bIsChargingAttack = false;

	// if we've done the charge loop at least once, release the charged attack right away
	if (TXPlayerAnimInstance->bHasLoopedChargedAttack)
	{
		CheckChargedAttack();
	}
}

void ACombatCharacter::ResetHP()
{
	//if (nullptr == CombatComponent)
	//	return;

	TXCombatComponent->ResetHP();

	// update the life bar
	//LifeBarWidget->SetLifePercentage(1.0f);
}

void ACombatCharacter::ComboAttack()
{
	if (nullptr == TXCombatComponent || nullptr == TXPlayerAnimInstance)
		return;

	TXCombatComponent->SetIsAttacking(true);

	TXPlayerAnimInstance->ComboAttack(OnAttackMontageEnded, ComboAttackMontage);
}

void ACombatCharacter::ChargedAttack()
{
	if (nullptr == TXCombatComponent || nullptr == TXPlayerAnimInstance)
		return;
	TXCombatComponent->SetIsAttacking(true);

	TXPlayerAnimInstance->ChargedAttack(OnAttackMontageEnded, ChargedAttackMontage);
}

void ACombatCharacter::AttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (nullptr == TXCombatComponent || nullptr == TXPlayerAnimInstance)
		return;
	TXCombatComponent->SetIsAttacking(false);

	// check if we have a non-stale cached input
	if (TXPlayerAnimInstance->GetNonStaleCachedInput())
	{
		// are we holding the charged attack button?
		if (TXPlayerAnimInstance->bIsChargingAttack)
		{
			// do a charged attack
			ChargedAttack();
		}
		else
		{
			// do a regular attack
			ComboAttack();
		}
	}
}

void ACombatCharacter::SetLifePercentage(float Percent)
{
	//LifeBarWidget->SetLifePercentage(Percent);
}


//
// void ACombatCharacter::SwapWeapon()
// {
// 	if (nullptr == TXCombatComponent || nullptr == MeleeWeapon || nullptr == HitscanWeapon)
// 		return;
//
// 	if (TXCombatComponent->GetWeaponType() == MeleeWeapon->GetWeaponType())
// 	{
// 		TXCombatComponent->EquipWeapon(HitscanWeapon);
// 	}
// 	else
// 	{
// 		TXCombatComponent->EquipWeapon(MeleeWeapon);
// 	}
// }

void ACombatCharacter::WeaponAttack()
{
	if (nullptr == TXCombatComponent || nullptr == TXPlayerAnimInstance)
		return;

	// TXCombatComponent->SetIsAttacking(true);

	TXCombatComponent->WeaponAttack();

	PlayWeaponMontage();
}

void ACombatCharacter::FireWeapon()
{
	if (TXCombatComponent && TXCombatComponent->IsWeaponEquipped())
	{
		TXCombatComponent->Fire();
	}
}

void ACombatCharacter::PlayWeaponMontage()
{
	if (TXCombatComponent && TXCombatComponent->IsWeaponEquipped())
	{
		if (TXPlayerAnimInstance)
		{
			EWeaponType Type = TXCombatComponent->GetWeaponType();
			if (Type == EWeaponType::EWT_Melee)
				TXPlayerAnimInstance->WeaponAttack(OnAttackMontageEnded, MeleeAttackMontage);
			else
				TXPlayerAnimInstance->WeaponAttack(OnAttackMontageEnded, HitscanAttackMontage);
		}
	}
}

void ACombatCharacter::DoAttackTrace(FName DamageSourceBone)
{
	if (nullptr == TXCombatComponent)
		return;

	if (TXCombatComponent->IsWeaponEquipped())
	{
		return;
	}
	
	float MeleeTraceDistance = TXCombatComponent->GetMeleeTraceDistance();
	float MeleeKnockbackImpulse = TXCombatComponent->GetMeleeKnockbackImpulse();
	float MeleeLaunchImpulse = TXCombatComponent->GetMeleeLaunchImpulse();
	float MeleeDamage = TXCombatComponent->GetMeleeDamage();

	// sweep for objects in front of the character to be hit by the attack
	TArray<FHitResult> OutHits;

	// start at the provided socket location, sweep forward
	const FVector TraceStart = GetMesh()->GetSocketLocation(DamageSourceBone);
	const FVector TraceEnd = TraceStart + (GetActorForwardVector() * MeleeTraceDistance);

	const FRotator TraceRot = GetMesh()->GetSocketRotation(DamageSourceBone);
	
	// check for pawn and world dynamic collision object types
	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);
	ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);

	// use a sphere shape for the sweep
	FCollisionShape CollisionShape;
	CollisionShape.SetSphere(TXCombatComponent->GetMeleeTraceRadius());

	// ignore self
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	if (GetWorld()->SweepMultiByObjectType(OutHits, TraceStart, TraceEnd, FQuat::Identity, ObjectParams, CollisionShape, QueryParams))
	{
		// iterate over each object hit
		for (const FHitResult& CurrentHit : OutHits)
		{
			// check if we've hit a damageable actor
			IDamageable* Damageable = Cast<IDamageable>(CurrentHit.GetActor());

			if (Damageable)
			{
				// knock upwards and away from the impact normal
				const FVector Impulse = (CurrentHit.ImpactNormal * -MeleeKnockbackImpulse) + (FVector::UpVector * MeleeLaunchImpulse);

				// pass the damage event to the actor
				Damageable->ApplyDamage(MeleeDamage, this, CurrentHit.ImpactPoint, Impulse);

				// call the BP handler to play effects, etc.
				DealtDamage(MeleeDamage, CurrentHit.ImpactPoint);
			}
		}
	}
}

void ACombatCharacter::CheckCombo()
{
	if (nullptr == GetMesh())
		return;

	if (TXCombatComponent && TXPlayerAnimInstance)
		TXPlayerAnimInstance->CheckCombo(TXCombatComponent->IsAttacking(), ComboAttackMontage);
}

void ACombatCharacter::CheckChargedAttack()
{
	if (nullptr == GetMesh())
		return;

	if (TXPlayerAnimInstance)
		TXPlayerAnimInstance->CheckChargedAttack(ChargedAttackMontage);
}

void ACombatCharacter::ApplyDamage(float Damage, AActor* DamageCauser, const FVector& DamageLocation, const FVector& DamageImpulse)
{
	// pass the damage event to the actor
	FDamageEvent DamageEvent;
	const float ActualDamage = TakeDamage(Damage, DamageEvent, nullptr, DamageCauser);
	
	// only process knockback and effects if we received nonzero damage
	if (ActualDamage > 0.0f)
	{
		// pass control to BP to play effects, etc.
		ReceivedDamage(ActualDamage, DamageLocation, DamageImpulse.GetSafeNormal());
	}
}

void ACombatCharacter::HandleDeath()
{
	// disable movement while we're dead
	GetCharacterMovement()->DisableMovement();

	// enable full ragdoll physics
	GetMesh()->SetSimulatePhysics(true);

	// hide the life bar
	//LifeBar->SetHiddenInGame(true);

	// pull back the camera
	GetCameraBoom()->TargetArmLength = DeathCameraDistance;

	// schedule respawning
	GetWorld()->GetTimerManager().SetTimer(RespawnTimer, this, &ACombatCharacter::RespawnCharacter, RespawnTime, false);
}

void ACombatCharacter::ApplyHealing(float Healing, AActor* Healer)
{
	// stub
}

void ACombatCharacter::RespawnCharacter()
{
	// destroy the character and let it be respawned by the Player Controller
	Destroy();
}

float ACombatCharacter::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (nullptr == TXCombatComponent)
		return 0.0f;

	return TXCombatComponent->TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);;
}

void ACombatCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	if (nullptr == TXCombatComponent)
		return;

	TXCombatComponent->Landed(Hit);
}

void ACombatCharacter::BeginPlay()
{
	Super::BeginPlay();
	if (TXCombatComponent)
	{
		TXCombatComponent->OwnerCharacter = this;
	}
	if (TXLockOnComponent)
	{
		TXLockOnComponent->SetOwnerCharacter(this);
	}
	if (TXMiniMapComponent)
	{
		TXMiniMapComponent->SetOwnerCharacter(this);
	}
	
	if (ATXPlayerController* PlayerController = Cast<ATXPlayerController>(Controller)) 
	{
		FInputModeGameOnly InputMode;
		PlayerController->SetInputMode(InputMode);
		PlayerController->bShowMouseCursor = false;
	}

	TXPlayerAnimInstance = Cast<UTXPlayerAnimInstance>(GetMesh()->GetAnimInstance());

	// get the life bar from the widget component
	//LifeBarWidget = Cast<UCombatLifeBar>(LifeBar->GetUserWidgetObject());
	//check(LifeBarWidget);

	// initialize the camera
	GetCameraBoom()->TargetArmLength = DefaultCameraDistance;

	// set the life bar color
	//LifeBarWidget->SetBarColor(LifeBarColor);

	MeshStartingTransform = GetMesh()->GetRelativeTransform();

	// reset HP to maximum
	ResetHP();
}

void ACombatCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// clear the respawn timer
	GetWorld()->GetTimerManager().ClearTimer(RespawnTimer);
}

void ACombatCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACombatCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACombatCharacter::Look);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &ACombatCharacter::Look);

		// Combo Attack
		EnhancedInputComponent->BindAction(ComboAttackAction, ETriggerEvent::Started, this, &ACombatCharacter::ComboAttackPressed);

		// System ESC
		EnhancedInputComponent->BindAction(ESCAction, ETriggerEvent::Started, this, &ACombatCharacter::ESC);

		// System Chat
		EnhancedInputComponent->BindAction(ChatAction, ETriggerEvent::Started, this, &ACombatCharacter::Chat);

		// Combat Dash
		EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Started, this, &ACombatCharacter::OnDashStarted);
		EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Triggered, this, &ACombatCharacter::OnDashTriggered);
		EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Completed, this, &ACombatCharacter::OnDashCompleted);
		EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Canceled, this, &ACombatCharacter::OnDashCanceled);
		/*EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Canceled, this, &ACombatCharacter::DashOnTap);
		EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Triggered, this, &ACombatCharacter::DashOnHold);*/

		// Combat LockOn
		EnhancedInputComponent->BindAction(LockOnAction, ETriggerEvent::Completed, this, &ACombatCharacter::LockOn);
		EnhancedInputComponent->BindAction(LockChangeAction, ETriggerEvent::Completed, this, &ACombatCharacter::LockOnChange);

		// System Inventory
		EnhancedInputComponent->BindAction(InventoryAction, ETriggerEvent::Started, this, &ACombatCharacter::HandleInventory);

		// System Interaction
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &ACombatCharacter::Interaction);

		// Combat E
		EnhancedInputComponent->BindAction(SkillEAction, ETriggerEvent::Started, this, &ACombatCharacter::SkillE);

		// Combat Q
		EnhancedInputComponent->BindAction(SkillQAction, ETriggerEvent::Started, this, &ACombatCharacter::SkillQ);

		// Charged Attack
		EnhancedInputComponent->BindAction(ChargedAttackAction, ETriggerEvent::Started, this, &ACombatCharacter::ChargedAttackPressed);
		EnhancedInputComponent->BindAction(ChargedAttackAction, ETriggerEvent::Completed, this, &ACombatCharacter::ChargedAttackReleased);

		EnhancedInputComponent->BindAction(Use1Action, ETriggerEvent::Completed, this, &ACombatCharacter::Use1ActionPressed);
		EnhancedInputComponent->BindAction(Use2Action, ETriggerEvent::Completed, this, &ACombatCharacter::Use2ActionPressed);
		EnhancedInputComponent->BindAction(Use3Action, ETriggerEvent::Completed, this, &ACombatCharacter::Use3ActionPressed);
		EnhancedInputComponent->BindAction(Use4Action, ETriggerEvent::Completed, this, &ACombatCharacter::Use4ActionPressed);
		EnhancedInputComponent->BindAction(Use5Action, ETriggerEvent::Completed, this, &ACombatCharacter::Use5ActionPressed);
	}
}

void ACombatCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// update the respawn transform on the Player Controller
	if (ACombatPlayerController* PC = Cast<ACombatPlayerController>(GetController()))
	{
		PC->SetRespawnTransform(GetActorTransform());
	}
}

void ACombatCharacter::OnDashStarted(const FInputActionInstance& Instance)
{
	DashHoldStartTime = FPlatformTime::Seconds();
}

void ACombatCharacter::OnDashTriggered(const FInputActionInstance& Instance)
{
	StartDash(); // 0.2초 이상 누른 경우
}

void ACombatCharacter::OnDashCompleted(const FInputActionInstance& Instance)
{
	if (TXCombatComponent->IsRunning())
	{
		StopDash();
	}
}

void ACombatCharacter::OnDashCanceled(const FInputActionInstance& Instance)
{
	// 0.2초 되기 전에 뗀 경우
	const double HeldDuration = FPlatformTime::Seconds() - DashHoldStartTime;
	if (!TXCombatComponent->IsDodging() && HeldDuration < 0.2)
	{
		DoDodge();
	}
}

void ACombatCharacter::SetIsAttacking(bool bIsAttack)
{
	if (!TXCombatComponent)
		return;
	
	TXCombatComponent->SetIsAttacking(bIsAttack);
}

void ACombatCharacter::SpawnWeapon(TSubclassOf<ATXWeapon> SpawnWeaponClass)
{
	UWorld* World = GetWorld();
	if (World && SpawnWeaponClass)
	{
		if (EquippedWeapon)
		{
			EquippedWeapon->Destroy();
			EquippedWeapon = nullptr;
		}
		
		EquippedWeapon = World->SpawnActor<ATXWeapon>(SpawnWeaponClass);
		if (UTXCollisionComponent* cc = EquippedWeapon->CollisionComponent)
		{
			// TXCombatComponent->WeaponAttackStart.AddUFunction(EquippedWeapon, FName("WeaponAttackStart"));
			TXCombatComponent->WeaponAttackStart.AddUObject(EquippedWeapon, &ATXWeapon::AttackStart);
		}

		TXCombatComponent->WeaponEquipped;
		
		if (!EquippedWeapon)
			return;
		
		EquippedWeapon->SetOwnerCharacter(this);

		if (TXCombatComponent)
			TXCombatComponent->EquipWeapon(EquippedWeapon);
	}	
}

