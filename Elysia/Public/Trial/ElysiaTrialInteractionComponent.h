// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ElysiaTrialInteractionComponent.generated.h"

class AElysiaTrialInteractableActor;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ELYSIA_API UElysiaTrialInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UElysiaTrialInteractionComponent();

	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintPure, Category = "Trial|Interaction")
	AElysiaTrialInteractableActor* GetCurrentTrialOfferActor() const;

	UFUNCTION(BlueprintPure, Category = "Trial|Interaction")
	AElysiaTrialInteractableActor* GetCurrentInteractableTrialOfferActor() const;

	UFUNCTION(BlueprintPure, Category = "Trial|Interaction")
	bool ShouldShowTrialDirectionIndicator() const;

	UFUNCTION(BlueprintPure, Category = "Trial|Interaction")
	bool ShouldShowTrialInteractPrompt() const;

	UFUNCTION(BlueprintCallable, Category = "Trial|Interaction")
	bool TryInteractWithCurrentTrialOffer();

protected:

	UPROPERTY(EditDefaultsOnly, Category = "Trial|Interaction", meta = (ClampMin = "0.01"))
	float TrialOfferCacheRefreshInterval = 0.1f;

private:

	UFUNCTION(Server, Reliable)
	void ServerInteractWithTrialOffer(AElysiaTrialInteractableActor* TrialOffer);

	void UpdateCachedTrialOfferActor();
	AElysiaTrialInteractableActor* FindBestTrialOffer() const;
	AElysiaTrialInteractableActor* FindBestTrialOfferToInteract() const;
	bool IsValidTrialOfferActor(const AElysiaTrialInteractableActor* TrialOffer) const;
	AActor* GetInteractingActor() const;

	TWeakObjectPtr<AElysiaTrialInteractableActor> CurrentTrialOfferActor;
	float TrialOfferCacheRefreshAccumulator = 0.f;
};
