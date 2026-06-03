// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "Actor/ElysiaPickupBase.h"
#include "ElysiaHealthPickup.generated.h"

class UGameplayEffect;

UCLASS()
class ELYSIA_API AElysiaHealthPickup : public AElysiaPickupBase
{
	GENERATED_BODY()

protected:

	virtual void HandlePickedBy(AElysiaCharacter* Character) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup|Health")
	TSubclassOf<UGameplayEffect> HealEffectClass;
};
