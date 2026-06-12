// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"

class AActor;
class UWorld;

/**
 * Character 类 Actor 的地面生成工具。
 * 统一处理胶囊尺寸、真实地面定位与生成后脚下检查。
 */
class ELYSIA_API FElysiaGroundSpawnUtility
{
public:

	static bool GetCharacterCapsuleSize(
		const UClass* CharacterClass,
		float& OutCapsuleRadius,
		float& OutCapsuleHalfHeight);

	static bool TryProjectCandidateToGround(
		UWorld* World,
		const FVector& CandidateLocation,
		float CapsuleHalfHeight,
		float TraceUpDistance,
		float TraceDownDistance,
		float GroundClearance,
		const AActor* IgnoredActor,
		FVector& OutSpawnLocation);

	static bool HasGroundBelowCharacter(
		UWorld* World,
		const FVector& CharacterLocation,
		float CapsuleHalfHeight,
		float GroundClearance,
		float PostSpawnGroundCheckDistance,
		const AActor* IgnoredActor);
};
