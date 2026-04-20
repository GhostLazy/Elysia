// Copyright GhostLazy


#include "Character/ElysiaEnemy.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CharacterMovementComponentAsync.h"
#include "AbilitySystem/ElysiaAbilitySystemComponent.h"
#include "AbilitySystem/ElysiaAttributeSet.h"
#include "Components/CapsuleComponent.h"
#include "Actor/ElysiaXPBall.h"
#include "Elysia/Elysia.h"

AElysiaEnemy::AElysiaEnemy()
{
	AbilitySystemComponent = CreateDefaultSubobject<UElysiaAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	
	AttributeSet = CreateDefaultSubobject<UElysiaAttributeSet>("AttributeSet");
	
	// 两套碰撞逻辑，分别处理
	GetCapsuleComponent()->SetCollisionObjectType(ECC_Minion);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Player, ECR_Overlap);
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

void AElysiaEnemy::Die()
{
	bDead = true;
	if (HasAuthority())
	{
		FTransform SpawnTransform;
		const FFindFloorResult FloorResult = GetCharacterMovement()->CurrentFloor;
		SpawnTransform.SetLocation(FloorResult.HitResult.ImpactPoint + FVector(0, 0, 10));
		AElysiaXPBall* XPBall = GetWorld()->SpawnActorDeferred<AElysiaXPBall>(XPBallClass, SpawnTransform);
		
		XPBall->SetXPValue(XPRewards.GetValueAtLevel(Level));
		XPBall->SetColorByLevel(Level);
		
		XPBall->FinishSpawning(SpawnTransform);
		Destroy();
	}
}
