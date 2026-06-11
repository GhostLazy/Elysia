// Copyright GhostLazy


#include "Trial/ElysiaTrialEventBase.h"

#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Elysia/Elysia.h"
#include "GameFramework/GameStateBase.h"
#include "Interface/CombatInterface.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "Trial/ElysiaTrialInteractableActor.h"

AElysiaTrialEventBase::AElysiaTrialEventBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
	TriggerSphere->SetupAttachment(SceneRoot);
	TriggerSphere->SetSphereRadius(TriggerRadius);
	TriggerSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TriggerSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerSphere->SetCollisionResponseToChannel(ECC_Player, ECR_Overlap);
}

void AElysiaTrialEventBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AElysiaTrialEventBase, SpawnPoint);
	DOREPLIFETIME(AElysiaTrialEventBase, TriggeringActor);
	DOREPLIFETIME(AElysiaTrialEventBase, TrialOfferActor);
	DOREPLIFETIME(AElysiaTrialEventBase, OfferLifetime);
	DOREPLIFETIME(AElysiaTrialEventBase, TrialDuration);
	DOREPLIFETIME(AElysiaTrialEventBase, TrialEventState);
	DOREPLIFETIME(AElysiaTrialEventBase, OfferStartedServerTime);
	DOREPLIFETIME(AElysiaTrialEventBase, OfferExpirationServerTime);
	DOREPLIFETIME(AElysiaTrialEventBase, TrialStartedServerTime);
	DOREPLIFETIME(AElysiaTrialEventBase, TrialExpirationServerTime);
}

void AElysiaTrialEventBase::BeginPlay()
{
	Super::BeginPlay();

	if (TriggerSphere)
	{
		TriggerSphere->SetSphereRadius(TriggerRadius);
		if (bEnableOverlapTrigger)
		{
			TriggerSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &AElysiaTrialEventBase::HandleTriggerSphereBeginOverlap);
		}
		else
		{
			TriggerSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void AElysiaTrialEventBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearAllExpirationTimers();
	DestroyTrialOfferActor();
	Super::EndPlay(EndPlayReason);
}

void AElysiaTrialEventBase::InitializeTrial(
	AActor* InSpawnPoint,
	float InUntriggeredLifetime,
	TSubclassOf<AElysiaTrialInteractableActor> InTrialOfferActorClass)
{
	if (!HasAuthority())
	{
		return;
	}

	SpawnPoint = InSpawnPoint;
	TrialEventState = EElysiaTrialEventState::WaitingToBeTriggered;
	bTrialFinishedBroadcasted = false;
	OfferLifetime = InUntriggeredLifetime;
	TrialStartedServerTime = 0.f;
	TrialExpirationServerTime = 0.f;

	OfferStartedServerTime = GetCurrentServerWorldTime();
	OfferExpirationServerTime = OfferLifetime > 0.f ? OfferStartedServerTime + OfferLifetime : 0.f;

	SpawnTrialOfferActor(InTrialOfferActorClass);

	if (UWorld* World = GetWorld(); World && InUntriggeredLifetime > 0.f)
	{
		World->GetTimerManager().SetTimer(
			ExpirationTimerHandle,
			this,
			&AElysiaTrialEventBase::ExpireTrial,
			InUntriggeredLifetime,
		false);
	}

}

void AElysiaTrialEventBase::TriggerTrial(AActor* TriggerActor)
{
	if (!HasAuthority() || !CanBeTriggeredBy(TriggerActor))
	{
		return;
	}

	ClearOfferExpirationTimer();
	TrialEventState = EElysiaTrialEventState::Triggered;
	TriggeringActor = TriggerActor;
	StartTrialExpirationTimer();

	if (bDisableTriggerCollisionWhenTriggered && TriggerSphere)
	{
		TriggerSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	DestroyTrialOfferActor();
	HandleTrialTriggered(TriggerActor);
	ForceNetUpdate();

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

	ClearAllExpirationTimers();
	TrialEventState = EElysiaTrialEventState::Completed;
	HandleTrialCompleted();
	DestroyTrialOfferActor();
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

	ClearAllExpirationTimers();
	TrialEventState = EElysiaTrialEventState::Cancelled;
	HandleTrialCancelled();
	DestroyTrialOfferActor();
	BroadcastTrialFinished();
	Destroy();
}

bool AElysiaTrialEventBase::CanBeTriggeredBy(AActor* CandidateActor) const
{
	return TrialEventState == EElysiaTrialEventState::WaitingToBeTriggered && CanTriggerTrial(CandidateActor);
}

bool AElysiaTrialEventBase::IsFinished() const
{
	return TrialEventState == EElysiaTrialEventState::Completed
		|| TrialEventState == EElysiaTrialEventState::Expired
		|| TrialEventState == EElysiaTrialEventState::Cancelled;
}

FVector AElysiaTrialEventBase::GetIndicatorTargetLocation() const
{
	return GetActorLocation();
}

bool AElysiaTrialEventBase::ShouldShowActiveTrialDirectionIndicator() const
{
	return false;
}

FVector AElysiaTrialEventBase::GetCompletionRewardLocation() const
{
	return GetActorLocation();
}

float AElysiaTrialEventBase::GetRemainingOfferTime() const
{
	if (OfferLifetime <= 0.f || OfferExpirationServerTime <= 0.f)
	{
		return 0.f;
	}

	return FMath::Max(0.f, OfferExpirationServerTime - GetCurrentServerWorldTime());
}

float AElysiaTrialEventBase::GetRemainingTrialTime() const
{
	if (TrialDuration <= 0.f || TrialExpirationServerTime <= 0.f)
	{
		return 0.f;
	}

	return FMath::Max(0.f, TrialExpirationServerTime - GetCurrentServerWorldTime());
}

bool AElysiaTrialEventBase::CanTriggerTrial(AActor* CandidateActor) const
{
	if (!IsValid(CandidateActor))
	{
		return false;
	}

	const ICombatInterface* CombatInterface = Cast<ICombatInterface>(CandidateActor);
	return CombatInterface ? CombatInterface->IsPlayer() && !CombatInterface->IsDead() : CandidateActor->ActorHasTag(FName("Player"));
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
	if (!HasAuthority()
		|| TrialEventState == EElysiaTrialEventState::Completed
		|| TrialEventState == EElysiaTrialEventState::Expired
		|| TrialEventState == EElysiaTrialEventState::Cancelled)
	{
		return;
	}

	ClearAllExpirationTimers();
	TrialEventState = EElysiaTrialEventState::Expired;
	ForceNetUpdate();
	HandleTrialExpired();
	DestroyTrialOfferActor();
	BroadcastTrialFinished();

	if (ExpiredStateDisplayDuration > 0.f)
	{
		SetLifeSpan(ExpiredStateDisplayDuration);
	}
	else
	{
		Destroy();
	}
}

void AElysiaTrialEventBase::ClearOfferExpirationTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ExpirationTimerHandle);
	}
}

