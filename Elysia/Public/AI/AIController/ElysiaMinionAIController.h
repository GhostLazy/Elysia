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

protected:
	
	virtual void UpdateBehavior() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Soft Separation", meta = (ClampMin = "100.0"))
	float SeparationLookAheadDistance = 500.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Soft Separation", meta = (ClampMin = "0.0"))
	float SeparationSteeringScale = 1.5f;
	
};
