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

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	int32 GetXPValue() const { return XPValue; }
	void SetXPValue(const int32 InXP) { XPValue = InXP; }

	int32 GetXPBallLevel() const { return XPBallLevel; }
	void SetXPBallLevel(const int32 InLevel);
	
	void CollectBy(AActor* Collector);
	void BeginAttractionTo(AElysiaCharacter* Character);
	
	UFUNCTION(BlueprintImplementableEvent)
	void SetColorByLevel(const int32 InLevel);
	
protected:
	
	int32 XPValue = 0;

	UPROPERTY(ReplicatedUsing = OnRep_XPBallLevel)
	int32 XPBallLevel = 1;
	
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

	UFUNCTION()
	void OnRep_XPBallLevel();

	UFUNCTION()
	void OnRep_AttractionTarget();

	void ConfigureHomingMovement();

	// 只复制吸附目标，各端根据本地角色位置独立模拟追踪移动。
	UPROPERTY(ReplicatedUsing = OnRep_AttractionTarget)
	TObjectPtr<AElysiaCharacter> AttractionTarget;

};
