// Copyright GhostLazy


#include "Actor/ElysiaTrialEventBase.h"

#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Elysia/Elysia.h"
#include "Interface/CombatInterface.h"
#include "TimerManager.h"

AElysiaTrialEventBase::AElysiaTrialEventBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
	TriggerSphere->SetupAttachment(SceneRoot);
	TriggerSphere->SetSphereRadius(TriggerRadius);
	TriggerSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerSphere->SetCollisionResponseToChannel(ECC_Player, ECR_Overlap);
}

void AElysiaTrialEventBase::BeginPlay()
{
	Super::BeginPlay();

	if (TriggerSphere)
	{
		TriggerSphere->SetSphereRadius(TriggerRadius);
		TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &AElysiaTrialEventBase::HandleTriggerSphereBeginOverlap);
	}
}

void AElysiaTrialEventBase::InitializeTrial(AActor* InSpawnPoint, float InUntriggeredLifetime)
{
	if (!HasAuthority())
	{
		return;
	}

	SpawnPoint = InSpawnPoint;
	TrialEventState = EElysiaTrialEventState::WaitingToBeTriggered;
	bTrialFinishedBroadcasted = false;

	if (UWorld* World = GetWorld(); World && InUntriggeredLifetime > 0.f)
	{
		World->GetTimerManager().SetTimer(
			ExpirationTimerHandle,
			this,
			&AElysiaTrialEventBase::ExpireTrial,
			InUntriggeredLifetime,
			false);
	}

	OnTrialOffered();
}

void AElysiaTrialEventBase::TriggerTrial(AActor* TriggerActor)
{
	if (!HasAuthority() || TrialEventState != EElysiaTrialEventState::WaitingToBeTriggered || !CanTriggerTrial(TriggerActor))
	{
		return;
	}

	ClearExpirationTimer();
	TrialEventState = EElysiaTrialEventState::Triggered;
	TriggeringActor = TriggerActor;

	if (bDisableTriggerCollisionWhenTriggered && TriggerSphere)
	{
		TriggerSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	OnTrialTriggered(TriggerActor);

	if (bDestroyWhenTriggered)
	{
		CompleteTrial();
	}
}

void AElysiaTrialEventBase::CompleteTrial()
{
	if (!HasAuthority()
		|| TrialEventState == EElysiaTrialEventState::Completed
		|| TrialEventState == EElysiaTrialEventState::Expired
		|| TrialEventState == EElysiaTrialEventState::Cancelled)
	{
		return;
	}

	ClearExpirationTimer();
	TrialEventState = EElysiaTrialEventState::Completed;
	OnTrialCompleted();
	BroadcastTrialFinished();
	Destroy();
}

void AElysiaTrialEventBase::CancelTrial()
{
	if (!HasAuthority()
		|| TrialEventState == EElysiaTrialEventState::Completed
		|| TrialEventState == EElysiaTrialEventState::Expired
		|| TrialEventState == EElysiaTrialEventState::Cancelled)
	{
		return;
	}

	ClearExpirationTimer();
	TrialEventState = EElysiaTrialEventState::Cancelled;
	OnTrialCancelled();
	BroadcastTrialFinished();
	Destroy();
}

bool AElysiaTrialEventBase::CanTriggerTrial_Implementation(AActor* CandidateActor) const
{
	if (!IsValid(CandidateActor))
	{
		return false;
	}

	const ICombatInterface* CombatInterface = Cast<ICombatInterface>(CandidateActor);
	return CombatInterface ? CombatInterface->IsPlayer() && !CombatInterface->IsDead() : CandidateActor->ActorHasTag(FName("Player"));
}

void AElysiaTrialEventBase::OnTrialOffered_Implementation()
{
}

void AElysiaTrialEventBase::OnTrialTriggered_Implementation(AActor* TriggerActor)
{
}

void AElysiaTrialEventBase::OnTrialCompleted_Implementation()
{
}

void AElysiaTrialEventBase::OnTrialExpired_Implementation()
{
}

void AElysiaTrialEventBase::OnTrialCancelled_Implementation()
{
}

void AElysiaTrialEventBase::HandleTriggerSphereBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	TriggerTrial(OtherActor);
}

void AElysiaTrialEventBase::ExpireTrial()
{
	if (!HasAuthority() || TrialEventState != EElysiaTrialEventState::WaitingToBeTriggered)
	{
		return;
	}

	TrialEventState = EElysiaTrialEventState::Expired;
	OnTrialExpired();
	BroadcastTrialFinished();
	Destroy();
}

void AElysiaTrialEventBase::ClearExpirationTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ExpirationTimerHandle);
	}
}

void AElysiaTrialEventBase::BroadcastTrialFinished()
{
	if (bTrialFinishedBroadcasted)
	{
		return;
	}

	bTrialFinishedBroadcasted = true;
	OnTrialEventFinished.Broadcast(this);
}
