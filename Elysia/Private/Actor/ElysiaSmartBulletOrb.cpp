// Copyright GhostLazy


#include "Actor/ElysiaSmartBulletOrb.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"

AElysiaSmartBulletOrb::AElysiaSmartBulletOrb()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PostPhysics;
	bReplicates = true;
	SetReplicatingMovement(false);
	SetNetUpdateFrequency(10.f);
	SetMinNetUpdateFrequency(2.f);

	SceneRoot = CreateDefaultSubobject<USceneComponent>("SceneRoot");
	SetRootComponent(SceneRoot);

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>("VisualMesh");
	VisualMesh->SetupAttachment(SceneRoot);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AElysiaSmartBulletOrb::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AElysiaSmartBulletOrb, FollowTarget);
	DOREPLIFETIME(AElysiaSmartBulletOrb, OrbIndex);
	DOREPLIFETIME(AElysiaSmartBulletOrb, OrbCount);
	DOREPLIFETIME(AElysiaSmartBulletOrb, OrbitRadius);
	DOREPLIFETIME(AElysiaSmartBulletOrb, HeightOffset);
	DOREPLIFETIME(AElysiaSmartBulletOrb, FollowInterpSpeed);
}

void AElysiaSmartBulletOrb::BeginPlay()
{
	Super::BeginPlay();

	// 蓝图资产可能保存过旧的 Replicate Movement 默认值，运行时再次关闭以避免和本地跟随同时修改位置。
	SetReplicateMovement(false);
	RefreshFollowTickPrerequisite();

	// 客户端收到 FollowTarget 前先隐藏，避免在初始复制位置短暂闪现。
	if (!HasAuthority() && !IsValid(FollowTarget))
	{
		SetActorHiddenInGame(true);
	}
}

void AElysiaSmartBulletOrb::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (TickPrerequisiteTarget.IsValid())
	{
		RemoveTickPrerequisiteActor(TickPrerequisiteTarget.Get());
		TickPrerequisiteTarget.Reset();
	}

	Super::EndPlay(EndPlayReason);
}

void AElysiaSmartBulletOrb::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!IsValid(FollowTarget))
	{
		if (HasAuthority())
		{
			Destroy();
		}
		return;
	}

	// 每端都依据本端角色的视觉平滑位置计算 Orb，服务器仍保留权威的发射点。
	const FVector DesiredLocation = GetDesiredLocation();
	const FVector NewLocation = FMath::VInterpTo(GetActorLocation(), DesiredLocation, DeltaSeconds, FollowInterpSpeed);
	SetActorLocation(NewLocation);
}

void AElysiaSmartBulletOrb::InitializeOrb(AActor* InFollowTarget, int32 InOrbIndex, int32 InOrbCount, float InOrbitRadius, float InHeightOffset, float InFollowInterpSpeed)
{
	FollowTarget = InFollowTarget;
	RefreshFollowTickPrerequisite();
	SetActorHiddenInGame(!IsValid(FollowTarget));
	UpdateOrbLayout(InOrbIndex, InOrbCount, InOrbitRadius, InHeightOffset, InFollowInterpSpeed);
}

void AElysiaSmartBulletOrb::UpdateOrbLayout(int32 InOrbIndex, int32 InOrbCount, float InOrbitRadius, float InHeightOffset, float InFollowInterpSpeed)
{
	OrbIndex = FMath::Max(0, InOrbIndex);
	OrbCount = FMath::Max(1, InOrbCount);
	OrbitRadius = FMath::Max(0.f, InOrbitRadius);
	HeightOffset = InHeightOffset;
	FollowInterpSpeed = FMath::Max(0.f, InFollowInterpSpeed);

	if (HasAuthority())
	{
		ForceNetUpdate();
	}
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

FTransform AElysiaSmartBulletOrb::GetFollowTransform() const
{
	if (!IsValid(FollowTarget))
	{
		return GetActorTransform();
	}

	const ACharacter* FollowCharacter = Cast<ACharacter>(FollowTarget);
	const USkeletalMeshComponent* CharacterMesh = FollowCharacter ? FollowCharacter->GetMesh() : nullptr;
	if (!CharacterMesh)
	{
		return FollowTarget->GetActorTransform();
	}

	// UE 的客户端网络平滑施加在 Character Mesh，而 Capsule/Actor Transform 会随网络包阶跃更新。
	// 通过 Mesh 当前世界变换和初始相对偏移，反推出与画面中角色一致的平滑根变换。
	const FTransform BaseMeshTransform(
		FollowCharacter->GetBaseRotationOffset(),
		FollowCharacter->GetBaseTranslationOffset());
	return BaseMeshTransform.GetRelativeTransformReverse(CharacterMesh->GetComponentTransform());
}

FVector AElysiaSmartBulletOrb::GetDesiredLocation() const
{
	if (!IsValid(FollowTarget))
	{
		return GetActorLocation();
	}

	const FTransform FollowTransform = GetFollowTransform();
	const FVector RightVector = FollowTransform.GetUnitAxis(EAxis::Y).GetSafeNormal2D();
	const float SideSign = OrbCount > 1 && OrbIndex % 2 == 1 ? -1.f : 1.f;
	const FVector OrbitOffset = RightVector * SideSign * OrbitRadius + FVector::UpVector * HeightOffset;

	return FollowTransform.GetLocation() + OrbitOffset;
}

void AElysiaSmartBulletOrb::RefreshFollowTickPrerequisite()
{
	if (TickPrerequisiteTarget.Get() == FollowTarget)
	{
		return;
	}

	if (TickPrerequisiteTarget.IsValid())
	{
		RemoveTickPrerequisiteActor(TickPrerequisiteTarget.Get());
	}

	TickPrerequisiteTarget = FollowTarget;
	if (TickPrerequisiteTarget.IsValid())
	{
		AddTickPrerequisiteActor(TickPrerequisiteTarget.Get());
	}
}

void AElysiaSmartBulletOrb::OnRep_FollowTarget()
{
	RefreshFollowTickPrerequisite();
	SetActorHiddenInGame(!IsValid(FollowTarget));
}

void AElysiaSmartBulletOrb::OnRep_OrbLayout()
{
	// 布局变化由 Tick 平滑过渡，不在复制回调中瞬移。
}
