// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ElysiaTrialInteractableActor.generated.h"

class AElysiaTrialEventBase;
class USceneComponent;
class USphereComponent;
class UStaticMeshComponent;
class UWidgetComponent;
class UPrimitiveComponent;

UCLASS(Blueprintable)
class ELYSIA_API AElysiaTrialInteractableActor : public AActor
{
	GENERATED_BODY()

public:

	AElysiaTrialInteractableActor();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void InitializeTrialOffer(AElysiaTrialEventBase* InTrialEvent);

	UFUNCTION(BlueprintCallable, Category = "Trial|Interaction")
	bool Interact(AActor* InteractingActor);

	UFUNCTION(BlueprintPure, Category = "Trial|Interaction")
	bool CanInteract(AActor* InteractingActor) const;

	UFUNCTION(BlueprintPure, Category = "Trial")
	AElysiaTrialEventBase* GetTrialEvent() const { return TrialEvent; }

	UFUNCTION(BlueprintPure, Category = "Trial|Indicator")
	float GetRemainingOfferTime() const;

	UFUNCTION(BlueprintPure, Category = "Trial|Indicator")
	float GetTotalOfferTime() const;

	UFUNCTION(BlueprintPure, Category = "Trial|Indicator")
	FVector GetIndicatorTargetLocation() const;

	UFUNCTION(BlueprintCallable, Category = "Trial|Indicator")
	void SetIndicatorVisible(bool bVisible);

protected:

	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<USphereComponent> InteractionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UWidgetComponent> IndicatorWidgetComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trial|Interaction", meta = (ClampMin = "0.0"))
	float InteractionRadius = 220.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trial|Indicator")
	bool bShowIndicatorWhileWaiting = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trial|Indicator")
	FVector IndicatorRelativeLocation = FVector(0.f, 0.f, 180.f);

	UFUNCTION(BlueprintNativeEvent, Category = "Trial|Interaction")
	void OnTrialOfferInitialized(AElysiaTrialEventBase* InTrialEvent);
	virtual void OnTrialOfferInitialized_Implementation(AElysiaTrialEventBase* InTrialEvent);

	UFUNCTION(BlueprintNativeEvent, Category = "Trial|Interaction")
	void OnTrialOfferInteracted(AActor* InteractingActor);
	virtual void OnTrialOfferInteracted_Implementation(AActor* InteractingActor);

	UFUNCTION(BlueprintNativeEvent, Category = "Trial|Interaction")
	void OnInteractorEnteredInteractionRange(AActor* InteractingActor);
	virtual void OnInteractorEnteredInteractionRange_Implementation(AActor* InteractingActor);

	UFUNCTION(BlueprintNativeEvent, Category = "Trial|Interaction")
	void OnInteractorLeftInteractionRange(AActor* InteractingActor);
	virtual void OnInteractorLeftInteractionRange_Implementation(AActor* InteractingActor);

private:

	UFUNCTION()
	void HandleInteractionSphereBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void HandleInteractionSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	UFUNCTION()
	void OnRep_TrialEvent();

	void RefreshComponentSettings();
	void InitializeIndicatorWidget();

	UPROPERTY(ReplicatedUsing = OnRep_TrialEvent)
	TObjectPtr<AElysiaTrialEventBase> TrialEvent;
};
