// Copyright GhostLazy


#include "Actor/ElysiaProjectile.h"

// Sets default values
AElysiaProjectile::AElysiaProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AElysiaProjectile::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AElysiaProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

