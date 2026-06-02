// Copyright GhostLazy


#include "AbilitySystem/Abilities/ElysiaBossLaserAbility.h"

#include "Actor/ElysiaBossLaserActor.h"
#include "Character/ElysiaBossBase.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"

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

void UElysiaBossLaserAbility::OnBossAbilityWindupStarted_Implementation(AElysiaBossBase* Boss, FVector Origin, FVector Direction)
{
	Super::OnBossAbilityWindupStarted_Implementation(Boss, Origin, Direction);
	DestroyWarningEffect();

	ActiveWarningEffect = SpawnEffectAtLocation(LaserWarningEffect, Origin, Direction);
	ApplyLaserEffectParameters(ActiveWarningEffect, WindupTime);
	if (ActiveWarningEffect)
	{
		const float LengthScale = FMath::Max(0.01f, LaserLength / 100.f);
		const float WidthScale = FMath::Max(0.01f, LaserWidth / 100.f);
		ActiveWarningEffect->SetWorldScale3D(FVector(LengthScale, WidthScale, 1.f));
	}
}

void UElysiaBossLaserAbility::OnBossAbilityExecuted_Implementation(AElysiaBossBase* Boss, FVector Origin, FVector Direction)
{
	DestroyWarningEffect();
	Super::OnBossAbilityExecuted_Implementation(Boss, Origin, Direction);

	if (UNiagaraComponent* FireEffect = SpawnEffectAtLocation(LaserFireEffect, Origin, Direction))
	{
		ApplyLaserEffectParameters(FireEffect, LaserDuration);
	}

	if (LaserFireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, LaserFireSound, Origin);
	}
}

void UElysiaBossLaserAbility::OnBossAbilityRecovered_Implementation(AElysiaBossBase* Boss)
{
	DestroyWarningEffect();
	if (ActiveLaserActor)
	{
		ActiveLaserActor->Destroy();
		ActiveLaserActor = nullptr;
	}

	Super::OnBossAbilityRecovered_Implementation(Boss);
}

void UElysiaBossLaserAbility::ApplyLaserEffectParameters(UNiagaraComponent* NiagaraComponent, float Duration) const
{
	if (!NiagaraComponent)
	{
		return;
	}

	if (!NiagaraLengthParameterName.IsNone())
	{
		NiagaraComponent->SetVariableFloat(NiagaraLengthParameterName, LaserLength);
	}
	if (!NiagaraWidthParameterName.IsNone())
	{
		NiagaraComponent->SetVariableFloat(NiagaraWidthParameterName, LaserWidth);
	}
	if (!NiagaraDurationParameterName.IsNone())
	{
		NiagaraComponent->SetVariableFloat(NiagaraDurationParameterName, Duration);
	}
}

void UElysiaBossLaserAbility::DestroyWarningEffect()
{
	if (ActiveWarningEffect)
	{
		ActiveWarningEffect->Deactivate();
		ActiveWarningEffect->DestroyComponent();
		ActiveWarningEffect = nullptr;
	}
}
