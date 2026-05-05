// Copyright GhostLazy


#include "Game/ElysiaGameState.h"

#include "Net/UnrealNetwork.h"

void AElysiaGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AElysiaGameState, CurrentRunPhase);
	DOREPLIFETIME(AElysiaGameState, CurrentBossRound);
	DOREPLIFETIME(AElysiaGameState, NormalPhaseElapsedSeconds);
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

void AElysiaGameState::SetNormalPhaseElapsedSeconds(int32 InElapsedSeconds)
{
	NormalPhaseElapsedSeconds = FMath::Max(0, InElapsedSeconds);
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
}

void AElysiaGameState::SetRunFinished(bool bInRunFinished)
{
	bRunFinished = bInRunFinished;
}
