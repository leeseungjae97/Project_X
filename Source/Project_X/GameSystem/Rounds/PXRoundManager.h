#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/TimerHandle.h"
#include "GameFramework/Actor.h"
#include "GameSystem/Augments/Data/PXStatAugmentData.h"
#include "PXRoundManager.generated.h"

class ACombatEnemy;
class APXHealthPickup;
class UDataTable;

UENUM(BlueprintType)
enum class EPXRoundState : uint8
{
	Waiting,
	Spawning,
	InProgress,
	ChoosingAugment,
	Cleared,
	Failed
};

USTRUCT(BlueprintType)
struct FPXEnemyWaveEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Round")
	TSubclassOf<ACombatEnemy> EnemyClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Round", meta = (ClampMin = 0))
	int32 Count = 0;
};

USTRUCT(BlueprintType)
struct FPXRoundData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Round", meta = (ClampMin = 1))
	int32 RoundIndex = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Round")
	TArray<FPXEnemyWaveEntry> Enemies;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Round", meta = (ClampMin = 1))
	int32 MaxAliveEnemies = 10;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Round", meta = (ClampMin = 0))
	int32 HealthPickupCount = 0;
};

USTRUCT(BlueprintType)
struct FPXPrototypeRoundTableRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Round", meta = (ClampMin = 1))
	int32 RoundIndex = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Round", meta = (ClampMin = 1))
	int32 MaxAliveEnemies = 10;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Round", meta = (ClampMin = 0))
	int32 NormalCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Round", meta = (ClampMin = 0))
	int32 FastCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Round", meta = (ClampMin = 0))
	int32 TankCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Round", meta = (ClampMin = 0))
	int32 RangedCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Round", meta = (ClampMin = 0))
	int32 BossCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Round", meta = (ClampMin = 0))
	int32 HealthPickupCount = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPXRoundStarted, int32, RoundIndex, int32, TotalEnemies);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FPXRoundProgressChanged, int32, RoundIndex, int32, RemainingEnemies, int32, AliveEnemies);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPXRoundEnded, int32, RoundIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPXAugmentChoiceRequested, int32, CompletedRoundIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPXBossSpawned, ACombatEnemy*, BossEnemy);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPXRunCleared);

UCLASS()
class PROJECT_X_API APXRoundManager : public AActor
{
	GENERATED_BODY()

public:
	APXRoundManager();

protected:
	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	UFUNCTION(BlueprintCallable, Category = "Round")
	void StartRun();

	UFUNCTION(BlueprintCallable, Category = "Round")
	void StartRound(int32 RoundArrayIndex);

	UFUNCTION(BlueprintCallable, Category = "Round")
	void ContinueAfterAugmentChoice();

	UFUNCTION(BlueprintCallable, Category = "Round")
	void NotifyAugmentSelected(FName AugmentID);

	UFUNCTION(BlueprintPure, Category = "Round")
	TArray<FPXStatAugmentData> GetCurrentAugmentChoices() const;

	UFUNCTION(BlueprintPure, Category = "Round")
	int32 GetCurrentRoundNumber() const;

	UFUNCTION(BlueprintPure, Category = "Round")
	int32 GetRemainingEnemies() const;

	UFUNCTION(BlueprintPure, Category = "Round")
	EPXRoundState GetRoundState() const;

private:
	UFUNCTION()
	void HandleEnemyDied();

