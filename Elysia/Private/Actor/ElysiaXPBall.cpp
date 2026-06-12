// Copyright GhostLazy


#include "Actor/ElysiaXPBall.h"

#include "Character/ElysiaCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Player/ElysiaPlayerState.h"
#include "Elysia/Elysia.h"
#include "Net/UnrealNetwork.h"

AElysiaXPBall::AElysiaXPBall()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicatingMovement(false);

	Sphere->SetCollisionObjectType(ECC_Projectile);
	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	Sphere->SetCollisionResponseToChannel(ECC_Player, ECR_Overlap);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovement");
	ProjectileMovement->InitialSpeed = 0.f;
	ProjectileMovement->MaxSpeed = MaxSpeed;
	ProjectileMovement->ProjectileGravityScale = 0.f;
	ProjectileMovement->bRotationFollowsVelocity = false;
}

void AElysiaXPBall::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AElysiaXPBall, XPBallLevel);
	DOREPLIFETIME(AElysiaXPBall, AttractionTarget);
}

void AElysiaXPBall::SetXPBallLevel(const int32 InLevel)
{
	XPBallLevel = FMath::Max(1, InLevel);
	SetColorByLevel(XPBallLevel);
}

void AElysiaXPBall::BeginPlay()
{
	Super::BeginPlay();

	// 蓝图资产可能保存过旧的 Replicate Movement 默认值，运行时再次关闭，
	// 避免服务端位置复制和各端本地 Homing 同时修改经验球位置。
	SetReplicateMovement(false);
	SetColorByLevel(XPBallLevel);
	ConfigureHomingMovement();
}

void AElysiaXPBall::HandlePickedBy(AElysiaCharacter* Character)
{
	CollectBy(Character);
}

bool AElysiaXPBall::CanBePickedBy(const AElysiaCharacter* Character, const UPrimitiveComponent* OtherComp) const
{
	return IsValid(Character) && OtherComp == Character->GetCapsuleComponent();
}

void AElysiaXPBall::BeginAttractionTo(AElysiaCharacter* Character)
{
	if (!HasAuthority() || IsValid(AttractionTarget) || !IsValid(Character))
	{
		return;
	}

	AttractionTarget = Character;
	SetLifeSpan(LifeSpan);
	ConfigureHomingMovement();
	ForceNetUpdate();
}

void AElysiaXPBall::CollectBy(AActor* Collector)
{
	if (!HasAuthority() || !IsValid(Collector) || !Collector->ActorHasTag(FName("Player")))
	{
		return;
	}

	if (AElysiaCharacter* ElysiaCharacter = Cast<AElysiaCharacter>(Collector))
	{
		if (AElysiaPlayerState* PlayerState = Cast<AElysiaPlayerState>(ElysiaCharacter->GetPlayerState()))
		{
			PlayerState->AddToXP(XPValue);
		}
	}

	Destroy();
}

void AElysiaXPBall::OnRep_XPBallLevel()
{
	SetColorByLevel(XPBallLevel);
}

void AElysiaXPBall::OnRep_AttractionTarget()
{
	ConfigureHomingMovement();
}

void AElysiaXPBall::ConfigureHomingMovement()
{
	if (!ProjectileMovement)
	{
		return;
	}

	USceneComponent* TargetComponent = nullptr;
	if (IsValid(AttractionTarget))
	{
		// 客户端角色的网络平滑施加在 Mesh 上；服务器仍追踪碰撞根组件，
		// 保证最终重叠拾取由服务器权威判定。
		TargetComponent = HasAuthority()
			? AttractionTarget->GetRootComponent()
			: AttractionTarget->GetMesh();

		if (!TargetComponent)
		{
			TargetComponent = AttractionTarget->GetRootComponent();
		}
	}

	ProjectileMovement->MaxSpeed = MaxSpeed;
	ProjectileMovement->HomingAccelerationMagnitude = Acceleration;
	ProjectileMovement->HomingTargetComponent = TargetComponent;
	ProjectileMovement->bIsHomingProjectile = TargetComponent != nullptr;

	if (TargetComponent)
	{
		ProjectileMovement->Activate(true);
		ProjectileMovement->SetComponentTickEnabled(true);
	}
}
