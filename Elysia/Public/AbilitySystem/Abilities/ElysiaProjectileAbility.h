// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/ElysiaDamageGameplayAbility.h"
#include "ElysiaProjectileAbility.generated.h"

class AElysiaProjectile;
/**
 * 
 */
UCLASS()
class ELYSIA_API UElysiaProjectileAbility : public UElysiaDamageGameplayAbility
{
	GENERATED_BODY()
	
protected:
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AElysiaProjectile> ProjectileClass;
	
	FTimerHandle SpawnProjectileTimer;
	
	// 清理子弹生成定时器
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                        const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility,
	                        bool bWasCancelled) override;
	
	// 根据武器等级获取基础连发次数
	int32 GetBaseProjectileCount(TArray<int32>& ProjectileCountByLevel) const;
	
};
