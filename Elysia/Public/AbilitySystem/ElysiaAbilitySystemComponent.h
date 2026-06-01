// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "ElysiaAbilitySystemComponent.generated.h"

USTRUCT(BlueprintType)
struct ELYSIA_API FElysiaCooldownInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Cooldown")
	bool bIsActive = false;

	UPROPERTY(BlueprintReadOnly, Category = "Cooldown")
	float Remaining = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Cooldown")
	float Duration = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Cooldown")
	float Percent = 0.f;
};

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

	UFUNCTION(BlueprintPure, Category = "Cooldown")
	bool GetCooldownRemainingAndDuration(FGameplayTag CooldownTag, float& OutRemaining, float& OutDuration) const;

	UFUNCTION(BlueprintPure, Category = "Cooldown")
	FElysiaCooldownInfo GetCooldownInfo(FGameplayTag CooldownTag) const;

	void ReduceCooldownRemaining(FGameplayTag CooldownTag, float Reduction);
	
};
