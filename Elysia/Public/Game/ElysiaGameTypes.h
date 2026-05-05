// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "ElysiaGameTypes.generated.h"

UENUM(BlueprintType)
enum class EElysiaRunPhase : uint8
{
	Normal,
	BossBattle,
	FinalBossBattle,
	Finished
};

UENUM(BlueprintType)
enum class EElysiaEnemyType : uint8
{
	Minion,
	Elite,
	Boss,
	FinalBoss
};

UENUM(BlueprintType)
enum class EElysiaBossSkillType : uint8
{
	None,
	SweepAOE,
	FixedLaser,
	Charge
};
