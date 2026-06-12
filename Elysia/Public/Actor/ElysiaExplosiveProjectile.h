// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "Actor/ElysiaProjectile.h"
#include "GameplayTagContainer.h"
#include "ElysiaExplosiveProjectile.generated.h"

UCLASS()
class ELYSIA_API AElysiaExplosiveProjectile : public AElysiaProjectile
{
	GENERATED_BODY()

public:

	void SetExplosionRadius(float InExplosionRadius);

protected:

	virtual void HandleEnemyHit(AActor* OtherActor) override;

	UPROPERTY(EditDefaultsOnly, Category = "Explosion", meta = (ClampMin = "0.0"))
	float ExplosionRadius = 300.f;

	UPROPERTY(EditDefaultsOnly, Category = "Explosion|GameplayCue", meta = (Categories = "GameplayCue"))
	FGameplayTag ExplosionGameplayCueTag;

private:

	void ExecuteExplosionGameplayCue() const;

	bool bHasExploded = false;
};
