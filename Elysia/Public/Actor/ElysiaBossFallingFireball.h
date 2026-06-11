// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ElysiaBossFallingFireball.generated.h"

class AElysiaBossBase;
class USceneComponent;
class UGameplayEffect;

UCLASS()
class ELYSIA_API AElysiaBossFallingFireball : public AActor
{
	GENERATED_BODY()

public:

	AElysiaBossFallingFireball();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaSeconds) override;

	void InitializeFireball(
		AElysiaBossBase* InSourceBoss,
		const FVector& InTargetLocation,
		TSubclassOf<UGameplayEffect> InDamageEffectClass,
		float InDamageEffectLevel,
		float InFallDuration,
		float InImpactRadius);

protected:

	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Fireball")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Boss|Fireball")
	FVector TargetLocation = FVector::ZeroVector;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Boss|Fireball")
	float FallDuration = 0.8f;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Boss|Fireball")
	float ImpactRadius = 240.f;

private:

	void Impact();

	UPROPERTY()
	TObjectPtr<AElysiaBossBase> SourceBoss;

	UPROPERTY()
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	float DamageEffectLevel = 1.f;
	FVector StartLocation = FVector::ZeroVector;
	float ElapsedFallTime = 0.f;
	bool bInitialized = false;
	bool bImpacted = false;
};
