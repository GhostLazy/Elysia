// Copyright GhostLazy


#include "Player/ElysiaPlayerState.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/ElysiaAbilitySystemComponent.h"
#include "AbilitySystem/ElysiaAttributeSet.h"

AElysiaPlayerState::AElysiaPlayerState()
{
	// 每秒进行100次复制
	SetNetUpdateFrequency(100.f);
	
	AbilitySystemComponent = CreateDefaultSubobject<UElysiaAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	
	AttributeSet = CreateDefaultSubobject<UElysiaAttributeSet>("AttributeSet");
}
