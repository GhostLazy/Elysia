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
	float Remaining = 0.f;
	float Duration = 0.f;
	GetCooldownRemainingAndDuration(CooldownTag, Remaining, Duration);
	return Remaining;
}

bool UElysiaAbilitySystemComponent::GetCooldownRemainingAndDuration(FGameplayTag CooldownTag, float& OutRemaining, float& OutDuration) const
{
	if (!CooldownTag.IsValid())
	{
		OutRemaining = 0.f;
		OutDuration = 0.f;
		return false;
	}

	FGameplayTagContainer CooldownTags;
	CooldownTags.AddTag(CooldownTag);

	const FGameplayEffectQuery CooldownQuery = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(CooldownTags);
	const TArray<TPair<float, float>> DurationAndTimeRemaining = GetActiveEffectsTimeRemainingAndDuration(CooldownQuery);

	OutRemaining = 0.f;
	OutDuration = 0.f;
	for (const TPair<float, float>& ActiveCooldownTime : DurationAndTimeRemaining)
	{
		const float Remaining = FMath::Max(0.f, ActiveCooldownTime.Key);
		if (Remaining > OutRemaining)
		{
			OutRemaining = Remaining;
			OutDuration = FMath::Max(0.f, ActiveCooldownTime.Value);
		}
	}

	return OutRemaining > 0.f;
}

FElysiaCooldownInfo UElysiaAbilitySystemComponent::GetCooldownInfo(FGameplayTag CooldownTag) const
{
	FElysiaCooldownInfo CooldownInfo;
	CooldownInfo.bIsActive = GetCooldownRemainingAndDuration(
		CooldownTag,
		CooldownInfo.Remaining,
		CooldownInfo.Duration);
	CooldownInfo.Percent = CooldownInfo.Duration > 0.f
		? FMath::Clamp(CooldownInfo.Remaining / CooldownInfo.Duration, 0.f, 1.f)
		: 0.f;

	return CooldownInfo;
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
