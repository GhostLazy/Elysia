// Copyright GhostLazy


#include "AI/AIController/ElysiaBossAIController.h"
#include "Character/ElysiaBossBase.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Interface/CombatInterface.h"

void AElysiaBossAIController::OnPossess(APawn* InPawn)
{
	BindToBoss(Cast<AElysiaBossBase>(InPawn));
	Super::OnPossess(InPawn);
}

void AElysiaBossAIController::OnUnPossess()
{
	UnbindFromBoss(BoundBoss.Get());
	Super::OnUnPossess();
}

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

	if (IsBossAbilityCooldownReady() && TryActivateBestBossAbility())
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
	if (!IsBossAbilityCooldownReady())
	{
		return false;
	}

	if (AElysiaBossBase* ControlledBoss = GetControlledBoss())
	{
		return ControlledBoss->TryActivateBestBossAbility();
	}

	return false;
}

void AElysiaBossAIController::BindToBoss(AElysiaBossBase* Boss)
{
	if (!Boss || BoundBoss.Get() == Boss)
	{
		return;
	}

	UnbindFromBoss(BoundBoss.Get());
	Boss->OnBossAbilityFinished.RemoveAll(this);
	Boss->OnBossAbilityFinished.AddUObject(this, &AElysiaBossAIController::HandleBossAbilityFinished);
	BoundBoss = Boss;
	NextAllowedBossAbilityTime = 0.f;
}

void AElysiaBossAIController::UnbindFromBoss(AElysiaBossBase* Boss)
{
	if (Boss)
	{
		Boss->OnBossAbilityFinished.RemoveAll(this);
	}

	if (BoundBoss.Get() == Boss)
	{
		BoundBoss.Reset();
	}
}

void AElysiaBossAIController::HandleBossAbilityFinished()
{
	if (const UWorld* World = GetWorld())
	{
		NextAllowedBossAbilityTime = World->GetTimeSeconds() + FMath::Max(0.f, BossAbilityCooldownTime);
	}
}

bool AElysiaBossAIController::IsBossAbilityCooldownReady() const
{
	if (BossAbilityCooldownTime <= 0.f)
	{
		return true;
	}

	const UWorld* World = GetWorld();
	return !World || World->GetTimeSeconds() >= NextAllowedBossAbilityTime;
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