void AElysiaTrialEventBase::ClearTrialExpirationTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TrialExpirationTimerHandle);
	}
}

void AElysiaTrialEventBase::ClearAllExpirationTimers()
{
	ClearOfferExpirationTimer();
	ClearTrialExpirationTimer();
}

void AElysiaTrialEventBase::StartTrialExpirationTimer()
{
	TrialStartedServerTime = GetCurrentServerWorldTime();
	TrialExpirationServerTime = TrialDuration > 0.f ? TrialStartedServerTime + TrialDuration : 0.f;

	if (UWorld* World = GetWorld(); World && TrialDuration > 0.f)
	{
		World->GetTimerManager().SetTimer(
			TrialExpirationTimerHandle,
			this,
			&AElysiaTrialEventBase::ExpireTrial,
			TrialDuration,
			false);
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

void AElysiaTrialEventBase::SpawnTrialOfferActor(TSubclassOf<AElysiaTrialInteractableActor> InTrialOfferActorClass)
{
	if (!HasAuthority() || !bSpawnTrialOfferActor || !InTrialOfferActorClass || TrialOfferActor)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FTransform TrialTransform = GetActorTransform();
	FVector OfferLocation = TrialTransform.TransformPosition(TrialOfferSpawnOffset);
	FRotator OfferRotation = TrialTransform.GetRotation().Rotator();
	if (SpawnPoint)
	{
		const FTransform SpawnPointTransform = SpawnPoint->GetActorTransform();
		OfferLocation = SpawnPointTransform.TransformPosition(TrialOfferSpawnOffset);
		OfferRotation = SpawnPointTransform.GetRotation().Rotator();
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	TrialOfferActor = World->SpawnActor<AElysiaTrialInteractableActor>(
		InTrialOfferActorClass,
		OfferLocation,
		OfferRotation,
		SpawnParameters);

	if (TrialOfferActor)
	{
		TrialOfferActor->InitializeTrialOffer(this);
	}
}

void AElysiaTrialEventBase::DestroyTrialOfferActor()
{
	if (!HasAuthority())
	{
		return;
	}

	if (AElysiaTrialInteractableActor* OfferActor = TrialOfferActor.Get())
	{
		TrialOfferActor = nullptr;
		if (!OfferActor->IsActorBeingDestroyed())
		{
			OfferActor->Destroy();
		}
	}
}

float AElysiaTrialEventBase::GetCurrentServerWorldTime() const
{
	if (const UWorld* World = GetWorld())
	{
		if (const AGameStateBase* GameState = World->GetGameState())
		{
			return GameState->GetServerWorldTimeSeconds();
		}

		return World->GetTimeSeconds();
	}

	return 0.f;
}
