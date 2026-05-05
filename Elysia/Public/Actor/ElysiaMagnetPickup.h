// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "Actor/ElysiaPickupBase.h"
#include "ElysiaMagnetPickup.generated.h"

UCLASS()
class ELYSIA_API AElysiaMagnetPickup : public AElysiaPickupBase
{
	GENERATED_BODY()

protected:
	virtual void HandlePickedBy(AElysiaCharacter* Character) override;

	UPROPERTY(EditDefaultsOnly, Category = "Pickup", meta = (ClampMin = "0.0"))
	float CollectRadius = 3000.f;
};
