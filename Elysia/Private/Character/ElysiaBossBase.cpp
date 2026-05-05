// Copyright GhostLazy


#include "Character/ElysiaBossBase.h"

#include "AI/AIController/ElysiaBossAIController.h"
#include "Actor/ElysiaBossLaserActor.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/OverlapResult.h"
#include "Elysia/Elysia.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Interface/CombatInterface.h"

AElysiaBossBase::AElysiaBossBase()
{
	EnemyType = EElysiaEnemyType::Boss;
	AIControllerClass = AElysiaBossAIController::StaticClass();
}

void AElysiaBossBase::Die()
{
	ClearBossSkillState();
	Super::Die();
}

void AElysiaBossBase::SetCombatTarget(AActor* InTargetActor)
{
	CombatTarget = InTargetActor;
}

bool AElysiaBossBase::HasValidCombatTarget() const
{
	if (AActor* TargetActor = CombatTarget.Get())
	{
		const ICombatInterface* CombatInterface = Cast<ICombatInterface>(TargetActor);
		return CombatInterface && CombatInterface->IsPlayer() && !CombatInterface->IsDead();
	}

	return false;
}

float AElysiaBossBase::GetDistanceToCombatTarget2D() const
{
	if (const AActor* TargetActor = CombatTarget.Get())
	{
		return FVector::Dist2D(GetActorLocation(), TargetActor->GetActorLocation());
	}

	return FLT_MAX;
}

bool AElysiaBossBase::IsTargetInRangeForSkill(EElysiaBossSkillType SkillType) const
{
	const FElysiaBossSkillSpec* SkillSpec = FindSkillSpec(SkillType);
	if (!SkillSpec || !HasValidCombatTarget())
	{
		return false;
	}

	const float DistanceToTarget = GetDistanceToCombatTarget2D();
	return DistanceToTarget >= SkillSpec->MinRange && DistanceToTarget <= SkillSpec->CastRange;
}

bool AElysiaBossBase::CanCastSkill(EElysiaBossSkillType SkillType) const
{
	if (bIsCastingSkill || bIsCharging)
	{
		return false;
	}

	const FElysiaBossSkillSpec* SkillSpec = FindSkillSpec(SkillType);
	if (!SkillSpec || SkillSpec->SkillType == EElysiaBossSkillType::None)
	{
		return false;
	}

	if (!HasValidCombatTarget())
	{
		return false;
	}

	if (!IsTargetInRangeForSkill(SkillType))
	{
		return false;
	}

	const float* CooldownEndTime = SkillCooldownEndTimeByType.Find(SkillType);
	return !CooldownEndTime || GetWorld()->GetTimeSeconds() >= *CooldownEndTime;
}

bool AElysiaBossBase::TryCastSkill(EElysiaBossSkillType SkillType)
{
	if (!HasAuthority() || !CanCastSkill(SkillType))
	{
		return false;
	}

	const FElysiaBossSkillSpec* SkillSpec = FindSkillSpec(SkillType);
	if (!SkillSpec)
	{
		return false;
	}

	CurrentSkillType = SkillType;
	bIsCastingSkill = true;
	bIsCharging = false;
	LockedSkillDirection = GetLockedSkillDirection();

	if (!LockedSkillDirection.IsNearlyZero())
	{
		SetActorRotation(LockedSkillDirection.Rotation());
	}

	if (SkillSpec->CastMontage)
	{
		PlayAnimMontage(SkillSpec->CastMontage);
	}

	SkillCooldownEndTimeByType.FindOrAdd(SkillType) = GetWorld()->GetTimeSeconds() + FMath::Max(0.f, SkillSpec->Cooldown);

	switch (SkillType)
	{
	case EElysiaBossSkillType::SweepAOE:
		ExecuteSweepAOE();
		break;
	case EElysiaBossSkillType::FixedLaser:
		SpawnFixedLaser();
		break;
	case EElysiaBossSkillType::Charge:
		BeginCharge();
		break;
	default:
		ClearBossSkillState();
		return false;
	}

	return true;
}

bool AElysiaBossBase::TryCastBestAvailableSkill()
{
	float TotalWeight = 0.f;
	TArray<const FElysiaBossSkillSpec*> AvailableSkills;
	for (const FElysiaBossSkillSpec& SkillSpec : BossSkills)
	{
		if (CanCastSkill(SkillSpec.SkillType) && SkillSpec.Weight > 0.f)
		{
			AvailableSkills.Add(&SkillSpec);
			TotalWeight += SkillSpec.Weight;
		}
	}

	if (AvailableSkills.IsEmpty() || TotalWeight <= 0.f)
	{
		return false;
	}

	float RemainingWeight = FMath::FRandRange(0.f, TotalWeight);
	for (const FElysiaBossSkillSpec* SkillSpec : AvailableSkills)
	{
		RemainingWeight -= SkillSpec->Weight;
		if (RemainingWeight <= 0.f)
		{
			return TryCastSkill(SkillSpec->SkillType);
		}
	}

	return TryCastSkill(AvailableSkills[0]->SkillType);
}

