// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ElysiaTrialSpawnPoint.generated.h"

class UArrowComponent;
class USceneComponent;

UCLASS()
class ELYSIA_API AElysiaTrialSpawnPoint : public AActor
{
	GENERATED_BODY()

public:

	AElysiaTrialSpawnPoint();

	UFUNCTION(BlueprintPure, Category = "Trial")
	bool IsTrialSpawnEnabled() const { return bEnabled; }

	UFUNCTION(BlueprintPure, Category = "Trial")
	FTransform GetTrialSpawnTransform() const;

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UArrowComponent> ArrowComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trial")
	bool bEnabled = true;
};
