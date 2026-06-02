// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ElysiaSpawnManager.generated.h"

class AElysiaEnemy;

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
	bool IsNormalSpawnActive() const { return bNormalSpawnEnabled; }
	void SetNormalEnemyLevel(int32 InLevel);
	AElysiaEnemy* SpawnSpecialEnemy(TSubclassOf<AElysiaEnemy> EnemyClass, int32 EnemyLevel = 1);

protected:

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	TArray<FElysiaSpawnEntry> SpawnPool;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	TArray<FElysiaSpawnEntry> EliteSpawnPool;

	UPROPERTY(EditAnywhere, Category = "Spawn", meta = (ClampMin = "1"))
	int32 MaxAliveMinions = 20;

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

private:

	void HandleSpawnTick();
	void HandleEliteSpawnTick();
	int32 CountAliveMinions() const;
	APawn* FindSpawnTargetPlayer() const;
	TSubclassOf<AElysiaEnemy> ChooseEnemyClassToSpawn() const;
	TSubclassOf<AElysiaEnemy> ChooseEliteClassToSpawn() const;
	bool TryFindSpawnLocation(const FVector& PlayerLocation, FVector& OutSpawnLocation) const;
	bool IsSpawnLocationAvailable(const FVector& SpawnLocation, const AActor* PlayerActor) const;
	bool TryFindGroundedBossSpawnLocation(const FVector& PlayerLocation, TSubclassOf<AElysiaEnemy> EnemyClass, FVector& OutSpawnLocation) const;
	bool TryProjectBossCandidateToGround(const FVector& CandidateLocation, float CapsuleHalfHeight, FVector& OutSpawnLocation) const;
	bool IsBossSpawnLocationClear(const FVector& SpawnLocation, float CapsuleRadius, float CapsuleHalfHeight, const AActor* PlayerActor) const;
	bool HasGroundBelowBoss(const FVector& BossLocation, float CapsuleHalfHeight, const AActor* IgnoredActor) const;
	bool GetEnemyCapsuleSize(TSubclassOf<AElysiaEnemy> EnemyClass, float& OutCapsuleRadius, float& OutCapsuleHalfHeight) const;
	FVector GenerateSpawnOffsetInBand() const;
	AElysiaEnemy* SpawnEnemyOfClass(TSubclassOf<AElysiaEnemy> EnemyClass);

	FTimerHandle SpawnTimerHandle;
	FTimerHandle EliteSpawnTimerHandle;
	bool bNormalSpawnEnabled = false;
	int32 NormalEnemyLevel = 1;
};
