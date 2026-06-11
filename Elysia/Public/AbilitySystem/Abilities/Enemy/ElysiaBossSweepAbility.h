// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/Enemy/ElysiaBossGameplayAbility.h"
#include "ElysiaBossSweepAbility.generated.h"

UCLASS()
class ELYSIA_API UElysiaBossSweepAbility : public UElysiaBossGameplayAbility
{
	GENERATED_BODY()

protected:

	virtual void ExecuteBossSkill() override;
	virtual FGameplayTag GetDefaultExecuteGameplayCueTag() const override;
	virtual void BuildExecuteGameplayCueParameters(
		FGameplayCueParameters& OutParameters,
		AElysiaBossBase* Boss,
		const FVector& Origin,
		const FVector& Direction) const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|Sweep", meta = (ClampMin = "0.0"))
	float SweepRadius = 350.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|Sweep", meta = (ClampMin = "0.0", ClampMax = "360.0"))
	float SweepAngle = 360.f;

};
