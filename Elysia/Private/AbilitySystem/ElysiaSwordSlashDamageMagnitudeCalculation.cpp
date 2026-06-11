// Copyright GhostLazy

#include "AbilitySystem/ElysiaSwordSlashDamageMagnitudeCalculation.h"

#include "AbilitySystem/ElysiaAttributeSet.h"
#include "ElysiaGameplayTags.h"
#include "GameplayEffectExecutionCalculation.h"

namespace
{
	struct FElysiaSwordSlashDamageStatics
	{
		FGameplayEffectAttributeCaptureDefinition AttackDef;

		FElysiaSwordSlashDamageStatics()
			: AttackDef(
				UElysiaAttributeSet::GetAttackAttribute(),
				EGameplayEffectAttributeCaptureSource::Source,
				false)
		{
		}
	};

	const FElysiaSwordSlashDamageStatics& GetSwordSlashDamageStatics()
	{
		static FElysiaSwordSlashDamageStatics Statics;
		return Statics;
	}
}

UElysiaSwordSlashDamageMagnitudeCalculation::UElysiaSwordSlashDamageMagnitudeCalculation()
{
	RelevantAttributesToCapture.Add(GetSwordSlashDamageStatics().AttackDef);
}

float UElysiaSwordSlashDamageMagnitudeCalculation::CalculateBaseMagnitude_Implementation(
	const FGameplayEffectSpec& Spec) const
{
	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvaluationParameters.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	float Attack = 0.f;
	GetCapturedAttributeMagnitude(
		GetSwordSlashDamageStatics().AttackDef,
		Spec,
		EvaluationParameters,
		Attack);

	const float DamageMultiplier = Spec.GetSetByCallerMagnitude(
		FElysiaGameplayTags::Get().Data_DamageMultiplier,
		false,
		0.f);

	return -FMath::Max(0.f, Attack) * FMath::Max(0.f, DamageMultiplier);
}
