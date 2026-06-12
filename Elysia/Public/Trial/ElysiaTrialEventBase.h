// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ElysiaTrialEventBase.generated.h"

class USphereComponent;
class UPrimitiveComponent;
class USceneComponent;
class AElysiaTrialInteractableActor;

UENUM(BlueprintType)
enum class EElysiaTrialEventState : uint8
{
	WaitingToBeTriggered,
	Triggered,
	Completed,
	Expired,
	Cancelled
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnTrialEventFinishedSignature, class AElysiaTrialEventBase*);

UCLASS(Abstract, Blueprintable)
class ELYSIA_API AElysiaTrialEventBase : public AActor
{
	GENERATED_BODY()

public:

	AElysiaTrialEventBase();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void InitializeTrial(
		AActor* InSpawnPoint,
		float InUntriggeredLifetime,
		float InTrialDuration,
		TSubclassOf<AElysiaTrialInteractableActor> InTrialOfferActorClass);

	UFUNCTION(BlueprintCallable, Category = "Trial")
	void TriggerTrial(AActor* TriggerActor);

	UFUNCTION(BlueprintCallable, Category = "Trial")
	void CompleteTrial();

	UFUNCTION(BlueprintCallable, Category = "Trial")
	void CancelTrial();

	UFUNCTION(BlueprintPure, Category = "Trial")
	EElysiaTrialEventState GetTrialEventState() const { return TrialEventState; }

	UFUNCTION(BlueprintPure, Category = "Trial")
	bool IsWaitingToBeTriggered() const { return TrialEventState == EElysiaTrialEventState::WaitingToBeTriggered; }

	UFUNCTION(BlueprintPure, Category = "Trial")
	bool HasBeenTriggered() const { return TrialEventState == EElysiaTrialEventState::Triggered; }

	UFUNCTION(BlueprintPure, Category = "Trial")
	bool HasExpired() const { return TrialEventState == EElysiaTrialEventState::Expired; }

	UFUNCTION(BlueprintPure, Category = "Trial")
	bool IsFinished() const;

	UFUNCTION(BlueprintPure, Category = "Trial")
	bool CanBeTriggeredBy(AActor* CandidateActor) const;

	UFUNCTION(BlueprintPure, Category = "Trial")
	float GetRemainingOfferTime() const;

	UFUNCTION(BlueprintPure, Category = "Trial")
	float GetTotalOfferTime() const { return OfferLifetime; }

	UFUNCTION(BlueprintPure, Category = "Trial")
	float GetRemainingTrialTime() const;

	UFUNCTION(BlueprintPure, Category = "Trial")
	float GetTotalTrialTime() const { return TrialDuration; }

	UFUNCTION(BlueprintPure, Category = "Trial")
	AElysiaTrialInteractableActor* GetTrialOfferActor() const { return TrialOfferActor; }

	UFUNCTION(BlueprintPure, Category = "Trial")
	AActor* GetTriggeringActor() const { return TriggeringActor; }

	UFUNCTION(BlueprintPure, Category = "Trial|Indicator")
	virtual FVector GetIndicatorTargetLocation() const;

	UFUNCTION(BlueprintPure, Category = "Trial|Indicator")
	virtual bool ShouldShowActiveTrialDirectionIndicator() const;

	UFUNCTION(BlueprintPure, Category = "Trial|Indicator")
	virtual bool ShouldShowIndicatorCountdown() const;

	UFUNCTION(BlueprintPure, Category = "Trial|Rewards")
	virtual FVector GetCompletionRewardLocation() const;

	FOnTrialEventFinishedSignature OnTrialEventFinished;

protected:

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<USphereComponent> TriggerSphere;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trial|Overlap Trigger")
	bool bEnableOverlapTrigger = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trial|Overlap Trigger", meta = (ClampMin = "0.0"))
	float TriggerRadius = 180.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trial")
	bool bDestroyWhenTriggered = false;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Trial")
	float TrialDuration = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trial|UI", meta = (ClampMin = "0.0"))
	float ExpiredStateDisplayDuration = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trial|Overlap Trigger")
	bool bDisableTriggerCollisionWhenTriggered = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trial|Offer")
	bool bSpawnTrialOfferActor = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trial|Offer")
	FVector TrialOfferSpawnOffset = FVector::ZeroVector;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Trial")
	TObjectPtr<AActor> SpawnPoint;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Trial")
	TObjectPtr<AActor> TriggeringActor;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Trial|Offer")
	TObjectPtr<AElysiaTrialInteractableActor> TrialOfferActor;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Trial|Offer")
	float OfferLifetime = 0.f;

	virtual bool CanTriggerTrial(AActor* CandidateActor) const;
	virtual void HandleTrialTriggered(AActor* TriggerActor) {}
	virtual void HandleTrialCompleted() {}
	virtual void HandleTrialExpired() {}
	virtual void HandleTrialCancelled() {}

private:

	UFUNCTION()
	void HandleTriggerSphereBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	void ExpireTrial();
	void ClearOfferExpirationTimer();
	void ClearTrialExpirationTimer();
	void ClearAllExpirationTimers();
	void StartTrialExpirationTimer();
	void BroadcastTrialFinished();
	void SpawnTrialOfferActor(TSubclassOf<AElysiaTrialInteractableActor> InTrialOfferActorClass);
	void DestroyTrialOfferActor();
	float GetCurrentServerWorldTime() const;

	FTimerHandle ExpirationTimerHandle;
	FTimerHandle TrialExpirationTimerHandle;

	UPROPERTY(Replicated)
	EElysiaTrialEventState TrialEventState = EElysiaTrialEventState::WaitingToBeTriggered;

	UPROPERTY(Replicated)
	float OfferStartedServerTime = 0.f;

	UPROPERTY(Replicated)
	float OfferExpirationServerTime = 0.f;

	UPROPERTY(Replicated)
	float TrialStartedServerTime = 0.f;

	UPROPERTY(Replicated)
	float TrialExpirationServerTime = 0.f;

	bool bTrialFinishedBroadcasted = false;
};
