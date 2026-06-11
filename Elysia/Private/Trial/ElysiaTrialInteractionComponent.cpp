// Copyright GhostLazy


#include "Trial/ElysiaTrialInteractionComponent.h"

#include "EngineUtils.h"
#include "GameFramework/Controller.h"
#include "Trial/ElysiaTrialEventBase.h"
#include "Trial/ElysiaTrialInteractableActor.h"

UElysiaTrialInteractionComponent::UElysiaTrialInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UElysiaTrialInteractionComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TrialOfferCacheRefreshAccumulator += DeltaTime;
	if (TrialOfferCacheRefreshAccumulator >= TrialOfferCacheRefreshInterval
		|| !IsValidTrialOfferActor(CurrentTrialOfferActor.Get()))
	{
		UpdateCachedTrialOfferActor();
		UpdateCachedTrialEventActor();
		TrialOfferCacheRefreshAccumulator = 0.f;
	}
}

AElysiaTrialInteractableActor* UElysiaTrialInteractionComponent::GetCurrentTrialOfferActor() const
{
	AElysiaTrialInteractableActor* TrialOffer = CurrentTrialOfferActor.Get();
	return IsValidTrialOfferActor(TrialOffer) ? TrialOffer : nullptr;
}

AElysiaTrialInteractableActor* UElysiaTrialInteractionComponent::GetCurrentInteractableTrialOfferActor() const
{
	AElysiaTrialInteractableActor* TrialOffer = GetCurrentTrialOfferActor();
	AActor* InteractingActor = GetInteractingActor();
	if (!TrialOffer || !InteractingActor || !TrialOffer->CanInteract(InteractingActor))
	{
		return nullptr;
	}

	return TrialOffer;
}

AElysiaTrialEventBase* UElysiaTrialInteractionComponent::GetCurrentTrialEvent() const
{
	AElysiaTrialEventBase* TrialEvent = CurrentTrialEvent.Get();
	return IsValidTrialEventActor(TrialEvent) ? TrialEvent : nullptr;
}

bool UElysiaTrialInteractionComponent::ShouldShowTrialDirectionIndicator() const
{
	return GetCurrentTrialOfferActor() && !GetCurrentInteractableTrialOfferActor();
}

bool UElysiaTrialInteractionComponent::ShouldShowTrialDestinationIndicator() const
{
	const AElysiaTrialEventBase* TrialEvent = GetCurrentTrialEvent();
	return TrialEvent && TrialEvent->ShouldShowActiveTrialDirectionIndicator();
}

bool UElysiaTrialInteractionComponent::ShouldShowTrialInteractPrompt() const
{
	return GetCurrentInteractableTrialOfferActor() != nullptr;
}

bool UElysiaTrialInteractionComponent::TryInteractWithCurrentTrialOffer()
{
	AElysiaTrialInteractableActor* TrialOffer = FindBestTrialOfferToInteract();
	if (!TrialOffer)
	{
		return false;
	}

	if (AActor* OwnerActor = GetOwner(); OwnerActor && OwnerActor->HasAuthority())
	{
		return TrialOffer->Interact(GetInteractingActor());
	}

	ServerInteractWithTrialOffer(TrialOffer);
	return true;
}

void UElysiaTrialInteractionComponent::ServerInteractWithTrialOffer_Implementation(
	AElysiaTrialInteractableActor* TrialOffer)
{
	if (TrialOffer && IsValidTrialOfferActor(TrialOffer))
	{
		TrialOffer->Interact(GetInteractingActor());
	}
}

void UElysiaTrialInteractionComponent::UpdateCachedTrialOfferActor()
{
	CurrentTrialOfferActor = FindBestTrialOffer();
}

void UElysiaTrialInteractionComponent::UpdateCachedTrialEventActor()
{
	CurrentTrialEvent = FindCurrentTrialEvent();
}

AElysiaTrialInteractableActor* UElysiaTrialInteractionComponent::FindBestTrialOffer() const
{
	AActor* InteractingActor = GetInteractingActor();
	UWorld* World = GetWorld();
	if (!InteractingActor || !World)
	{
		return nullptr;
	}

	AElysiaTrialInteractableActor* BestTrialOffer = nullptr;
	float BestDistanceSq = TNumericLimits<float>::Max();

	for (TActorIterator<AElysiaTrialInteractableActor> It(World); It; ++It)
	{
		AElysiaTrialInteractableActor* TrialOffer = *It;
		if (!IsValidTrialOfferActor(TrialOffer))
		{
			continue;
		}

		const float DistanceSq = FVector::DistSquared(InteractingActor->GetActorLocation(), TrialOffer->GetActorLocation());
		if (DistanceSq < BestDistanceSq)
		{
			BestDistanceSq = DistanceSq;
			BestTrialOffer = TrialOffer;
		}
	}

	return BestTrialOffer;
}

AElysiaTrialEventBase* UElysiaTrialInteractionComponent::FindCurrentTrialEvent() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	for (TActorIterator<AElysiaTrialEventBase> It(World); It; ++It)
	{
		AElysiaTrialEventBase* TrialEvent = *It;
		if (IsValidTrialEventActor(TrialEvent))
		{
			return TrialEvent;
		}
	}

	return nullptr;
}

AElysiaTrialInteractableActor* UElysiaTrialInteractionComponent::FindBestTrialOfferToInteract() const
{
	return GetCurrentInteractableTrialOfferActor();
}

bool UElysiaTrialInteractionComponent::IsValidTrialOfferActor(const AElysiaTrialInteractableActor* TrialOffer) const
{
	if (!IsValid(TrialOffer))
	{
		return false;
	}

	const AElysiaTrialEventBase* TrialEvent = TrialOffer->GetTrialEvent();
	return !TrialEvent || TrialEvent->IsWaitingToBeTriggered();
}

bool UElysiaTrialInteractionComponent::IsValidTrialEventActor(const AElysiaTrialEventBase* TrialEvent) const
{
	if (!IsValid(TrialEvent) || TrialEvent->IsWaitingToBeTriggered() || TrialEvent->IsFinished() && !TrialEvent->HasExpired())
	{
		return false;
	}

	const AActor* InteractingActor = GetInteractingActor();
	const AActor* TriggeringActor = TrialEvent->GetTriggeringActor();
	return !TriggeringActor || TriggeringActor == InteractingActor;
}

AActor* UElysiaTrialInteractionComponent::GetInteractingActor() const
{
	const AController* OwnerController = Cast<AController>(GetOwner());
	if (OwnerController)
	{
		return OwnerController->GetPawn();
	}

	return GetOwner();
}
