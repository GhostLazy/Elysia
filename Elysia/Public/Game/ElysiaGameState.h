// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "Game/ElysiaGameTypes.h"
#include "GameFramework/GameStateBase.h"
#include "ElysiaGameState.generated.h"

class AElysiaEnemy;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnGameStateChange, int32);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnCurrentBossChangedSignature, AElysiaEnemy*);

UCLASS()
class ELYSIA_API AElysiaGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	EElysiaRunPhase GetCurrentRunPhase() const { return CurrentRunPhase; }
	int32 GetCurrentBossRound() const { return CurrentBossRound; }
	int32 GetNormalPhaseElapsedSeconds() const { return NormalPhaseElapsedSeconds; }
	int32 GetNormalPhaseTotalSeconds() const { return NormalPhaseTotalSeconds; }
	int32 GetCurrentBossElapsedSeconds() const { return CurrentBossElapsedSeconds; }
	int32 GetNormalPhaseTotalDuration() const { return NormalPhaseTotalDuration; }
	int32 GetNormalScore() const { return NormalScore; }
	int32 GetBossScore() const { return BossScore; }
	int32 GetTotalScore() const { return TotalScore; }
	bool IsRunFinished() const { return bRunFinished; }
	AElysiaEnemy* GetCurrentBoss() const { return CurrentBoss.Get(); }

	void SetCurrentRunPhase(EElysiaRunPhase InPhase);
	void SetCurrentBossRound(int32 InBossRound);
	void SetCurrentBoss(AElysiaEnemy* InCurrentBoss);
	void SetNormalPhaseElapsedSeconds(int32 InElapsedSeconds);
	void SetNormalPhaseTotalSeconds(int32 InTotalSeconds);
	void SetNormalPhaseTotalDuration(int32 InTotalDuration);
	void SetCurrentBossElapsedSeconds(int32 InElapsedSeconds);
	void SetScores(int32 InNormalScore, int32 InBossScore, int32 InTotalScore);
	void SetRunFinished(bool bInRunFinished);
	
	FOnGameStateChange OnTotalScoreChanged;
	FOnGameStateChange OnNormalPhaseTotalSecondsChanged;
	FOnCurrentBossChangedSignature OnCurrentBossChanged;

protected:
	
	// 当前游戏阶段
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Run")
	EElysiaRunPhase CurrentRunPhase = EElysiaRunPhase::Normal;

	// 当前Boss轮次
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Run")
	int32 CurrentBossRound = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentBoss, Category = "Run")
	TObjectPtr<AElysiaEnemy> CurrentBoss;

	// 当前小兵阶段计时
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Run")
	int32 NormalPhaseElapsedSeconds = 0;
	
	// 小兵阶段总计时
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_NormalPhaseTotalSeconds, Category = "Run")
	int32 NormalPhaseTotalSeconds = 0;
	
	// 小兵阶段总时长
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Run")
	int32 NormalPhaseTotalDuration = 0;

	// 当前Boss阶段计时
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Run")
	int32 CurrentBossElapsedSeconds = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Score")
	int32 NormalScore = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Score")
	int32 BossScore = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_TotalScore, Category = "Score")
	int32 TotalScore = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Run")
	bool bRunFinished = false;
	
private:
	
	UFUNCTION()
	void OnRep_TotalScore() const { OnTotalScoreChanged.Broadcast(TotalScore); }
	
	UFUNCTION()
	void OnRep_NormalPhaseTotalSeconds() const { OnNormalPhaseTotalSecondsChanged.Broadcast(NormalPhaseTotalSeconds); }

	UFUNCTION()
	void OnRep_CurrentBoss() const { OnCurrentBossChanged.Broadcast(CurrentBoss.Get()); }
	
};
