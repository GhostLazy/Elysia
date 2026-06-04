// Copyright GhostLazy


#include "AbilitySystem/Abilities/Enemy/ElysiaBossSweepAbility.h"

#include "Character/ElysiaBossBase.h"
#include "Engine/OverlapResult.h"
#include "Elysia/Elysia.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"

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

void UElysiaBossSweepAbility::OnBossAbilityWindupStarted_Implementation(AElysiaBossBase* Boss, FVector Origin, FVector Direction)
{
	Super::OnBossAbilityWindupStarted_Implementation(Boss, Origin, Direction);
	DestroyWarningEffect();

	ActiveWarningEffect = SpawnEffectAtLocation(SweepWarningEffect, Origin, Direction);
	ApplySweepEffectParameters(ActiveWarningEffect);
	if (ActiveWarningEffect)
	{
		const float Scale = FMath::Max(0.01f, SweepRadius / 100.f);
		ActiveWarningEffect->SetWorldScale3D(FVector(Scale, Scale, 1.f));
	}
}

void UElysiaBossSweepAbility::OnBossAbilityExecuted_Implementation(AElysiaBossBase* Boss, FVector Origin, FVector Direction)
{
	DestroyWarningEffect();
	Super::OnBossAbilityExecuted_Implementation(Boss, Origin, Direction);

	if (UNiagaraComponent* ImpactEffect = SpawnEffectAtLocation(SweepImpactEffect, Origin, Direction))
	{
		ApplySweepEffectParameters(ImpactEffect);
		const float Scale = FMath::Max(0.01f, SweepRadius / 100.f);
		ImpactEffect->SetWorldScale3D(FVector(Scale, Scale, 1.f));
	}

	if (SweepImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, SweepImpactSound, Origin);
	}
}

void UElysiaBossSweepAbility::OnBossAbilityRecovered_Implementation(AElysiaBossBase* Boss)
{
	DestroyWarningEffect();
	Super::OnBossAbilityRecovered_Implementation(Boss);
}

void UElysiaBossSweepAbility::ApplySweepEffectParameters(UNiagaraComponent* NiagaraComponent) const
{
	if (!NiagaraComponent)
	{
		return;
	}

	if (!NiagaraRadiusParameterName.IsNone())
	{
		NiagaraComponent->SetVariableFloat(NiagaraRadiusParameterName, SweepRadius);
	}
	if (!NiagaraAngleParameterName.IsNone())
	{
		NiagaraComponent->SetVariableFloat(NiagaraAngleParameterName, SweepAngle);
	}
	if (!NiagaraDurationParameterName.IsNone())
	{
		NiagaraComponent->SetVariableFloat(NiagaraDurationParameterName, WindupTime);
	}
}

void UElysiaBossSweepAbility::DestroyWarningEffect()
{
	if (ActiveWarningEffect)
	{
		ActiveWarningEffect->Deactivate();
		ActiveWarningEffect->DestroyComponent();
		ActiveWarningEffect = nullptr;
	}
}
