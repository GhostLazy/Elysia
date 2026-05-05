// Copyright GhostLazy


#include "Game/ElysiaGameModeBase.h"

#include "Character/ElysiaEnemy.h"
#include "Game/ElysiaGameState.h"
#include "Game/ElysiaSpawnManager.h"
#include "Kismet/GameplayStatics.h"

AElysiaGameModeBase::AElysiaGameModeBase()
{
	GameStateClass = AElysiaGameState::StaticClass();
}

void AElysiaGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority() || !SpawnManagerClass)
	{
		return;
	}

	if (AElysiaSpawnManager* ExistingSpawnManager = Cast<AElysiaSpawnManager>(UGameplayStatics::GetActorOfClass(this, SpawnManagerClass)))
	{
		SpawnManager = ExistingSpawnManager;
	}
	else
	{
		SpawnManager = GetWorld()->SpawnActor<AElysiaSpawnManager>(SpawnManagerClass, FVector::ZeroVector, FRotator::ZeroRotator);
	}

	StartRun();
}

void AElysiaGameModeBase::StartRun()
{
	CurrentRunPhase = EElysiaRunPhase::Normal;
	TriggeredBossRounds = 0;
	NormalPhaseElapsedSeconds = 0;
	CurrentBossStartTime = 0.f;
	NormalScore = 0;
	BossScore = 0;
	TotalScore = 0;
	CurrentBoss = nullptr;

	if (SpawnManager)
	{
		SpawnManager->StartNormalSpawn();
		SpawnManager->StartEliteSpawn();
	}

	UpdateGameStateSnapshot();
	GetWorldTimerManager().SetTimer(RunLoopTimer, this, &AElysiaGameModeBase::AdvanceRunOneSecond, 1.f, true);
}

void AElysiaGameModeBase::AdvanceRunOneSecond()
{
	switch (CurrentRunPhase)
	{
	case EElysiaRunPhase::Normal:
		++NormalPhaseElapsedSeconds;
		NormalScore += FMath::Max(0, NormalScorePerSecond);
		TotalScore = NormalScore + BossScore;

		if (NormalPhaseElapsedSeconds >= FMath::Max(1, FMath::RoundToInt(NormalPhaseDuration)))
		{
			NormalPhaseElapsedSeconds = 0;
			++TriggeredBossRounds;

			if (TriggeredBossRounds >= TotalBossRounds)
			{
				EnterFinalBossBattle(TriggeredBossRounds);
			}
			else
			{
				EnterBossBattle(TriggeredBossRounds);
			}
		}
		break;

	case EElysiaRunPhase::BossBattle:
	case EElysiaRunPhase::FinalBossBattle:
		if (CurrentBoss.IsValid())
		{
			UpdateGameStateSnapshot();
		}
		break;

	case EElysiaRunPhase::Finished:
	default:
		break;
	}

	UpdateGameStateSnapshot();
}

void AElysiaGameModeBase::EnterBossBattle(int32 BossRound)
{
	CurrentRunPhase = EElysiaRunPhase::BossBattle;
	CurrentBossStartTime = GetWorld()->GetTimeSeconds();

	if (SpawnManager)
	{
		SpawnManager->StopEliteSpawn();
	}

	if (AElysiaEnemy* SpawnedBoss = SpawnManager ? SpawnManager->SpawnSpecialEnemy(GetBossClassForRound(BossRound)) : nullptr)
	{
		CurrentBoss = SpawnedBoss;
		SpawnedBoss->OnEnemyDied.RemoveAll(this);
		SpawnedBoss->OnEnemyDied.AddUObject(this, &AElysiaGameModeBase::HandleTrackedBossDied);
	}
	else
	{
		FinishRun();
		return;
	}
}