void AElysiaBossBase::ApplyLaserDamageInDirection(const FVector& LaserOrigin, const FVector& LaserDirection, const FElysiaBossSkillSpec& SkillSpec)
{
	if (!HasAuthority() || !AbilitySystemComponent || !SkillSpec.DamageEffectClass)
	{
		return;
	}

	TArray<FOverlapResult> Overlaps;
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Player);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ElysiaBossLaserDamage), false);
	QueryParams.AddIgnoredActor(this);

	const FVector BoxCenter = LaserOrigin + LaserDirection.GetSafeNormal2D() * (SkillSpec.LaserLength * 0.5f);
	const FVector HalfExtent(SkillSpec.LaserLength * 0.5f, SkillSpec.LaserWidth * 0.5f, 120.f);
	const FQuat BoxRotation = LaserDirection.Rotation().Quaternion();

	if (GetWorld()->OverlapMultiByObjectType(Overlaps, BoxCenter, BoxRotation, ObjectQueryParams, FCollisionShape::MakeBox(HalfExtent), QueryParams))
	{
		for (const FOverlapResult& Overlap : Overlaps)
		{
			TryApplyDamageToActor(Overlap.GetActor(), SkillSpec.DamageEffectClass, SkillSpec.DamageEffectLevel);
		}
	}
}

const FElysiaBossSkillSpec* AElysiaBossBase::FindSkillSpec(EElysiaBossSkillType SkillType) const
{
	return BossSkills.FindByPredicate([SkillType](const FElysiaBossSkillSpec& SkillSpec)
	{
		return SkillSpec.SkillType == SkillType;
	});
}

bool AElysiaBossBase::TryApplyDamageToActor(AActor* TargetActor, TSubclassOf<UGameplayEffect> DamageEffectClass, float DamageEffectLevel)
{
	if (!IsValid(TargetActor) || !AbilitySystemComponent || !DamageEffectClass)
	{
		return false;
	}

	const ICombatInterface* CombatInterface = Cast<ICombatInterface>(TargetActor);
	if (!CombatInterface || !CombatInterface->IsPlayer() || CombatInterface->IsDead())
	{
		return false;
	}

	if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor))
	{
		FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
		EffectContext.AddSourceObject(this);

		const FGameplayEffectSpecHandle EffectSpecHandle = AbilitySystemComponent->MakeOutgoingSpec(
			DamageEffectClass,
			FMath::Max(1.f, DamageEffectLevel),
			EffectContext);

		if (EffectSpecHandle.IsValid())
		{
			TargetASC->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
			return true;
		}
	}

	return false;
}

void AElysiaBossBase::BeginSkillRecovery(float RecoveryTime, EElysiaBossSkillType SkillType)
{
	CurrentSkillType = SkillType;
	GetWorldTimerManager().ClearTimer(SkillRecoveryTimerHandle);
	GetWorldTimerManager().SetTimer(SkillRecoveryTimerHandle, this, &AElysiaBossBase::FinishRecovery, FMath::Max(0.f, RecoveryTime), false);
}

void AElysiaBossBase::FinishRecovery()
{
	const EElysiaBossSkillType FinishedSkillType = CurrentSkillType;
	ClearBossSkillState();
	OnBossSkillFinished.Broadcast(FinishedSkillType);
}

void AElysiaBossBase::ExecuteSweepAOE()
{
	const FElysiaBossSkillSpec* SkillSpec = FindSkillSpec(CurrentSkillType);
	if (!SkillSpec)
	{
		ClearBossSkillState();
		return;
	}

	TArray<FOverlapResult> Overlaps;
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Player);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ElysiaBossSweepDamage), false);
	QueryParams.AddIgnoredActor(this);

	if (GetWorld()->OverlapMultiByObjectType(
		Overlaps,
		GetActorLocation(),
		FQuat::Identity,
		ObjectQueryParams,
		FCollisionShape::MakeSphere(SkillSpec->SweepRadius),
		QueryParams))
	{
		for (const FOverlapResult& Overlap : Overlaps)
		{
			TryApplyDamageToActor(Overlap.GetActor(), SkillSpec->DamageEffectClass, SkillSpec->DamageEffectLevel);
		}
	}

	BeginSkillRecovery(SkillSpec->RecoveryTime, SkillSpec->SkillType);
}

