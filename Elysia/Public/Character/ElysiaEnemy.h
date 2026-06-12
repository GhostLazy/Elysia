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
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void Die() override;
	FOnEnemyDiedSignature OnEnemyDied;
	EElysiaEnemyType GetEnemyType() const { return EnemyType; }
	int32 GetEnemyLevel() const { return Level; }
	void SetLevel(int32 InLevel) { Level = FMath::Max(1, InLevel); }
	void SetDeathRewardsEnabled(bool bEnabled) { bDeathRewardsEnabled = bEnabled; }
	bool AreDeathRewardsEnabled() const { return bDeathRewardsEnabled; }
	void SetSoftSeparationOffset(const FVector& InOffset, float BlendAlpha);
	const FVector& GetSoftSeparationOffset() const { return SoftSeparationOffset; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool HasOverlappingPlayers() const { return CurrentOverlappingPlayers.Num() > 0; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	AActor* GetPreferredDamageTarget() const;

protected:
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AElysiaXPBall> XPBallClass;
	
	UPROPERTY(EditDefaultsOnly)
	FScalableFloat XPRewards = FScalableFloat();
	
	UPROPERTY(Replicated, EditDefaultsOnly)
	int32 Level = 1;

	UPROPERTY(EditDefaultsOnly, Category = "Reward")
	EElysiaEnemyType EnemyType = EElysiaEnemyType::Minion;

	UPROPERTY(EditDefaultsOnly, Category = "Reward", meta = (ClampMin = "0"))
	int32 MinionXPBallDropCount = 1;

	UPROPERTY(EditDefaultsOnly, Category = "Reward", meta = (ClampMin = "0"))
	int32 EliteXPBallDropCount = 5;

	UPROPERTY(EditDefaultsOnly, Category = "Reward", meta = (ClampMin = "0"))
	int32 BossXPBallDropCount = 10;

	UPROPERTY(EditDefaultsOnly, Category = "Reward", meta = (ClampMin = "0.0"))
	float XPBallDropScatterRadius = 80.f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TSubclassOf<UGameplayEffect> ContactDamageEffectClass;

	UPROPERTY(EditDefaultsOnly, Category = "Reward")
	TSubclassOf<AElysiaRunePickup> RunePickupClass;

	UPROPERTY(EditDefaultsOnly, Category = "Reward")
	TSubclassOf<AElysiaMagnetPickup> MagnetPickupClass;

	// 试炼等特殊来源可关闭怪物自身的死亡掉落，但不影响死亡通知与销毁流程。
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reward")
	bool bDeathRewardsEnabled = true;

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
	int32 GetXPBallDropCount() const;

	TSet<TWeakObjectPtr<AActor>> CurrentOverlappingPlayers;
	TMap<TWeakObjectPtr<AActor>, FActiveGameplayEffectHandle> ActiveContactDamageEffects;
	FVector SoftSeparationOffset = FVector::ZeroVector;
	
};
