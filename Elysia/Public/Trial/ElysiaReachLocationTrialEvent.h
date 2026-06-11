// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "Trial/ElysiaTrialEventBase.h"
#include "ElysiaReachLocationTrialEvent.generated.h"

UCLASS(Blueprintable)
class ELYSIA_API AElysiaReachLocationTrialEvent : public AElysiaTrialEventBase
{
	GENERATED_BODY()

public:

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintPure, Category = "Trial|Reach Location")
	AActor* GetDestinationActor() const { return DestinationActor; }

	UFUNCTION(BlueprintPure, Category = "Trial|Reach Location")
	FVector GetDestinationLocation() const { return DestinationLocation; }

	UFUNCTION(BlueprintPure, Category = "Trial|Reach Location")
	float GetReachRadius() const { return ReachRadius; }

	UFUNCTION(BlueprintPure, Category = "Trial|Reach Location")
	float GetDistanceToDestination(AActor* CandidateActor) const;

	virtual FVector GetIndicatorTargetLocation() const override;
	virtual bool ShouldShowActiveTrialDirectionIndicator() const override;
	virtual FVector GetCompletionRewardLocation() const override;

protected:

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void HandleTrialTriggered(AActor* TriggerActor) override;
	virtual void HandleTrialCompleted() override;
	virtual void HandleTrialExpired() override;
	virtual void HandleTrialCancelled() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trial|Reach Location")
	TSubclassOf<AActor> DestinationActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trial|Reach Location", meta = (ClampMin = "0.0"))
	float DestinationRadius = 1800.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trial|Reach Location", meta = (ClampMin = "0.0"))
	float ReachRadius = 220.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trial|Reach Location", meta = (ClampMin = "0.01"))
	float ReachCheckInterval = 0.1f;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Trial|Reach Location")
	TObjectPtr<AActor> DestinationActor;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Trial|Reach Location")
	FVector DestinationLocation = FVector::ZeroVector;

private:

	void SpawnDestinationActor();
	void DestroyDestinationActor();
	void ClearReachCheckTimer();
	void CheckReachCondition();
	bool HasReachedDestination(AActor* CandidateActor) const;

	FTimerHandle ReachCheckTimerHandle;
};
