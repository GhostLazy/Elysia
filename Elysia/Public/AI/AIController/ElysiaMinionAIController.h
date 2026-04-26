// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "AI/AIController/ElysiaAIControllerBase.h"
#include "ElysiaMinionAIController.generated.h"

class UStateTree;
/**
 * 
 */
UCLASS()
class ELYSIA_API AElysiaMinionAIController : public AElysiaAIControllerBase
{
	GENERATED_BODY()

public:

	virtual void StartAI() override;
	virtual void StopAI() override;

	UFUNCTION(BlueprintCallable, Category = "AI")
	void RefreshTarget();

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|StateTree")
	TObjectPtr<UStateTree> MinionStateTree = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI", meta = (ClampMin = "0.05"))
	float BehaviorTickInterval = 0.2f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI", meta = (ClampMin = "0.0"))
	float SearchRadius = 2000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI", meta = (ClampMin = "0.0"))
	float AcceptanceRadius = 120.f;

	AActor* FindClosestPlayerInRange() const;
	void UpdateBehavior();
	void StartBehaviorTimer();
	void StopBehaviorTimer();

	FTimerHandle BehaviorTimerHandle;
};
