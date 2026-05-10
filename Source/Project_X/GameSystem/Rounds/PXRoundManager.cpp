#include "GameSystem/Rounds/PXRoundManager.h"

#include "AI/CombatEnemy.h"
#include "Components/SceneComponent.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Pickups/PXHealthPickup.h"
#include "PXComponents/PXCombatComponent.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

APXRoundManager::APXRoundManager()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.5f;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	static ConstructorHelpers::FObjectFinder<UDataTable> PrototypeRoundTableFinder(TEXT("/Game/Data/DT_Rounds.DT_Rounds"));
	if (PrototypeRoundTableFinder.Succeeded())
	{
		PrototypeRoundTable = PrototypeRoundTableFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UDataTable> StatAugmentTableFinder(TEXT("/Game/Data/DT_StatAugments.DT_StatAugments"));
	if (StatAugmentTableFinder.Succeeded())
	{
		StatAugmentTable = StatAugmentTableFinder.Object;
	}
}

void APXRoundManager::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (HasAuthority() && (RoundState == EPXRoundState::Spawning || RoundState == EPXRoundState::InProgress))
	{
		SyncEnemyCounts();
	}
}

void APXRoundManager::BeginPlay()
{
	Super::BeginPlay();

	BuildPrototypeRoundsIfNeeded();
	CollectTaggedSpawnPointsIfNeeded();

	if (bStartOnBeginPlay && HasAuthority())
	{
		GetWorldTimerManager().SetTimer(InitialStartTimer, this, &APXRoundManager::StartRun, InitialStartDelay, false);
	}
}

void APXRoundManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearSpawnTimer();
	GetWorldTimerManager().ClearTimer(InitialStartTimer);

	Super::EndPlay(EndPlayReason);
}

void APXRoundManager::StartRun()
{
	if (!HasAuthority())
	{
		return;
	}

	BuildPrototypeRoundsIfNeeded();

	if (Rounds.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("PXRoundManager has no round data."));
		return;
	}

	StartRound(0);
}

