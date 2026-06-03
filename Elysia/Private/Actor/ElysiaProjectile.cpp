// Copyright GhostLazy


#include "Actor/ElysiaProjectile.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Elysia/Elysia.h"
#include "AbilitySystem/ElysiaAbilitySystemComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Interface/CombatInterface.h"

AElysiaProjectile::AElysiaProjectile()
{
	// 禁用Tick，启用复制
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	
	// 设置碰撞球体
	Sphere = CreateDefaultSubobject<USphereComponent>("Sphere");
	SetRootComponent(Sphere);
	Sphere->SetCollisionObjectType(ECC_Projectile);
	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	Sphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	Sphere->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	Sphere->SetCollisionResponseToChannel(ECC_Minion, ECR_Overlap);
	Sphere->SetCollisionResponseToChannel(ECC_Boss, ECR_Overlap);
	
	// 设置移动组件
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovement");
	ProjectileMovement->InitialSpeed = 600.f;
	ProjectileMovement->MaxSpeed = 600.f;
	ProjectileMovement->ProjectileGravityScale = 0.f;
}

void AElysiaProjectile::BeginPlay()
{
	Super::BeginPlay();
	SetLifeSpan(LifeSpan);
	SetReplicateMovement(true);
	Sphere->OnComponentBeginOverlap.AddDynamic(this, &AElysiaProjectile::HandleSphereOverlapBegin);
}

void AElysiaProjectile::SetMovementSpeed(float MovementSpeed) const
{
	ProjectileMovement->InitialSpeed = MovementSpeed;
	ProjectileMovement->MaxSpeed = MovementSpeed;
}

void AElysiaProjectile::HandleSphereOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                                 UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                                 const FHitResult& SweepResult)
{
	const ICombatInterface* CombatInterface = Cast<ICombatInterface>(OtherActor);
	if (!HasAuthority() || !IsValid(OtherActor) || !CombatInterface
		|| !CombatInterface->HasTag(FName("Damageable")) || CombatInterface->IsDead())
	{
		return;
	}

	const TWeakObjectPtr<AActor> HitActor(OtherActor);
	if (bPenetrate && HitActors.Contains(HitActor))
	{
		return;
	}

	HandleEnemyHit(OtherActor);
}

void AElysiaProjectile::HandleEnemyHit(AActor* OtherActor)
{
	if (ApplyDamageToActor(OtherActor) && bPenetrate)
	{
		HitActors.Add(TWeakObjectPtr<AActor>(OtherActor));
	}

	if (!bPenetrate)
	{
		Destroy();
	}
}

bool AElysiaProjectile::ApplyDamageToActor(AActor* TargetActor)
{
	if (!HasAuthority() || !IsValid(TargetActor) || !EffectSpecHandle.IsValid())
	{
		return false;
	}

	if (UElysiaAbilitySystemComponent* TargetASC = Cast<UElysiaAbilitySystemComponent>(
		UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor)))
	{
		TargetASC->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
		return true;
	}

	return false;
}
