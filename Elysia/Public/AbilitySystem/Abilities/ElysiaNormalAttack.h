// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "ElysiaProjectileAbility.h"
#include "Abilities/GameplayAbility.h"
#include "Character/ElysiaCharacterBase.h"
#include "ElysiaNormalAttack.generated.h"

class AElysiaProjectile;
class UElysiaEquipmentComponent;
/**
 * 
 */
UCLASS()
class ELYSIA_API UElysiaNormalAttack : public UElysiaProjectileAbility
{
	GENERATED_BODY()
	
protected:
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                             const FGameplayAbilityActivationInfo ActivationInfo,
	                             const FGameplayEventData* TriggerEventData) override;
	
	// 接受动画事件时，生成子弹
	UFUNCTION(BlueprintCallable)
	void SpawnProjectile(FGameplayEventData Payload);
	
	// 当攻速属性发生变化时，重设普攻间隔
	UFUNCTION()
	void ResetTimer(float NewAttackSpeed);
	
	// 执行一次齐射；进化前为单箭，进化后为并排双箭
	void FireProjectileVolley(const FVector& SpawnLocation, const FRotator& SpawnRotation, int32 ArrowsPerVolley) const;
	
	UPROPERTY()
	FOnAttributeChangeSignature OnAttackSpeedChanged;
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TArray<int32> ProjectileCountByLevel = { 1, 2, 3, 5 };

	// 同一轮普攻内，多次连发之间的时间间隔
	UPROPERTY(EditDefaultsOnly, Category = "Weapon", meta = (ClampMin = "0.0"))
	float BurstShotInterval = 0.1f;

	// 武器进化后，并排双箭之间的横向间距
	UPROPERTY(EditDefaultsOnly, Category = "Weapon", meta = (ClampMin = "0.0"))
	float EvolvedPairSpacing = 18.f;
	
private:
	
	// 用作定时器回调函数，周期性寻找目标并执行普攻动作
	void FindTargetAndPlayMontage();
	
	UPROPERTY()
	TObjectPtr<AActor> TargetActor;
	
	// 基础攻击间隔
	float Interval = 1.f;
	
};
