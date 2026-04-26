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
	
	AElysiaAIControllerBase();

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
	
protected:
	
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UStateTreeAIComponent> StateTreeAIComponent;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "AI")
	TObjectPtr<AActor> TargetActor;
	
};
