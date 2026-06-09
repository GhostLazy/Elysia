// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/Enemy/ElysiaBossGameplayAbility.h"
#include "ElysiaBossLaserAbility.generated.h"

class AElysiaBossLaserActor;

UCLASS()
class ELYSIA_API UElysiaBossLaserAbility : public UElysiaBossGameplayAbility
{
	GENERATED_BODY()

protected:

	virtual void ExecuteBossSkill() override;
	virtual float GetPostExecuteRecoveryTime() const override;
	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;
	virtual FGameplayTag GetDefaultWindupGameplayCueTag() const override;
	virtual FGameplayTag GetDefaultExecuteGameplayCueTag() const override;
	virtual void BuildWindupGameplayCueParameters(
		FGameplayCueParameters& OutParameters,
		AElysiaBossBase* Boss,
		const FVector& Origin,
		const FVector& Direction) const override;
	virtual void BuildExecuteGameplayCueParameters(
		FGameplayCueParameters& OutParameters,
		AElysiaBossBase* Boss,
		const FVector& Origin,
		const FVector& Direction) const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|Laser", meta = (ClampMin = "0.0"))
	float LaserLength = 1200.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|Laser", meta = (ClampMin = "0.0"))
	float LaserWidth = 180.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|Laser", meta = (ClampMin = "0.0"))
	float LaserDuration = 1.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|Laser", meta = (ClampMin = "0.01"))
	float LaserTickInterval = 0.2f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|Laser")
	TSubclassOf<AElysiaBossLaserActor> LaserActorClass;

private:

	UPROPERTY()
	TObjectPtr<AElysiaBossLaserActor> ActiveLaserActor;
};
