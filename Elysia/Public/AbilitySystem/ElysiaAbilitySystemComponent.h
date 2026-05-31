// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "ElysiaAbilitySystemComponent.generated.h"

/**
 * 
 */
UCLASS()
class ELYSIA_API UElysiaAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
	
public:
	
	void AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities);
	void GrantOrUpdateAbilityLevel(TSubclassOf<UGameplayAbility> AbilityClass, int32 AbilityLevel);

	UFUNCTION(BlueprintPure, Category = "Cooldown")
	bool IsCooldownActive(FGameplayTag CooldownTag) const;

	UFUNCTION(BlueprintPure, Category = "Cooldown")
	float GetCooldownRemaining(FGameplayTag CooldownTag) const;

	void ReduceCooldownRemaining(FGameplayTag CooldownTag, float Reduction);
	
};
