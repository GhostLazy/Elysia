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

bool UElysiaTrialInteractionComponent::ShouldShowTrialDirectionIndicator() const
{
	return GetCurrentTrialOfferActor() && !GetCurrentInteractableTrialOfferActor();
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

AActor* UElysiaTrialInteractionComponent::GetInteractingActor() const
{
	const AController* OwnerController = Cast<AController>(GetOwner());
	if (OwnerController)
	{
		return OwnerController->GetPawn();
	}

	return GetOwner();
}
