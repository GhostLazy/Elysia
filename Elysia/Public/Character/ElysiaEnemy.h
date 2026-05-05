// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "Game/ElysiaGameTypes.h"
#include "GameplayEffectTypes.h"
#include "ScalableFloat.h"
#include "Character/ElysiaCharacterBase.h"
#include "ElysiaEnemy.generated.h"

class AElysiaEnemy;
class AElysiaXPBall;
class AElysiaMagnetPickup;
class AElysiaRunePickup;
class UGameplayEffect;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnEnemyDiedSignature, AElysiaEnemy*);
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
	FOnEnemyDiedSignature OnEnemyDied;
	EElysiaEnemyType GetEnemyType() const { return EnemyType; }

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

	UPROPERTY(EditDefaultsOnly, Category = "Reward")
	EElysiaEnemyType EnemyType = EElysiaEnemyType::Minion;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TSubclassOf<UGameplayEffect> ContactDamageEffectClass;

	UPROPERTY(EditDefaultsOnly, Category = "Reward")
	TSubclassOf<AElysiaRunePickup> RunePickupClass;

	UPROPERTY(EditDefaultsOnly, Category = "Reward")
	TSubclassOf<AElysiaMagnetPickup> MagnetPickupClass;

private:

	UFUNCTION()
	void HandlePlayerOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void HandlePlayerOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void ApplyContactDamageEffectToTarget(AActor* DamageTarget);
	void RemoveContactDamageEffectFromTarget(AActor* DamageTarget);
	void ClearActiveContactDamageEffects();
	static bool IsValidDamageTargetActor(AActor* Actor);
	void SpawnDeathRewards(const FVector& DropLocation);

	TSet<TWeakObjectPtr<AActor>> CurrentOverlappingPlayers;
	TMap<TWeakObjectPtr<AActor>, FActiveGameplayEffectHandle> ActiveContactDamageEffects;
	
};
