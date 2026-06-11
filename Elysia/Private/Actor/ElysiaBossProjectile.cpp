// Copyright GhostLazy

#include "Actor/ElysiaBossProjectile.h"

#include "Components/SphereComponent.h"
#include "Elysia/Elysia.h"

AElysiaBossProjectile::AElysiaBossProjectile()
{
	SetNetUpdateFrequency(30.f);
	SetMinNetUpdateFrequency(15.f);
	
	Sphere->SetCollisionObjectType(ECC_Projectile);
	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	Sphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	Sphere->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	Sphere->SetCollisionResponseToChannel(ECC_Player, ECR_Overlap);
}
