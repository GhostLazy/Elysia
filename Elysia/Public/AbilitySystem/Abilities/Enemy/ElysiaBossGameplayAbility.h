// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "ElysiaBossGameplayAbility.generated.h"

class AElysiaBossBase;
class UGameplayEffect;

/**
 * Base class for Boss abilities. Boss AI selects these abilities through GAS,
 * while each derived ability owns its own gameplay and presentation parameters.
 */
UCLASS(Abstract)
class ELYSIA_API UElysiaBossGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:

	UElysiaBossGameplayAbility();

	virtual bool CanActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr,
		const FGameplayTagContainer* TargetTags = nullptr,
		FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	bool IsTargetInRange(const AElysiaBossBase* Boss) const;

protected:

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

	virtual void ExecuteBossSkill();
	virtual bool ShouldAutoEndAfterExecute() const { return true; }
	void FinishBossAbility();

	AElysiaBossBase* GetBossAvatar(const FGameplayAbilityActorInfo* ActorInfo = nullptr) const;
	AActor* GetCombatTarget() const;
	FVector GetLockedSkillDirection() const { return LockedSkillDirection; }
	bool ApplyDamageToTarget(AActor* TargetActor) const;

	virtual FGameplayTag GetDefaultExecuteGameplayCueTag() const;
	virtual void BuildExecuteGameplayCueParameters(
		FGameplayCueParameters& OutParameters,
		AElysiaBossBase* Boss,
		const FVector& Origin,
		const FVector& Direction) const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|Range", meta = (ClampMin = "0.0"))
	float MinRange = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|Range", meta = (ClampMin = "0.0"))
	float CastRange = 600.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|Damage", meta = (ClampMin = "0.0"))
	float DamageEffectLevel = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|GameplayCue")
	FGameplayTag ExecuteGameplayCueTagOverride;

private:

	FVector CalculateTargetDirection() const;
	void ExecuteBossAbilityCue(FGameplayTag CueTag, const FGameplayCueParameters& Parameters) const;
	void BuildBaseGameplayCueParameters(
		FGameplayCueParameters& OutParameters,
		AElysiaBossBase* Boss,
		const FVector& Origin,
		const FVector& Direction) const;
	FGameplayTag GetExecuteGameplayCueTag() const;

	FVector LockedSkillDirection = FVector::ForwardVector;
	bool bNotifiedBossAbilityActive = false;
};
