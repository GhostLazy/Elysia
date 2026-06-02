// Copyright GhostLazy


#include "AI/AIController/ElysiaBossAIController.h"
#include "Character/ElysiaBossBase.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Interface/CombatInterface.h"

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
		SetTargetActor(FindFallbackCombatTarget());
	}
	
	if (!HasValidTarget())
	{
		StopMovement();
		ClearFocus(EAIFocusPriority::Gameplay);
		return;
	}

	SetFocus(GetTargetActor(), EAIFocusPriority::Gameplay);

	if (ControlledBoss->IsUsingBossAbility())
	{
		StopMovement();
		return;
	}

	if (ControlledBoss->TryTeleportNearCombatTargetIfTooFar())
	{
		StopMovement();
		return;
	}

	if (TryActivateBestBossAbility())
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

bool AElysiaBossAIController::TryActivateBestBossAbility()
{
	if (AElysiaBossBase* ControlledBoss = GetControlledBoss())
	{
		return ControlledBoss->TryActivateBestBossAbility();
	}

	return false;
}

AActor* AElysiaBossAIController::FindFallbackCombatTarget() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	if (const APlayerController* PlayerController = World->GetFirstPlayerController())
	{
		if (APawn* PlayerPawn = PlayerController->GetPawn(); IsValidCombatTarget(PlayerPawn))
		{
			return PlayerPawn;
		}
	}

	for (TActorIterator<APawn> It(World); It; ++It)
	{
		if (IsValidCombatTarget(*It))
		{
			return *It;
		}
	}

	return nullptr;
}

bool AElysiaBossAIController::IsValidCombatTarget(const AActor* Actor)
{
	if (!IsValid(Actor))
	{
		return false;
	}

	const ICombatInterface* CombatInterface = Cast<ICombatInterface>(Actor);
	return CombatInterface && CombatInterface->IsPlayer() && !CombatInterface->IsDead();
}
