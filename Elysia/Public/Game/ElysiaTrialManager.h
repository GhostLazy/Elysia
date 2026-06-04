// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "Game/ElysiaGameTypes.h"
#include "GameFramework/Actor.h"
#include "ElysiaTrialManager.generated.h"

class AElysiaTrialEventBase;
class AElysiaTrialSpawnPoint;

USTRUCT(BlueprintType)
struct FElysiaTrialEventEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trial")
	TSubclassOf<AElysiaTrialEventBase> TrialEventClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trial", meta = (ClampMin = "0.0"))
	float Weight = 1.f;
};

UCLASS()
class ELYSIA_API AElysiaTrialManager : public AActor
{
	GENERATED_BODY()

public:

	AElysiaTrialManager();

	UFUNCTION(BlueprintCallable, Category = "Trial")
	void StartPhaseTrial(EElysiaRunPhase Phase, int32 PhaseIndex);

	UFUNCTION(BlueprintCallable, Category = "Trial")
	void StopPhaseTrial();

	UFUNCTION(BlueprintCallable, Category = "Trial")
	void StopAllTrials();

protected:

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trial")
	TArray<FElysiaTrialEventEntry> TrialEventPool;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trial", meta = (ClampMin = "0.0"))
	float TrialSpawnDelay = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trial", meta = (ClampMin = "0.0"))
	float UntriggeredTrialLifetime = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trial")
	bool bEnableTrialsInNormalPhase = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trial")
	bool bEnableTrialsInBossPhase = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trial")
	bool bEnableTrialsInFinalBossPhase = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trial")
	bool bCancelTriggeredTrialOnPhaseEnd = true;

private:

	void RefreshTrialSpawnPoints();
	void HandleTrialSpawnTimer();
	void HandleActiveTrialFinished(AElysiaTrialEventBase* FinishedTrial);
	bool ShouldScheduleTrialForPhase(EElysiaRunPhase Phase) const;
	AElysiaTrialSpawnPoint* ChooseTrialSpawnPoint() const;
	TSubclassOf<AElysiaTrialEventBase> ChooseTrialEventClass() const;
	void CancelActiveTrialForPhaseTransition();

	UPROPERTY()
	TArray<TObjectPtr<AElysiaTrialSpawnPoint>> CachedTrialSpawnPoints;

	TWeakObjectPtr<AElysiaTrialEventBase> ActiveTrialEvent;
	FTimerHandle TrialSpawnTimerHandle;
	EElysiaRunPhase CurrentPhase = EElysiaRunPhase::Finished;
	int32 CurrentPhaseIndex = 0;
};
