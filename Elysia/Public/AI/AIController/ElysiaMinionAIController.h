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
	
};
