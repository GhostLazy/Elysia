// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/Enemy/ElysiaBossGameplayAbility.h"
#include "ElysiaBossChargeAbility.generated.h"

UCLASS()
class ELYSIA_API UElysiaBossChargeAbility : public UElysiaBossGameplayAbility
{
	GENERATED_BODY()

protected:

	virtual void ExecuteBossSkill() override;
	virtual bool ShouldAutoRecoverAfterExecute() const override { return false; }
	virtual FGameplayTag GetDefaultWindupGameplayCueTag() const override;
	virtual FGameplayTag GetDefaultExecuteGameplayCueTag() const override;
	virtual void BuildWindupGameplayCueParameters(
		FGameplayCueParameters& OutParameters,
		AElysiaBossBase* Boss,
		const FVector& Origin,
		const FVector& Direction) const override;
	virtual void BuildExecuteGameplayCueParameters(
		FGameplayCueParameters& OutParameters,
		AElysiaBossBase* Boss,
		const FVector& Origin,
		const FVector& Direction) const override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|Charge", meta = (ClampMin = "0.0"))
	float ChargeSpeed = 1800.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|Charge", meta = (ClampMin = "0.0"))
	float ChargeMaxDistance = 1000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|Charge", meta = (ClampMin = "0.01"))
	float ChargeTickInterval = 0.02f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|Charge", meta = (ClampMin = "0.0"))
	float ChargeHitRadiusPadding = 50.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|Charge")
	bool bStopChargeOnFirstHit = false;

private:

	void TickChargeMovement();
	void StopCharge(bool bInterrupted);

	FTimerHandle ChargeTickTimerHandle;
	FVector ChargeStartLocation = FVector::ZeroVector;
	TSet<TWeakObjectPtr<AActor>> ChargeHitTargets;
	bool bChargeActive = false;
};
