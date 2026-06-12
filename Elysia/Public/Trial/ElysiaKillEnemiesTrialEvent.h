// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "Trial/ElysiaTrialEventBase.h"
#include "ElysiaKillEnemiesTrialEvent.generated.h"

class AElysiaEnemy;

/**
 * 限时击杀试炼。试炼怪物不生成自身死亡奖励，全部击败后由 TrialManager
 * 在试炼触发点统一生成完成奖励。
 */
UCLASS(Blueprintable)
class ELYSIA_API AElysiaKillEnemiesTrialEvent : public AElysiaTrialEventBase
{
	GENERATED_BODY()

public:

	virtual FVector GetCompletionRewardLocation() const override;

	UFUNCTION(BlueprintPure, Category = "Trial|Kill Enemies")
	int32 GetRemainingEnemyCount() const;

protected:

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void HandleTrialTriggered(AActor* TriggerActor) override;
	virtual void HandleTrialCompleted() override;
	virtual void HandleTrialExpired() override;
	virtual void HandleTrialCancelled() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trial|Kill Enemies")
	TArray<TSubclassOf<AElysiaEnemy>> TrialEnemyClasses;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trial|Kill Enemies", meta = (ClampMin = "1"))
	int32 EnemyLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trial|Kill Enemies", meta = (ClampMin = "0.0"))
	float EnemySpawnRadius = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trial|Kill Enemies", meta = (ClampMin = "1"))
	int32 MaxSpawnAttemptsPerEnemy = 8;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trial|Kill Enemies")
	FVector NavProjectionExtent = FVector(200.f, 200.f, 600.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trial|Kill Enemies|Ground", meta = (ClampMin = "0.0"))
	float GroundTraceUpDistance = 2000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trial|Kill Enemies|Ground", meta = (ClampMin = "0.0"))
	float GroundTraceDownDistance = 5000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trial|Kill Enemies|Ground", meta = (ClampMin = "0.0"))
	float GroundClearance = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trial|Kill Enemies|Ground", meta = (ClampMin = "0.0"))
	float PostSpawnGroundCheckDistance = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trial|Kill Enemies", meta = (ClampMin = "0.05"))
	float SpawnRetryInterval = 0.5f;

private:

	void SpawnTrialEnemies();
	bool SpawnSingleTrialEnemy(const TArray<TSubclassOf<AElysiaEnemy>>& ValidEnemyClasses);
	bool TryFindEnemySpawnLocation(float CapsuleHalfHeight, FVector& OutSpawnLocation) const;
	void ScheduleSpawnRetry();
	void ClearSpawnRetryTimer();
	void PruneInvalidTrialEnemies();
	void HandleTrialEnemyDied(AElysiaEnemy* DeadEnemy);

	UFUNCTION()
	void HandleTrialEnemyDestroyed(AActor* DestroyedActor);

	void UnregisterTrialEnemy(AElysiaEnemy* Enemy);
	void ClearTrialEnemies(bool bDestroyRemainingEnemies);

	static constexpr int32 RequiredEnemyKillCount = 2;

	FVector TrialTriggerLocation = FVector::ZeroVector;
	TSet<TWeakObjectPtr<AElysiaEnemy>> ActiveTrialEnemies;
	FTimerHandle SpawnRetryTimerHandle;
	int32 DefeatedEnemyCount = 0;
};