void AElysiaGameModeBase::EnterFinalBossBattle(int32 BossRound)
{
	CurrentRunPhase = EElysiaRunPhase::FinalBossBattle;
	CurrentBossStartTime = GetWorld()->GetTimeSeconds();

	if (SpawnManager)
	{
		SpawnManager->StopEliteSpawn();
		SpawnManager->StopNormalSpawn();
	}

	if (AElysiaEnemy* SpawnedBoss = SpawnManager ? SpawnManager->SpawnSpecialEnemy(GetBossClassForRound(BossRound)) : nullptr)
	{
		CurrentBoss = SpawnedBoss;
		SpawnedBoss->OnEnemyDied.RemoveAll(this);
		SpawnedBoss->OnEnemyDied.AddUObject(this, &AElysiaGameModeBase::HandleTrackedBossDied);
	}
	else
	{
		FinishRun();
		return;
	}
}

void AElysiaGameModeBase::ResumeNormalPhase()
{
	CurrentRunPhase = EElysiaRunPhase::Normal;
	CurrentBoss = nullptr;
	CurrentBossStartTime = 0.f;

	if (SpawnManager)
	{
		SpawnManager->StartEliteSpawn();
		SpawnManager->StartNormalSpawn();
	}

	UpdateGameStateSnapshot();
}

void AElysiaGameModeBase::HandleTrackedBossDied(AElysiaEnemy* DeadEnemy)
{
	if (!CurrentBoss.IsValid() || CurrentBoss.Get() != DeadEnemy)
	{
		return;
	}

	const float BossDuration = FMath::Max(0.f, GetWorld()->GetTimeSeconds() - CurrentBossStartTime);
	BossScore += CalculateBossRoundScore(BossDuration);
	TotalScore = NormalScore + BossScore;

	if (TriggeredBossRounds >= TotalBossRounds)
	{
		FinishRun();
	}
	else
	{
		ResumeNormalPhase();
	}
}

void AElysiaGameModeBase::FinishRun()
{
	CurrentRunPhase = EElysiaRunPhase::Finished;
	CurrentBoss = nullptr;

	GetWorldTimerManager().ClearTimer(RunLoopTimer);
	if (SpawnManager)
	{
		SpawnManager->StopEliteSpawn();
		SpawnManager->StopNormalSpawn();
	}

	UpdateGameStateSnapshot();
}

void AElysiaGameModeBase::UpdateGameStateSnapshot() const
{
	if (AElysiaGameState* ElysiaGameState = GetElysiaGameState())
	{
		ElysiaGameState->SetCurrentRunPhase(CurrentRunPhase);
		ElysiaGameState->SetCurrentBossRound(CurrentRunPhase == EElysiaRunPhase::Normal || CurrentRunPhase == EElysiaRunPhase::Finished ? 0 : TriggeredBossRounds);
		ElysiaGameState->SetNormalPhaseElapsedSeconds(CurrentRunPhase == EElysiaRunPhase::Normal ? NormalPhaseElapsedSeconds : 0);
		ElysiaGameState->SetCurrentBossElapsedSeconds(
			(CurrentRunPhase == EElysiaRunPhase::BossBattle || CurrentRunPhase == EElysiaRunPhase::FinalBossBattle)
				? FMath::Max(0, FMath::FloorToInt(GetWorld()->GetTimeSeconds() - CurrentBossStartTime))
				: 0);
		ElysiaGameState->SetScores(NormalScore, BossScore, TotalScore);
		ElysiaGameState->SetRunFinished(CurrentRunPhase == EElysiaRunPhase::Finished);
	}
}

int32 AElysiaGameModeBase::CalculateBossRoundScore(float BossDuration) const
{
	const float RawScore = static_cast<float>(BossBaseScore) - BossDuration * BossScorePenaltyPerSecond;
	return FMath::Max(0, FMath::RoundToInt(RawScore));
}

TSubclassOf<AElysiaEnemy> AElysiaGameModeBase::GetBossClassForRound(int32 BossRound) const
{
	if (BossRoundClasses.IsEmpty() || BossRound <= 0)
	{
		return nullptr;
	}

	const int32 BossIndex = FMath::Clamp(BossRound - 1, 0, BossRoundClasses.Num() - 1);
	return BossRoundClasses[BossIndex];
}

AElysiaGameState* AElysiaGameModeBase::GetElysiaGameState() const
{
	return GetGameState<AElysiaGameState>();
}

