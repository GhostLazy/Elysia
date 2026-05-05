// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "Game/ElysiaGameTypes.h"
#include "ElysiaBossTypes.generated.h"

class AElysiaBossLaserActor;
class UAnimMontage;
class UGameplayEffect;

USTRUCT(BlueprintType)
struct FElysiaBossSkillSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss Skill")
	EElysiaBossSkillType SkillType = EElysiaBossSkillType::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss Skill", meta = (ClampMin = "0.0"))
	float Weight = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss Skill", meta = (ClampMin = "0.0"))
	float Cooldown = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss Skill", meta = (ClampMin = "0.0"))
	float MinRange = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss Skill", meta = (ClampMin = "0.0"))
	float CastRange = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss Skill", meta = (ClampMin = "0.0"))
	float WindupTime = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss Skill", meta = (ClampMin = "0.0"))
	float RecoveryTime = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss Skill")
	TObjectPtr<UAnimMontage> CastMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss Skill")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss Skill", meta = (ClampMin = "0.0"))
	float DamageEffectLevel = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss Skill|Sweep", meta = (ClampMin = "0.0"))
	float SweepRadius = 350.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss Skill|Laser", meta = (ClampMin = "0.0"))
	float LaserLength = 1200.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss Skill|Laser", meta = (ClampMin = "0.0"))
	float LaserWidth = 180.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss Skill|Laser", meta = (ClampMin = "0.0"))
	float LaserDuration = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss Skill|Laser", meta = (ClampMin = "0.01"))
	float LaserTickInterval = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss Skill|Laser")
	TSubclassOf<AElysiaBossLaserActor> LaserActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss Skill|Charge", meta = (ClampMin = "0.0"))
	float ChargeSpeed = 1800.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss Skill|Charge", meta = (ClampMin = "0.0"))
	float ChargeMaxDistance = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss Skill|Charge")
	bool bStopChargeOnFirstHit = true;
};
