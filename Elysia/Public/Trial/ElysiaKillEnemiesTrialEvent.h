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
	int32 GetRemainingEnemyCount() const { return ActiveTrialEnemies.Num(); }

protected:

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void HandleTrialTriggered(AActor* TriggerActor) override;
	virtual void HandleTrialCompleted() override;
	virtual void HandleTrialExpired() override;
	virtual void HandleTrialCancelled() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trial|Kill Enemies")
	TArray<TSubclassOf<AElysiaEnemy>> TrialEnemyClasses;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trial|Kill Enemies", meta = (ClampMin = "1"))
	int32 EnemyCount = 5;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trial|Kill Enemies", meta = (ClampMin = "1"))
	int32 EnemyLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trial|Kill Enemies", meta = (ClampMin = "0.0"))
	float EnemySpawnRadius = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trial|Kill Enemies", meta = (ClampMin = "1"))
	int32 MaxSpawnAttemptsPerEnemy = 8;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trial|Kill Enemies")
	FVector NavProjectionExtent = FVector(200.f, 200.f, 600.f);

private:

	void SpawnTrialEnemies();
	bool TryFindEnemySpawnLocation(FVector& OutSpawnLocation) const;
	void HandleTrialEnemyDied(AElysiaEnemy* DeadEnemy);
	void ClearTrialEnemies(bool bDestroyRemainingEnemies);

	FVector TrialTriggerLocation = FVector::ZeroVector;
	TSet<TWeakObjectPtr<AElysiaEnemy>> ActiveTrialEnemies;
};
