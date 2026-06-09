// Copyright GhostLazy


#include "Actor/ElysiaXPBall.h"

#include "Character/ElysiaCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Player/ElysiaPlayerState.h"
#include "Elysia/Elysia.h"
#include "Net/UnrealNetwork.h"

AElysiaXPBall::AElysiaXPBall()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

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
}

void AElysiaXPBall::SetXPBallLevel(const int32 InLevel)
{
	XPBallLevel = FMath::Max(1, InLevel);
	SetColorByLevel(XPBallLevel);
}

void AElysiaXPBall::BeginPlay()
{
	Super::BeginPlay();
	SetReplicateMovement(true);
	SetColorByLevel(XPBallLevel);
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
	if (!HasAuthority() || bTargetHasSet || !IsValid(Character))
	{
		return;
	}

	if (USceneComponent* TargetComponent = Character->GetRootComponent())
	{
		bTargetHasSet = true;
		SetLifeSpan(LifeSpan);
		ProjectileMovement->MaxSpeed = MaxSpeed;
		ProjectileMovement->HomingTargetComponent = TargetComponent;
		ProjectileMovement->HomingAccelerationMagnitude = Acceleration;
		ProjectileMovement->bIsHomingProjectile = true;
	}
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
