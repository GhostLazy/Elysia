// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "ElysiaSwordSlashDamageMagnitudeCalculation.generated.h"

/**
 * 捕获攻击者的 Attack，并结合 Ability 写入的 SetByCaller 倍率计算剑气伤害。
 * 返回负值，供 GameplayEffect 以 Add 方式修改目标 Health。
 */
UCLASS()
class ELYSIA_API UElysiaSwordSlashDamageMagnitudeCalculation : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()

public:

	UElysiaSwordSlashDamageMagnitudeCalculation();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
};
