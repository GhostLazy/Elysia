// Copyright GhostLazy


#include "AbilitySystem/Abilities/ElysiaBossChargeAbility.h"

#include "Character/ElysiaBossBase.h"
#include "Components/CapsuleComponent.h"
#include "Engine/OverlapResult.h"
#include "Elysia/Elysia.h"
#include "TimerManager.h"

void UElysiaBossChargeAbility::ExecuteBossSkill()
{
	AElysiaBossBase* Boss = GetBossAvatar();
	if (!Boss || !Boss->HasAuthority())
	{
		return;
	}

	bChargeActive = true;
	ChargeStartLocation = Boss->GetActorLocation();
	ChargeHitTargets.Empty();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ChargeTickTimerHandle);
		World->GetTimerManager().SetTimer(
			ChargeTickTimerHandle,
			this,
			&UElysiaBossChargeAbility::TickChargeMovement,
			FMath::Max(0.01f, ChargeTickInterval),
			true);
	}
}

void UElysiaBossChargeAbility::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ChargeTickTimerHandle);
	}

	bChargeActive = false;
	ChargeHitTargets.Empty();
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UElysiaBossChargeAbility::TickChargeMovement()
{
	AElysiaBossBase* Boss = GetBossAvatar();
	if (!Boss || !Boss->HasAuthority() || !bChargeActive)
	{
		StopCharge(true);
		return;
	}

	const float TickInterval = FMath::Max(0.01f, ChargeTickInterval);
	const FVector DeltaMove = GetLockedSkillDirection().GetSafeNormal2D() * ChargeSpeed * TickInterval;

	FHitResult SweepHitResult;
	Boss->AddActorWorldOffset(DeltaMove, true, &SweepHitResult);

	TArray<FOverlapResult> Overlaps;
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Player);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ElysiaBossChargeDamage), false);
	QueryParams.AddIgnoredActor(Boss);

	const UCapsuleComponent* CapsuleComponent = Boss->GetCapsuleComponent();
	const float HitRadius = (CapsuleComponent ? CapsuleComponent->GetScaledCapsuleRadius() : 0.f) + ChargeHitRadiusPadding;
	if (Boss->GetWorld()->OverlapMultiByObjectType(
		Overlaps,
		Boss->GetActorLocation(),
		FQuat::Identity,
		ObjectQueryParams,
		FCollisionShape::MakeSphere(HitRadius),
		QueryParams))
	{
		for (const FOverlapResult& Overlap : Overlaps)
		{
			AActor* OverlapActor = Overlap.GetActor();
			if (!IsValid(OverlapActor))
			{
				continue;
			}

			const TWeakObjectPtr<AActor> WeakActor(OverlapActor);
			if (ChargeHitTargets.Contains(WeakActor))
			{
				continue;
			}

			if (ApplyDamageToTarget(OverlapActor))
			{
				ChargeHitTargets.Add(WeakActor);
				if (bStopChargeOnFirstHit)
				{
					StopCharge(false);
					return;
				}
			}
		}
	}

	if (SweepHitResult.bBlockingHit || FVector::Dist2D(ChargeStartLocation, Boss->GetActorLocation()) >= ChargeMaxDistance)
	{
		StopCharge(SweepHitResult.bBlockingHit);
	}
}

void UElysiaBossChargeAbility::StopCharge(bool bInterrupted)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ChargeTickTimerHandle);
	}

	bChargeActive = false;
	ChargeHitTargets.Empty();
	BeginBossAbilityRecovery(RecoveryTime);
}
