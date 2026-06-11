// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/Enemy/ElysiaBossGameplayAbility.h"
#include "ElysiaBossArrowAbility.generated.h"

class AElysiaBossProjectile;

UCLASS()
class ELYSIA_API UElysiaBossArrowAbility : public UElysiaBossGameplayAbility
{
	GENERATED_BODY()

public:

	UElysiaBossArrowAbility();

protected:

	virtual void ExecuteBossSkill() override;
	virtual bool ShouldAutoEndAfterExecute() const override { return false; }
	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;
	virtual FGameplayTag GetDefaultExecuteGameplayCueTag() const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|Arrow")
	TSubclassOf<AElysiaBossProjectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|Arrow")
	FName MuzzleSocketName = FName("Muzzle");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|Arrow")
	FVector ProjectileSpawnOffset = FVector(100.f, 0.f, 50.f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|Arrow", meta = (ClampMin = "0.0"))
	float ProjectileSpeed = 900.f;

	// Boss 未施放其他技能时，每隔该时间重新索敌并发射一箭。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|Arrow", meta = (ClampMin = "0.01"))
	float ShotInterval = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss Ability|Arrow", meta = (ClampMin = "1"))
	int32 ShotsPerCast = 3;

private:

	void FireArrow();
	AActor* FindClosestPlayer() const;

	FTimerHandle ShotTimerHandle;
	int32 FiredShotCount = 0;
};
