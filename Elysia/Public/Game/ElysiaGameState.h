// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "Game/ElysiaGameTypes.h"
#include "GameFramework/GameStateBase.h"
#include "ElysiaGameState.generated.h"

UCLASS()
class ELYSIA_API AElysiaGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	EElysiaRunPhase GetCurrentRunPhase() const { return CurrentRunPhase; }
	int32 GetCurrentBossRound() const { return CurrentBossRound; }
	int32 GetNormalPhaseElapsedSeconds() const { return NormalPhaseElapsedSeconds; }
	int32 GetCurrentBossElapsedSeconds() const { return CurrentBossElapsedSeconds; }
	int32 GetNormalScore() const { return NormalScore; }
	int32 GetBossScore() const { return BossScore; }
	int32 GetTotalScore() const { return TotalScore; }
	bool IsRunFinished() const { return bRunFinished; }

	void SetCurrentRunPhase(EElysiaRunPhase InPhase);
	void SetCurrentBossRound(int32 InBossRound);
	void SetNormalPhaseElapsedSeconds(int32 InElapsedSeconds);
	void SetCurrentBossElapsedSeconds(int32 InElapsedSeconds);
	void SetScores(int32 InNormalScore, int32 InBossScore, int32 InTotalScore);
	void SetRunFinished(bool bInRunFinished);

protected:
	
	// 当前游戏阶段
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Run")
	EElysiaRunPhase CurrentRunPhase = EElysiaRunPhase::Normal;

	// 当前Boss轮次
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Run")
	int32 CurrentBossRound = 0;

	// 小兵阶段计时
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Run")
	int32 NormalPhaseElapsedSeconds = 0;

	// 当前Boss阶段计时
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Run")
	int32 CurrentBossElapsedSeconds = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Score")
	int32 NormalScore = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Score")
	int32 BossScore = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Score")
	int32 TotalScore = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Run")
	bool bRunFinished = false;
	
};
