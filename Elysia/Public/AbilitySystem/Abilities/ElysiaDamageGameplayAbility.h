// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "ElysiaDamageGameplayAbility.generated.h"

class UElysiaEquipmentComponent;
/**
 * 
 */
UCLASS()
class ELYSIA_API UElysiaDamageGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	
	UElysiaDamageGameplayAbility();
	
protected:
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
								 const FGameplayAbilityActivationInfo ActivationInfo,
								 const FGameplayEventData* TriggerEventData) override;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> DamageEffectClass;
	
	// 查询武器等级
	int32 GetWeaponAbilityLevel() const;
	// 查询武器是否已进化
	bool IsWeaponEvolved() const;
	// 从 PlayerState 上获取装备组件
	UElysiaEquipmentComponent* GetEquipmentComponent() const;
	
};
