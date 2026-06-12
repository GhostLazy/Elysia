// Copyright GhostLazy

#include "Trial/ElysiaKillEnemiesTrialEvent.h"
#include "Character/ElysiaEnemy.h"
#include "Game/ElysiaGroundSpawnUtility.h"
#include "NavigationSystem.h"
#include "TimerManager.h"

FVector AElysiaKillEnemiesTrialEvent::GetCompletionRewardLocation() const
{
	return TrialTriggerLocation;
}

int32 AElysiaKillEnemiesTrialEvent::GetRemainingEnemyCount() const
{
	return FMath::Max(0, RequiredEnemyKillCount - DefeatedEnemyCount);
}

void AElysiaKillEnemiesTrialEvent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearSpawnRetryTimer();
	ClearTrialEnemies(!HasAuthority() ? false : !IsFinished());
	Super::EndPlay(EndPlayReason);
}

void AElysiaKillEnemiesTrialEvent::HandleTrialTriggered(AActor* TriggerActor)
{
	if (!HasAuthority())
	{
		return;
	}

	TrialTriggerLocation = GetActorLocation();
	DefeatedEnemyCount = 0;
	SpawnTrialEnemies();
}

void AElysiaKillEnemiesTrialEvent::HandleTrialCompleted()
{
	ClearSpawnRetryTimer();
	ClearTrialEnemies(false);
}

void AElysiaKillEnemiesTrialEvent::HandleTrialExpired()
{
	ClearSpawnRetryTimer();
	ClearTrialEnemies(true);
}

void AElysiaKillEnemiesTrialEvent::HandleTrialCancelled()
{
	ClearSpawnRetryTimer();
	ClearTrialEnemies(true);
}

void AElysiaKillEnemiesTrialEvent::SpawnTrialEnemies()
{
	if (!HasAuthority() || !HasBeenTriggered())
	{
		return;
	}

	// 一次性重试定时器进入回调后先清理句柄，允许本轮失败时继续安排下一次重试。
	ClearSpawnRetryTimer();
	PruneInvalidTrialEnemies();

	TArray<TSubclassOf<AElysiaEnemy>> ValidEnemyClasses;
	for (const TSubclassOf<AElysiaEnemy>& EnemyClass : TrialEnemyClasses)
	{
		if (EnemyClass)
		{
			ValidEnemyClasses.Add(EnemyClass);
		}
	}

	if (ValidEnemyClasses.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("%s cannot spawn trial enemies because TrialEnemyClasses has no valid class."), *GetName());
		CancelTrial();
		return;
	}

	const int32 MissingEnemyCount = FMath::Max(
		0,
		RequiredEnemyKillCount - DefeatedEnemyCount - ActiveTrialEnemies.Num());
	const int32 MaxTotalSpawnAttempts = MissingEnemyCount * FMath::Max(1, MaxSpawnAttemptsPerEnemy);
	int32 SpawnAttemptCount = 0;

	while (DefeatedEnemyCount + ActiveTrialEnemies.Num() < RequiredEnemyKillCount
		&& SpawnAttemptCount < MaxTotalSpawnAttempts)
	{
		++SpawnAttemptCount;
		SpawnSingleTrialEnemy(ValidEnemyClasses);
	}

	if (DefeatedEnemyCount + ActiveTrialEnemies.Num() < RequiredEnemyKillCount)
	{
		ScheduleSpawnRetry();
	}
}

