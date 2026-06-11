// Copyright GhostLazy

#include "Trial/ElysiaKillEnemiesTrialEvent.h"

#include "Character/ElysiaEnemy.h"
#include "NavigationSystem.h"

FVector AElysiaKillEnemiesTrialEvent::GetCompletionRewardLocation() const
{
	return TrialTriggerLocation;
}

void AElysiaKillEnemiesTrialEvent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
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
	SpawnTrialEnemies();
	if (ActiveTrialEnemies.IsEmpty())
	{
		CancelTrial();
	}
}

void AElysiaKillEnemiesTrialEvent::HandleTrialCompleted()
{
	ClearTrialEnemies(false);
}

void AElysiaKillEnemiesTrialEvent::HandleTrialExpired()
{
	ClearTrialEnemies(true);
}

void AElysiaKillEnemiesTrialEvent::HandleTrialCancelled()
{
	ClearTrialEnemies(true);
}

void AElysiaKillEnemiesTrialEvent::SpawnTrialEnemies()
{
	if (!HasAuthority() || TrialEnemyClasses.IsEmpty())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (int32 EnemyIndex = 0; EnemyIndex < FMath::Max(1, EnemyCount); ++EnemyIndex)
	{
		TSubclassOf<AElysiaEnemy> EnemyClass;
		for (int32 ClassAttempt = 0; ClassAttempt < TrialEnemyClasses.Num(); ++ClassAttempt)
		{
			const int32 ClassIndex = FMath::RandRange(0, TrialEnemyClasses.Num() - 1);
			if (TrialEnemyClasses[ClassIndex])
			{
				EnemyClass = TrialEnemyClasses[ClassIndex];
				break;
			}
		}
		if (!EnemyClass)
		{
			continue;
		}

		for (int32 SpawnAttempt = 0; SpawnAttempt < FMath::Max(1, MaxSpawnAttemptsPerEnemy); ++SpawnAttempt)
		{
			FVector SpawnLocation;
			if (!TryFindEnemySpawnLocation(SpawnLocation))
			{
				continue;
			}

			const FTransform SpawnTransform(FRotator::ZeroRotator, SpawnLocation);
			AElysiaEnemy* SpawnedEnemy = World->SpawnActorDeferred<AElysiaEnemy>(
				EnemyClass,
				SpawnTransform,
				this,
				nullptr,
				ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding);
			if (!SpawnedEnemy)
			{
				continue;
			}

			SpawnedEnemy->SetLevel(EnemyLevel);
			SpawnedEnemy->SetDeathRewardsEnabled(false);
			SpawnedEnemy->OnEnemyDied.AddUObject(this, &AElysiaKillEnemiesTrialEvent::HandleTrialEnemyDied);
			SpawnedEnemy->FinishSpawning(SpawnTransform);
			ActiveTrialEnemies.Add(SpawnedEnemy);
			break;
		}
	}
}

bool AElysiaKillEnemiesTrialEvent::TryFindEnemySpawnLocation(FVector& OutSpawnLocation) const
{
	UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (!NavSystem)
	{
		return false;
	}

	const float Radius = FMath::Sqrt(FMath::FRandRange(0.f, FMath::Square(FMath::Max(0.f, EnemySpawnRadius))));
	const float AngleRadians = FMath::FRandRange(0.f, 2.f * PI);
	const FVector CandidateLocation = TrialTriggerLocation
		+ FVector(FMath::Cos(AngleRadians) * Radius, FMath::Sin(AngleRadians) * Radius, 0.f);

	FNavLocation NavLocation;
	if (!NavSystem->ProjectPointToNavigation(CandidateLocation, NavLocation, NavProjectionExtent))
	{
		return false;
	}

	OutSpawnLocation = NavLocation.Location;
	return true;
}

void AElysiaKillEnemiesTrialEvent::HandleTrialEnemyDied(AElysiaEnemy* DeadEnemy)
{
	if (!HasAuthority() || !DeadEnemy)
	{
		return;
	}

	DeadEnemy->OnEnemyDied.RemoveAll(this);
	ActiveTrialEnemies.Remove(DeadEnemy);
	if (ActiveTrialEnemies.IsEmpty() && HasBeenTriggered())
	{
		CompleteTrial();
	}
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
			if (bDestroyRemainingEnemies && !Enemy->IsActorBeingDestroyed())
			{
				Enemy->Destroy();
			}
		}
	}
}
