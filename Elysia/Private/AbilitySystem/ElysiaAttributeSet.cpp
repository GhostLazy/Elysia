// Copyright GhostLazy


#include "AbilitySystem/ElysiaAttributeSet.h"
#include "Net/UnrealNetwork.h"

void UElysiaAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION_NOTIFY(UElysiaAttributeSet, Health, COND_None, REPNOTIFY_Always)
	DOREPLIFETIME_CONDITION_NOTIFY(UElysiaAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always)
	DOREPLIFETIME_CONDITION_NOTIFY(UElysiaAttributeSet, Attack, COND_None, REPNOTIFY_Always)
	DOREPLIFETIME_CONDITION_NOTIFY(UElysiaAttributeSet, Defence, COND_None, REPNOTIFY_Always)
	DOREPLIFETIME_CONDITION_NOTIFY(UElysiaAttributeSet, AttackSpeed, COND_None, REPNOTIFY_Always)
	DOREPLIFETIME_CONDITION_NOTIFY(UElysiaAttributeSet, CoolDown, COND_None, REPNOTIFY_Always)
	DOREPLIFETIME_CONDITION_NOTIFY(UElysiaAttributeSet, HealthRegen, COND_None, REPNOTIFY_Always)
	DOREPLIFETIME_CONDITION_NOTIFY(UElysiaAttributeSet, MoveSpeed, COND_None, REPNOTIFY_Always)
}

void UElysiaAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
}

void UElysiaAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
}

void UElysiaAttributeSet::OnRep_Health(const FGameplayAttributeData& PreviousHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UElysiaAttributeSet, Health, PreviousHealth);
}

void UElysiaAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& PreviousMaxHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UElysiaAttributeSet, MaxHealth, PreviousMaxHealth);
}

void UElysiaAttributeSet::OnRep_Attack(const FGameplayAttributeData& PreviousAttack) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UElysiaAttributeSet, Attack, PreviousAttack);
}

void UElysiaAttributeSet::OnRep_Defence(const FGameplayAttributeData& PreviousDefence) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UElysiaAttributeSet, Defence, PreviousDefence);
}

void UElysiaAttributeSet::OnRep_AttackSpeed(const FGameplayAttributeData& PreviousAttackSpeed) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UElysiaAttributeSet, AttackSpeed, PreviousAttackSpeed);
}

void UElysiaAttributeSet::OnRep_CoolDown(const FGameplayAttributeData& PreviousCoolDown) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UElysiaAttributeSet, CoolDown, PreviousCoolDown);
}

void UElysiaAttributeSet::OnRep_HealthRegen(const FGameplayAttributeData& PreviousHealthRegen) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UElysiaAttributeSet, HealthRegen, PreviousHealthRegen);
}

void UElysiaAttributeSet::OnRep_MoveSpeed(const FGameplayAttributeData& PreviousMoveSpeed) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UElysiaAttributeSet, MoveSpeed, PreviousMoveSpeed);
}
