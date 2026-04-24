// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
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
	void AddCharacterPassiveAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupPassiveAbilities);
	void GrantOrUpdateAbilityLevel(TSubclassOf<UGameplayAbility> AbilityClass, int32 AbilityLevel);
	
};
