// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/ElysiaProjectileAbility.h"
#include "ElysiaChalkLauncher.generated.h"

/**
 * 
 */
UCLASS()
class ELYSIA_API UElysiaChalkLauncher : public UElysiaProjectileAbility
{
	GENERATED_BODY()
	
protected:
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
								 const FGameplayAbilityActivationInfo ActivationInfo,
								 const FGameplayEventData* TriggerEventData) override;
	
	// 每隔一段时间生成粉笔头
	UFUNCTION(BlueprintCallable)
	void SpawnProjectile();
	
	UPROPERTY(EditDefaultsOnly)
	TArray<int32> ProjectileCountByLevel = { 5, 5, 10, 10 };
	
	UPROPERTY(EditDefaultsOnly)
	int32 EvolvedProjectileCount = 21;
	
	UPROPERTY(EditDefaultsOnly)
	float MovementSpeed = 1000.f;
	
	// 基础攻击间隔
	float Interval = 5.f;
	
};
