// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "ElysiaOffensiveGameplayAbility.generated.h"

/**
 * Base class for player offensive active abilities.
 */
UCLASS(Abstract)
class ELYSIA_API UElysiaOffensiveGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:

	UElysiaOffensiveGameplayAbility();

	virtual void ApplyCooldown(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) const override;

protected:

	virtual float GetCooldownDurationWithReduction(const FGameplayAbilityActorInfo* ActorInfo) const;

	// Base cooldown before applying UElysiaAttributeSet::CoolDown.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cooldown", meta = (ClampMin = "0.0"))
	float CooldownDuration = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cooldown", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MaxCooldownReduction = 0.8f;
};
