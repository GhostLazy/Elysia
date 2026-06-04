// Copyright GhostLazy


#include "Game/ElysiaTrialManager.h"

#include "Actor/ElysiaTrialEventBase.h"
#include "Actor/ElysiaTrialSpawnPoint.h"
#include "EngineUtils.h"
#include "TimerManager.h"

AElysiaTrialManager::AElysiaTrialManager()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;
}

void AElysiaTrialManager::BeginPlay()
{
	Super::BeginPlay();
	RefreshTrialSpawnPoints();
}

void AElysiaTrialManager::StartPhaseTrial(EElysiaRunPhase Phase, int32 PhaseIndex)
{
	if (!HasAuthority())
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(TrialSpawnTimerHandle);
	CancelActiveTrialForPhaseTransition();

	CurrentPhase = Phase;
	CurrentPhaseIndex = PhaseIndex;

	if (!ShouldScheduleTrialForPhase(Phase))
	{
		return;
	}

	if (TrialSpawnDelay <= 0.f)
	{
		HandleTrialSpawnTimer();
		return;
	}

	GetWorldTimerManager().SetTimer(
		TrialSpawnTimerHandle,
		this,
		&AElysiaTrialManager::HandleTrialSpawnTimer,
		TrialSpawnDelay,
		false);
}

void AElysiaTrialManager::StopPhaseTrial()
{
	if (!HasAuthority())
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(TrialSpawnTimerHandle);
	CancelActiveTrialForPhaseTransition();
	CurrentPhase = EElysiaRunPhase::Finished;
	CurrentPhaseIndex = 0;
}

void AElysiaTrialManager::StopAllTrials()
{
	if (!HasAuthority())
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(TrialSpawnTimerHandle);
	if (AElysiaTrialEventBase* TrialEvent = ActiveTrialEvent.Get())
	{
		TrialEvent->OnTrialEventFinished.RemoveAll(this);
		TrialEvent->CancelTrial();
	}

	ActiveTrialEvent.Reset();
	CurrentPhase = EElysiaRunPhase::Finished;
	CurrentPhaseIndex = 0;
}

void AElysiaTrialManager::RefreshTrialSpawnPoints()
{
	CachedTrialSpawnPoints.Reset();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (TActorIterator<AElysiaTrialSpawnPoint> It(World); It; ++It)
	{
		if (AElysiaTrialSpawnPoint* TrialSpawnPoint = *It)
		{
			CachedTrialSpawnPoints.Add(TrialSpawnPoint);
		}
	}
}

void AElysiaTrialManager::HandleTrialSpawnTimer()
{
	if (!HasAuthority() || !ShouldScheduleTrialForPhase(CurrentPhase) || ActiveTrialEvent.IsValid())
	{
		return;
	}

	RefreshTrialSpawnPoints();

	AElysiaTrialSpawnPoint* TrialSpawnPoint = ChooseTrialSpawnPoint();
	const TSubclassOf<AElysiaTrialEventBase> TrialEventClass = ChooseTrialEventClass();
	if (!TrialSpawnPoint || !TrialEventClass)
	{
		return;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AElysiaTrialEventBase* TrialEvent = GetWorld()->SpawnActor<AElysiaTrialEventBase>(
		TrialEventClass,
		TrialSpawnPoint->GetTrialSpawnTransform(),
		SpawnParameters);

	if (!TrialEvent)
	{
		return;
	}

	ActiveTrialEvent = TrialEvent;
	TrialEvent->OnTrialEventFinished.AddUObject(this, &AElysiaTrialManager::HandleActiveTrialFinished);
	TrialEvent->InitializeTrial(TrialSpawnPoint, UntriggeredTrialLifetime);
}

void AElysiaTrialManager::HandleActiveTrialFinished(AElysiaTrialEventBase* FinishedTrial)
{
	if (ActiveTrialEvent.Get() == FinishedTrial)
	{
		ActiveTrialEvent.Reset();
	}
}

bool AElysiaTrialManager::ShouldScheduleTrialForPhase(EElysiaRunPhase Phase) const
{
	if (TrialEventPool.IsEmpty() || TrialSpawnDelay < 0.f)
	{
		return false;
	}

	switch (Phase)
	{
	case EElysiaRunPhase::Normal:
		return bEnableTrialsInNormalPhase;
	case EElysiaRunPhase::BossBattle:
		return bEnableTrialsInBossPhase;
	case EElysiaRunPhase::FinalBossBattle:
		return bEnableTrialsInFinalBossPhase;
	case EElysiaRunPhase::Finished:
	default:
		return false;
	}
}

AElysiaTrialSpawnPoint* AElysiaTrialManager::ChooseTrialSpawnPoint() const
{
	TArray<AElysiaTrialSpawnPoint*> EnabledSpawnPoints;
	for (const TObjectPtr<AElysiaTrialSpawnPoint>& TrialSpawnPointPtr : CachedTrialSpawnPoints)
	{
		AElysiaTrialSpawnPoint* TrialSpawnPoint = TrialSpawnPointPtr.Get();
		if (IsValid(TrialSpawnPoint) && TrialSpawnPoint->IsTrialSpawnEnabled())
		{
			EnabledSpawnPoints.Add(TrialSpawnPoint);
		}
	}

	if (EnabledSpawnPoints.IsEmpty())
	{
		return nullptr;
	}

	return EnabledSpawnPoints[FMath::RandRange(0, EnabledSpawnPoints.Num() - 1)];
}

TSubclassOf<AElysiaTrialEventBase> AElysiaTrialManager::ChooseTrialEventClass() const
{
	float TotalWeight = 0.f;
	for (const FElysiaTrialEventEntry& TrialEventEntry : TrialEventPool)
	{
		if (TrialEventEntry.TrialEventClass && TrialEventEntry.Weight > 0.f)
		{
			TotalWeight += TrialEventEntry.Weight;
		}
	}

	if (TotalWeight <= 0.f)
	{
		return nullptr;
	}

	float RemainingWeight = FMath::FRandRange(0.f, TotalWeight);
	for (const FElysiaTrialEventEntry& TrialEventEntry : TrialEventPool)
	{
		if (!TrialEventEntry.TrialEventClass || TrialEventEntry.Weight <= 0.f)
		{
			continue;
		}

		RemainingWeight -= TrialEventEntry.Weight;
		if (RemainingWeight <= 0.f)
		{
			return TrialEventEntry.TrialEventClass;
		}
	}

	return nullptr;
}

void AElysiaTrialManager::CancelActiveTrialForPhaseTransition()
{
	AElysiaTrialEventBase* TrialEvent = ActiveTrialEvent.Get();
	if (!TrialEvent)
	{
		return;
	}

	if (TrialEvent->IsWaitingToBeTriggered() || bCancelTriggeredTrialOnPhaseEnd)
	{
		TrialEvent->OnTrialEventFinished.RemoveAll(this);
		TrialEvent->CancelTrial();
		ActiveTrialEvent.Reset();
	}
}
