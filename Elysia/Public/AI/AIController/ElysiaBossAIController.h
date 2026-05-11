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

	UFUNCTION(BlueprintPure, Category = "AI")
	AElysiaBossBase* GetControlledBoss() const;

	UFUNCTION(BlueprintCallable, Category = "AI")
	bool TryCastBestSkill();

protected:
	
	virtual void UpdateBehavior() override;
	virtual void OnTargetActorChanged(AActor* NewTargetActor) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI", meta = (ClampMin = "0.01"))
	float RetargetInterval = 0.5f;

private:

	FTimerHandle RetargetTimerHandle;
	
};
