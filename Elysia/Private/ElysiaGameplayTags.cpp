// Copyright GhostLazy


#include "ElysiaGameplayTags.h"
#include "GameplayTagsManager.h"

FElysiaGameplayTags FElysiaGameplayTags::GameplayTags;

void FElysiaGameplayTags::InitNativeGameplayTags()
{
	// 主属性
	GameplayTags.Attribute_Primary_MaxHealth = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attribute.Primary.MaxHealth"));
	GameplayTags.Attribute_Primary_Attack = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attribute.Primary.Attack"));
	GameplayTags.Attribute_Primary_Defense = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attribute.Primary.Defense"));
	GameplayTags.Attribute_Primary_AttackSpeed = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attribute.Primary.AttackSpeed"));
	GameplayTags.Attribute_Primary_CoolDown = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attribute.Primary.CoolDown"));
	GameplayTags.Attribute_Primary_HealthRegen = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attribute.Primary.HealthRegen"));
	GameplayTags.Attribute_Primary_MaxHealth = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attribute.Primary.MaxHealth"));
	
	// 必要属性
	GameplayTags.Attribute_Vital_Health = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Attribute.Vital.Health"));
	
	// 技能类型
	GameplayTags.Ability_Type_NormalAttack = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Ability.Type.NormalAttack"));
	GameplayTags.Ability_Type_Skill = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Ability.Type.Skill"));
	GameplayTags.Ability_Type_Ultimate = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Ability.Type.Ultimate"));
	
	// 角色技能
	GameplayTags.Ability_Elysia_NormalAttack = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Ability.Elysia.NormalAttack"));
	GameplayTags.Ability_Elysia_Skill = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Ability.Elysia.Skill"));
	GameplayTags.Ability_Elysia_Ultimate = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Ability.Elysia.Ultimate"));
	
	// 角色状态
	GameplayTags.Status_Elysia_Enhanced = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Status.Elysia.Enhanced"));
	
	// 技能冷却
	GameplayTags.Cooldown_Elysia_Skill = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Cooldown.Elysia.Skill"));
	
	// 动画事件
	GameplayTags.Event_Montage_Elysia_NormalAttack = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("Event.Montage.Elysia.NormalAttack"));

	// GameplayCue：角色武器表现
	GameplayTags.GameplayCue_Weapon_SwordSlash = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("GameplayCue.Weapon.SwordSlash"));

	// GameplayCue：Boss技能表现
	GameplayTags.GameplayCue_Boss_Charge_Windup = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("GameplayCue.Boss.Charge.Windup"));
	GameplayTags.GameplayCue_Boss_Charge_Execute = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("GameplayCue.Boss.Charge.Execute"));
	GameplayTags.GameplayCue_Boss_Sweep_Windup = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("GameplayCue.Boss.Sweep.Windup"));
	GameplayTags.GameplayCue_Boss_Sweep_Execute = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("GameplayCue.Boss.Sweep.Execute"));
	GameplayTags.GameplayCue_Boss_Laser_Windup = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("GameplayCue.Boss.Laser.Windup"));
	GameplayTags.GameplayCue_Boss_Laser_Execute = UGameplayTagsManager::Get().AddNativeGameplayTag(FName("GameplayCue.Boss.Laser.Execute"));
}