void APXRoundManager::StartRound(int32 RoundArrayIndex)
{
	if (!Rounds.IsValidIndex(RoundArrayIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid round index: %d"), RoundArrayIndex);
		return;
	}

	ClearSpawnTimer();

	const FPXRoundData& RoundData = Rounds[RoundArrayIndex];
	CurrentRoundArrayIndex = RoundArrayIndex;
	CurrentRoundNumber = RoundData.RoundIndex;
	MaxAliveEnemies = FMath::Max(1, RoundData.MaxAliveEnemies);
	AliveEnemies = 0;
	RoundState = EPXRoundState::Spawning;

	BuildPendingEnemyList(RoundData);
	RemainingEnemies = PendingEnemyClasses.Num();
	SpawnRoundHealthPickups(RoundData.HealthPickupCount);

	OnRoundStarted.Broadcast(CurrentRoundNumber, RemainingEnemies);
	BroadcastProgress();
	SpawnNextEnemy();
}

void APXRoundManager::ContinueAfterAugmentChoice()
{
	if (RoundState != EPXRoundState::ChoosingAugment)
	{
		return;
	}

	StartRound(CurrentRoundArrayIndex + 1);
}

void APXRoundManager::NotifyAugmentSelected(FName AugmentID)
{
	if (RoundState != EPXRoundState::ChoosingAugment)
	{
		return;
	}

	ApplyAugmentToPlayer(AugmentID);
	ContinueAfterAugmentChoice();
}

TArray<FPXStatAugmentData> APXRoundManager::GetCurrentAugmentChoices() const
{
	return CurrentAugmentChoices;
}

int32 APXRoundManager::GetCurrentRoundNumber() const
{
	return CurrentRoundNumber;
}

int32 APXRoundManager::GetRemainingEnemies() const
{
	return RemainingEnemies;
}

EPXRoundState APXRoundManager::GetRoundState() const
{
	return RoundState;
}

void APXRoundManager::HandleEnemyDied()
{
	AliveEnemies = FMath::Max(0, AliveEnemies - 1);
	RemainingEnemies = FMath::Max(0, RemainingEnemies - 1);

	BroadcastProgress();

	if (RemainingEnemies <= 0 && AliveEnemies <= 0)
	{
		FinishRound();
		return;
	}

	if (!PendingEnemyClasses.IsEmpty())
	{
		ScheduleNextSpawn();
	}
}

void APXRoundManager::BuildPendingEnemyList(const FPXRoundData& RoundData)
{
	PendingEnemyClasses.Reset();

	for (const FPXEnemyWaveEntry& Entry : RoundData.Enemies)
	{
		if (!Entry.EnemyClass || Entry.Count <= 0)
		{
			continue;
		}

		for (int32 CountIndex = 0; CountIndex < Entry.Count; ++CountIndex)
		{
			PendingEnemyClasses.Add(Entry.EnemyClass);
		}
	}
}

void APXRoundManager::SpawnNextEnemy()
{
	ClearSpawnTimer();

	if (RoundState != EPXRoundState::Spawning && RoundState != EPXRoundState::InProgress)
	{
		return;
	}

	if (RemainingEnemies <= 0 && AliveEnemies <= 0)
	{
		FinishRound();
		return;
	}

	if (PendingEnemyClasses.IsEmpty() || AliveEnemies >= MaxAliveEnemies)
	{
		RoundState = EPXRoundState::InProgress;
		return;
	}

	TSubclassOf<ACombatEnemy> EnemyClass = PendingEnemyClasses[0];
	PendingEnemyClasses.RemoveAt(0);

	if (!EnemyClass)
	{
		ScheduleNextSpawn();
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ACombatEnemy* SpawnedEnemy = GetWorld()->SpawnActor<ACombatEnemy>(EnemyClass, GetSpawnTransform(), SpawnParams);
	if (SpawnedEnemy)
	{
		++AliveEnemies;
		SpawnedEnemy->OnEnemyDied.AddDynamic(this, &APXRoundManager::HandleEnemyDied);
		if (SpawnedEnemy->IsBossEnemy())
		{
			OnBossSpawned.Broadcast(SpawnedEnemy);
		}
		BroadcastProgress();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to spawn enemy class: %s"), *GetNameSafe(EnemyClass));
		RemainingEnemies = FMath::Max(0, RemainingEnemies - 1);
		BroadcastProgress();
	}

	if (RemainingEnemies <= 0 && AliveEnemies <= 0)
	{
		FinishRound();
		return;
	}

	if (!PendingEnemyClasses.IsEmpty() && AliveEnemies < MaxAliveEnemies)
	{
		ScheduleNextSpawn();
		return;
	}

	RoundState = EPXRoundState::InProgress;
}

void APXRoundManager::ScheduleNextSpawn()
{
	if (GetWorldTimerManager().IsTimerActive(SpawnTimer))
	{
		return;
	}

	RoundState = EPXRoundState::Spawning;

	if (SpawnInterval <= 0.0f)
	{
		SpawnNextEnemy();
		return;
	}

	GetWorldTimerManager().SetTimer(SpawnTimer, this, &APXRoundManager::SpawnNextEnemy, SpawnInterval, false);
}

void APXRoundManager::FinishRound()
{
	ClearSpawnTimer();

	OnRoundEnded.Broadcast(CurrentRoundNumber);

	if (CurrentRoundArrayIndex >= Rounds.Num() - 1)
	{
		RoundState = EPXRoundState::Cleared;
		OnRunCleared.Broadcast();
		return;
	}

	RequestAugmentChoice();
}

void APXRoundManager::RequestAugmentChoice()
{
	RoundState = EPXRoundState::ChoosingAugment;
	GenerateAugmentChoices();
	OnAugmentChoiceRequested.Broadcast(CurrentRoundNumber);
}

void APXRoundManager::GenerateAugmentChoices()
{
	CurrentAugmentChoices.Reset();

	if (!StatAugmentTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("PXRoundManager has no StatAugmentTable."));
		AddDefaultAugmentChoices();
		return;
	}

	TArray<FPXStatAugmentData*> AllRows;
	StatAugmentTable->GetAllRows<FPXStatAugmentData>(TEXT("PXRoundManager::GenerateAugmentChoices"), AllRows);

	while (!AllRows.IsEmpty() && CurrentAugmentChoices.Num() < AugmentChoiceCount)
	{
		const int32 RowIndex = FMath::RandRange(0, AllRows.Num() - 1);
		if (AllRows[RowIndex])
		{
			CurrentAugmentChoices.Add(*AllRows[RowIndex]);
		}
		AllRows.RemoveAtSwap(RowIndex);
	}

	if (CurrentAugmentChoices.IsEmpty())
	{
		AddDefaultAugmentChoices();
	}
}

void APXRoundManager::BuildPrototypeRoundsIfNeeded()
{
	if (!bUsePrototypeRoundsIfEmpty || !Rounds.IsEmpty())
	{
		return;
	}

	TSubclassOf<ACombatEnemy> NormalClass = LoadClass<ACombatEnemy>(nullptr, TEXT("/Game/Blueprints/AI/BP_CombatEnemyNormal.BP_CombatEnemyNormal_C"));
	TSubclassOf<ACombatEnemy> FastClass = LoadClass<ACombatEnemy>(nullptr, TEXT("/Game/Blueprints/AI/BP_CombatEnemyFast.BP_CombatEnemyFast_C"));
	TSubclassOf<ACombatEnemy> TankClass = LoadClass<ACombatEnemy>(nullptr, TEXT("/Game/Blueprints/AI/BP_CombatEnemyNormalTank.BP_CombatEnemyNormalTank_C"));
	TSubclassOf<ACombatEnemy> RangedClass = LoadClass<ACombatEnemy>(nullptr, TEXT("/Game/Blueprints/AI/BP_CombatEnemyNormalRange.BP_CombatEnemyNormalRange_C"));
	TSubclassOf<ACombatEnemy> BossClass = LoadClass<ACombatEnemy>(nullptr, TEXT("/Game/Blueprints/AI/BP_CombatEnemyNormalBoss.BP_CombatEnemyNormalBoss_C"));

	if (!NormalClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("PXRoundManager could not load prototype enemy classes."));
		return;
	}

	if (BuildPrototypeRoundsFromDataTable(NormalClass, FastClass, TankClass, RangedClass, BossClass))
	{
		return;
	}

	AddPrototypeRound(1, 10, 1, NormalClass, 1, FastClass, 1, TankClass, 1, RangedClass, 1, BossClass, 0);
	AddPrototypeRound(2, 10, 1, NormalClass, 1, FastClass, 1, TankClass, 1, RangedClass, 1, BossClass, 0);
	AddPrototypeRound(3, 10, 2, NormalClass, 1, FastClass, 1, TankClass, 1, RangedClass, 1, BossClass, 0);
	AddPrototypeRound(4, 10, 2, NormalClass, 1, FastClass, 1, TankClass, 1, RangedClass, 1, BossClass, 0);
	AddPrototypeRound(5, 10, 3, NormalClass, 1, FastClass, 1, TankClass, 1, RangedClass, 1, BossClass, 1);
}

