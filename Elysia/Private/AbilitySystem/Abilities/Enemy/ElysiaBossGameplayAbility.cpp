// Copyright GhostLazy


#include "AbilitySystem/Abilities/Enemy/ElysiaBossGameplayAbility.h"

#include "Character/ElysiaBossBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "TimerManager.h"

UElysiaBossGameplayAbility::UElysiaBossGameplayAbility()
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}

bool UElysiaBossGameplayAbility::CanActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	const AElysiaBossBase* Boss = GetBossAvatar(ActorInfo);
	if (!Boss || Boss->IsUsingBossAbility() || !Boss->HasValidCombatTarget())
	{
		return false;
	}

	return IsTargetInRange(Boss);
}

bool UElysiaBossGameplayAbility::IsTargetInRange(const AElysiaBossBase* Boss) const
{
	if (!Boss)
	{
		return false;
	}

	const float DistanceToTarget = Boss->GetDistanceToCombatTarget2D();
	return DistanceToTarget >= MinRange && DistanceToTarget <= CastRange;
}

void UElysiaBossGameplayAbility::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AElysiaBossBase* Boss = GetBossAvatar(ActorInfo);
	if (!Boss)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	bNotifiedBossAbilityActive = true;
	Boss->NotifyBossAbilityStarted(this);

	LockedSkillDirection = CalculateTargetDirection();
	if (!LockedSkillDirection.IsNearlyZero())
	{
		Boss->SetActorRotation(LockedSkillDirection.Rotation());
	}

	if (UCharacterMovementComponent* MovementComponent = Boss->GetCharacterMovement())
	{
		MovementComponent->StopMovementImmediately();
	}

	if (CastMontage)
	{
		Boss->PlayAnimMontage(CastMontage);
	}

	const FVector Origin = Boss->GetActorLocation();
	OnBossAbilityWindupStarted(Boss, Origin, LockedSkillDirection);

	if (WindupTime <= 0.f)
	{
		ExecuteBossSkillAfterWindup();
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			WindupTimerHandle,
			this,
			&UElysiaBossGameplayAbility::ExecuteBossSkillAfterWindup,
			FMath::Max(0.f, WindupTime),
			false);
	}
}

void UElysiaBossGameplayAbility::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	ClearBossAbilityTimers();

	AElysiaBossBase* Boss = GetBossAvatar(ActorInfo);
	if (Boss)
	{
		OnBossAbilityRecovered(Boss);
		if (bNotifiedBossAbilityActive)
		{
			Boss->NotifyBossAbilityEnded(this);
		}
	}

	bNotifiedBossAbilityActive = false;
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UElysiaBossGameplayAbility::ExecuteBossSkill()
{
}

void UElysiaBossGameplayAbility::BeginBossAbilityRecovery(float Delay)
{
	if (Delay <= 0.f)
	{
		FinishBossAbilityRecovery();
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			RecoveryTimerHandle,
			this,
			&UElysiaBossGameplayAbility::FinishBossAbilityRecovery,
			Delay,
			false);
	}
}

void UElysiaBossGameplayAbility::FinishBossAbilityRecovery()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

AElysiaBossBase* UElysiaBossGameplayAbility::GetBossAvatar(const FGameplayAbilityActorInfo* ActorInfo) const
{
	const FGameplayAbilityActorInfo* AbilityActorInfo = ActorInfo ? ActorInfo : CurrentActorInfo;
	return AbilityActorInfo ? Cast<AElysiaBossBase>(AbilityActorInfo->AvatarActor.Get()) : nullptr;
}

AActor* UElysiaBossGameplayAbility::GetCombatTarget() const
{
	const AElysiaBossBase* Boss = GetBossAvatar();
	return Boss ? Boss->GetCombatTarget() : nullptr;
}

bool UElysiaBossGameplayAbility::ApplyDamageToTarget(AActor* TargetActor) const
{
	const AElysiaBossBase* Boss = GetBossAvatar();
	return Boss ? Boss->ApplyBossDamageToTarget(TargetActor, DamageEffectClass, DamageEffectLevel) : false;
}

UNiagaraComponent* UElysiaBossGameplayAbility::SpawnEffectAtLocation(UNiagaraSystem* Effect, const FVector& Location, const FVector& Direction) const
{
	if (!Effect)
	{
		return nullptr;
	}

	const FVector SafeDirection = Direction.GetSafeNormal2D();
	const FRotator Rotation = SafeDirection.IsNearlyZero() ? FRotator::ZeroRotator : SafeDirection.Rotation();
	return UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		this,
		Effect,
		Location + EffectLocationOffset,
		Rotation);
}

UNiagaraComponent* UElysiaBossGameplayAbility::SpawnEffectAttachedToBoss(UNiagaraSystem* Effect, const FVector& RelativeLocation, const FVector& Direction) const
{
	AElysiaBossBase* Boss = GetBossAvatar();
	if (!Effect || !Boss || !Boss->GetRootComponent())
	{
		return nullptr;
	}

	const FVector SafeDirection = Direction.GetSafeNormal2D();
	const FRotator Rotation = SafeDirection.IsNearlyZero() ? FRotator::ZeroRotator : SafeDirection.Rotation();
	return UNiagaraFunctionLibrary::SpawnSystemAttached(
		Effect,
		Boss->GetRootComponent(),
		NAME_None,
		RelativeLocation + EffectLocationOffset,
		Rotation,
		EAttachLocation::KeepRelativeOffset,
		true);
}

void UElysiaBossGameplayAbility::OnBossAbilityWindupStarted_Implementation(AElysiaBossBase* Boss, FVector Origin, FVector Direction)
{
	if (WindupSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, WindupSound, Origin);
	}
}

void UElysiaBossGameplayAbility::OnBossAbilityExecuted_Implementation(AElysiaBossBase* Boss, FVector Origin, FVector Direction)
{
	if (ExecuteSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ExecuteSound, Origin);
	}
}

void UElysiaBossGameplayAbility::OnBossAbilityRecovered_Implementation(AElysiaBossBase* Boss)
{
	if (RecoveredSound && Boss)
	{
		UGameplayStatics::PlaySoundAtLocation(this, RecoveredSound, Boss->GetActorLocation());
	}
}

void UElysiaBossGameplayAbility::ExecuteBossSkillAfterWindup()
{
	AElysiaBossBase* Boss = GetBossAvatar();
	if (!Boss)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	const FVector Origin = Boss->GetActorLocation();
	OnBossAbilityExecuted(Boss, Origin, LockedSkillDirection);
	ExecuteBossSkill();

	if (ShouldAutoRecoverAfterExecute())
	{
		BeginBossAbilityRecovery(GetPostExecuteRecoveryTime());
	}
}

void UElysiaBossGameplayAbility::ClearBossAbilityTimers()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(WindupTimerHandle);
		World->GetTimerManager().ClearTimer(RecoveryTimerHandle);
	}
}

FVector UElysiaBossGameplayAbility::CalculateTargetDirection() const
{
	const AElysiaBossBase* Boss = GetBossAvatar();
	const AActor* TargetActor = Boss ? Boss->GetCombatTarget() : nullptr;
	if (Boss && TargetActor)
	{
		const FVector Direction = (TargetActor->GetActorLocation() - Boss->GetActorLocation()).GetSafeNormal2D();
		if (!Direction.IsNearlyZero())
		{
			return Direction;
		}
	}

	return Boss ? Boss->GetActorForwardVector().GetSafeNormal2D() : FVector::ForwardVector;
}
