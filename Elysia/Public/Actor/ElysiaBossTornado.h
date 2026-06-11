// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "GameFramework/Actor.h"
#include "ElysiaBossTornado.generated.h"

class AElysiaBossBase;
class USphereComponent;
class UGameplayEffect;

UCLASS()
class ELYSIA_API AElysiaBossTornado : public AActor
{
	GENERATED_BODY()

public:

	AElysiaBossTornado();

	void InitializeTornado(
		AElysiaBossBase* InSourceBoss,
		TSubclassOf<UGameplayEffect> InDamageEffectClass,
		float InDamageEffectLevel,
		float InTrackingSpeed,
		float InDuration,
		float InContactRadius);

protected:

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Tornado")
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(BlueprintReadOnly, Category = "Boss|Tornado")
	float TrackingSpeed = 350.f;

	UPROPERTY(BlueprintReadOnly, Category = "Boss|Tornado")
	float Duration = 10.f;

	UPROPERTY(BlueprintReadOnly, Category = "Boss|Tornado")
	float ContactRadius = 80.f;

private:

	UFUNCTION()
	void HandleOverlapBegin(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void HandleOverlapEnd(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	AActor* FindClosestPlayer() const;
	void ApplyContactDamage(AActor* TargetActor);
	void RemoveContactDamage(AActor* TargetActor);
	void ClearContactDamageEffects();
	static bool IsValidPlayerTarget(AActor* TargetActor);

	UPROPERTY()
	TObjectPtr<AElysiaBossBase> SourceBoss;

	UPROPERTY()
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	float DamageEffectLevel = 1.f;
	bool bInitialized = false;
	TMap<TWeakObjectPtr<AActor>, FActiveGameplayEffectHandle> ActiveContactDamageEffects;
};