void AElysiaBossBase::SpawnFixedLaser()
{
	const FElysiaBossSkillSpec* SkillSpec = FindSkillSpec(CurrentSkillType);
	if (!SkillSpec)
	{
		ClearBossSkillState();
		return;
	}

	if (SkillSpec->LaserActorClass)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		if (AElysiaBossLaserActor* LaserActor = GetWorld()->SpawnActor<AElysiaBossLaserActor>(
			SkillSpec->LaserActorClass,
			GetActorLocation(),
			LockedSkillDirection.Rotation(),
			SpawnParameters))
		{
			LaserActor->InitializeLaser(this, GetActorLocation(), LockedSkillDirection, *SkillSpec);
			ActiveLaserActor = LaserActor;
		}
	}
	else
	{
		ApplyLaserDamageInDirection(GetActorLocation(), LockedSkillDirection, *SkillSpec);
	}

	BeginSkillRecovery(SkillSpec->LaserDuration + SkillSpec->RecoveryTime, SkillSpec->SkillType);
}

void AElysiaBossBase::BeginCharge()
{
	const FElysiaBossSkillSpec* SkillSpec = FindSkillSpec(CurrentSkillType);
	if (!SkillSpec)
	{
		ClearBossSkillState();
		return;
	}

	bIsCharging = true;
	ChargeStartLocation = GetActorLocation();
	ChargeHitTargets.Empty();
	GetWorldTimerManager().ClearTimer(ChargeTickTimerHandle);
	GetWorldTimerManager().SetTimer(ChargeTickTimerHandle, this, &AElysiaBossBase::TickChargeMovement, 0.02f, true);
}

void AElysiaBossBase::TickChargeMovement()
{
	const FElysiaBossSkillSpec* SkillSpec = FindSkillSpec(CurrentSkillType);
	if (!SkillSpec || !bIsCharging)
	{
		StopCharge(true);
		return;
	}

	FHitResult SweepHitResult;
	const FVector DeltaMove = LockedSkillDirection.GetSafeNormal2D() * SkillSpec->ChargeSpeed * 0.02f;
	AddActorWorldOffset(DeltaMove, true, &SweepHitResult);

	TArray<FOverlapResult> Overlaps;
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Player);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ElysiaBossChargeDamage), false);
	QueryParams.AddIgnoredActor(this);

	if (GetWorld()->OverlapMultiByObjectType(
		Overlaps,
		GetActorLocation(),
		FQuat::Identity,
		ObjectQueryParams,
		FCollisionShape::MakeSphere(GetCapsuleComponent()->GetScaledCapsuleRadius() + 50.f),
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

			if (TryApplyDamageToActor(OverlapActor, SkillSpec->DamageEffectClass, SkillSpec->DamageEffectLevel))
			{
				ChargeHitTargets.Add(WeakActor);
				if (SkillSpec->bStopChargeOnFirstHit)
				{
					StopCharge(false);
					return;
				}
			}
		}
	}

	if (SweepHitResult.bBlockingHit || FVector::Dist2D(ChargeStartLocation, GetActorLocation()) >= SkillSpec->ChargeMaxDistance)
	{
		StopCharge(SweepHitResult.bBlockingHit);
	}
}

void AElysiaBossBase::StopCharge(bool bInterrupted)
{
	GetWorldTimerManager().ClearTimer(ChargeTickTimerHandle);
	bIsCharging = false;
	ChargeHitTargets.Empty();

	if (const FElysiaBossSkillSpec* SkillSpec = FindSkillSpec(CurrentSkillType))
	{
		BeginSkillRecovery(SkillSpec->RecoveryTime, SkillSpec->SkillType);
	}
	else if (bInterrupted)
	{
		ClearBossSkillState();
	}
}

void AElysiaBossBase::ClearBossSkillState()
{
	GetWorldTimerManager().ClearTimer(SkillRecoveryTimerHandle);
	GetWorldTimerManager().ClearTimer(ChargeTickTimerHandle);
	bIsCastingSkill = false;
	bIsCharging = false;
	CurrentSkillType = EElysiaBossSkillType::None;
	ChargeHitTargets.Empty();

	if (AElysiaBossLaserActor* LaserActor = ActiveLaserActor.Get())
	{
		LaserActor->Destroy();
	}
	ActiveLaserActor = nullptr;
}

FVector AElysiaBossBase::GetLockedSkillDirection() const
{
	if (const AActor* TargetActor = CombatTarget.Get())
	{
		const FVector Direction = (TargetActor->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
		if (!Direction.IsNearlyZero())
		{
			return Direction;
		}
	}

	return GetActorForwardVector().GetSafeNormal2D();
}
