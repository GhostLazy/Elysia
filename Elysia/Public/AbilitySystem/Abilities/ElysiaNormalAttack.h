// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "ElysiaNormalAttack.generated.h"

class AElysiaProjectile;
/**
 * 
 */
UCLASS()
class ELYSIA_API UElysiaNormalAttack : public UGameplayAbility
{
	GENERATED_BODY()
	
protected:
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                             const FGameplayAbilityActivationInfo ActivationInfo,
	                             const FGameplayEventData* TriggerEventData) override;
	
	UFUNCTION(BlueprintCallable)
	void SpawnProjectile(const AActor* TargetActor) const;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AElysiaProjectile> ProjectileClass;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> DamageEffectClass;
};
