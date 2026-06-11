// Copyright GhostLazy

#include "AbilitySystem/Abilities/Enemy/ElysiaBossFireballAbility.h"

#include "Actor/ElysiaBossFallingFireball.h"
#include "Character/ElysiaBossBase.h"
#include "EngineUtils.h"
#include "ElysiaGameplayTags.h"
#include "GameFramework/Pawn.h"
#include "Interface/CombatInterface.h"
#include "TimerManager.h"

void UElysiaBossFireballAbility::ExecuteBossSkill()
{
	AElysiaBossBase* Boss = GetBossAvatar();
	if (!Boss || !Boss->HasAuthority() || !FireballClass || SummonCount <= 0)
	{
		FinishBossAbility();
		return;
	}

	SummonedFireballCount = 0;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			SummonTimerHandle,
			this,
			&UElysiaBossFireballAbility::SummonNextFireball,
			FMath::Max(0.01f, SummonInterval),
			true);
	}
}

void UElysiaBossFireballAbility::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SummonTimerHandle);
	}
	SummonedFireballCount = 0;

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

FGameplayTag UElysiaBossFireballAbility::GetDefaultExecuteGameplayCueTag() const
{
	return FElysiaGameplayTags::Get().GameplayCue_Boss_Fireball_Execute;
}

void UElysiaBossFireballAbility::SummonNextFireball()
{
	AElysiaBossBase* Boss = GetBossAvatar();
	AActor* TargetActor = FindClosestPlayer();
	if (Boss && Boss->HasAuthority() && TargetActor && FireballClass)
	{
		const FVector TargetLocation = TargetActor->GetActorLocation();
		const FVector SpawnLocation = TargetLocation + FVector::UpVector * SpawnHeight;
		const FTransform SpawnTransform(FRotator::ZeroRotator, SpawnLocation);

		if (AElysiaBossFallingFireball* Fireball = Boss->GetWorld()->SpawnActorDeferred<AElysiaBossFallingFireball>(
			FireballClass,
			SpawnTransform,
			Boss,
			Boss,
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn))
		{
			Fireball->InitializeFireball(
				Boss,
				TargetLocation,
				DamageEffectClass,
				DamageEffectLevel,
				FallDuration,
				ImpactRadius);
			Fireball->FinishSpawning(SpawnTransform);
		}
	}

	++SummonedFireballCount;
	if (SummonedFireballCount >= FMath::Max(1, SummonCount))
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(SummonTimerHandle);
		}
		FinishBossAbility();
	}
}

AActor* UElysiaBossFireballAbility::FindClosestPlayer() const
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
		if (DistanceSquared < ClosestDistanceSquared)
		{
			ClosestDistanceSquared = DistanceSquared;
			ClosestPlayer = Candidate;
		}
	}

	return ClosestPlayer;
}