bool APXRoundManager::BuildPrototypeRoundsFromDataTable(TSubclassOf<ACombatEnemy> NormalClass, TSubclassOf<ACombatEnemy> FastClass, TSubclassOf<ACombatEnemy> TankClass, TSubclassOf<ACombatEnemy> RangedClass, TSubclassOf<ACombatEnemy> BossClass)
{
	if (!PrototypeRoundTable)
	{
		return false;
	}

	TArray<FPXPrototypeRoundTableRow*> Rows;
	PrototypeRoundTable->GetAllRows<FPXPrototypeRoundTableRow>(TEXT("PXRoundManager::BuildPrototypeRoundsFromDataTable"), Rows);
	if (Rows.IsEmpty())
	{
		return false;
	}

	Rows.Sort([](const FPXPrototypeRoundTableRow& Left, const FPXPrototypeRoundTableRow& Right)
	{
		return Left.RoundIndex < Right.RoundIndex;
	});

	for (const FPXPrototypeRoundTableRow* Row : Rows)
	{
		if (!Row)
		{
			continue;
		}

		AddPrototypeRound(
			Row->RoundIndex,
			Row->MaxAliveEnemies,
			Row->HealthPickupCount,
			NormalClass,
			Row->NormalCount,
			FastClass,
			Row->FastCount,
			TankClass,
			Row->TankCount,
			RangedClass,
			Row->RangedCount,
			BossClass,
			Row->BossCount);
	}

	return !Rounds.IsEmpty();
}

void APXRoundManager::CollectTaggedSpawnPointsIfNeeded()
{
	if (!SpawnPoints.IsEmpty() || SpawnPointTag.IsNone())
	{
		return;
	}

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsWithTag(this, SpawnPointTag, FoundActors);
	SpawnPoints = FoundActors;
}

