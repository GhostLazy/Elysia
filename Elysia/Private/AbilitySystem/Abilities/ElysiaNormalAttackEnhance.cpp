// Copyright GhostLazy


#include "AbilitySystem/Abilities/ElysiaNormalAttackEnhance.h"

#include "AbilitySystem/ElysiaAbilitySystemComponent.h"
#include "GameplayEffect.h"

UElysiaNormalAttackEnhance::UElysiaNormalAttackEnhance()
{
	CooldownDuration = 24.f;
}

void UElysiaNormalAttackEnhance::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UElysiaAbilitySystemComponent* ElysiaASC = ActorInfo
		? Cast<UElysiaAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get())
		: nullptr;
	if (!ElysiaASC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (EnhanceEffectClass)
	{
		FGameplayEffectContextHandle EffectContext = ElysiaASC->MakeEffectContext();
		EffectContext.AddSourceObject(this);

		const FGameplayEffectSpecHandle EffectSpecHandle = ElysiaASC->MakeOutgoingSpec(
			EnhanceEffectClass,
			GetAbilityLevel(),
			EffectContext);
		if (EffectSpecHandle.IsValid())
		{
			ElysiaASC->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
