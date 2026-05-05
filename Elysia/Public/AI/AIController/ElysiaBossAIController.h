// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "AI/AIController/ElysiaAIControllerBase.h"
#include "Game/ElysiaGameTypes.h"
#include "ElysiaBossAIController.generated.h"

class AElysiaBossBase;

/**
 * 
 */
UCLASS()
class ELYSIA_API AElysiaBossAIController : public AElysiaAIControllerBase
{
	GENERATED_BODY()

public:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void StartAI() override;
	virtual void StopAI() override;

	UFUNCTION(BlueprintCallable, Category = "AI")
	void RefreshTarget();

	UFUNCTION(BlueprintPure, Category = "AI")
	AElysiaBossBase* GetControlledBoss() const;

	UFUNCTION(BlueprintCallable, Category = "AI")
	bool MoveToCurrentTarget();

	UFUNCTION(BlueprintCallable, Category = "AI")
	bool TryCastBestSkill();

	UFUNCTION(BlueprintPure, Category = "AI")
	bool CanCastSkill(EElysiaBossSkillType SkillType) const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI", meta = (ClampMin = "0.0"))
	float SearchRadius = 3000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI", meta = (ClampMin = "0.01"))
	float RetargetInterval = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI", meta = (ClampMin = "0.0"))
	float ChaseAcceptanceRadius = 200.f;

private:
	AActor* FindClosestLivePlayerInRange() const;

	FTimerHandle RetargetTimerHandle;
};
