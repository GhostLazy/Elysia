// Copyright GhostLazy


#include "AbilitySystem/ElysiaAbilitySystemComponent.h"

void UElysiaAbilitySystemComponent::AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities)
{
	for (const TSubclassOf<UGameplayAbility>& AbilityClass : StartupAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		GiveAbility(AbilitySpec);
	}
}

void UElysiaAbilitySystemComponent::AddCharacterPassiveAbilities(
	const TArray<TSubclassOf<UGameplayAbility>>& StartupPassiveAbilities)
{
	for (const TSubclassOf<UGameplayAbility>& AbilityClass : StartupPassiveAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		GiveAbilityAndActivateOnce(AbilitySpec);
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

	GiveAbility(FGameplayAbilitySpec(AbilityClass, ClampedAbilityLevel));
}
