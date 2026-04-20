// Copyright GhostLazy


#include "Player/ElysiaPlayerState.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/ElysiaAbilitySystemComponent.h"
#include "AbilitySystem/ElysiaAttributeSet.h"
#include "Net/UnrealNetwork.h"

AElysiaPlayerState::AElysiaPlayerState()
{
	// 每秒进行100次复制
	SetNetUpdateFrequency(100.f);
	
	AbilitySystemComponent = CreateDefaultSubobject<UElysiaAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	
	AttributeSet = CreateDefaultSubobject<UElysiaAttributeSet>("AttributeSet");
}

void AElysiaPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION_NOTIFY(AElysiaPlayerState, Level, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(AElysiaPlayerState, XP, COND_None, REPNOTIFY_Always);
}

void AElysiaPlayerState::AddToLevel(int32 InLevel)
{
	Level += InLevel;
	OnLevelChanged.Broadcast(Level, true);
}

void AElysiaPlayerState::AddToXP(int32 InXP)
{
	XP += InXP;
	OnXPChanged.Broadcast(XP);
}

void AElysiaPlayerState::OnRep_Level() const
{
	OnLevelChanged.Broadcast(Level, true);
}

void AElysiaPlayerState::OnRep_XP() const
{
	OnXPChanged.Broadcast(XP);
}
