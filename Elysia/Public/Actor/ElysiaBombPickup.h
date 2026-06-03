// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "Actor/ElysiaPickupBase.h"
#include "ElysiaBombPickup.generated.h"

class UAbilitySystemComponent;
class UGameplayEffect;

UCLASS()
class ELYSIA_API AElysiaBombPickup : public AElysiaPickupBase
{
	GENERATED_BODY()

protected:

	virtual void HandlePickedBy(AElysiaCharacter* Character) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup|Bomb", meta = (ClampMin = "0.0"))
	float KillRadius = 600.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup|Bomb")
	TSubclassOf<UGameplayEffect> InstantKillEffectClass;

private:

	bool ApplyInstantKillEffectToTarget(AActor* TargetActor, UAbilitySystemComponent* SourceASC) const;
};
