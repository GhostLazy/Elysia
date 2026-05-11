// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "ElysiaAIControllerBase.generated.h"

class UStateTreeAIComponent;
/**
 * 
 */
UCLASS()
class ELYSIA_API AElysiaAIControllerBase : public AAIController
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "AI")
	void SetTargetActor(AActor* InTargetActor);

	UFUNCTION(BlueprintPure, Category = "AI")
	AActor* GetTargetActor() const { return TargetActor; }

	UFUNCTION(BlueprintCallable, Category = "AI")
	void ClearTargetActor();

	UFUNCTION(BlueprintPure, Category = "AI")
	bool HasValidTarget() const;

	UFUNCTION(BlueprintCallable, Category = "AI")
	virtual void StartAI();

	UFUNCTION(BlueprintCallable, Category = "AI")
	virtual void StopAI();
	
	UFUNCTION(BlueprintCallable, Category = "AI")
	void RefreshTarget();
	
	UFUNCTION(BlueprintCallable, Category = "AI")
	bool MoveToCurrentTarget();
	
protected:
	
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	
	AActor* FindClosestPlayerInRange() const;
	
	virtual void UpdateBehavior();
	virtual void OnTargetActorChanged(AActor* NewTargetActor);
	void StartBehaviorTimer();
	void StopBehaviorTimer();
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "AI")
	TObjectPtr<AActor> TargetActor;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI", meta = (ClampMin = "0.0"))
	float SearchRadius = 2000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI", meta = (ClampMin = "0.0"))
	float AcceptanceRadius = 5.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI", meta = (ClampMin = "0.05"))
	float BehaviorTickInterval = 0.2f;
	
private:
	
	FTimerHandle BehaviorTimerHandle;
	
};
