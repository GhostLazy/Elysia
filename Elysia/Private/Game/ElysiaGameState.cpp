// Copyright GhostLazy


#include "Game/ElysiaGameState.h"
#include "Character/ElysiaEnemy.h"
#include "Net/UnrealNetwork.h"

void AElysiaGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AElysiaGameState, CurrentRunPhase);
	DOREPLIFETIME(AElysiaGameState, CurrentBossRound);
	DOREPLIFETIME_CONDITION_NOTIFY(AElysiaGameState, CurrentBoss, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME(AElysiaGameState, NormalPhaseElapsedSeconds);
	DOREPLIFETIME_CONDITION_NOTIFY(AElysiaGameState, NormalPhaseTotalSeconds, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(AElysiaGameState, NormalPhaseTotalDuration, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME(AElysiaGameState, CurrentBossElapsedSeconds);
	DOREPLIFETIME(AElysiaGameState, NormalScore);
	DOREPLIFETIME(AElysiaGameState, BossScore);
	DOREPLIFETIME(AElysiaGameState, TotalScore);
	DOREPLIFETIME(AElysiaGameState, bRunFinished);
}

void AElysiaGameState::SetCurrentRunPhase(EElysiaRunPhase InPhase)
{
	CurrentRunPhase = InPhase;
}

void AElysiaGameState::SetCurrentBossRound(int32 InBossRound)
{
	CurrentBossRound = FMath::Max(0, InBossRound);
}

void AElysiaGameState::SetCurrentBoss(AElysiaEnemy* InCurrentBoss)
{
	CurrentBoss = InCurrentBoss;
	OnCurrentBossChanged.Broadcast(CurrentBoss.Get());
	ForceNetUpdate();
}

void AElysiaGameState::SetNormalPhaseElapsedSeconds(int32 InElapsedSeconds)
{
	NormalPhaseElapsedSeconds = FMath::Max(0, InElapsedSeconds);
}

void AElysiaGameState::SetNormalPhaseTotalSeconds(int32 InTotalSeconds)
{
	NormalPhaseTotalSeconds = FMath::Max(0, InTotalSeconds);
	OnNormalPhaseTotalSecondsChanged.Broadcast(NormalPhaseTotalSeconds);
}

void AElysiaGameState::SetNormalPhaseTotalDuration(int32 InTotalDuration)
{
	NormalPhaseTotalDuration = FMath::Max(0, InTotalDuration);
	OnNormalPhaseTotalSecondsChanged.Broadcast(NormalPhaseTotalSeconds);
	ForceNetUpdate();
}

void AElysiaGameState::SetCurrentBossElapsedSeconds(int32 InElapsedSeconds)
{
	CurrentBossElapsedSeconds = FMath::Max(0, InElapsedSeconds);
}

void AElysiaGameState::SetScores(int32 InNormalScore, int32 InBossScore, int32 InTotalScore)
{
	NormalScore = FMath::Max(0, InNormalScore);
	BossScore = FMath::Max(0, InBossScore);
	TotalScore = FMath::Max(0, InTotalScore);
	
	OnTotalScoreChanged.Broadcast(TotalScore);
}

void AElysiaGameState::SetRunFinished(bool bInRunFinished)
{
	bRunFinished = bInRunFinished;
}
