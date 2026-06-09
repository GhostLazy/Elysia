// Copyright GhostLazy


#include "AbilitySystem/Abilities/Enemy/ElysiaBossLaserAbility.h"

#include "Actor/ElysiaBossLaserActor.h"
#include "Character/ElysiaBossBase.h"
#include "ElysiaGameplayTags.h"

void UElysiaBossLaserAbility::ExecuteBossSkill()
{
	AElysiaBossBase* Boss = GetBossAvatar();
	if (!Boss || !Boss->HasAuthority() || !LaserActorClass)
	{
		return;
	}

	const FVector Origin = Boss->GetActorLocation();
	const FVector Direction = GetLockedSkillDirection();
	const FTransform SpawnTransform(Direction.Rotation(), Origin);
	if (AElysiaBossLaserActor* LaserActor = Boss->GetWorld()->SpawnActorDeferred<AElysiaBossLaserActor>(
		LaserActorClass,
		SpawnTransform,
		Boss,
		Boss,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn))
	{
		LaserActor->InitializeLaser(
			Boss,
			Origin,
			Direction,
			DamageEffectClass,
			DamageEffectLevel,
			LaserLength,
			LaserWidth,
			LaserDuration,
			LaserTickInterval);
		LaserActor->FinishSpawning(SpawnTransform);
		ActiveLaserActor = LaserActor;
	}
}

float UElysiaBossLaserAbility::GetPostExecuteRecoveryTime() const
{
	return FMath::Max(0.f, LaserDuration) + RecoveryTime;
}

void UElysiaBossLaserAbility::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	if (IsValid(ActiveLaserActor))
	{
		ActiveLaserActor->Destroy();
	}
	ActiveLaserActor = nullptr;

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

FGameplayTag UElysiaBossLaserAbility::GetDefaultWindupGameplayCueTag() const
{
	return FElysiaGameplayTags::Get().GameplayCue_Boss_Laser_Windup;
}

FGameplayTag UElysiaBossLaserAbility::GetDefaultExecuteGameplayCueTag() const
{
	return FElysiaGameplayTags::Get().GameplayCue_Boss_Laser_Execute;
}

void UElysiaBossLaserAbility::BuildWindupGameplayCueParameters(
	FGameplayCueParameters& OutParameters,
	AElysiaBossBase* Boss,
	const FVector& Origin,
	const FVector& Direction) const
{
	Super::BuildWindupGameplayCueParameters(OutParameters, Boss, Origin, Direction);
	OutParameters.RawMagnitude = LaserLength;
	OutParameters.NormalizedMagnitude = LaserWidth;
}

void UElysiaBossLaserAbility::BuildExecuteGameplayCueParameters(
	FGameplayCueParameters& OutParameters,
	AElysiaBossBase* Boss,
	const FVector& Origin,
	const FVector& Direction) const
{
	Super::BuildExecuteGameplayCueParameters(OutParameters, Boss, Origin, Direction);
	OutParameters.RawMagnitude = LaserLength;
	OutParameters.NormalizedMagnitude = LaserWidth;
}
