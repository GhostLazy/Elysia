// Copyright GhostLazy


#include "AbilitySystem/Abilities/Enemy/ElysiaBossSweepAbility.h"

#include "Character/ElysiaBossBase.h"
#include "Engine/OverlapResult.h"
#include "ElysiaGameplayTags.h"
#include "Elysia/Elysia.h"

void UElysiaBossSweepAbility::ExecuteBossSkill()
{
	AElysiaBossBase* Boss = GetBossAvatar();
	if (!Boss || !Boss->HasAuthority())
	{
		return;
	}

	UWorld* World = Boss->GetWorld();
	if (!World)
	{
		return;
	}

	TArray<FOverlapResult> Overlaps;
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Player);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ElysiaBossSweepDamage), false);
	QueryParams.AddIgnoredActor(Boss);

	const float ClampedSweepAngle = FMath::Clamp(SweepAngle, 0.f, 360.f);
	const FVector SweepDirection = GetLockedSkillDirection().GetSafeNormal2D();
	const bool bUseAngleFilter = ClampedSweepAngle < 360.f && !SweepDirection.IsNearlyZero();
	const float MinSweepDot = FMath::Cos(FMath::DegreesToRadians(ClampedSweepAngle * 0.5f));

	if (World->OverlapMultiByObjectType(
		Overlaps,
		Boss->GetActorLocation(),
		FQuat::Identity,
		ObjectQueryParams,
		FCollisionShape::MakeSphere(SweepRadius),
		QueryParams))
	{
		for (const FOverlapResult& Overlap : Overlaps)
		{
			AActor* OverlapActor = Overlap.GetActor();
			if (!IsValid(OverlapActor))
			{
				continue;
			}

			if (bUseAngleFilter)
			{
				const FVector DirectionToTarget = (OverlapActor->GetActorLocation() - Boss->GetActorLocation()).GetSafeNormal2D();
				if (DirectionToTarget.IsNearlyZero() || FVector::DotProduct(SweepDirection, DirectionToTarget) < MinSweepDot)
				{
					continue;
				}
			}

			ApplyDamageToTarget(OverlapActor);
		}
	}
}

FGameplayTag UElysiaBossSweepAbility::GetDefaultWindupGameplayCueTag() const
{
	return FElysiaGameplayTags::Get().GameplayCue_Boss_Sweep_Windup;
}

FGameplayTag UElysiaBossSweepAbility::GetDefaultExecuteGameplayCueTag() const
{
	return FElysiaGameplayTags::Get().GameplayCue_Boss_Sweep_Execute;
}

void UElysiaBossSweepAbility::BuildWindupGameplayCueParameters(
	FGameplayCueParameters& OutParameters,
	AElysiaBossBase* Boss,
	const FVector& Origin,
	const FVector& Direction) const
{
	Super::BuildWindupGameplayCueParameters(OutParameters, Boss, Origin, Direction);
	OutParameters.RawMagnitude = SweepRadius;
	OutParameters.NormalizedMagnitude = SweepAngle;
}

void UElysiaBossSweepAbility::BuildExecuteGameplayCueParameters(
	FGameplayCueParameters& OutParameters,
	AElysiaBossBase* Boss,
	const FVector& Origin,
	const FVector& Direction) const
{
	Super::BuildExecuteGameplayCueParameters(OutParameters, Boss, Origin, Direction);
	OutParameters.RawMagnitude = SweepRadius;
	OutParameters.NormalizedMagnitude = SweepAngle;
}
