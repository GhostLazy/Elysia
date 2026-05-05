// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ElysiaPickupBase.generated.h"

class AElysiaCharacter;
class UPrimitiveComponent;
class USphereComponent;

UCLASS(Abstract)
class ELYSIA_API AElysiaPickupBase : public AActor
{
	GENERATED_BODY()

public:
	
	AElysiaPickupBase();

protected:
	
	virtual void BeginPlay() override;
	virtual void HandlePickedBy(AElysiaCharacter* Character);
	virtual bool CanBePickedBy(const AElysiaCharacter* Character, const UPrimitiveComponent* OtherComp) const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pickup")
	TObjectPtr<USphereComponent> Sphere;

private:
	
	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
};
