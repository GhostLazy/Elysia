// Copyright GhostLazy


#include "AI/AIController/ElysiaBossAIController.h"
#include "Character/ElysiaBossBase.h"

void AElysiaBossAIController::UpdateBehavior()
{
	AElysiaBossBase* ControlledBoss = GetControlledBoss();
	if (!ControlledBoss || ControlledBoss->IsDead())
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

	if (ControlledBoss->IsCastingSkill() || ControlledBoss->IsCharging())
	{
		StopMovement();
		return;
	}

	if (TryCastBestSkill())
	{
		StopMovement();
		return;
	}

	if (ControlledBoss->HasOverlappingPlayers())
	{
		StopMovement();
		return;
	}
	
	MoveToCurrentTarget();
}

AElysiaBossBase* AElysiaBossAIController::GetControlledBoss() const
{
	return Cast<AElysiaBossBase>(GetPawn());
}

void AElysiaBossAIController::OnTargetActorChanged(AActor* NewTargetActor)
{
	if (AElysiaBossBase* ControlledBoss = GetControlledBoss())
	{
		ControlledBoss->SetCombatTarget(NewTargetActor);
	}
}

bool AElysiaBossAIController::TryCastBestSkill()
{
	if (AElysiaBossBase* ControlledBoss = GetControlledBoss())
	{
		return ControlledBoss->TryCastBestAvailableSkill();
	}

	return false;
}
