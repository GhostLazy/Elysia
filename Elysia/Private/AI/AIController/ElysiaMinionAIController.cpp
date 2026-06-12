// Copyright GhostLazy


#include "AI/AIController/ElysiaMinionAIController.h"
#include "Character/ElysiaEnemy.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"

void AElysiaMinionAIController::UpdateBehavior()
{
	AElysiaEnemy* ElysiaEnemy = Cast<AElysiaEnemy>(GetPawn());
	if (!ElysiaEnemy || ElysiaEnemy->IsDead())
	{
		StopAI();
		return;
	}

	RefreshTarget();
	if (!HasValidTarget())
	{
		StopMovement();
		ClearFocus(EAIFocusPriority::Gameplay);
		return;
	}
	SetFocus(GetTargetActor(), EAIFocusPriority::Gameplay);

	if (ElysiaEnemy->HasOverlappingPlayers())
	{
		StopMovement();
		ClearFocus(EAIFocusPriority::Gameplay);
		return;
	}

	const FVector SeparationOffset = ElysiaEnemy->GetSoftSeparationOffset();
	if (SeparationOffset.IsNearlyZero(1.f))
	{
		MoveToCurrentTarget();
		return;
	}

	FVector DirectionToTarget = GetTargetActor()->GetActorLocation() - ElysiaEnemy->GetActorLocation();
	DirectionToTarget.Z = 0.f;
	const float DistanceToTarget = DirectionToTarget.Size2D();
	const float ForwardDistance = FMath::Min(
		DistanceToTarget,
		FMath::Max(100.f, SeparationLookAheadDistance));
	const FVector MoveDestination = ElysiaEnemy->GetActorLocation()
		+ DirectionToTarget.GetSafeNormal2D() * ForwardDistance
		+ SeparationOffset * FMath::Max(0.f, SeparationSteeringScale);
	MoveToLocation(MoveDestination, AcceptanceRadius, true, true, true, true, nullptr, true);
}
