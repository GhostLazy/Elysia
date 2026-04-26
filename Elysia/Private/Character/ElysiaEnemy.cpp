// Copyright GhostLazy


#include "Character/ElysiaEnemy.h"
#include "AI/AIController/ElysiaAIControllerBase.h"
#include "AI/AIController/ElysiaMinionAIController.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CharacterMovementComponentAsync.h"
#include "AbilitySystem/ElysiaAbilitySystemComponent.h"
#include "AbilitySystem/ElysiaAttributeSet.h"
#include "Components/CapsuleComponent.h"
#include "Actor/ElysiaXPBall.h"
#include "Elysia/Elysia.h"
#include "GameFramework/Character.h"
#include "Interface/CombatInterface.h"

AElysiaEnemy::AElysiaEnemy()
{
	AbilitySystemComponent = CreateDefaultSubobject<UElysiaAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	
	AttributeSet = CreateDefaultSubobject<UElysiaAttributeSet>("AttributeSet");
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 720.f, 0.f);
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	
	// 两套碰撞逻辑，分别处理
	GetCapsuleComponent()->SetCollisionObjectType(ECC_Minion);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Player, ECR_Overlap);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	GetMesh()->SetCollisionResponseToAllChannels(ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Projectile, ECR_Overlap);

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AIControllerClass = AElysiaMinionAIController::StaticClass();
	
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

	if (HasAuthority())
	{
		GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &AElysiaEnemy::HandlePlayerOverlapBegin);
		GetCapsuleComponent()->OnComponentEndOverlap.AddDynamic(this, &AElysiaEnemy::HandlePlayerOverlapEnd);
	}
}

void AElysiaEnemy::Die()
{
	bDead = true;
	ClearActiveContactDamageEffects();
	CurrentOverlappingPlayers.Empty();

	if (AElysiaAIControllerBase* EnemyController = Cast<AElysiaAIControllerBase>(GetController()))
	{
		EnemyController->StopAI();
	}

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

AActor* AElysiaEnemy::GetPreferredDamageTarget() const
{
	for (const TWeakObjectPtr<AActor>& WeakActor : CurrentOverlappingPlayers)
	{
		if (AActor* OverlapActor = WeakActor.Get(); IsValidDamageTargetActor(OverlapActor))
		{
			return OverlapActor;
		}
	}

	return nullptr;
}

void AElysiaEnemy::HandlePlayerOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority() || !IsValidDamageTargetActor(OtherActor))
	{
		return;
	}

	CurrentOverlappingPlayers.Add(TWeakObjectPtr<AActor>(OtherActor));
	ApplyContactDamageEffectToTarget(OtherActor);
}

void AElysiaEnemy::HandlePlayerOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!HasAuthority() || !OtherActor)
	{
		return;
	}

	CurrentOverlappingPlayers.Remove(TWeakObjectPtr<AActor>(OtherActor));
	RemoveContactDamageEffectFromTarget(OtherActor);
}

void AElysiaEnemy::ApplyContactDamageEffectToTarget(AActor* DamageTarget)
{
	if (!HasAuthority() || !AbilitySystemComponent || !ContactDamageEffectClass || !IsValidDamageTargetActor(DamageTarget))
	{
		return;
	}

	const TWeakObjectPtr<AActor> WeakTarget(DamageTarget);
	if (const FActiveGameplayEffectHandle* ExistingHandle = ActiveContactDamageEffects.Find(WeakTarget))
	{
		if (ExistingHandle->IsValid())
		{
			return;
		}
	}

	if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(DamageTarget))
	{
		FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
		EffectContext.AddSourceObject(this);

		const FGameplayEffectSpecHandle EffectSpecHandle = AbilitySystemComponent->MakeOutgoingSpec(
			ContactDamageEffectClass,
			static_cast<float>(FMath::Max(1, Level)),
			EffectContext);

		if (EffectSpecHandle.IsValid())
		{
			const FActiveGameplayEffectHandle AppliedHandle = TargetASC->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
			if (AppliedHandle.IsValid())
			{
				ActiveContactDamageEffects.FindOrAdd(WeakTarget) = AppliedHandle;
			}
		}
	}
}

void AElysiaEnemy::RemoveContactDamageEffectFromTarget(AActor* DamageTarget)
{
	if (!HasAuthority() || !DamageTarget)
	{
		return;
	}

	const TWeakObjectPtr<AActor> WeakTarget(DamageTarget);
	const FActiveGameplayEffectHandle ActiveHandle = ActiveContactDamageEffects.FindRef(WeakTarget);
	if (!ActiveHandle.IsValid())
	{
		ActiveContactDamageEffects.Remove(WeakTarget);
		return;
	}

	if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(DamageTarget))
	{
		TargetASC->RemoveActiveGameplayEffect(ActiveHandle);
	}

	ActiveContactDamageEffects.Remove(WeakTarget);
}

void AElysiaEnemy::ClearActiveContactDamageEffects()
{
	TArray<TWeakObjectPtr<AActor>> ActiveTargets;
	ActiveContactDamageEffects.GetKeys(ActiveTargets);
	for (const TWeakObjectPtr<AActor>& WeakTarget : ActiveTargets)
	{
		RemoveContactDamageEffectFromTarget(WeakTarget.Get());
	}
}

bool AElysiaEnemy::IsValidDamageTargetActor(AActor* Actor)
{
	if (!IsValid(Actor))
	{
		return false;
	}

	const ICombatInterface* CombatInterface = Cast<ICombatInterface>(Actor);
	return CombatInterface && CombatInterface->IsPlayer() && !CombatInterface->IsDead();
}
