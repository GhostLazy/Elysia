// Copyright GhostLazy


#include "AbilitySystem/Abilities/Enemy/ElysiaBossGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "Character/ElysiaBossBase.h"
#include "GameFramework/CharacterMovementComponent.h"
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
	AddBossAbilityWindupCue(Boss, Origin, LockedSkillDirection);

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
		RemoveBossAbilityWindupCue(Boss);

		FGameplayCueParameters RecoveryCueParameters;
		const FVector RecoveryOrigin = Boss->GetActorLocation();
		const FVector RecoveryDirection = Boss->GetActorForwardVector().GetSafeNormal2D();
		BuildRecoveryGameplayCueParameters(RecoveryCueParameters, Boss, RecoveryOrigin, RecoveryDirection);
		ExecuteBossAbilityCue(GetRecoveryGameplayCueTag(), RecoveryCueParameters);

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

FGameplayTag UElysiaBossGameplayAbility::GetDefaultWindupGameplayCueTag() const
{
	return FGameplayTag();
}

FGameplayTag UElysiaBossGameplayAbility::GetDefaultExecuteGameplayCueTag() const
{
	return FGameplayTag();
}

FGameplayTag UElysiaBossGameplayAbility::GetDefaultRecoveryGameplayCueTag() const
{
	return FGameplayTag();
}

void UElysiaBossGameplayAbility::BuildWindupGameplayCueParameters(
	FGameplayCueParameters& OutParameters,
	AElysiaBossBase* Boss,
	const FVector& Origin,
	const FVector& Direction) const
{
	BuildBaseGameplayCueParameters(OutParameters, Boss, Origin, Direction);
	OutParameters.RawMagnitude = WindupTime;
	OutParameters.NormalizedMagnitude = RecoveryTime;
}

void UElysiaBossGameplayAbility::BuildExecuteGameplayCueParameters(
	FGameplayCueParameters& OutParameters,
	AElysiaBossBase* Boss,
	const FVector& Origin,
	const FVector& Direction) const
{
	BuildBaseGameplayCueParameters(OutParameters, Boss, Origin, Direction);
	OutParameters.RawMagnitude = WindupTime;
	OutParameters.NormalizedMagnitude = RecoveryTime;
}

void UElysiaBossGameplayAbility::BuildRecoveryGameplayCueParameters(
	FGameplayCueParameters& OutParameters,
	AElysiaBossBase* Boss,
	const FVector& Origin,
	const FVector& Direction) const
{
	BuildBaseGameplayCueParameters(OutParameters, Boss, Origin, Direction);
	OutParameters.RawMagnitude = RecoveryTime;
	OutParameters.NormalizedMagnitude = WindupTime;
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
	RemoveBossAbilityWindupCue(Boss);

	FGameplayCueParameters ExecuteCueParameters;
	BuildExecuteGameplayCueParameters(ExecuteCueParameters, Boss, Origin, LockedSkillDirection);
	ExecuteBossAbilityCue(GetExecuteGameplayCueTag(), ExecuteCueParameters);

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

void UElysiaBossGameplayAbility::AddBossAbilityWindupCue(
	AElysiaBossBase* Boss,
	const FVector& Origin,
	const FVector& Direction)
{
	const FGameplayTag CueTag = GetWindupGameplayCueTag();
	if (!Boss || !CueTag.IsValid())
	{
		return;
	}

	if (UAbilitySystemComponent* AbilitySystemComponent = Boss->GetAbilitySystemComponent())
	{
		FGameplayCueParameters CueParameters;
		BuildWindupGameplayCueParameters(CueParameters, Boss, Origin, Direction);
		AbilitySystemComponent->AddGameplayCue(CueTag, CueParameters);
		bWindupGameplayCueActive = true;
	}
}

void UElysiaBossGameplayAbility::RemoveBossAbilityWindupCue(AElysiaBossBase* Boss)
{
	if (!bWindupGameplayCueActive || !Boss)
	{
		return;
	}

	const FGameplayTag CueTag = GetWindupGameplayCueTag();
	if (!CueTag.IsValid())
	{
		bWindupGameplayCueActive = false;
		return;
	}

	if (UAbilitySystemComponent* AbilitySystemComponent = Boss->GetAbilitySystemComponent())
	{
		AbilitySystemComponent->RemoveGameplayCue(CueTag);
	}
	bWindupGameplayCueActive = false;
}

void UElysiaBossGameplayAbility::ExecuteBossAbilityCue(
	FGameplayTag CueTag,
	const FGameplayCueParameters& Parameters) const
{
	AElysiaBossBase* Boss = GetBossAvatar();
	if (!Boss || !CueTag.IsValid())
	{
		return;
	}

	if (UAbilitySystemComponent* AbilitySystemComponent = Boss->GetAbilitySystemComponent())
	{
		AbilitySystemComponent->ExecuteGameplayCue(CueTag, Parameters);
	}
}

void UElysiaBossGameplayAbility::BuildBaseGameplayCueParameters(
	FGameplayCueParameters& OutParameters,
	AElysiaBossBase* Boss,
	const FVector& Origin,
	const FVector& Direction) const
{
	OutParameters = FGameplayCueParameters();
	OutParameters.Location = Origin;
	OutParameters.Normal = Direction.GetSafeNormal2D();
	OutParameters.Instigator = Boss;
	OutParameters.EffectCauser = Boss;
	OutParameters.SourceObject = this;
	OutParameters.AbilityLevel = FMath::Max(1, GetAbilityLevel());
	OutParameters.GameplayEffectLevel = FMath::Max(1, FMath::RoundToInt(DamageEffectLevel));
}

FGameplayTag UElysiaBossGameplayAbility::GetWindupGameplayCueTag() const
{
	return WindupGameplayCueTagOverride.IsValid()
		? WindupGameplayCueTagOverride
		: GetDefaultWindupGameplayCueTag();
}

FGameplayTag UElysiaBossGameplayAbility::GetExecuteGameplayCueTag() const
{
	return ExecuteGameplayCueTagOverride.IsValid()
		? ExecuteGameplayCueTagOverride
		: GetDefaultExecuteGameplayCueTag();
}

FGameplayTag UElysiaBossGameplayAbility::GetRecoveryGameplayCueTag() const
{
	return RecoveryGameplayCueTagOverride.IsValid()
		? RecoveryGameplayCueTagOverride
		: GetDefaultRecoveryGameplayCueTag();
}
