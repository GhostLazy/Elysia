// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GameFramework/Actor.h"
#include "ElysiaProjectile.generated.h"

class UProjectileMovementComponent;
class USphereComponent;
class UGameplayEffect;

UCLASS()
class ELYSIA_API AElysiaProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	
	AElysiaProjectile();
	
	UPROPERTY()
	FGameplayEffectSpecHandle EffectSpecHandle;

protected:
	
	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	virtual void BeginPlay() override;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> DamageEffectClass;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> Sphere;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;
	
	UPROPERTY(EditDefaultsOnly)
	float LifeSpan = 5.f;
	
};