void APXRoundManager::SpawnRoundHealthPickups(int32 PickupCount)
{
	if (!HasAuthority() || PickupCount <= 0)
	{
		return;
	}

	TSubclassOf<APXHealthPickup> PickupClass = HealthPickupClass;
	if (!PickupClass)
	{
		PickupClass = LoadClass<APXHealthPickup>(nullptr, TEXT("/Game/Blueprints/Pickups/BP_HealthPickup.BP_HealthPickup_C"));
	}
	if (!PickupClass)
	{
		PickupClass = APXHealthPickup::StaticClass();
	}

	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(this, 0);
	const FVector CenterLocation = PlayerCharacter ? PlayerCharacter->GetActorLocation() : GetActorLocation();

	for (int32 PickupIndex = 0; PickupIndex < PickupCount; ++PickupIndex)
	{
		const FVector2D RandomOffset = FMath::RandPointInCircle(HealthPickupSpawnRadius);
		const FVector SpawnLocation = CenterLocation + FVector(RandomOffset.X, RandomOffset.Y, 80.0f);
		const FRotator SpawnRotation = FRotator::ZeroRotator;

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		GetWorld()->SpawnActor<APXHealthPickup>(PickupClass, SpawnLocation, SpawnRotation, SpawnParams);
	}
}

void APXRoundManager::AddPrototypeRound(int32 RoundIndex, int32 MaxAlive, int32 HealthPickupCount, TSubclassOf<ACombatEnemy> NormalClass, int32 NormalCount, TSubclassOf<ACombatEnemy> FastClass, int32 FastCount, TSubclassOf<ACombatEnemy> TankClass, int32 TankCount, TSubclassOf<ACombatEnemy> RangedClass, int32 RangedCount, TSubclassOf<ACombatEnemy> BossClass, int32 BossCount)
{
	FPXRoundData RoundData;
	RoundData.RoundIndex = RoundIndex;
	RoundData.MaxAliveEnemies = MaxAlive;
	RoundData.HealthPickupCount = HealthPickupCount;

	auto AddEntry = [&RoundData](TSubclassOf<ACombatEnemy> EnemyClass, int32 Count)
	{
		if (!EnemyClass || Count <= 0)
		{
			return;
		}

		FPXEnemyWaveEntry Entry;
		Entry.EnemyClass = EnemyClass;
		Entry.Count = Count;
		RoundData.Enemies.Add(Entry);
	};

	AddEntry(NormalClass, NormalCount);
	AddEntry(FastClass, FastCount);
	AddEntry(TankClass, TankCount);
	AddEntry(RangedClass, RangedCount);
	AddEntry(BossClass, BossCount);

	Rounds.Add(RoundData);
}

void APXRoundManager::AddDefaultAugmentChoices()
{
	TArray<FPXStatAugmentData> DefaultChoices;

	auto AddChoice = [&DefaultChoices](FName AugmentID, const TCHAR* Name, const TCHAR* Description, EPXAugmentRarity Rarity, EPXPlayerStatType StatType, float AddValue, float MultiplyValue)
	{
		FPXStatAugmentData Choice;
		Choice.AugmentID = AugmentID;
		Choice.Name = FText::FromString(Name);
		Choice.Description = FText::FromString(Description);
		Choice.Rarity = Rarity;
		Choice.StatType = StatType;
		Choice.AddValue = AddValue;
		Choice.MultiplyValue = MultiplyValue;
		DefaultChoices.Add(Choice);
	};

	AddChoice(TEXT("Damage_Common"), TEXT("Sharpened Edge"), TEXT("Attack damage +1"), EPXAugmentRarity::Common, EPXPlayerStatType::AttackDamage, 1.0f, 1.0f);
	AddChoice(TEXT("MoveSpeed_Common"), TEXT("Light Step"), TEXT("Move speed +80"), EPXAugmentRarity::Common, EPXPlayerStatType::MoveSpeed, 80.0f, 1.0f);
	AddChoice(TEXT("MaxHealth_Common"), TEXT("Hard Skin"), TEXT("Max health +2"), EPXAugmentRarity::Common, EPXPlayerStatType::MaxHealth, 2.0f, 1.0f);
	AddChoice(TEXT("AttackSpeed_Common"), TEXT("Quick Hands"), TEXT("Attack speed +15%"), EPXAugmentRarity::Common, EPXPlayerStatType::AttackSpeed, 0.0f, 1.15f);
	AddChoice(TEXT("AttackRange_Common"), TEXT("Long Reach"), TEXT("Attack range +40"), EPXAugmentRarity::Common, EPXPlayerStatType::AttackRange, 40.0f, 1.0f);
	AddChoice(TEXT("MaxStamina_Common"), TEXT("Deep Breath"), TEXT("Max stamina +20"), EPXAugmentRarity::Common, EPXPlayerStatType::MaxStamina, 20.0f, 1.0f);
	AddChoice(TEXT("HealthRegen_Rare"), TEXT("Blood Warmth"), TEXT("Health regen +0.25/s"), EPXAugmentRarity::Rare, EPXPlayerStatType::HealthRegen, 0.25f, 1.0f);
	AddChoice(TEXT("StaminaRegen_Rare"), TEXT("Second Wind"), TEXT("Stamina regen +8"), EPXAugmentRarity::Rare, EPXPlayerStatType::StaminaRegen, 8.0f, 1.0f);
	AddChoice(TEXT("Damage_Epic"), TEXT("Executioner"), TEXT("Attack damage +30%"), EPXAugmentRarity::Epic, EPXPlayerStatType::AttackDamage, 0.0f, 1.3f);
	AddChoice(TEXT("MoveSpeed_Legendary"), TEXT("Storm Step"), TEXT("Move speed +25%"), EPXAugmentRarity::Legendary, EPXPlayerStatType::MoveSpeed, 0.0f, 1.25f);

	while (!DefaultChoices.IsEmpty() && CurrentAugmentChoices.Num() < AugmentChoiceCount)
	{
		const int32 ChoiceIndex = FMath::RandRange(0, DefaultChoices.Num() - 1);
		CurrentAugmentChoices.Add(DefaultChoices[ChoiceIndex]);
		DefaultChoices.RemoveAtSwap(ChoiceIndex);
	}
}

