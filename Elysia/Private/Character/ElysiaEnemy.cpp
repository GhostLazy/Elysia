// Copyright GhostLazy


#include "Character/ElysiaEnemy.h"
#include "AbilitySystem/ElysiaAbilitySystemComponent.h"
#include "AbilitySystem/ElysiaAttributeSet.h"

AElysiaEnemy::AElysiaEnemy()
{
	AbilitySystemComponent = CreateDefaultSubobject<UElysiaAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	
	AttributeSet = CreateDefaultSubobject<UElysiaAttributeSet>("AttributeSet");
}

void AElysiaEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
}
