// Copyright GhostLazy


#include "Actor/ElysiaExplosiveProjectile.h"

#include "AbilitySystem/ElysiaAbilitySystemLibrary.h"
#include "Math/UnrealMathUtility.h"

void AElysiaExplosiveProjectile::SetExplosionRadius(float InExplosionRadius)
{
	ExplosionRadius = FMath::Max(0.f, InExplosionRadius);
}

void AElysiaExplosiveProjectile::HandleEnemyHit(AActor* OtherActor)
{
	if (bHasExploded)
	{
		return;
	}

	bHasExploded = true;

	TArray<AActor*> ActorsToIgnore;
	TArray<AActor*> OverlapActors;
	UElysiaAbilitySystemLibrary::GetLiveActorsWithInRadius(
		this,
		OverlapActors,
		ActorsToIgnore,
		ExplosionRadius,
		GetActorLocation(),
		FName("Damageable"));

	bool bAppliedDamage = false;
	for (AActor* OverlapActor : OverlapActors)
	{
		bAppliedDamage |= ApplyDamageToActor(OverlapActor);
	}

	if (!bAppliedDamage)
	{
		ApplyDamageToActor(OtherActor);
	}

	Destroy();
}
