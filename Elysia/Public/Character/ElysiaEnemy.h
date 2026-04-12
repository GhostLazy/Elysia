// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "Character/ElysiaCharacterBase.h"
#include "ElysiaEnemy.generated.h"

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
	
};
