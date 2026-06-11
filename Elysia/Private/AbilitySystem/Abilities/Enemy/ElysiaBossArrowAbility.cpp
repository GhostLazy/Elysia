// Copyright GhostLazy

#include "AbilitySystem/Abilities/Enemy/ElysiaBossArrowAbility.h"

#include "AbilitySystemComponent.h"
#include "Actor/ElysiaBossProjectile.h"
#include "Character/ElysiaBossBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "EngineUtils.h"
#include "ElysiaGameplayTags.h"
#include "GameFramework/Pawn.h"
#include "Interface/CombatInterface.h"
#include "TimerManager.h"

UElysiaBossArrowAbility::UElysiaBossArrowAbility()
{
	CastRange = 2000.f;
}

void UElysiaBossArrowAbility::ExecuteBossSkill()
{
	AElysiaBossBase* Boss = GetBossAvatar();
	if (!Boss || !Boss->HasAuthority())
	{
		FinishBossAbility();
		return;
	}

	FiredShotCount = 0;
	if (ShotsPerCast > 1)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(
				ShotTimerHandle,
				this,
				&UElysiaBossArrowAbility::FireArrow,
				FMath::Max(0.01f, ShotInterval),
				true);
		}
	}

	FireArrow();
}

void UElysiaBossArrowAbility::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ShotTimerHandle);
	}
	FiredShotCount = 0;

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UElysiaBossArrowAbility::FireArrow()
{
	const bool bFinishAfterThisShot = ++FiredShotCount >= FMath::Max(1, ShotsPerCast);
	const auto FinishShotIfNeeded = [this, bFinishAfterThisShot]()
	{
		if (!bFinishAfterThisShot)
		{
			return;
		}

		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(ShotTimerHandle);
		}
		FinishBossAbility();
	};

	AElysiaBossBase* Boss = GetBossAvatar();
	AActor* TargetActor = FindClosestPlayer();
	UAbilitySystemComponent* AbilitySystemComponent = Boss ? Boss->GetAbilitySystemComponent() : nullptr;
	if (!Boss || !Boss->HasAuthority() || !TargetActor || !AbilitySystemComponent || !ProjectileClass || !DamageEffectClass)
	{
		FinishShotIfNeeded();
		return;
	}

	FTransform SpawnTransform;
	if (USkeletalMeshComponent* BossMesh = Boss->GetMesh(); BossMesh && BossMesh->DoesSocketExist(MuzzleSocketName))
	{
		SpawnTransform = BossMesh->GetSocketTransform(MuzzleSocketName);
	}
	else
	{
		SpawnTransform = Boss->GetActorTransform();
		SpawnTransform.SetLocation(Boss->GetActorTransform().TransformPosition(ProjectileSpawnOffset));
	}

	const FVector AimDirection = (TargetActor->GetActorLocation() - SpawnTransform.GetLocation()).GetSafeNormal();
	if (AimDirection.IsNearlyZero())
	{
		FinishShotIfNeeded();
		return;
	}
	SpawnTransform.SetRotation(AimDirection.Rotation().Quaternion());

	AElysiaBossProjectile* Projectile = Boss->GetWorld()->SpawnActorDeferred<AElysiaBossProjectile>(
		ProjectileClass,
		SpawnTransform,
		Boss,
		Boss,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (!Projectile)
	{
		FinishShotIfNeeded();
		return;
	}

	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	EffectContext.AddSourceObject(this);
	Projectile->EffectSpecHandle = AbilitySystemComponent->MakeOutgoingSpec(
		DamageEffectClass,
		FMath::Max(1.f, DamageEffectLevel),
		EffectContext);
	Projectile->SetMovementSpeed(ProjectileSpeed);
	Projectile->FinishSpawning(SpawnTransform);
	FinishShotIfNeeded();
}

FGameplayTag UElysiaBossArrowAbility::GetDefaultExecuteGameplayCueTag() const
{
	return FElysiaGameplayTags::Get().GameplayCue_Boss_Arrow_Execute;
}

AActor* UElysiaBossArrowAbility::FindClosestPlayer() const
{
	const AElysiaBossBase* Boss = GetBossAvatar();
	UWorld* World = Boss ? Boss->GetWorld() : nullptr;
	if (!World)
	{
		return nullptr;
	}

	AActor* ClosestPlayer = nullptr;
	float ClosestDistanceSquared = TNumericLimits<float>::Max();
	for (TActorIterator<APawn> It(World); It; ++It)
	{
		APawn* Candidate = *It;
		const ICombatInterface* CombatInterface = Cast<ICombatInterface>(Candidate);
		if (!CombatInterface || !CombatInterface->IsPlayer() || CombatInterface->IsDead())
		{
			continue;
		}

		const float DistanceSquared = FVector::DistSquared(Boss->GetActorLocation(), Candidate->GetActorLocation());
		if (DistanceSquared <= FMath::Square(CastRange) && DistanceSquared < ClosestDistanceSquared)
		{
			ClosestDistanceSquared = DistanceSquared;
			ClosestPlayer = Candidate;
		}
	}

	return ClosestPlayer;
}
