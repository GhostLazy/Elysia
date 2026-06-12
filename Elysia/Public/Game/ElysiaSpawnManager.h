// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ElysiaSpawnManager.generated.h"

class AElysiaEnemy;
class AElysiaTreasureChest;

USTRUCT(BlueprintType)
struct FElysiaSpawnEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	TSubclassOf<AElysiaEnemy> EnemyClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn", meta = (ClampMin = "0.0"))
	float Weight = 1.f;
};

UCLASS()
class ELYSIA_API AElysiaSpawnManager : public AActor
{
	GENERATED_BODY()

public:

	AElysiaSpawnManager();
	void StartNormalSpawn();
	void StopNormalSpawn();
	void StartEliteSpawn();
	void StopEliteSpawn();
	void StartTreasureChestSpawn();
	void StopTreasureChestSpawn();
	bool IsNormalSpawnActive() const { return bNormalSpawnEnabled; }
	void SetNormalEnemyLevel(int32 InLevel);
	void SetNormalPhaseIndex(int32 InPhaseIndex);
	AElysiaEnemy* SpawnSpecialEnemy(TSubclassOf<AElysiaEnemy> EnemyClass, int32 EnemyLevel = 1);

protected:

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	TArray<FElysiaSpawnEntry> SpawnPool;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	TArray<FElysiaSpawnEntry> EliteSpawnPool;

	UPROPERTY(EditAnywhere, Category = "Spawn", meta = (ClampMin = "1"))
	int32 MaxAliveMinions = 80.f;

	UPROPERTY(EditAnywhere, Category = "Spawn", meta = (ClampMin = "1"))
	int32 SpawnBatchSize = 3;

	UPROPERTY(EditAnywhere, Category = "Spawn", meta = (ClampMin = "0.1"))
	float SpawnInterval = 1.f;

	UPROPERTY(EditAnywhere, Category = "Spawn", meta = (ClampMin = "0.1"))
	float EliteSpawnInterval = 45.f;

	// 近似当前屏幕可见区域的一半尺寸，刷怪点会落在其外侧
	UPROPERTY(EditAnywhere, Category = "Spawn")
	FVector2D VisibleHalfExtent = FVector2D(1800.f, 1000.f);

	// 在可见区域外再向四周扩出的缓冲带厚度
	UPROPERTY(EditAnywhere, Category = "Spawn", meta = (ClampMin = "0.0"))
	float SpawnBandThickness = 900.f;

	// 避免刷得离玩家太近
	UPROPERTY(EditAnywhere, Category = "Spawn", meta = (ClampMin = "0.0"))
	float PlayerSafeRadius = 700.f;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	FVector NavProjectExtent = FVector(200.f, 200.f, 600.f);

	UPROPERTY(EditAnywhere, Category = "Spawn", meta = (ClampMin = "1"))
	int32 MaxSpawnAttemptsPerTick = 16;

	UPROPERTY(EditAnywhere, Category = "Spawn|Soft Separation")
	bool bEnableSoftSeparation = true;

	UPROPERTY(EditAnywhere, Category = "Spawn|Soft Separation", meta = (ClampMin = "0.05"))
	float SoftSeparationInterval = 0.25f;

	UPROPERTY(EditAnywhere, Category = "Spawn|Soft Separation", meta = (ClampMin = "1.0"))
	float SoftSeparationRadius = 180.f;

	UPROPERTY(EditAnywhere, Category = "Spawn|Soft Separation", meta = (ClampMin = "0.0"))
	float SoftSeparationMaxOffset = 140.f;

