// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ElysiaGameModeBase.generated.h"

class AElysiaSpawnManager;

/**
 * 
 */
UCLASS()
class ELYSIA_API AElysiaGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

protected:

	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "Spawn")
	TSubclassOf<AElysiaSpawnManager> SpawnManagerClass;

private:

	UPROPERTY()
	TObjectPtr<AElysiaSpawnManager> SpawnManager;
};
