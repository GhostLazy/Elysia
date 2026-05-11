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

	MoveToCurrentTarget();
}
