// Copyright GhostLazy


#include "AbilitySystem/ElysiaAbilitySystemComponent.h"

#include "GameplayEffect.h"

void UElysiaAbilitySystemComponent::AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities)
{
	for (const TSubclassOf<UGameplayAbility>& AbilityClass : StartupAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		GiveAbility(AbilitySpec);
	}
}

void UElysiaAbilitySystemComponent::GrantOrUpdateAbilityLevel(TSubclassOf<UGameplayAbility> AbilityClass, int32 AbilityLevel)
{
	if (!AbilityClass)
	{
		return;
	}

	const int32 ClampedAbilityLevel = FMath::Max(1, AbilityLevel);
	
	ABILITYLIST_SCOPE_LOCK();
	for (FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		if (AbilitySpec.Ability && AbilitySpec.Ability->GetClass() == AbilityClass)
		{
			if (AbilitySpec.Level != ClampedAbilityLevel)
			{
				AbilitySpec.Level = ClampedAbilityLevel;
				MarkAbilitySpecDirty(AbilitySpec);
			}
			return;
		}
	}

	FGameplayAbilitySpec Spec = FGameplayAbilitySpec(AbilityClass, ClampedAbilityLevel);
	GiveAbilityAndActivateOnce(Spec);
}

bool UElysiaAbilitySystemComponent::IsCooldownActive(FGameplayTag CooldownTag) const
{
	return GetCooldownRemaining(CooldownTag) > 0.f;
}

float UElysiaAbilitySystemComponent::GetCooldownRemaining(FGameplayTag CooldownTag) const
{
	if (!CooldownTag.IsValid())
	{
		return 0.f;
	}

	FGameplayTagContainer CooldownTags;
	CooldownTags.AddTag(CooldownTag);

	const FGameplayEffectQuery CooldownQuery = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(CooldownTags);
	const TArray<TPair<float, float>> DurationAndTimeRemaining = GetActiveEffectsTimeRemainingAndDuration(CooldownQuery);

	float LongestRemainingTime = 0.f;
	for (const TPair<float, float>& ActiveCooldownTime : DurationAndTimeRemaining)
	{
		LongestRemainingTime = FMath::Max(LongestRemainingTime, ActiveCooldownTime.Key);
	}

	return LongestRemainingTime;
}

void UElysiaAbilitySystemComponent::ReduceCooldownRemaining(FGameplayTag CooldownTag, float Reduction)
{
	if (!GetWorld() || !CooldownTag.IsValid() || Reduction <= 0.f)
	{
		return;
	}

	FGameplayTagContainer CooldownTags;
	CooldownTags.AddTag(CooldownTag);

	const FGameplayEffectQuery CooldownQuery = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(CooldownTags);
	const float CurrentTime = GetWorld()->GetTimeSeconds();
	for (const FActiveGameplayEffectHandle& ActiveEffectHandle : GetActiveEffects(CooldownQuery))
	{
		const FActiveGameplayEffect* ActiveEffect = GetActiveGameplayEffect(ActiveEffectHandle);
		if (!ActiveEffect)
		{
			continue;
		}

		const float RemainingTime = ActiveEffect->GetTimeRemaining(CurrentTime);
		if (RemainingTime <= 0.f)
		{
			continue;
		}

		if (RemainingTime <= Reduction)
		{
			RemoveActiveGameplayEffect(ActiveEffectHandle);
		}
		else
		{
			ModifyActiveEffectStartTime(ActiveEffectHandle, Reduction);
		}
	}
}
