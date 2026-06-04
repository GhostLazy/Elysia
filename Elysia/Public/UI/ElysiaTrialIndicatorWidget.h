// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "UI/ElysiaUserWidget.h"
#include "ElysiaTrialIndicatorWidget.generated.h"

class AElysiaTrialEventBase;
class AElysiaTrialInteractableActor;

UCLASS()
class ELYSIA_API UElysiaTrialIndicatorWidget : public UElysiaUserWidget
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Trial|Indicator")
	void SetTrialOfferActor(AElysiaTrialInteractableActor* InTrialOfferActor);

	UFUNCTION(BlueprintPure, Category = "Trial|Indicator")
	AElysiaTrialInteractableActor* GetTrialOfferActor() const { return TrialOfferActor; }

	UFUNCTION(BlueprintPure, Category = "Trial|Indicator")
	AElysiaTrialEventBase* GetTrialEvent() const;

	UFUNCTION(BlueprintPure, Category = "Trial|Indicator")
	float GetRemainingOfferTime() const;

	UFUNCTION(BlueprintPure, Category = "Trial|Indicator")
	float GetTotalOfferTime() const;

	UFUNCTION(BlueprintPure, Category = "Trial|Indicator")
	FText GetRemainingOfferTimeText() const;

	UFUNCTION(BlueprintPure, Category = "Trial|Indicator")
	FVector GetTargetWorldLocation() const;

protected:

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Trial|Indicator")
	void TrialOfferActorSet();

	UFUNCTION(BlueprintImplementableEvent, Category = "Trial|Indicator")
	void TrialIndicatorDataUpdated(float RemainingTime, float TotalTime);

	UPROPERTY(BlueprintReadOnly, Category = "Trial|Indicator")
	TObjectPtr<AElysiaTrialInteractableActor> TrialOfferActor;
};
