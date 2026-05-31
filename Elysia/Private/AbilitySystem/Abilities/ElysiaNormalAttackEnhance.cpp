// Copyright GhostLazy


#include "AbilitySystem/Abilities/ElysiaNormalAttackEnhance.h"

#include "AbilitySystem/ElysiaAbilitySystemComponent.h"
#include "GameplayEffect.h"

UElysiaNormalAttackEnhance::UElysiaNormalAttackEnhance()
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
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

void UElysiaNormalAttackEnhance::ApplyCooldown(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo) const
{
	UGameplayEffect* CooldownEffect = GetCooldownGameplayEffect();
	UAbilitySystemComponent* AbilitySystemComponent = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
	if (!CooldownEffect || !AbilitySystemComponent)
	{
		return;
	}

	FGameplayEffectSpecHandle CooldownSpecHandle = MakeOutgoingGameplayEffectSpec(
		Handle,
		ActorInfo,
		ActivationInfo,
		CooldownEffect->GetClass(),
		GetAbilityLevel(Handle, ActorInfo));
	if (!CooldownSpecHandle.IsValid())
	{
		return;
	}

	CooldownSpecHandle.Data->SetDuration(FMath::Max(0.f, CooldownDuration), true);
	AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*CooldownSpecHandle.Data.Get());
}
