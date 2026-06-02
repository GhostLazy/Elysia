// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/ElysiaBossGameplayAbility.h"
#include "ElysiaBossSweepAbility.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;
class USoundBase;

UCLASS()
class ELYSIA_API UElysiaBossSweepAbility : public UElysiaBossGameplayAbility
{
	GENERATED_BODY()

protected:

	virtual void ExecuteBossSkill() override;
	virtual void OnBossAbilityWindupStarted_Implementation(AElysiaBossBase* Boss, FVector Origin, FVector Direction) override;
	virtual void OnBossAbilityExecuted_Implementation(AElysiaBossBase* Boss, FVector Origin, FVector Direction) override;
	virtual void OnBossAbilityRecovered_Implementation(AElysiaBossBase* Boss) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|Sweep", meta = (ClampMin = "0.0"))
	float SweepRadius = 350.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|Sweep", meta = (ClampMin = "0.0", ClampMax = "360.0"))
	float SweepAngle = 360.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|Sweep|VFX")
	TObjectPtr<UNiagaraSystem> SweepWarningEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|Sweep|VFX")
	TObjectPtr<UNiagaraSystem> SweepImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|Sweep|VFX")
	TObjectPtr<USoundBase> SweepImpactSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|Sweep|VFX")
	FName NiagaraRadiusParameterName = FName("User.Radius");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|Sweep|VFX")
	FName NiagaraAngleParameterName = FName("User.Angle");

private:

	void ApplySweepEffectParameters(UNiagaraComponent* NiagaraComponent) const;
	void DestroyWarningEffect();

	UPROPERTY()
	TObjectPtr<UNiagaraComponent> ActiveWarningEffect;
};
