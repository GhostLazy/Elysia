// Copyright GhostLazy


#include "Character/ElysiaEnemy.h"
#include "AbilitySystem/ElysiaAbilitySystemComponent.h"
#include "AbilitySystem/ElysiaAttributeSet.h"
#include "Components/CapsuleComponent.h"
#include "Elysia/Elysia.h"

AElysiaEnemy::AElysiaEnemy()
{
	AbilitySystemComponent = CreateDefaultSubobject<UElysiaAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	
	AttributeSet = CreateDefaultSubobject<UElysiaAttributeSet>("AttributeSet");
	
	// 两套碰撞逻辑，分别处理
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	GetMesh()->SetCollisionResponseToAllChannels(ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Projectile, ECR_Overlap);
	
	Tags.Add(FName("Enemy"));
}

void AElysiaEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	InitDefaultAttributes();
	InitHealthBar();
	
	if (UElysiaAttributeSet* ElysiaAS = Cast<UElysiaAttributeSet>(AttributeSet))
	{
		OnHealthChanged.Broadcast(ElysiaAS->GetHealth());
		OnMaxHealthChanged.Broadcast(ElysiaAS->GetMaxHealth());
	}
}
