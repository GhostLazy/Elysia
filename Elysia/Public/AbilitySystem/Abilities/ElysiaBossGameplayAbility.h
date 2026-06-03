// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "ElysiaBossGameplayAbility.generated.h"

class AElysiaBossBase;
class UNiagaraComponent;
class UNiagaraSystem;
class UAnimMontage;
class UGameplayEffect;
class USoundBase;

/**
 * Base class for Boss abilities. Boss AI selects these abilities through GAS,
 * while each derived ability owns its own gameplay and presentation parameters.
 */
UCLASS(Abstract)
class ELYSIA_API UElysiaBossGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:

	UElysiaBossGameplayAbility();

	virtual bool CanActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr,
		const FGameplayTagContainer* TargetTags = nullptr,
		FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	bool IsTargetInRange(const AElysiaBossBase* Boss) const;

protected:

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

	virtual void ExecuteBossSkill();
	virtual bool ShouldAutoRecoverAfterExecute() const { return true; }
	virtual float GetPostExecuteRecoveryTime() const { return RecoveryTime; }

	void BeginBossAbilityRecovery(float Delay);
	void FinishBossAbilityRecovery();

	AElysiaBossBase* GetBossAvatar(const FGameplayAbilityActorInfo* ActorInfo = nullptr) const;
	AActor* GetCombatTarget() const;
	FVector GetLockedSkillDirection() const { return LockedSkillDirection; }
	bool ApplyDamageToTarget(AActor* TargetActor) const;

	UNiagaraComponent* SpawnEffectAtLocation(UNiagaraSystem* Effect, const FVector& Location, const FVector& Direction) const;
	UNiagaraComponent* SpawnEffectAttachedToBoss(UNiagaraSystem* Effect, const FVector& RelativeLocation, const FVector& Direction) const;

	UFUNCTION(BlueprintNativeEvent, Category = "Boss Ability")
	void OnBossAbilityWindupStarted(AElysiaBossBase* Boss, FVector Origin, FVector Direction);
	virtual void OnBossAbilityWindupStarted_Implementation(AElysiaBossBase* Boss, FVector Origin, FVector Direction);

	UFUNCTION(BlueprintNativeEvent, Category = "Boss Ability")
	void OnBossAbilityExecuted(AElysiaBossBase* Boss, FVector Origin, FVector Direction);
	virtual void OnBossAbilityExecuted_Implementation(AElysiaBossBase* Boss, FVector Origin, FVector Direction);

	UFUNCTION(BlueprintNativeEvent, Category = "Boss Ability")
	void OnBossAbilityRecovered(AElysiaBossBase* Boss);
	virtual void OnBossAbilityRecovered_Implementation(AElysiaBossBase* Boss);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|Range", meta = (ClampMin = "0.0"))
	float MinRange = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|Range", meta = (ClampMin = "0.0"))
	float CastRange = 600.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|Timing", meta = (ClampMin = "0.0"))
	float WindupTime = 0.4f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|Timing", meta = (ClampMin = "0.0"))
	float RecoveryTime = 0.6f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|Timing")
	TObjectPtr<UAnimMontage> CastMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|Damage", meta = (ClampMin = "0.0"))
	float DamageEffectLevel = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|VFX")
	TObjectPtr<USoundBase> WindupSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|VFX")
	TObjectPtr<USoundBase> ExecuteSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|VFX")
	TObjectPtr<USoundBase> RecoveredSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|VFX")
	FVector EffectLocationOffset = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|VFX")
	FName NiagaraDurationParameterName = FName("User.Duration");

private:

	void ExecuteBossSkillAfterWindup();
	void ClearBossAbilityTimers();
	FVector CalculateTargetDirection() const;

	FTimerHandle WindupTimerHandle;
	FTimerHandle RecoveryTimerHandle;
	FVector LockedSkillDirection = FVector::ForwardVector;
	bool bNotifiedBossAbilityActive = false;
};
