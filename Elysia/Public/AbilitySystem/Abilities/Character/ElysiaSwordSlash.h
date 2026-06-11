// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/Character/ElysiaDamageGameplayAbility.h"
#include "ElysiaSwordSlash.generated.h"

struct FOnAttributeChangeData;

/**
 * 在角色周围的圆环上随机生成若干圆形剑气区域。
 * 服务端负责随机位置、范围查询与伤害结算，GameplayCue 负责客户端表现。
 */
UCLASS()
class ELYSIA_API UElysiaSwordSlash : public UElysiaDamageGameplayAbility
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

	// 1/2/2/4级对应的单次剑气数量。
	UPROPERTY(EditDefaultsOnly, Category = "Sword Slash|Level")
	TArray<int32> SlashCountByLevel = { 1, 2, 2, 4 };

	// 随机圆心到角色位置的固定距离。
	UPROPERTY(EditDefaultsOnly, Category = "Sword Slash|Area", meta = (ClampMin = "0.0"))
	float SpawnRingRadius = 500.f;

	// 单个剑气圆形区域的基础半径。
	UPROPERTY(EditDefaultsOnly, Category = "Sword Slash|Area", meta = (ClampMin = "0.0"))
	float BaseDamageRadius = 240.f;

	// 将随机圆心投影到地面，避免GameplayCue悬在角色胶囊中心高度。
	UPROPERTY(EditDefaultsOnly, Category = "Sword Slash|Area", meta = (ClampMin = "0.0"))
	float GroundTraceVerticalExtent = 1000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Sword Slash|Area")
	float GroundEffectOffset = 5.f;

	UPROPERTY(EditDefaultsOnly, Category = "Sword Slash|Evolution", meta = (ClampMin = "0.0"))
	float EvolvedRangeMultiplier = 1.66f;

	UPROPERTY(EditDefaultsOnly, Category = "Sword Slash|Debug")
	bool bDrawDebugDamageSphere = true;

	UPROPERTY(EditDefaultsOnly, Category = "Sword Slash|Debug", meta = (ClampMin = "0.0"))
	float DebugSphereDuration = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = "Sword Slash|Debug", meta = (ClampMin = "0.0"))
	float DebugSphereThickness = 2.f;

	// 攻速为1时的基础释放间隔。
	UPROPERTY(EditDefaultsOnly, Category = "Sword Slash|Timing", meta = (ClampMin = "0.01"))
	float BaseAttackInterval = 3.f;

	UPROPERTY(EditDefaultsOnly, Category = "Sword Slash|GameplayCue")
	FGameplayTag SwordSlashGameplayCueTag;

private:

	void ExecuteSlashVolley();
	void BindAttackSpeedChanged();
	void UnbindAttackSpeedChanged();
	void HandleAttackSpeedChanged(const FOnAttributeChangeData& Data);
	void ResetAttackTimer(float AttackSpeed);
	FVector MakeRandomSlashCenter(const FVector& AvatarLocation) const;
	void ExecuteSlashGameplayCue(const FVector& Center, float DamageRadius) const;
	int32 GetCurrentSlashCount() const;
	float GetCurrentDamageRadius() const;

	FTimerHandle SlashTimerHandle;
	FDelegateHandle AttackSpeedChangedHandle;
};
