// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "Actor/ElysiaPickupBase.h"
#include "ElysiaXPBall.generated.h"

class UProjectileMovementComponent;
class UPrimitiveComponent;
class AElysiaCharacter;

UCLASS()
class ELYSIA_API AElysiaXPBall : public AElysiaPickupBase
{
	GENERATED_BODY()
	
public:	
	
	AElysiaXPBall();
	
	int32 GetXPValue() const { return XPValue; }
	void SetXPValue(const int32 InXP) { XPValue = InXP; }
	
	void CollectBy(AActor* Collector);
	void BeginAttractionTo(AElysiaCharacter* Character);
	
	UFUNCTION(BlueprintImplementableEvent)
	void SetColorByLevel(const int32 InLevel);
	
protected:
	
	int32 XPValue = 0;
	
	virtual void BeginPlay() override;
	virtual void HandlePickedBy(AElysiaCharacter* Character) override;
	virtual bool CanBePickedBy(const AElysiaCharacter* Character, const UPrimitiveComponent* OtherComp) const override;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;
	
	UPROPERTY(EditDefaultsOnly)
	float MaxSpeed = 2000.f;
	
	UPROPERTY(EditDefaultsOnly)
	float Acceleration = 200.f;
	
	UPROPERTY(EditDefaultsOnly)
	float LifeSpan = 5.f;
	
private:
	
	bool bTargetHasSet = false;

};
