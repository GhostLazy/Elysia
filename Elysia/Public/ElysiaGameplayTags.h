// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

/**
 * 存储C++原生标签的“单例”类
 */

struct FElysiaGameplayTags
{
public:
	
	static const FElysiaGameplayTags& Get() { return GameplayTags; }
	static void InitNativeGameplayTags();
	
	// 主属性
	FGameplayTag Attribute_Primary_MaxHealth;
	FGameplayTag Attribute_Primary_Attack;
	FGameplayTag Attribute_Primary_Defense;
	FGameplayTag Attribute_Primary_AttackSpeed;
	FGameplayTag Attribute_Primary_CoolDown;
	FGameplayTag Attribute_Primary_HealthRegen;
	FGameplayTag Attribute_Primary_MoveSpeed;
	
	// 必要属性
	FGameplayTag Attribute_Vital_Health;
	
	// 技能类型
	FGameplayTag Ability_Type_NormalAttack;
	FGameplayTag Ability_Type_Skill;
	FGameplayTag Ability_Type_Ultimate;
	
	// 角色技能
	FGameplayTag Ability_Elysia_NormalAttack;
	FGameplayTag Ability_Elysia_Skill;
	FGameplayTag Ability_Elysia_Ultimate;
	
	// 动画事件
	FGameplayTag Event_Montage_Elysia_NormalAttack;
	
private:
	
	static FElysiaGameplayTags GameplayTags;
	
};