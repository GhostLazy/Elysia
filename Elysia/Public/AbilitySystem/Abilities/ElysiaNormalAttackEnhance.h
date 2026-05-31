// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/ElysiaOffensiveGameplayAbility.h"
#include "ElysiaNormalAttackEnhance.generated.h"

class UGameplayEffect;

UCLASS()
class ELYSIA_API UElysiaNormalAttackEnhance : public UElysiaOffensiveGameplayAbility
{
	GENERATED_BODY()

public:

	UElysiaNormalAttackEnhance();

protected:

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
	TSubclassOf<UGameplayEffect> EnhanceEffectClass;

};
