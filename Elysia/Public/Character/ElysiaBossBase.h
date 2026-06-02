// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "Character/ElysiaEnemy.h"
#include "ElysiaBossBase.generated.h"

class UElysiaBossGameplayAbility;
class UGameplayEffect;

DECLARE_MULTICAST_DELEGATE(FOnBossAbilityFinishedSignature);

UCLASS()
class ELYSIA_API AElysiaBossBase : public AElysiaEnemy
{
	GENERATED_BODY()

public:
	
	AElysiaBossBase();
	virtual void BeginPlay() override;
	virtual void Die() override;

	UFUNCTION(BlueprintCallable, Category = "Boss")
	void SetCombatTarget(AActor* InTargetActor);

	UFUNCTION(BlueprintPure, Category = "Boss")
	AActor* GetCombatTarget() const { return CombatTarget.Get(); }

	UFUNCTION(BlueprintPure, Category = "Boss")
	bool HasValidCombatTarget() const;

	UFUNCTION(BlueprintPure, Category = "Boss")
	float GetDistanceToCombatTarget2D() const;

	UFUNCTION(BlueprintPure, Category = "Boss")
	bool IsUsingBossAbility() const { return ActiveBossAbilityCount > 0; }

	UFUNCTION(BlueprintCallable, Category = "Boss")
	bool TryActivateBestBossAbility();

	UFUNCTION(BlueprintCallable, Category = "Boss|Teleport")
	bool TryTeleportNearCombatTargetIfTooFar();

	bool ApplyBossDamageToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> DamageEffectClass, float DamageEffectLevel) const;
	void NotifyBossAbilityStarted(const UElysiaBossGameplayAbility* BossAbility);
	void NotifyBossAbilityEnded(const UElysiaBossGameplayAbility* BossAbility);

	FOnBossAbilityFinishedSignature OnBossAbilityFinished;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Abilities")
	TArray<TSubclassOf<UElysiaBossGameplayAbility>> BossAbilityClasses;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Teleport")
	bool bTeleportNearTargetWhenTooFar = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Teleport", meta = (ClampMin = "0.0"))
	float TeleportTriggerDistance = 2400.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Teleport", meta = (ClampMin = "0.0"))
	float TeleportTargetMinRadius = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Teleport", meta = (ClampMin = "0.0"))
	float TeleportTargetMaxRadius = 900.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Teleport", meta = (ClampMin = "1"))
	int32 TeleportMaxAttempts = 16;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Teleport")
	FVector TeleportNavProjectExtent = FVector(300.f, 300.f, 800.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Teleport", meta = (ClampMin = "0.0"))
	float TeleportGroundTraceUpDistance = 2000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Teleport", meta = (ClampMin = "0.0"))
	float TeleportGroundTraceDownDistance = 5000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Teleport", meta = (ClampMin = "0.0"))
	float TeleportGroundClearance = 10.f;

private:

	void GrantBossAbilities();
	bool TryFindTeleportLocationNearCombatTarget(FVector& OutTeleportLocation) const;
	bool TryProjectTeleportCandidateToGround(const FVector& CandidateLocation, float CapsuleHalfHeight, FVector& OutTeleportLocation) const;
	bool IsTeleportLocationClear(const FVector& TeleportLocation, float CapsuleRadius, float CapsuleHalfHeight, const AActor* TargetActor) const;

	TWeakObjectPtr<AActor> CombatTarget;
	int32 ActiveBossAbilityCount = 0;
};
