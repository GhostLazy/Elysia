// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ElysiaXPBall.generated.h"

class USphereComponent;
class UProjectileMovementComponent;

UCLASS()
class ELYSIA_API AElysiaXPBall : public AActor
{
	GENERATED_BODY()
	
public:	
	
	AElysiaXPBall();
	virtual void Tick(float DeltaTime) override;
	
	int32 GetXPValue() const { return XPValue; }
	void SetXPValue(const int32 InXP) { XPValue = InXP; }
	
	UFUNCTION(BlueprintImplementableEvent)
	void SetColorByLevel(const int32 InLevel);
	
protected:
	
	int32 XPValue = 0;
	
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> Sphere;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;
	
	UPROPERTY(EditDefaultsOnly)
	float MaxSpeed = 2000.f;
	
	UPROPERTY(EditDefaultsOnly)
	float Acceleration = 200.f;
	
	UPROPERTY(EditDefaultsOnly)
	float LifeSpan = 5.f;
	
private:
	
	void MoveToClosestPlayer();
	
	UPROPERTY()
	TObjectPtr<AActor> TargetToMove;
	
	bool TargetHasSet = false;
	FVector SpawnLocation;
	
	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

};
