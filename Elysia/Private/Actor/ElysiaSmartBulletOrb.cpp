// Copyright GhostLazy


#include "Actor/ElysiaSmartBulletOrb.h"

#include "Components/StaticMeshComponent.h"

AElysiaSmartBulletOrb::AElysiaSmartBulletOrb()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetNetUpdateFrequency(100.f);

	SceneRoot = CreateDefaultSubobject<USceneComponent>("SceneRoot");
	SetRootComponent(SceneRoot);

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>("VisualMesh");
	VisualMesh->SetupAttachment(SceneRoot);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AElysiaSmartBulletOrb::BeginPlay()
{
	Super::BeginPlay();
	SetReplicateMovement(true);
}

void AElysiaSmartBulletOrb::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!HasAuthority())
	{
		return;
	}

	if (!IsValid(FollowTarget))
	{
		Destroy();
		return;
	}

	const FVector DesiredLocation = GetDesiredLocation();
	const FVector NewLocation = FMath::VInterpTo(GetActorLocation(), DesiredLocation, DeltaSeconds, FollowInterpSpeed);
	SetActorLocation(NewLocation);
}

void AElysiaSmartBulletOrb::InitializeOrb(AActor* InFollowTarget, int32 InOrbIndex, int32 InOrbCount, float InOrbitRadius, float InHeightOffset, float InFollowInterpSpeed)
{
	FollowTarget = InFollowTarget;
	UpdateOrbLayout(InOrbIndex, InOrbCount, InOrbitRadius, InHeightOffset, InFollowInterpSpeed);
}

void AElysiaSmartBulletOrb::UpdateOrbLayout(int32 InOrbIndex, int32 InOrbCount, float InOrbitRadius, float InHeightOffset, float InFollowInterpSpeed)
{
	OrbIndex = FMath::Max(0, InOrbIndex);
	OrbCount = FMath::Max(1, InOrbCount);
	OrbitRadius = FMath::Max(0.f, InOrbitRadius);
	HeightOffset = InHeightOffset;
	FollowInterpSpeed = FMath::Max(0.f, InFollowInterpSpeed);
}

FVector AElysiaSmartBulletOrb::GetMuzzleLocation() const
{
	static const FName MuzzleSocketName("Muzzle");
	if (VisualMesh && VisualMesh->DoesSocketExist(MuzzleSocketName))
	{
		return VisualMesh->GetSocketLocation(MuzzleSocketName);
	}

	return GetActorLocation();
}

FVector AElysiaSmartBulletOrb::GetDesiredLocation() const
{
	if (!IsValid(FollowTarget))
	{
		return GetActorLocation();
	}

	const FVector RightVector = FollowTarget->GetActorRightVector().GetSafeNormal2D();
	const float SideSign = OrbCount > 1 && OrbIndex % 2 == 1 ? -1.f : 1.f;
	const FVector OrbitOffset = RightVector * SideSign * OrbitRadius + FVector::UpVector * HeightOffset;

	return FollowTarget->GetActorLocation() + OrbitOffset;
}
