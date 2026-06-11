// Copyright GhostLazy


#include "AbilitySystem/Abilities/Enemy/ElysiaBossGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "Character/ElysiaBossBase.h"
#include "GameFramework/CharacterMovementComponent.h"

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

	const FVector Origin = Boss->GetActorLocation();
	FGameplayCueParameters ExecuteCueParameters;
	BuildExecuteGameplayCueParameters(ExecuteCueParameters, Boss, Origin, LockedSkillDirection);
	ExecuteBossAbilityCue(GetExecuteGameplayCueTag(), ExecuteCueParameters);

	ExecuteBossSkill();
	if (ShouldAutoEndAfterExecute() && IsActive())
	{
		FinishBossAbility();
	}
}

void UElysiaBossGameplayAbility::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	AElysiaBossBase* Boss = GetBossAvatar(ActorInfo);
	if (Boss)
	{
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

void UElysiaBossGameplayAbility::FinishBossAbility()
{
	if (IsActive())
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
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

FGameplayTag UElysiaBossGameplayAbility::GetDefaultExecuteGameplayCueTag() const
{
	return FGameplayTag();
}

void UElysiaBossGameplayAbility::BuildExecuteGameplayCueParameters(
	FGameplayCueParameters& OutParameters,
	AElysiaBossBase* Boss,
	const FVector& Origin,
	const FVector& Direction) const
{
	BuildBaseGameplayCueParameters(OutParameters, Boss, Origin, Direction);
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

FGameplayTag UElysiaBossGameplayAbility::GetExecuteGameplayCueTag() const
{
	return ExecuteGameplayCueTagOverride.IsValid()
		? ExecuteGameplayCueTagOverride
		: GetDefaultExecuteGameplayCueTag();
}