	void BuildPendingEnemyList(const FPXRoundData& RoundData);
	void SpawnNextEnemy();
	void ScheduleNextSpawn();
	void FinishRound();
	void RequestAugmentChoice();
	void GenerateAugmentChoices();
	void BuildPrototypeRoundsIfNeeded();
	bool BuildPrototypeRoundsFromDataTable(TSubclassOf<ACombatEnemy> NormalClass, TSubclassOf<ACombatEnemy> FastClass, TSubclassOf<ACombatEnemy> TankClass, TSubclassOf<ACombatEnemy> RangedClass, TSubclassOf<ACombatEnemy> BossClass);
	void CollectTaggedSpawnPointsIfNeeded();
	void SpawnRoundHealthPickups(int32 PickupCount);
	void AddPrototypeRound(int32 RoundIndex, int32 MaxAlive, int32 HealthPickupCount, TSubclassOf<ACombatEnemy> NormalClass, int32 NormalCount, TSubclassOf<ACombatEnemy> FastClass, int32 FastCount, TSubclassOf<ACombatEnemy> TankClass, int32 TankCount, TSubclassOf<ACombatEnemy> RangedClass, int32 RangedCount, TSubclassOf<ACombatEnemy> BossClass, int32 BossCount);
	void AddDefaultAugmentChoices();
	bool ApplyAugmentToPlayer(FName AugmentID);
	void SyncEnemyCounts();
	int32 CountLivingEnemies() const;
	void ClearSpawnTimer();
	FTransform GetSpawnTransform() const;
	void BroadcastProgress();

public:
	UPROPERTY(BlueprintAssignable, Category = "Round")
	FPXRoundStarted OnRoundStarted;

	UPROPERTY(BlueprintAssignable, Category = "Round")
	FPXRoundProgressChanged OnRoundProgressChanged;

	UPROPERTY(BlueprintAssignable, Category = "Round")
	FPXRoundEnded OnRoundEnded;

	UPROPERTY(BlueprintAssignable, Category = "Round")
	FPXAugmentChoiceRequested OnAugmentChoiceRequested;

	UPROPERTY(BlueprintAssignable, Category = "Round")
	FPXBossSpawned OnBossSpawned;

	UPROPERTY(BlueprintAssignable, Category = "Round")
	FPXRunCleared OnRunCleared;

private:
	UPROPERTY(EditAnywhere, Category = "Round")
	bool bStartOnBeginPlay = true;

	UPROPERTY(EditAnywhere, Category = "Round")
	bool bUsePrototypeRoundsIfEmpty = true;

	UPROPERTY(EditAnywhere, Category = "Round", meta = (ClampMin = 0))
	float InitialStartDelay = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Round", meta = (ClampMin = 0))
	float SpawnInterval = 0.2f;

	UPROPERTY(EditAnywhere, Category = "Round")
	TArray<FPXRoundData> Rounds;

	UPROPERTY(EditAnywhere, Category = "Round")
	UDataTable* PrototypeRoundTable = nullptr;

	UPROPERTY(EditAnywhere, Category = "Round")
	TArray<AActor*> SpawnPoints;

	UPROPERTY(EditAnywhere, Category = "Round")
	FName SpawnPointTag = TEXT("EnemySpawn");

	UPROPERTY(EditAnywhere, Category = "Augment")
	UDataTable* StatAugmentTable = nullptr;

	UPROPERTY(EditAnywhere, Category = "Augment", meta = (ClampMin = 1, ClampMax = 5))
	int32 AugmentChoiceCount = 3;

	UPROPERTY(EditAnywhere, Category = "Reward")
	TSubclassOf<APXHealthPickup> HealthPickupClass;

	UPROPERTY(EditAnywhere, Category = "Reward", meta = (ClampMin = 0))
	float HealthPickupSpawnRadius = 250.0f;

	UPROPERTY()
	TArray<TSubclassOf<ACombatEnemy>> PendingEnemyClasses;

	UPROPERTY()
	TArray<FPXStatAugmentData> CurrentAugmentChoices;

	FTimerHandle SpawnTimer;
	FTimerHandle InitialStartTimer;

	int32 CurrentRoundArrayIndex = INDEX_NONE;
	int32 CurrentRoundNumber = 0;
	int32 RemainingEnemies = 0;
	int32 AliveEnemies = 0;
	int32 MaxAliveEnemies = 10;

	EPXRoundState RoundState = EPXRoundState::Waiting;
};
