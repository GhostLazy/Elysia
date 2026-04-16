// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "ElysiaAbilityType.generated.h"

/**
 * 
 */

USTRUCT(Blueprintable)
struct FDamageEffectParams
{
	GENERATED_BODY()
};

USTRUCT(BlueprintType)
struct FElysiaGameplayEffectContext : public FGameplayEffectContext
{
	GENERATED_BODY()
};