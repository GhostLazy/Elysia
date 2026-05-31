// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "Character/ElysiaEnemy.h"
#include "Game/ElysiaBossTypes.h"
#include "ElysiaBossBase.generated.h"

class AElysiaBossLaserActor;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnBossSkillFinishedSignature, EElysiaBossSkillType);

UCLASS()
class ELYSIA_API AElysiaBossBase : public AElysiaEnemy
{
	GENERATED_BODY()

public:
	
	AElysiaBossBase();
	virtual void Die() override;

	UFUNCTION(BlueprintCallable, Category = "Boss")
	void SetCombatTarget(AActor* InTargetActor);

	UFUNCTION(BlueprintPure, Category = "Boss")
	AActor* GetCombatTarget() const { return CombatTarget.Get(); }

	UFUNCTION(BlueprintPure, Category = "Boss")
	bool HasValidCombatTarget() const;

	UFUNCTION(BlueprintPure, Category = "Boss")
	bool IsCastingSkill() const { return bIsCastingSkill; }

	UFUNCTION(BlueprintPure, Category = "Boss")
	bool IsCharging() const { return bIsCharging; }

	UFUNCTION(BlueprintPure, Category = "Boss")
	float GetDistanceToCombatTarget2D() const;

	UFUNCTION(BlueprintPure, Category = "Boss")
	bool IsTargetInRangeForSkill(EElysiaBossSkillType SkillType) const;

	UFUNCTION(BlueprintPure, Category = "Boss")
	bool CanCastSkill(EElysiaBossSkillType SkillType) const;

	UFUNCTION(BlueprintCallable, Category = "Boss")
	bool TryCastSkill(EElysiaBossSkillType SkillType);

	UFUNCTION(BlueprintCallable, Category = "Boss")
	bool TryCastBestAvailableSkill();

	UFUNCTION(BlueprintCallable, Category = "Boss|Teleport")
	bool TryTeleportNearCombatTargetIfTooFar();

	void ApplyLaserDamageInDirection(const FVector& LaserOrigin, const FVector& LaserDirection, const FElysiaBossSkillSpec& SkillSpec);

	FOnBossSkillFinishedSignature OnBossSkillFinished;

protected:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss")
	TArray<FElysiaBossSkillSpec> BossSkills;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss", meta = (ClampMin = "0.0"))
	float SkillFacingInterpSpeed = 12.f;

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
	
	const FElysiaBossSkillSpec* FindSkillSpec(EElysiaBossSkillType SkillType) const;
	bool TryApplyDamageToActor(AActor* TargetActor, TSubclassOf<UGameplayEffect> DamageEffectClass, float DamageEffectLevel);
	bool TryFindTeleportLocationNearCombatTarget(FVector& OutTeleportLocation) const;
	bool TryProjectTeleportCandidateToGround(const FVector& CandidateLocation, float CapsuleHalfHeight, FVector& OutTeleportLocation) const;
	bool IsTeleportLocationClear(const FVector& TeleportLocation, float CapsuleRadius, float CapsuleHalfHeight, const AActor* TargetActor) const;
	void BeginSkillRecovery(float RecoveryTime, EElysiaBossSkillType SkillType);
	void FinishRecovery();
	void ExecuteSweepAOE();
	void SpawnFixedLaser();
	void BeginCharge();
	void TickChargeMovement();
	void StopCharge(bool bInterrupted);
	void ClearBossSkillState();
	FVector GetLockedSkillDirection() const;

	TWeakObjectPtr<AActor> CombatTarget;
	TWeakObjectPtr<AElysiaBossLaserActor> ActiveLaserActor;
	TSet<TWeakObjectPtr<AActor>> ChargeHitTargets;
	TMap<EElysiaBossSkillType, float> SkillCooldownEndTimeByType;
	FTimerHandle SkillRecoveryTimerHandle;
	FTimerHandle ChargeTickTimerHandle;
	EElysiaBossSkillType CurrentSkillType = EElysiaBossSkillType::None;
	FVector LockedSkillDirection = FVector::ForwardVector;
	FVector ChargeStartLocation = FVector::ZeroVector;
	bool bIsCastingSkill = false;
	bool bIsCharging = false;
	
};
