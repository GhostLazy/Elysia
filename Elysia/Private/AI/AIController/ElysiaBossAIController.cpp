// Copyright GhostLazy


#include "AI/AIController/ElysiaBossAIController.h"

#include "AbilitySystem/ElysiaAbilitySystemLibrary.h"
#include "Character/ElysiaBossBase.h"

void AElysiaBossAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	RefreshTarget();
}

void AElysiaBossAIController::OnUnPossess()
{
	StopAI();
	Super::OnUnPossess();
}

void AElysiaBossAIController::StartAI()
{
	Super::StartAI();

	if (!GetWorldTimerManager().IsTimerActive(RetargetTimerHandle))
	{
		GetWorldTimerManager().SetTimer(RetargetTimerHandle, this, &AElysiaBossAIController::RefreshTarget, RetargetInterval, true);
	}
}

void AElysiaBossAIController::StopAI()
{
	GetWorldTimerManager().ClearTimer(RetargetTimerHandle);
	Super::StopAI();
}

void AElysiaBossAIController::RefreshTarget()
{
	if (AActor* NewTargetActor = FindClosestLivePlayerInRange())
	{
		SetTargetActor(NewTargetActor);
		if (AElysiaBossBase* ControlledBoss = GetControlledBoss())
		{
			ControlledBoss->SetCombatTarget(NewTargetActor);
		}
	}
	else
	{
		ClearTargetActor();
		if (AElysiaBossBase* ControlledBoss = GetControlledBoss())
		{
			ControlledBoss->SetCombatTarget(nullptr);
		}
	}
}

AElysiaBossBase* AElysiaBossAIController::GetControlledBoss() const
{
	return Cast<AElysiaBossBase>(GetPawn());
}

bool AElysiaBossAIController::MoveToCurrentTarget()
{
	if (!HasValidTarget())
	{
		return false;
	}

	MoveToActor(GetTargetActor(), ChaseAcceptanceRadius, true, true, true, nullptr, true);
	return true;
}

bool AElysiaBossAIController::TryCastBestSkill()
{
	if (AElysiaBossBase* ControlledBoss = GetControlledBoss())
	{
		return ControlledBoss->TryCastBestAvailableSkill();
	}

	return false;
}

bool AElysiaBossAIController::CanCastSkill(EElysiaBossSkillType SkillType) const
{
	if (const AElysiaBossBase* ControlledBoss = GetControlledBoss())
	{
		return ControlledBoss->CanCastSkill(SkillType);
	}

	return false;
}

AActor* AElysiaBossAIController::FindClosestLivePlayerInRange() const
{
	TArray<AActor*> ActorsToIgnore;
	TArray<AActor*> OverlapActors;

	UElysiaAbilitySystemLibrary::GetLiveActorsWithInRadius(this, OverlapActors, ActorsToIgnore, SearchRadius, GetPawn() ? GetPawn()->GetActorLocation() : FVector::ZeroVector, FName("Player"));
	return UElysiaAbilitySystemLibrary::GetClosestActor(OverlapActors, GetPawn() ? GetPawn()->GetActorLocation() : FVector::ZeroVector);
}
