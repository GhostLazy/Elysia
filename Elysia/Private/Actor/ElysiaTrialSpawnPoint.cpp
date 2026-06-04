// Copyright GhostLazy


#include "Actor/ElysiaTrialSpawnPoint.h"

#include "Components/ArrowComponent.h"
#include "Components/SceneComponent.h"

AElysiaTrialSpawnPoint::AElysiaTrialSpawnPoint()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	ArrowComponent->SetupAttachment(SceneRoot);
	ArrowComponent->ArrowSize = 1.25f;
}

FTransform AElysiaTrialSpawnPoint::GetTrialSpawnTransform() const
{
	return GetActorTransform();
}
