// Copyright GhostLazy


#include "Actor/ElysiaBossLaserActor.h"

#include "Character/ElysiaBossBase.h"
#include "Components/SceneComponent.h"
#include "Engine/OverlapResult.h"
#include "Elysia/Elysia.h"

AElysiaBossLaserActor::AElysiaBossLaserActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>("SceneRoot");
	SetRootComponent(SceneRoot);
}

void AElysiaBossLaserActor::InitializeLaser(
	AElysiaBossBase* InSourceBoss,
	const FVector& InOrigin,
	const FVector& InDirection,
	TSubclassOf<UGameplayEffect> InDamageEffectClass,
	float InDamageEffectLevel,
	float InLaserLength,
	float InLaserWidth,
	float InLaserDuration,
	float InLaserTickInterval)
{
	SourceBoss = InSourceBoss;
	Origin = InOrigin;
	Direction = InDirection.GetSafeNormal2D();
	DamageEffectClass = InDamageEffectClass;
	DamageEffectLevel = InDamageEffectLevel;
	LaserLength = FMath::Max(0.f, InLaserLength);
	LaserWidth = FMath::Max(0.f, InLaserWidth);
	LaserDuration = FMath::Max(0.f, InLaserDuration);
	LaserTickInterval = FMath::Max(0.01f, InLaserTickInterval);
	bLaserInitialized = true;
	SetActorLocation(Origin);
	SetActorRotation(Direction.Rotation());

	if (HasActorBegunPlay())
	{
		StartLaser();
	}
}

void AElysiaBossLaserActor::BeginPlay()
{
	Super::BeginPlay();

	StartLaser();
}

void AElysiaBossLaserActor::StartLaser()
{
	if (bLaserStarted || !bLaserInitialized)
	{
		return;
	}

	bLaserStarted = true;

	if (!HasAuthority())
	{
		return;
	}

	if (!SourceBoss)
	{
		Destroy();
		return;
	}

	GetWorldTimerManager().SetTimer(DamageTickTimerHandle, this, &AElysiaBossLaserActor::HandleDamageTick, LaserTickInterval, true);
	GetWorldTimerManager().SetTimer(FinishTimerHandle, this, &AElysiaBossLaserActor::FinishLaser, FMath::Max(0.01f, LaserDuration), false);
}

void AElysiaBossLaserActor::HandleDamageTick()
{
	if (!HasAuthority() || !SourceBoss)
	{
		return;
	}

	TArray<FOverlapResult> Overlaps;
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Player);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ElysiaBossLaserDamage), false);
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(SourceBoss);

	const FVector BoxCenter = Origin + Direction.GetSafeNormal2D() * (LaserLength * 0.5f);
	const FVector HalfExtent(LaserLength * 0.5f, LaserWidth * 0.5f, 120.f);
	const FQuat BoxRotation = Direction.Rotation().Quaternion();

	if (GetWorld()->OverlapMultiByObjectType(Overlaps, BoxCenter, BoxRotation, ObjectQueryParams, FCollisionShape::MakeBox(HalfExtent), QueryParams))
	{
		for (const FOverlapResult& Overlap : Overlaps)
		{
			SourceBoss->ApplyBossDamageToTarget(Overlap.GetActor(), DamageEffectClass, DamageEffectLevel);
		}
	}
}

void AElysiaBossLaserActor::FinishLaser()
{
	GetWorldTimerManager().ClearTimer(DamageTickTimerHandle);
	Destroy();
}
