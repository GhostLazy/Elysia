// Copyright GhostLazy


#include "Game/ElysiaGameModeBase.h"

#include "Game/ElysiaSpawnManager.h"
#include "Kismet/GameplayStatics.h"

void AElysiaGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority() || !SpawnManagerClass)
	{
		return;
	}

	if (AElysiaSpawnManager* ExistingSpawnManager = Cast<AElysiaSpawnManager>(UGameplayStatics::GetActorOfClass(this, SpawnManagerClass)))
	{
		SpawnManager = ExistingSpawnManager;
		return;
	}

	SpawnManager = GetWorld()->SpawnActor<AElysiaSpawnManager>(SpawnManagerClass, FVector::ZeroVector, FRotator::ZeroRotator);
}

