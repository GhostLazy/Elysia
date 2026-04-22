// Copyright GhostLazy


#include "Player/ElysiaPlayerState.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/ElysiaAbilitySystemComponent.h"
#include "AbilitySystem/ElysiaAttributeSet.h"
#include "Equipment/ElysiaEquipmentComponent.h"
#include "Net/UnrealNetwork.h"

AElysiaPlayerState::AElysiaPlayerState()
{
	// 每秒进行100次复制
	SetNetUpdateFrequency(100.f);
	
	AbilitySystemComponent = CreateDefaultSubobject<UElysiaAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	
	AttributeSet = CreateDefaultSubobject<UElysiaAttributeSet>("AttributeSet");
	EquipmentComponent = CreateDefaultSubobject<UElysiaEquipmentComponent>("EquipmentComponent");
}

void AElysiaPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION_NOTIFY(AElysiaPlayerState, Level, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(AElysiaPlayerState, XP, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME(AElysiaPlayerState, MaxLevel);
}

void AElysiaPlayerState::SetLevel(int32 InLevel)
{
	if (Level == InLevel)
	{
		return;
	}

	const bool bLevelUp = InLevel > Level;
	Level = InLevel;
	OnLevelChanged.Broadcast(Level, bLevelUp);
}

void AElysiaPlayerState::AddToXP(int32 InXP)
{
	XP += InXP;
	
	const int32 PreviousLevel = Level;
	int32 NewLevel = 1; 
	for (int32 i = 2; i <= MaxLevel; ++i)
	{
		if (XP >= LevelUpRequirement.GetValueAtLevel(i - 1)) NewLevel = i;
		else break;
	}
	if (PreviousLevel != NewLevel)
	{
		SetLevel(NewLevel);
		if (EquipmentComponent && NewLevel > PreviousLevel)
		{
			EquipmentComponent->QueueLevelUpSelections(NewLevel - PreviousLevel);
		}
	}
	
	OnXPChanged.Broadcast(XP);
}

void AElysiaPlayerState::OnRep_Level(int32 OldLevel)
{
	OnLevelChanged.Broadcast(Level, Level > OldLevel);
}

void AElysiaPlayerState::OnRep_XP()
{
	OnXPChanged.Broadcast(XP);
}
