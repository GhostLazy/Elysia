// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "Game/ElysiaGameTypes.h"
#include "GameFramework/GameModeBase.h"
#include "ElysiaGameModeBase.generated.h"

class AElysiaEnemy;
class AElysiaGameState;
class AElysiaSpawnManager;

/**
 * 
 */
UCLASS()
class ELYSIA_API AElysiaGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	
	AElysiaGameModeBase();

protected:

	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "Run", meta = (ClampMin = "1.0"))
	float NormalPhaseDuration = 120.f;

	UPROPERTY(EditDefaultsOnly, Category = "Run", meta = (ClampMin = "1"))
	int32 TotalBossRounds = 4;

	UPROPERTY(EditDefaultsOnly, Category = "Score", meta = (ClampMin = "0"))
	int32 NormalScorePerSecond = 10;

	UPROPERTY(EditDefaultsOnly, Category = "Score", meta = (ClampMin = "0"))
	int32 BossBaseScore = 1000;

	UPROPERTY(EditDefaultsOnly, Category = "Score", meta = (ClampMin = "0.0"))
	float BossScorePenaltyPerSecond = 10.f;

	UPROPERTY(EditDefaultsOnly, Category = "Boss")
	TArray<TSubclassOf<AElysiaEnemy>> BossRoundClasses;

	UPROPERTY(EditDefaultsOnly, Category = "Spawn")
	TSubclassOf<AElysiaSpawnManager> SpawnManagerClass;

private:
	void StartRun();
	void AdvanceRunOneSecond();
	void EnterBossBattle(int32 BossRound);
	void EnterFinalBossBattle(int32 BossRound);
	void ResumeNormalPhase();
	void HandleTrackedBossDied(AElysiaEnemy* DeadEnemy);
	void FinishRun();
	void UpdateGameStateSnapshot() const;
	int32 CalculateBossRoundScore(float BossDuration) const;
	TSubclassOf<AElysiaEnemy> GetBossClassForRound(int32 BossRound) const;
	AElysiaGameState* GetElysiaGameState() const;

	UPROPERTY()
	TObjectPtr<AElysiaSpawnManager> SpawnManager;

	TWeakObjectPtr<AElysiaEnemy> CurrentBoss;
	FTimerHandle RunLoopTimer;
	EElysiaRunPhase CurrentRunPhase = EElysiaRunPhase::Normal;
	int32 TriggeredBossRounds = 0;
	int32 NormalPhaseElapsedSeconds = 0;
	float CurrentBossStartTime = 0.f;
	int32 NormalScore = 0;
	int32 BossScore = 0;
	int32 TotalScore = 0;
};
