// Copyright GhostLazy


#include "Trial/ElysiaReachLocationTrialEvent.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

void AElysiaReachLocationTrialEvent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AElysiaReachLocationTrialEvent, DestinationActor);
	DOREPLIFETIME(AElysiaReachLocationTrialEvent, DestinationLocation);
}

float AElysiaReachLocationTrialEvent::GetDistanceToDestination(AActor* CandidateActor) const
{
	if (!IsValid(CandidateActor))
	{
		return 0.f;
	}

	return FVector::Dist(CandidateActor->GetActorLocation(), DestinationLocation);
}

FVector AElysiaReachLocationTrialEvent::GetIndicatorTargetLocation() const
{
	return DestinationLocation;
}

bool AElysiaReachLocationTrialEvent::ShouldShowActiveTrialDirectionIndicator() const
{
	return HasBeenTriggered();
}

FVector AElysiaReachLocationTrialEvent::GetCompletionRewardLocation() const
{
	return DestinationLocation;
}

void AElysiaReachLocationTrialEvent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearReachCheckTimer();
	DestroyDestinationActor();
	Super::EndPlay(EndPlayReason);
}

void AElysiaReachLocationTrialEvent::HandleTrialTriggered(AActor* TriggerActor)
{
	if (!HasAuthority())
	{
		return;
	}

	const float AngleRadians = FMath::FRandRange(0.f, 2.f * PI);
	const FVector DestinationOffset(
		FMath::Cos(AngleRadians) * DestinationRadius,
		FMath::Sin(AngleRadians) * DestinationRadius,
		0.f);
	DestinationLocation = GetActorLocation() + DestinationOffset;
	SpawnDestinationActor();
	ForceNetUpdate();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			ReachCheckTimerHandle,
			this,
			&AElysiaReachLocationTrialEvent::CheckReachCondition,
			ReachCheckInterval,
			true);
	}

	CheckReachCondition();
}

void AElysiaReachLocationTrialEvent::HandleTrialCompleted()
{
	ClearReachCheckTimer();
	DestroyDestinationActor();
}

void AElysiaReachLocationTrialEvent::HandleTrialExpired()
{
	ClearReachCheckTimer();
	DestroyDestinationActor();
}

void AElysiaReachLocationTrialEvent::HandleTrialCancelled()
{
	ClearReachCheckTimer();
	DestroyDestinationActor();
}

void AElysiaReachLocationTrialEvent::SpawnDestinationActor()
{
	if (!HasAuthority() || !DestinationActorClass || DestinationActor)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	DestinationActor = World->SpawnActor<AActor>(
		DestinationActorClass,
		DestinationLocation,
		GetActorRotation(),
		SpawnParameters);

	if (DestinationActor)
	{
		DestinationActor->SetReplicates(true);
	}
}

void AElysiaReachLocationTrialEvent::DestroyDestinationActor()
{
	if (!HasAuthority())
	{
		return;
	}

	if (AActor* ActorToDestroy = DestinationActor.Get())
	{
		DestinationActor = nullptr;
		if (!ActorToDestroy->IsActorBeingDestroyed())
		{
			ActorToDestroy->Destroy();
		}
	}
}

void AElysiaReachLocationTrialEvent::ClearReachCheckTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ReachCheckTimerHandle);
	}
}

void AElysiaReachLocationTrialEvent::CheckReachCondition()
{
	if (!HasAuthority() || !HasBeenTriggered())
	{
		return;
	}

	if (HasReachedDestination(TriggeringActor))
	{
		CompleteTrial();
	}
}

bool AElysiaReachLocationTrialEvent::HasReachedDestination(AActor* CandidateActor) const
{
	if (!IsValid(CandidateActor))
	{
		return false;
	}

	return FVector::DistSquared(CandidateActor->GetActorLocation(), DestinationLocation) <= FMath::Square(ReachRadius);
}
