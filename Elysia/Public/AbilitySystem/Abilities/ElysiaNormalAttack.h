// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Character/ElysiaCharacterBase.h"
#include "ElysiaNormalAttack.generated.h"

class AElysiaProjectile;
class UElysiaEquipmentComponent;
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

	int32 GetBaseProjectileCount() const;
	int32 GetProjectileCount() const;
	bool IsWeaponEvolved() const;
	UElysiaEquipmentComponent* GetEquipmentComponent() const;
	void FireProjectileVolley(const FVector& SpawnLocation, const FRotator& SpawnRotation, int32 ArrowsPerVolley) const;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AElysiaProjectile> ProjectileClass;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> DamageEffectClass;
	
	UPROPERTY()
	FOnAttributeChangeSignature OnAttackSpeedChanged;
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TArray<int32> ProjectileCountByLevel = { 1, 2, 3, 5 };

	UPROPERTY(EditDefaultsOnly, Category = "Weapon", meta = (ClampMin = "0.0"))
	float BurstShotInterval = 0.08f;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon", meta = (ClampMin = "0.0"))
	float EvolvedPairSpacing = 18.f;
	
	FTimerHandle SpawnProjectileTimer;
	
private:
	
	// 用作定时器回调函数，周期性执行普攻动作
	void PlayAttackMontage();
	
	UPROPERTY()
	TObjectPtr<AActor> TargetActor;
	
	float Interval = 1.f;
	
};
