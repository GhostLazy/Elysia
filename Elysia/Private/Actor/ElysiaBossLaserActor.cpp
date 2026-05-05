// Copyright GhostLazy


#include "Actor/ElysiaBossLaserActor.h"

#include "Character/ElysiaBossBase.h"

AElysiaBossLaserActor::AElysiaBossLaserActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

void AElysiaBossLaserActor::InitializeLaser(AElysiaBossBase* InSourceBoss, const FVector& InOrigin, const FVector& InDirection, const FElysiaBossSkillSpec& InSkillSpec)
{
	SourceBoss = InSourceBoss;
	Origin = InOrigin;
	Direction = InDirection.GetSafeNormal2D();
	SkillSpec = InSkillSpec;
	SetActorLocation(Origin);
	SetActorRotation(Direction.Rotation());
}

void AElysiaBossLaserActor::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
	{
		return;
	}

	GetWorldTimerManager().SetTimer(DamageTickTimerHandle, this, &AElysiaBossLaserActor::HandleDamageTick, FMath::Max(0.01f, SkillSpec.LaserTickInterval), true);
	GetWorldTimerManager().SetTimer(FinishTimerHandle, this, &AElysiaBossLaserActor::FinishLaser, FMath::Max(0.01f, SkillSpec.LaserDuration), false);
}

void AElysiaBossLaserActor::HandleDamageTick()
{
	if (!HasAuthority() || !SourceBoss.IsValid())
	{
		return;
	}

	SourceBoss->ApplyLaserDamageInDirection(Origin, Direction, SkillSpec);
}

void AElysiaBossLaserActor::FinishLaser()
{
	GetWorldTimerManager().ClearTimer(DamageTickTimerHandle);
	Destroy();
}
