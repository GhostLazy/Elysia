// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Game/ElysiaBossTypes.h"
#include "ElysiaBossLaserActor.generated.h"

class AElysiaBossBase;

UCLASS()
class ELYSIA_API AElysiaBossLaserActor : public AActor
{
	GENERATED_BODY()

public:
	AElysiaBossLaserActor();

	void InitializeLaser(AElysiaBossBase* InSourceBoss, const FVector& InOrigin, const FVector& InDirection, const FElysiaBossSkillSpec& InSkillSpec);

protected:
	virtual void BeginPlay() override;

private:
	void HandleDamageTick();
	void FinishLaser();

	TWeakObjectPtr<AElysiaBossBase> SourceBoss;
	FElysiaBossSkillSpec SkillSpec;
	FVector Origin = FVector::ZeroVector;
	FVector Direction = FVector::ForwardVector;
	FTimerHandle DamageTickTimerHandle;
	FTimerHandle FinishTimerHandle;
};
