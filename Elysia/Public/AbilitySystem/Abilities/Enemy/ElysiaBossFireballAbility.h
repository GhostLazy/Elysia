// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/Enemy/ElysiaBossGameplayAbility.h"
#include "ElysiaBossFireballAbility.generated.h"

class AElysiaBossFallingFireball;

UCLASS()
class ELYSIA_API UElysiaBossFireballAbility : public UElysiaBossGameplayAbility
{
	GENERATED_BODY()

protected:

	virtual void ExecuteBossSkill() override;
	virtual bool ShouldAutoEndAfterExecute() const override { return false; }
	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;
	virtual FGameplayTag GetDefaultExecuteGameplayCueTag() const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|Fireball")
	TSubclassOf<AElysiaBossFallingFireball> FireballClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|Fireball", meta = (ClampMin = "1"))
	int32 SummonCount = 5;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|Fireball", meta = (ClampMin = "0.01"))
	float SummonInterval = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|Fireball", meta = (ClampMin = "0.0"))
	float SpawnHeight = 1000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|Fireball", meta = (ClampMin = "0.01"))
	float FallDuration = 0.8f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|Fireball", meta = (ClampMin = "0.0"))
	float ImpactRadius = 150.f;

private:

	void SummonNextFireball();
	AActor* FindClosestPlayer() const;

	FTimerHandle SummonTimerHandle;
	int32 SummonedFireballCount = 0;
};