bool AElysiaKillEnemiesTrialEvent::SpawnSingleTrialEnemy(
	const TArray<TSubclassOf<AElysiaEnemy>>& ValidEnemyClasses)
{
	UWorld* World = GetWorld();
	if (!World || ValidEnemyClasses.IsEmpty())
	{
		return false;
	}

	const TSubclassOf<AElysiaEnemy> EnemyClass = ValidEnemyClasses[FMath::RandHelper(ValidEnemyClasses.Num())];
	float CapsuleRadius = 0.f;
	float CapsuleHalfHeight = 0.f;
	if (!FElysiaGroundSpawnUtility::GetCharacterCapsuleSize(
		EnemyClass.Get(),
		CapsuleRadius,
		CapsuleHalfHeight))
	{
		return false;
	}

	FVector SpawnLocation;
	if (!TryFindEnemySpawnLocation(CapsuleHalfHeight, SpawnLocation))
	{
		return false;
	}

	const FTransform SpawnTransform(FRotator::ZeroRotator, SpawnLocation);
	AElysiaEnemy* SpawnedEnemy = World->SpawnActorDeferred<AElysiaEnemy>(
		EnemyClass,
		SpawnTransform,
		this,
		nullptr,
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
	if (!SpawnedEnemy)
	{
		return false;
	}

	SpawnedEnemy->SetLevel(EnemyLevel);
	SpawnedEnemy->SetDeathRewardsEnabled(false);
	SpawnedEnemy->FinishSpawning(SpawnTransform);

	// 延迟生成可能在 FinishSpawning 阶段失败或触发销毁，不能把无效弱指针计入剩余数量。
	if (!IsValid(SpawnedEnemy) || SpawnedEnemy->IsActorBeingDestroyed() || SpawnedEnemy->IsDead())
	{
		return false;
	}

	if (!FElysiaGroundSpawnUtility::HasGroundBelowCharacter(
		World,
		SpawnedEnemy->GetActorLocation(),
		CapsuleHalfHeight,
		GroundClearance,
		PostSpawnGroundCheckDistance,
		SpawnedEnemy))
	{
		SpawnedEnemy->Destroy();
		return false;
	}

	ActiveTrialEnemies.Add(SpawnedEnemy);
	SpawnedEnemy->OnEnemyDied.AddUObject(this, &AElysiaKillEnemiesTrialEvent::HandleTrialEnemyDied);
	SpawnedEnemy->OnDestroyed.AddUniqueDynamic(this, &AElysiaKillEnemiesTrialEvent::HandleTrialEnemyDestroyed);
	return true;
}

bool AElysiaKillEnemiesTrialEvent::TryFindEnemySpawnLocation(
	float CapsuleHalfHeight,
	FVector& OutSpawnLocation) const
{
	UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	const float Radius = FMath::Sqrt(FMath::FRandRange(0.f, FMath::Square(FMath::Max(0.f, EnemySpawnRadius))));
	const float AngleRadians = FMath::FRandRange(0.f, 2.f * PI);
	const FVector CandidateLocation = TrialTriggerLocation
		+ FVector(FMath::Cos(AngleRadians) * Radius, FMath::Sin(AngleRadians) * Radius, 0.f);

	FVector GroundCandidate = CandidateLocation;
	if (NavSystem)
	{
		FNavLocation NavLocation;
		if (NavSystem->ProjectPointToNavigation(CandidateLocation, NavLocation, NavProjectionExtent)
			|| NavSystem->ProjectPointToNavigation(TrialTriggerLocation, NavLocation, NavProjectionExtent))
		{
			GroundCandidate = NavLocation.Location;
		}
	}

	if (FElysiaGroundSpawnUtility::TryProjectCandidateToGround(
		GetWorld(),
		GroundCandidate,
		CapsuleHalfHeight,
		GroundTraceUpDistance,
		GroundTraceDownDistance,
		GroundClearance,
		this,
		OutSpawnLocation))
	{
		return true;
	}

	// 随机候选点找不到地面时，再以试炼触发点进行一次相同的地面投射。
	return FElysiaGroundSpawnUtility::TryProjectCandidateToGround(
		GetWorld(),
		TrialTriggerLocation,
		CapsuleHalfHeight,
		GroundTraceUpDistance,
		GroundTraceDownDistance,
		GroundClearance,
		this,
		OutSpawnLocation);
}

void AElysiaKillEnemiesTrialEvent::ScheduleSpawnRetry()
{
	if (!HasAuthority() || !HasBeenTriggered())
	{
		return;
	}

	if (UWorld* World = GetWorld(); World && !World->GetTimerManager().IsTimerActive(SpawnRetryTimerHandle))
	{
		World->GetTimerManager().SetTimer(
			SpawnRetryTimerHandle,
			this,
			&AElysiaKillEnemiesTrialEvent::SpawnTrialEnemies,
			FMath::Max(0.05f, SpawnRetryInterval),
			false);
	}
}

void AElysiaKillEnemiesTrialEvent::ClearSpawnRetryTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SpawnRetryTimerHandle);
	}
}

void AElysiaKillEnemiesTrialEvent::PruneInvalidTrialEnemies()
{
	for (auto It = ActiveTrialEnemies.CreateIterator(); It; ++It)
	{
		if (!It->IsValid())
		{
			It.RemoveCurrent();
		}
	}
}

void AElysiaKillEnemiesTrialEvent::HandleTrialEnemyDied(AElysiaEnemy* DeadEnemy)
{
	if (!HasAuthority() || !DeadEnemy)
	{
		return;
	}

	UnregisterTrialEnemy(DeadEnemy);
	++DefeatedEnemyCount;

	if (DefeatedEnemyCount >= RequiredEnemyKillCount)
	{
		CompleteTrial();
	}
	else
	{
		SpawnTrialEnemies();
	}
}

void AElysiaKillEnemiesTrialEvent::HandleTrialEnemyDestroyed(AActor* DestroyedActor)
{
	if (!HasAuthority())
	{
		return;
	}

	// 非死亡流程导致的销毁不计入击杀数，继续补生成缺失怪物。
	UnregisterTrialEnemy(Cast<AElysiaEnemy>(DestroyedActor));
	SpawnTrialEnemies();
}

void AElysiaKillEnemiesTrialEvent::UnregisterTrialEnemy(AElysiaEnemy* Enemy)
{
	if (!Enemy)
	{
		return;
	}

	Enemy->OnEnemyDied.RemoveAll(this);
	Enemy->OnDestroyed.RemoveAll(this);
	ActiveTrialEnemies.Remove(Enemy);
}

void AElysiaKillEnemiesTrialEvent::ClearTrialEnemies(bool bDestroyRemainingEnemies)
{
	TArray<TWeakObjectPtr<AElysiaEnemy>> TrialEnemies = ActiveTrialEnemies.Array();
	ActiveTrialEnemies.Empty();

	for (const TWeakObjectPtr<AElysiaEnemy>& WeakEnemy : TrialEnemies)
	{
		if (AElysiaEnemy* Enemy = WeakEnemy.Get())
		{
			Enemy->OnEnemyDied.RemoveAll(this);
			Enemy->OnDestroyed.RemoveAll(this);
			if (bDestroyRemainingEnemies && !Enemy->IsActorBeingDestroyed())
			{
				Enemy->Destroy();
			}
		}
	}
}
