// Copyright GhostLazy


#include "AbilitySystem/Abilities/Character/ElysiaOffensiveGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/ElysiaAttributeSet.h"
#include "GameplayEffect.h"

UElysiaOffensiveGameplayAbility::UElysiaOffensiveGameplayAbility()
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UElysiaOffensiveGameplayAbility::ApplyCooldown(
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

	CooldownSpecHandle.Data->SetDuration(GetCooldownDurationWithReduction(ActorInfo), true);
	AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*CooldownSpecHandle.Data.Get());
}

float UElysiaOffensiveGameplayAbility::GetCooldownDurationWithReduction(const FGameplayAbilityActorInfo* ActorInfo) const
{
	const UAbilitySystemComponent* AbilitySystemComponent = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
	if (!AbilitySystemComponent)
	{
		return FMath::Max(0.f, CooldownDuration);
	}

	const float CooldownReduction = AbilitySystemComponent->GetNumericAttribute(UElysiaAttributeSet::GetCoolDownAttribute());
	const float ClampedMaxReduction = FMath::Clamp(MaxCooldownReduction, 0.f, 1.f);
	const float ClampedCooldownReduction = FMath::Clamp(CooldownReduction, 0.f, ClampedMaxReduction);

	return FMath::Max(0.f, CooldownDuration * (1.f - ClampedCooldownReduction));
}
