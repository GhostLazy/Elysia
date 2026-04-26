// Copyright GhostLazy


#include "AI/AIController/ElysiaMinionAIController.h"
#include "AbilitySystem/ElysiaAbilitySystemLibrary.h"
#include "Character/ElysiaEnemy.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"

void AElysiaMinionAIController::StartAI()
{
	Super::StartAI();

	StartBehaviorTimer();
	UpdateBehavior();
}

void AElysiaMinionAIController::StopAI()
{
	StopBehaviorTimer();

	Super::StopAI();
}

void AElysiaMinionAIController::RefreshTarget()
{
	if (const AElysiaEnemy* ElysiaEnemy = Cast<AElysiaEnemy>(GetPawn()))
	{
		if (AActor* OverlapTarget = ElysiaEnemy->GetPreferredDamageTarget())
		{
			SetTargetActor(OverlapTarget);
			return;
		}
	}

	SetTargetActor(FindClosestPlayerInRange());
}

AActor* AElysiaMinionAIController::FindClosestPlayerInRange() const
{
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
	{
		return nullptr;
	}

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(ControlledPawn);

	TArray<AActor*> CandidatePlayers;
	UElysiaAbilitySystemLibrary::GetLiveActorsWithInRadius(
		this,
		CandidatePlayers,
		ActorsToIgnore,
		SearchRadius,
		ControlledPawn->GetActorLocation(),
		FName("Player"));

	return UElysiaAbilitySystemLibrary::GetClosestActor(CandidatePlayers, ControlledPawn->GetActorLocation());
}

void AElysiaMinionAIController::UpdateBehavior()
{
	AElysiaEnemy* ElysiaEnemy = Cast<AElysiaEnemy>(GetPawn());
	if (!ElysiaEnemy || ElysiaEnemy->IsDead())
	{
		StopAI();
		return;
	}

	RefreshTarget();

	if (ElysiaEnemy->HasOverlappingPlayers())
	{
		StopMovement();
		ClearFocus(EAIFocusPriority::Gameplay);
		return;
	}

	if (AActor* CurrentTarget = GetTargetActor())
	{
		MoveToActor(CurrentTarget, AcceptanceRadius, true, true, true, nullptr, true);
		return;
	}

	StopMovement();
	ClearFocus(EAIFocusPriority::Gameplay);
}

void AElysiaMinionAIController::StartBehaviorTimer()
{
	if (!GetWorld() || GetWorld()->GetTimerManager().IsTimerActive(BehaviorTimerHandle))
	{
		return;
	}

	GetWorld()->GetTimerManager().SetTimer(
		BehaviorTimerHandle,
		this,
		&AElysiaMinionAIController::UpdateBehavior,
		FMath::Max(0.05f, BehaviorTickInterval),
		true);
}

void AElysiaMinionAIController::StopBehaviorTimer()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(BehaviorTimerHandle);
	}
}
