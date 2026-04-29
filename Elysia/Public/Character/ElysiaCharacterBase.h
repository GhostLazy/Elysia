// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "Interface/CombatInterface.h"
#include "ElysiaCharacterBase.generated.h"

class UGameplayAbility;
class UWidgetComponent;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttributeChangeSignature, float, NewValue);

class UGameplayEffect;
class UAttributeSet;
class UAbilitySystemComponent;
class UCameraComponent;
class USpringArmComponent;
struct FOnAttributeChangeData;

UCLASS()
class ELYSIA_API AElysiaCharacterBase : public ACharacter, public IAbilitySystemInterface, public ICombatInterface
{
	GENERATED_BODY()

public:
	
	AElysiaCharacterBase();
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }
	UAttributeSet* GetAttributeSet() const { return AttributeSet; }
	
	virtual void BeginPlay() override;
	// 角色销毁时，清理已绑定的属性委托，避免重复初始化后残留回调
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	/* 战斗接口 */
	virtual void Die() override {}
	virtual bool IsDead() const override { return bDead; }
	virtual bool IsPlayer() const override { return ActorHasTag(FName("Player")); }
	virtual bool IsEnemy() const override { return ActorHasTag(FName("Enemy")); }
	virtual bool HasTag(const FName Tag) const override { return ActorHasTag(Tag); }

protected:
	
	// 初始化默认属性，并同步基础表现（如移速）
	void InitDefaultAttributes();
		
	UPROPERTY(VisibleAnywhere, Category = "Component")
	TObjectPtr<UWidgetComponent> HealthBar;
	
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> DefaultPrimaryAttributeClass;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> DefaultVitalAttributeClass;
	
	UPROPERTY(EditDefaultsOnly)
	TArray<TSubclassOf<UGameplayAbility>> StartupAbilities;
	
	UPROPERTY(EditDefaultsOnly)
	int32 HealthRegenInterval = 1.f;
	
	UPROPERTY(BlueprintAssignable)
	FOnAttributeChangeSignature OnHealthChanged;
	
	UPROPERTY(BlueprintAssignable)
	FOnAttributeChangeSignature OnMaxHealthChanged;
	
	UPROPERTY(BlueprintAssignable)
	FOnAttributeChangeSignature OnMoveSpeedChanged;
	
	// 绑定血量相关回调，供血条/UI刷新使用
	virtual void InitHealthBar();
	void InitCharacterAbilities() const;
	
	bool bDead = false;

private:
	
	void ApplyEffectToSelf(const TSubclassOf<UGameplayEffect>& EffectClass, const float Level) const;
	void ApplyHealthRegen();
	// 单独拆分绑定函数，避免 InitAbilityActorInfo 多次调用时反复叠绑
	void BindMoveSpeedDelegate();
	void BindHealthBarDelegates();
	void BindHealthRegenDelegate();
	void UnbindAttributeDelegates();
	void HandleMoveSpeedChanged(const FOnAttributeChangeData& Data);
	void HandleHealthChanged(const FOnAttributeChangeData& Data);
	void HandleMaxHealthChanged(const FOnAttributeChangeData& Data);
	void HandleHealthRegenChanged(const FOnAttributeChangeData& Data);

	FDelegateHandle MoveSpeedChangedHandle;
	FDelegateHandle HealthChangedHandle;
	FDelegateHandle MaxHealthChangedHandle;
	FDelegateHandle HealthRegenChangedHandle;
	
	FTimerHandle HealthRegenTimer;
	float HealthRegenValue = 0.f;
	
};
