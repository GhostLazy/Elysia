// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/ElysiaProjectileAbility.h"
#include "ElysiaSmartBulletAssistant.generated.h"

class AElysiaSmartBulletOrb;

UCLASS()
class ELYSIA_API UElysiaSmartBulletAssistant : public UElysiaProjectileAbility
{
	GENERATED_BODY()

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

	UPROPERTY(EditDefaultsOnly, Category = "Smart Bullet|Orb")
	TSubclassOf<AElysiaSmartBulletOrb> OrbClass;

	UPROPERTY(EditDefaultsOnly, Category = "Smart Bullet|Orb", meta = (ClampMin = "1"))
	int32 BaseOrbCount = 1;

	UPROPERTY(EditDefaultsOnly, Category = "Smart Bullet|Orb", meta = (ClampMin = "0"))
	int32 EvolvedBonusOrbCount = 1;

	UPROPERTY(EditDefaultsOnly, Category = "Smart Bullet|Orb", meta = (ClampMin = "0.0"))
	float OrbOrbitRadius = 80.f;

	UPROPERTY(EditDefaultsOnly, Category = "Smart Bullet|Orb")
	float OrbHeightOffset = 90.f;

	UPROPERTY(EditDefaultsOnly, Category = "Smart Bullet|Orb", meta = (ClampMin = "0.0"))
	float OrbFollowInterpSpeed = 6.f;

	UPROPERTY(EditDefaultsOnly, Category = "Smart Bullet|Fire", meta = (ClampMin = "0.01"))
	float FireInterval = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = "Smart Bullet|Fire", meta = (ClampMin = "1"))
	int32 BulletsPerOrb = 4;

	UPROPERTY(EditDefaultsOnly, Category = "Smart Bullet|Fire", meta = (ClampMin = "0.0"))
	float VolleySpreadAngle = 24.f;

	UPROPERTY(EditDefaultsOnly, Category = "Smart Bullet|Fire", meta = (ClampMin = "0.0"))
	float ProjectileMovementSpeed = 900.f;

	UPROPERTY(EditDefaultsOnly, Category = "Smart Bullet|Targeting", meta = (ClampMin = "0.0"))
	float TargetSearchRadius = 1200.f;

	UPROPERTY(EditDefaultsOnly, Category = "Smart Bullet|Explosion")
	TSubclassOf<AElysiaProjectile> ExplosionProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category = "Smart Bullet|Explosion")
	TSubclassOf<UGameplayEffect> ExplosionDamageEffectClass;

	UPROPERTY(EditDefaultsOnly, Category = "Smart Bullet|Explosion", meta = (ClampMin = "0.0"))
	float ExplosionRadius = 300.f;

	UPROPERTY(EditDefaultsOnly, Category = "Smart Bullet|Explosion", meta = (ClampMin = "1"))
	int32 ExplosionUnlockLevel = 3;

	UPROPERTY(EditDefaultsOnly, Category = "Smart Bullet|Explosion", meta = (ClampMin = "1"))
	int32 ExplosionShotInterval = 15;

	UPROPERTY(EditDefaultsOnly, Category = "Smart Bullet|Explosion", meta = (ClampMin = "1"))
	int32 EvolvedExplosionShotInterval = 10;

private:

	void SpawnOrUpdateOrbs();
	void DestroyOrbs();
	void FireFromOrbs();
	void FireBulletFromOrb(AElysiaSmartBulletOrb* Orb, int32 OrbIndex, int32 BulletIndex, int32 BulletCount);
	AActor* FindTarget(const FVector& Origin) const;
	int32 GetDesiredOrbCount() const;
	int32 GetCurrentExplosionShotInterval() const;
	bool ShouldFireExplosionShot(int32 OrbIndex);

	UPROPERTY()
	TArray<TObjectPtr<AElysiaSmartBulletOrb>> ActiveOrbs;

	TArray<int32> FiredBulletCountsPerOrb;
};