	UPROPERTY(EditAnywhere, Category = "Spawn|Soft Separation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SoftSeparationBlendAlpha = 0.35f;

	UPROPERTY(EditAnywhere, Category = "Spawn|Boss", meta = (ClampMin = "1"))
	int32 BossMaxSpawnAttempts = 24;

	UPROPERTY(EditAnywhere, Category = "Spawn|Boss", meta = (ClampMin = "0.0"))
	float BossGroundTraceUpDistance = 2000.f;

	UPROPERTY(EditAnywhere, Category = "Spawn|Boss", meta = (ClampMin = "0.0"))
	float BossGroundTraceDownDistance = 5000.f;

	UPROPERTY(EditAnywhere, Category = "Spawn|Boss", meta = (ClampMin = "0.0"))
	float BossGroundClearance = 10.f;

	UPROPERTY(EditAnywhere, Category = "Spawn|Boss", meta = (ClampMin = "0.0"))
	float BossPostSpawnGroundCheckDistance = 300.f;

	UPROPERTY(EditAnywhere, Category = "Spawn|Treasure Chest")
	TSubclassOf<AElysiaTreasureChest> TreasureChestClass;

	UPROPERTY(EditAnywhere, Category = "Spawn|Treasure Chest", meta = (ClampMin = "0.1"))
	float TreasureChestSpawnInterval = 5.f;

	UPROPERTY(EditAnywhere, Category = "Spawn|Treasure Chest", meta = (ClampMin = "0.0"))
	float TreasureChestRespawnDelay = 8.f;

	UPROPERTY(EditAnywhere, Category = "Spawn|Treasure Chest", meta = (ClampMin = "0.0"))
	float TreasureChestMinSpawnDistance = 1600.f;

	UPROPERTY(EditAnywhere, Category = "Spawn|Treasure Chest", meta = (ClampMin = "0.0"))
	float TreasureChestMaxSpawnDistance = 2600.f;

	UPROPERTY(EditAnywhere, Category = "Spawn|Treasure Chest")
	FVector TreasureChestNavProjectExtent = FVector(300.f, 300.f, 800.f);

	UPROPERTY(EditAnywhere, Category = "Spawn|Treasure Chest", meta = (ClampMin = "1"))
	int32 TreasureChestMaxSpawnAttempts = 16;

	UPROPERTY(EditAnywhere, Category = "Spawn|Treasure Chest", meta = (ClampMin = "0.0"))
	float TreasureChestSpawnClearanceRadius = 160.f;

private:

	void HandleSpawnTick();
	void HandleEliteSpawnTick();
	void HandleTreasureChestSpawnTick();
	void HandleSoftSeparationTick();
	int32 CountAliveMinions() const;
	APawn* FindSpawnTargetPlayer() const;
	TSubclassOf<AElysiaEnemy> ChooseEnemyClassToSpawn() const;
	TSubclassOf<AElysiaEnemy> ChooseEliteClassToSpawn() const;
	bool TryFindSpawnLocation(const FVector& PlayerLocation, FVector& OutSpawnLocation) const;
	bool IsSpawnLocationAvailable(const FVector& SpawnLocation, const AActor* PlayerActor) const;
	bool TryFindGroundedBossSpawnLocation(const FVector& PlayerLocation, TSubclassOf<AElysiaEnemy> EnemyClass, FVector& OutSpawnLocation) const;
	bool IsBossSpawnLocationClear(const FVector& SpawnLocation, float CapsuleRadius, float CapsuleHalfHeight, const AActor* PlayerActor) const;
	FVector GenerateSpawnOffsetInBand() const;
	AElysiaEnemy* SpawnEnemyOfClass(TSubclassOf<AElysiaEnemy> EnemyClass);
	AElysiaTreasureChest* FindExistingTreasureChest();
	AElysiaTreasureChest* SpawnTreasureChest(const FVector& SpawnLocation);
	bool TryFindTreasureChestSpawnLocation(const FVector& PlayerLocation, FVector& OutSpawnLocation) const;
	bool IsTreasureChestSpawnLocationClear(const FVector& SpawnLocation, const AActor* PlayerActor) const;
	FVector GenerateTreasureChestSpawnOffset() const;
	void HandleTreasureChestOpened(AElysiaTreasureChest* OpenedChest);

	FTimerHandle SpawnTimerHandle;
	FTimerHandle EliteSpawnTimerHandle;
	FTimerHandle TreasureChestSpawnTimerHandle;
	FTimerHandle SoftSeparationTimerHandle;
	TWeakObjectPtr<AElysiaTreasureChest> ActiveTreasureChest;
	bool bNormalSpawnEnabled = false;
	bool bTreasureChestSpawnEnabled = false;
	int32 NormalEnemyLevel = 1;
	int32 NormalPhaseIndex = 1;
	float NextTreasureChestSpawnTime = 0.f;
};
