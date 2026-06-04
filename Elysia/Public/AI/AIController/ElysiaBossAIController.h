// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "AI/AIController/ElysiaAIControllerBase.h"
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

	UFUNCTION(BlueprintPure, Category = "AI")
	AElysiaBossBase* GetControlledBoss() const;

	UFUNCTION(BlueprintCallable, Category = "AI")
	bool TryActivateBestBossAbility();

protected:
	
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void UpdateBehavior() override;
	virtual void OnTargetActorChanged(AActor* NewTargetActor) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Abilities", meta = (ClampMin = "0.0"))
	float BossAbilityCooldownTime = 3.f;

private:

	void BindToBoss(AElysiaBossBase* Boss);
	void UnbindFromBoss(AElysiaBossBase* Boss);
	void HandleBossAbilityFinished();
	bool IsBossAbilityCooldownReady() const;
	AActor* FindFallbackCombatTarget() const;
	static bool IsValidCombatTarget(const AActor* Actor);

	TWeakObjectPtr<AElysiaBossBase> BoundBoss;
	float NextAllowedBossAbilityTime = 0.f;
};