bool APXRoundManager::ApplyAugmentToPlayer(FName AugmentID)
{
	const FPXStatAugmentData* SelectedAugment = CurrentAugmentChoices.FindByPredicate(
		[AugmentID](const FPXStatAugmentData& Candidate)
		{
			return Candidate.AugmentID == AugmentID;
		});

	if (!SelectedAugment)
	{
		UE_LOG(LogTemp, Warning, TEXT("Selected augment is not in current choices: %s"), *AugmentID.ToString());
		return false;
	}

	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(this, 0);
	if (!PlayerCharacter)
	{
		return false;
	}

	UPXCombatComponent* CombatComponent = PlayerCharacter->FindComponentByClass<UPXCombatComponent>();
	if (!CombatComponent)
	{
		return false;
	}

	CombatComponent->ApplyStatModifier(SelectedAugment->StatType, SelectedAugment->AddValue, SelectedAugment->MultiplyValue);
	return true;
}

void APXRoundManager::SyncEnemyCounts()
{
	const int32 LivingEnemies = CountLivingEnemies();
	const int32 SyncedRemainingEnemies = PendingEnemyClasses.Num() + LivingEnemies;
	const bool bChanged = AliveEnemies != LivingEnemies || RemainingEnemies != SyncedRemainingEnemies;

	AliveEnemies = LivingEnemies;
	RemainingEnemies = SyncedRemainingEnemies;

	if (bChanged)
	{
		BroadcastProgress();
	}

	if (RemainingEnemies <= 0 && AliveEnemies <= 0)
	{
		FinishRound();
		return;
	}

	if (!PendingEnemyClasses.IsEmpty() && AliveEnemies < MaxAliveEnemies)
	{
		ScheduleNextSpawn();
	}
}

int32 APXRoundManager::CountLivingEnemies() const
{
	int32 LivingEnemies = 0;
	for (TActorIterator<ACombatEnemy> It(GetWorld()); It; ++It)
	{
		const ACombatEnemy* Enemy = *It;
		if (IsValid(Enemy) && Enemy->CurrentHP > 0.0f)
		{
			++LivingEnemies;
		}
	}
	return LivingEnemies;
}

void APXRoundManager::ClearSpawnTimer()
{
	GetWorldTimerManager().ClearTimer(SpawnTimer);
}

FTransform APXRoundManager::GetSpawnTransform() const
{
	if (!SpawnPoints.IsEmpty())
	{
		const int32 SpawnIndex = FMath::RandRange(0, SpawnPoints.Num() - 1);
		if (IsValid(SpawnPoints[SpawnIndex]))
		{
			return SpawnPoints[SpawnIndex]->GetActorTransform();
		}
	}

	return GetActorTransform();
}

void APXRoundManager::BroadcastProgress()
{
	OnRoundProgressChanged.Broadcast(CurrentRoundNumber, RemainingEnemies, AliveEnemies);
}
