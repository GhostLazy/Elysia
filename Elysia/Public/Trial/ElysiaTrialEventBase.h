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

	void InitializeTrial(AActor* InSpawnPoint, float InUntriggeredLifetime, TSubclassOf<AElysiaTrialInteractableActor> InTrialOfferActorClass);

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
	bool CanBeTriggeredBy(AActor* CandidateActor) const;

	UFUNCTION(BlueprintPure, Category = "Trial")
	float GetRemainingOfferTime() const;

	UFUNCTION(BlueprintPure, Category = "Trial")
	float GetTotalOfferTime() const { return OfferLifetime; }

	UFUNCTION(BlueprintPure, Category = "Trial")
	AElysiaTrialInteractableActor* GetTrialOfferActor() const { return TrialOfferActor; }

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

	UFUNCTION(BlueprintNativeEvent, Category = "Trial")
	bool CanTriggerTrial(AActor* CandidateActor) const;
	virtual bool CanTriggerTrial_Implementation(AActor* CandidateActor) const;

	UFUNCTION(BlueprintNativeEvent, Category = "Trial")
	void OnTrialOffered();
	virtual void OnTrialOffered_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category = "Trial")
	void OnTrialTriggered(AActor* TriggerActor);
	virtual void OnTrialTriggered_Implementation(AActor* TriggerActor);

	UFUNCTION(BlueprintNativeEvent, Category = "Trial")
	void OnTrialCompleted();
	virtual void OnTrialCompleted_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category = "Trial")
	void OnTrialExpired();
	virtual void OnTrialExpired_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category = "Trial")
	void OnTrialCancelled();
	virtual void OnTrialCancelled_Implementation();

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
	void ClearExpirationTimer();
	void BroadcastTrialFinished();
	void SpawnTrialOfferActor(TSubclassOf<AElysiaTrialInteractableActor> InTrialOfferActorClass);
	void DestroyTrialOfferActor();
	float GetCurrentServerWorldTime() const;

	FTimerHandle ExpirationTimerHandle;

	UPROPERTY(Replicated)
	EElysiaTrialEventState TrialEventState = EElysiaTrialEventState::WaitingToBeTriggered;

	UPROPERTY(Replicated)
	float OfferStartedServerTime = 0.f;

	UPROPERTY(Replicated)
	float OfferExpirationServerTime = 0.f;

	bool bTrialFinishedBroadcasted = false;
};
