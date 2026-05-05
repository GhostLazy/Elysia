// Copyright GhostLazy


#include "Actor/ElysiaMagnetPickup.h"

#include "Actor/ElysiaXPBall.h"
#include "Character/ElysiaCharacter.h"
#include "EngineUtils.h"

void AElysiaMagnetPickup::HandlePickedBy(AElysiaCharacter* Character)
{
	if (!Character)
	{
		return;
	}

	const FVector PlayerLocation = Character->GetActorLocation();
	const float RadiusSquared = FMath::Square(CollectRadius);

	for (TActorIterator<AElysiaXPBall> It(GetWorld()); It; ++It)
	{
		if (IsValid(*It) && FVector::DistSquared2D(PlayerLocation, It->GetActorLocation()) <= RadiusSquared)
		{
			It->CollectBy(Character);
		}
	}
}
