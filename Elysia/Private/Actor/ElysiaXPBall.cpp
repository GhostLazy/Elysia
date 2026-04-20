// Copyright GhostLazy


#include "Actor/ElysiaXPBall.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "AbilitySystem/ElysiaAbilitySystemLibrary.h"
#include "Character/ElysiaCharacter.h"
#include "Elysia/Elysia.h"
#include "Player/ElysiaPlayerState.h"

AElysiaXPBall::AElysiaXPBall()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	
	Sphere = CreateDefaultSubobject<USphereComponent>("Sphere");
	SetRootComponent(Sphere);
	Sphere->SetCollisionObjectType(ECC_Projectile);
	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	// 确保只会与Player触发重叠事件
	Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	Sphere->SetCollisionResponseToChannel(ECC_Player, ECR_Overlap);
	
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovement");
	ProjectileMovement->InitialSpeed = 0;
	ProjectileMovement->MaxSpeed = MaxSpeed;
	ProjectileMovement->ProjectileGravityScale = 0.f;
}

void AElysiaXPBall::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	MoveToClosestPlayer();
}

void AElysiaXPBall::BeginPlay()
{
	Super::BeginPlay();
	
	SpawnLocation = GetActorLocation();
	SetReplicateMovement(true);
	Sphere->OnComponentBeginOverlap.AddDynamic(this, &AElysiaXPBall::OnSphereOverlap);
}

void AElysiaXPBall::MoveToClosestPlayer()
{
	if (TargetHasSet) return;
	
	TArray<AActor*> ActorsToIgnore;
	TArray<AActor*> OverlapActors;
	
	UElysiaAbilitySystemLibrary::GetLiveActorsWithInRadius(this, OverlapActors, ActorsToIgnore, 400.f, SpawnLocation, FName("Player"));
	TargetToMove = UElysiaAbilitySystemLibrary::GetClosestActor(OverlapActors, SpawnLocation);
	
	if (TargetToMove)
	{
		TargetHasSet = true;
		SetLifeSpan(LifeSpan);
		
		ProjectileMovement->HomingTargetComponent = TargetToMove->GetRootComponent();
		ProjectileMovement->HomingAccelerationMagnitude = Acceleration;
		ProjectileMovement->bIsHomingProjectile = true;
	}
}

void AElysiaXPBall::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor->ActorHasTag(FName("Player")) || !HasAuthority()) return;
	
	if (AElysiaCharacter* ElysiaCharacter = Cast<AElysiaCharacter>(OtherActor))
	{
		if (AElysiaPlayerState* PlayerState = Cast<AElysiaPlayerState>(ElysiaCharacter->GetPlayerState()))
		{
			PlayerState->AddToXP(XPValue);
		}
	}
	
	Destroy();
}



