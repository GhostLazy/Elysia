// Copyright GhostLazy


#include "AI/AIController/ElysiaAIControllerBase.h"
#include "AbilitySystem/ElysiaAbilitySystemLibrary.h"
#include "Character/ElysiaEnemy.h"

void AElysiaAIControllerBase::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	StartAI();
}

void AElysiaAIControllerBase::OnUnPossess()
{
	StopAI();
	Super::OnUnPossess();
}

AActor* AElysiaAIControllerBase::FindClosestPlayerInRange() const
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

void AElysiaAIControllerBase::UpdateBehavior()
{
}

void AElysiaAIControllerBase::SetTargetActor(AActor* InTargetActor)
{
	TargetActor = InTargetActor;
	OnTargetActorChanged(TargetActor);
}

void AElysiaAIControllerBase::ClearTargetActor()
{
	SetTargetActor(nullptr);
}

bool AElysiaAIControllerBase::HasValidTarget() const
{
	return IsValid(TargetActor);
}

void AElysiaAIControllerBase::OnTargetActorChanged(AActor* NewTargetActor)
{
}

void AElysiaAIControllerBase::StartAI()
{
	StartBehaviorTimer();
	UpdateBehavior();
}

void AElysiaAIControllerBase::StopAI()
{
	StopBehaviorTimer();
	StopMovement();
	
	ClearFocus(EAIFocusPriority::Gameplay);
	ClearTargetActor();
}

void AElysiaAIControllerBase::RefreshTarget()
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

bool AElysiaAIControllerBase::MoveToCurrentTarget()
{
	if (!HasValidTarget())
	{
		return false;
	}

	MoveToActor(GetTargetActor(), AcceptanceRadius, true, true, true, nullptr, true);
	return true;
}

void AElysiaAIControllerBase::StartBehaviorTimer()
{
	if (!GetWorld() || GetWorld()->GetTimerManager().IsTimerActive(BehaviorTimerHandle))
	{
		return;
	}

	GetWorld()->GetTimerManager().SetTimer(
		BehaviorTimerHandle,
		this,
		&AElysiaAIControllerBase::UpdateBehavior,
		FMath::Max(0.05f, BehaviorTickInterval),
		true);
}

void AElysiaAIControllerBase::StopBehaviorTimer()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(BehaviorTimerHandle);
	}
}
