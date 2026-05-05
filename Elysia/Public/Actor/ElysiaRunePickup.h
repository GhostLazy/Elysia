// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "Actor/ElysiaPickupBase.h"
#include "ElysiaRunePickup.generated.h"

UCLASS()
class ELYSIA_API AElysiaRunePickup : public AElysiaPickupBase
{
	GENERATED_BODY()

protected:
	virtual void HandlePickedBy(AElysiaCharacter* Character) override;
};
