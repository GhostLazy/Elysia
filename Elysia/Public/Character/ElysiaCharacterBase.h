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

UCLASS()
class ELYSIA_API AElysiaCharacterBase : public ACharacter, public IAbilitySystemInterface, public ICombatInterface
{
	GENERATED_BODY()

public:
	
	AElysiaCharacterBase();
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }
	UAttributeSet* GetAttributeSet() const { return AttributeSet; }
	
	/* 战斗接口 */
	virtual void Die() override {}
	virtual bool IsDead() const override { return bDead; }
	virtual bool IsPlayer() const override { return ActorHasTag(FName("Player")); }
	virtual bool IsEnemy() const override { return ActorHasTag(FName("Enemy")); }
	virtual bool HasTag(const FName Tag) const override { return ActorHasTag(Tag); }

protected:

	virtual void BeginPlay() override;
	void InitDefaultAttributes() const;
		
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
	
	UPROPERTY(BlueprintAssignable)
	FOnAttributeChangeSignature OnHealthChanged;
	
	UPROPERTY(BlueprintAssignable)
	FOnAttributeChangeSignature OnMaxHealthChanged;
	
	UPROPERTY(BlueprintAssignable)
	FOnAttributeChangeSignature OnMoveSpeedChanged;
	
	virtual void InitHealthBar();
	void InitCharacterAbilities() const;
	
	bool bDead = false;

private:
	
	void ApplyEffectToSelf(const TSubclassOf<UGameplayEffect>& EffectClass, const float Level) const;
	
};
