// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "ScalableFloat.h"
#include "Character/ElysiaCharacterBase.h"
#include "ElysiaEnemy.generated.h"

class AElysiaXPBall;
class UGameplayEffect;
/**
 * 
 */
UCLASS()
class ELYSIA_API AElysiaEnemy : public AElysiaCharacterBase
{
	GENERATED_BODY()

public:
	
	AElysiaEnemy();
	virtual void BeginPlay() override;
	virtual void Die() override;

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool HasOverlappingPlayers() const { return CurrentOverlappingPlayers.Num() > 0; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	AActor* GetPreferredDamageTarget() const;

protected:
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AElysiaXPBall> XPBallClass;
	
	UPROPERTY(EditDefaultsOnly)
	FScalableFloat XPRewards = FScalableFloat();
	
	UPROPERTY(EditDefaultsOnly)
	int32 Level = 1;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TSubclassOf<UGameplayEffect> ContactDamageEffectClass;

	UPROPERTY(EditDefaultsOnly, Category = "Combat", meta = (ClampMin = "0.05"))
	float ContactDamageInterval = 1.f;

private:

	UFUNCTION()
	void HandlePlayerOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void HandlePlayerOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void StartContactDamage();
	void StopContactDamage();
	void ApplyContactDamage();
	static bool IsValidDamageTargetActor(AActor* Actor);

	TSet<TWeakObjectPtr<AActor>> CurrentOverlappingPlayers;
	FTimerHandle ContactDamageTimerHandle;
	
};
