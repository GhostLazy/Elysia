// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "ElysiaAttributeSet.generated.h"

/**
 * 
 */
UCLASS()
class ELYSIA_API UElysiaAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	
	// 登记属性同步“白名单”
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	// 仅在服务器执行，进行GE生效后的操作（如死亡判定）
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	
	// 生成属性访问接口
	ATTRIBUTE_ACCESSORS_BASIC(UElysiaAttributeSet, Health)
	ATTRIBUTE_ACCESSORS_BASIC(UElysiaAttributeSet, MaxHealth)
	ATTRIBUTE_ACCESSORS_BASIC(UElysiaAttributeSet, Attack)
	ATTRIBUTE_ACCESSORS_BASIC(UElysiaAttributeSet, Defence)
	ATTRIBUTE_ACCESSORS_BASIC(UElysiaAttributeSet, AttackSpeed)
	ATTRIBUTE_ACCESSORS_BASIC(UElysiaAttributeSet, CoolDown)
	ATTRIBUTE_ACCESSORS_BASIC(UElysiaAttributeSet, HealthRegen)
	ATTRIBUTE_ACCESSORS_BASIC(UElysiaAttributeSet, MoveSpeed)
	
protected:
	
	// 属性在网络同步时，在客户端触发回调函数
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Attributes")
	FGameplayAttributeData Health;
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "Attributes")
	FGameplayAttributeData MaxHealth;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Attack, Category = "Attributes")
	FGameplayAttributeData Attack;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Defence, Category = "Attributes")
	FGameplayAttributeData Defence;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_AttackSpeed, Category = "Attributes")
	FGameplayAttributeData AttackSpeed;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CoolDown, Category = "Attributes")
	FGameplayAttributeData CoolDown;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_HealthRegen, Category = "Attributes")
	FGameplayAttributeData HealthRegen;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MoveSpeed, Category = "Attributes")
	FGameplayAttributeData MoveSpeed;

private:
	
	// 回调函数将属性更新事件通知给GAS
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& PreviousHealth) const;
	
	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& PreviousMaxHealth) const;

	UFUNCTION()
	void OnRep_Attack(const FGameplayAttributeData& PreviousAttack) const;

	UFUNCTION()
	void OnRep_Defence(const FGameplayAttributeData& PreviousDefence) const;

	UFUNCTION()
	void OnRep_AttackSpeed(const FGameplayAttributeData& PreviousAttackSpeed) const;

	UFUNCTION()
	void OnRep_CoolDown(const FGameplayAttributeData& PreviousCoolDown) const;

	UFUNCTION()
	void OnRep_HealthRegen(const FGameplayAttributeData& PreviousHealthRegen) const;

	UFUNCTION()
	void OnRep_MoveSpeed(const FGameplayAttributeData& PreviousMoveSpeed) const;
	
};
