// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Character/ElysiaCharacterBase.h"
#include "ElysiaNormalAttack.generated.h"

class AElysiaProjectile;
/**
 * 
 */
UCLASS()
class ELYSIA_API UElysiaNormalAttack : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	
	UElysiaNormalAttack();
	
protected:
	
	// 角色普攻在GAS初始化后即激活
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                             const FGameplayAbilityActivationInfo ActivationInfo,
	                             const FGameplayEventData* TriggerEventData) override;
	
	// 接受动画事件时，生成子弹
	UFUNCTION(BlueprintCallable)
	void SpawnProjectile(FGameplayEventData Payload);
	
	// 当攻速属性发生变化时，重设普攻间隔
	UFUNCTION()
	void ResetTimer(float NewAttackSpeed);
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AElysiaProjectile> ProjectileClass;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> DamageEffectClass;
	
	UPROPERTY()
	FOnAttributeChangeSignature OnAttackSpeedChanged;
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UAnimMontage> AttackMontage;
	
	FTimerHandle SpawnProjectileTimer;
	
private:
	
	// 用作定时器回调函数，周期性执行普攻动作
	void PlayAttackMontage();
	
	UPROPERTY()
	TObjectPtr<AActor> TargetActor;
	
	float Interval = 1.f;
	
};
