// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/ElysiaBossGameplayAbility.h"
#include "ElysiaBossLaserAbility.generated.h"

class AElysiaBossLaserActor;
class UNiagaraComponent;
class UNiagaraSystem;
class USoundBase;

UCLASS()
class ELYSIA_API UElysiaBossLaserAbility : public UElysiaBossGameplayAbility
{
	GENERATED_BODY()

protected:

	virtual void ExecuteBossSkill() override;
	virtual float GetPostExecuteRecoveryTime() const override;
	virtual void OnBossAbilityWindupStarted_Implementation(AElysiaBossBase* Boss, FVector Origin, FVector Direction) override;
	virtual void OnBossAbilityExecuted_Implementation(AElysiaBossBase* Boss, FVector Origin, FVector Direction) override;
	virtual void OnBossAbilityRecovered_Implementation(AElysiaBossBase* Boss) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|Laser", meta = (ClampMin = "0.0"))
	float LaserLength = 1200.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|Laser", meta = (ClampMin = "0.0"))
	float LaserWidth = 180.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|Laser", meta = (ClampMin = "0.0"))
	float LaserDuration = 1.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|Laser", meta = (ClampMin = "0.01"))
	float LaserTickInterval = 0.2f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|Laser")
	TSubclassOf<AElysiaBossLaserActor> LaserActorClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|Laser|VFX")
	TObjectPtr<UNiagaraSystem> LaserWarningEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|Laser|VFX")
	TObjectPtr<UNiagaraSystem> LaserFireEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|Laser|VFX")
	TObjectPtr<USoundBase> LaserFireSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|Laser|VFX")
	FName NiagaraLengthParameterName = FName("User.Length");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|Laser|VFX")
	FName NiagaraWidthParameterName = FName("User.Width");

private:

	void ApplyLaserEffectParameters(UNiagaraComponent* NiagaraComponent, float Duration) const;
	void DestroyWarningEffect();

	UPROPERTY()
	TObjectPtr<UNiagaraComponent> ActiveWarningEffect;

	UPROPERTY()
	TObjectPtr<AElysiaBossLaserActor> ActiveLaserActor;
};
