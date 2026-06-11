// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/Enemy/ElysiaBossGameplayAbility.h"
#include "ElysiaBossWindFieldAbility.generated.h"

class AElysiaBossTornado;

UCLASS()
class ELYSIA_API UElysiaBossWindFieldAbility : public UElysiaBossGameplayAbility
{
	GENERATED_BODY()

protected:

	virtual void ExecuteBossSkill() override;
	virtual FGameplayTag GetDefaultExecuteGameplayCueTag() const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|Wind Field")
	TSubclassOf<AElysiaBossTornado> TornadoClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|Wind Field", meta = (ClampMin = "1"))
	int32 TornadoCount = 2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|Wind Field", meta = (ClampMin = "0.0"))
	float SpawnRadius = 250.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|Wind Field", meta = (ClampMin = "0.0"))
	float TrackingSpeed = 350.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|Wind Field", meta = (ClampMin = "0.01"))
	float TornadoDuration = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|Wind Field", meta = (ClampMin = "0.0"))
	float ContactRadius = 80.f;
};
