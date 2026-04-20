// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "ScalableFloat.h"
#include "Character/ElysiaCharacterBase.h"
#include "ElysiaEnemy.generated.h"

class AElysiaXPBall;
/**
 * 
 */
UCLASS()
class ELYSIA_API AElysiaEnemy : public AElysiaCharacterBase
{
	GENERATED_BODY()

public:
	
	AElysiaEnemy();

protected:

	virtual void BeginPlay() override;
	virtual void Die() override;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AElysiaXPBall> XPBallClass;
	
	UPROPERTY(EditDefaultsOnly)
	FScalableFloat XPRewards = FScalableFloat();
	
	UPROPERTY(EditDefaultsOnly)
	int32 Level = 1;
	
};
