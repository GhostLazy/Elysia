// Copyright GhostLazy

#include "Game/ElysiaGroundSpawnUtility.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"

bool FElysiaGroundSpawnUtility::GetCharacterCapsuleSize(
	const UClass* CharacterClass,
	float& OutCapsuleRadius,
	float& OutCapsuleHalfHeight)
{
	const ACharacter* DefaultCharacter = CharacterClass
		? Cast<ACharacter>(CharacterClass->GetDefaultObject())
		: nullptr;
	const UCapsuleComponent* CapsuleComponent = DefaultCharacter
		? DefaultCharacter->GetCapsuleComponent()
		: nullptr;
	if (!CapsuleComponent)
	{
		return false;
	}

	OutCapsuleRadius = CapsuleComponent->GetScaledCapsuleRadius();
	OutCapsuleHalfHeight = CapsuleComponent->GetScaledCapsuleHalfHeight();
	return OutCapsuleRadius > 0.f && OutCapsuleHalfHeight > 0.f;
}

bool FElysiaGroundSpawnUtility::TryProjectCandidateToGround(
	UWorld* World,
	const FVector& CandidateLocation,
	float CapsuleHalfHeight,
	float TraceUpDistance,
	float TraceDownDistance,
	float GroundClearance,
	const AActor* IgnoredActor,
	FVector& OutSpawnLocation)
{
	if (!World || CapsuleHalfHeight <= 0.f)
	{
		return false;
	}

	FHitResult GroundHit;
	const FVector TraceStart = CandidateLocation + FVector::UpVector * FMath::Max(0.f, TraceUpDistance);
	const FVector TraceEnd = CandidateLocation - FVector::UpVector * FMath::Max(0.f, TraceDownDistance);

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ElysiaGroundSpawnTrace), false);
	if (IgnoredActor)
	{
		QueryParams.AddIgnoredActor(IgnoredActor);
	}

	if (!World->LineTraceSingleByObjectType(
		GroundHit,
		TraceStart,
		TraceEnd,
		ObjectQueryParams,
		QueryParams)
		|| !GroundHit.bBlockingHit)
	{
		return false;
	}

	OutSpawnLocation = GroundHit.ImpactPoint
		+ FVector::UpVector * (CapsuleHalfHeight + FMath::Max(0.f, GroundClearance));
	return true;
}

bool FElysiaGroundSpawnUtility::HasGroundBelowCharacter(
	UWorld* World,
	const FVector& CharacterLocation,
	float CapsuleHalfHeight,
	float GroundClearance,
	float PostSpawnGroundCheckDistance,
	const AActor* IgnoredActor)
{
	if (!World || CapsuleHalfHeight <= 0.f)
	{
		return false;
	}

	FHitResult GroundHit;
	const float TraceDistance = CapsuleHalfHeight
		+ FMath::Max(0.f, GroundClearance)
		+ FMath::Max(0.f, PostSpawnGroundCheckDistance);

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ElysiaGroundSpawnPostCheck), false);
	if (IgnoredActor)
	{
		QueryParams.AddIgnoredActor(IgnoredActor);
	}

	return World->LineTraceSingleByObjectType(
		GroundHit,
		CharacterLocation,
		CharacterLocation - FVector::UpVector * TraceDistance,
		ObjectQueryParams,
		QueryParams)
		&& GroundHit.bBlockingHit;
}
