// Copyright GhostLazy


#include "AI/AIController/ElysiaAIControllerBase.h"
#include "Components/StateTreeAIComponent.h"

AElysiaAIControllerBase::AElysiaAIControllerBase()
{
	StateTreeAIComponent = CreateDefaultSubobject<UStateTreeAIComponent>("StateTreeAIComponent");
}

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

void AElysiaAIControllerBase::SetTargetActor(AActor* InTargetActor)
{
	TargetActor = InTargetActor;
}

void AElysiaAIControllerBase::ClearTargetActor()
{
	TargetActor = nullptr;
}

bool AElysiaAIControllerBase::HasValidTarget() const
{
	return IsValid(TargetActor);
}

void AElysiaAIControllerBase::StartAI()
{
	// StateTree can be configured to autostart from the component details in Blueprints.
}

void AElysiaAIControllerBase::StopAI()
{
	StopMovement();
	ClearFocus(EAIFocusPriority::Gameplay);
	ClearTargetActor();

	if (StateTreeAIComponent && StateTreeAIComponent->IsRunning())
	{
		StateTreeAIComponent->StopLogic(TEXT("AI stopped"));
	}
}
