// Copyright GhostLazy


#include "Actor/ElysiaTrialInteractableActor.h"

#include "Actor/ElysiaTrialEventBase.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Elysia/Elysia.h"
#include "Net/UnrealNetwork.h"
#include "UI/ElysiaTrialIndicatorWidget.h"

AElysiaTrialInteractableActor::AElysiaTrialInteractableActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(SceneRoot);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->SetupAttachment(SceneRoot);
	InteractionSphere->SetSphereRadius(InteractionRadius);
	InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	InteractionSphere->SetCollisionResponseToChannel(ECC_Player, ECR_Overlap);

	IndicatorWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("IndicatorWidgetComponent"));
	IndicatorWidgetComponent->SetupAttachment(SceneRoot);
	IndicatorWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	IndicatorWidgetComponent->SetDrawAtDesiredSize(true);
	IndicatorWidgetComponent->SetRelativeLocation(IndicatorRelativeLocation);
	IndicatorWidgetComponent->SetVisibility(false);
}

void AElysiaTrialInteractableActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AElysiaTrialInteractableActor, TrialEvent);
}

void AElysiaTrialInteractableActor::BeginPlay()
{
	Super::BeginPlay();

	RefreshComponentSettings();

	if (InteractionSphere)
	{
		InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &AElysiaTrialInteractableActor::HandleInteractionSphereBeginOverlap);
		InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &AElysiaTrialInteractableActor::HandleInteractionSphereEndOverlap);
	}

	if (TrialEvent)
	{
		InitializeIndicatorWidget();
	}
}

void AElysiaTrialInteractableActor::InitializeTrialOffer(AElysiaTrialEventBase* InTrialEvent)
{
	TrialEvent = InTrialEvent;
	SetOwner(InTrialEvent);
	RefreshComponentSettings();
	SetIndicatorVisible(bShowIndicatorWhileWaiting);
	InitializeIndicatorWidget();
	OnTrialOfferInitialized(InTrialEvent);
}

bool AElysiaTrialInteractableActor::Interact(AActor* InteractingActor)
{
	if (!HasAuthority() || !CanInteract(InteractingActor))
	{
		return false;
	}

	SetIndicatorVisible(false);
	OnTrialOfferInteracted(InteractingActor);

	if (TrialEvent)
	{
		TrialEvent->TriggerTrial(InteractingActor);
		return true;
	}

	return false;
}

bool AElysiaTrialInteractableActor::CanInteract(AActor* InteractingActor) const
{
	if (!IsValid(InteractingActor) || !IsValid(TrialEvent) || !TrialEvent->CanBeTriggeredBy(InteractingActor))
	{
		return false;
	}

	if (InteractionRadius <= 0.f)
	{
		return true;
	}

	return FVector::DistSquared(InteractingActor->GetActorLocation(), GetActorLocation()) <= FMath::Square(InteractionRadius);
}

float AElysiaTrialInteractableActor::GetRemainingOfferTime() const
{
	return TrialEvent ? TrialEvent->GetRemainingOfferTime() : 0.f;
}

float AElysiaTrialInteractableActor::GetTotalOfferTime() const
{
	return TrialEvent ? TrialEvent->GetTotalOfferTime() : 0.f;
}

FVector AElysiaTrialInteractableActor::GetIndicatorTargetLocation() const
{
	return GetActorLocation() + IndicatorRelativeLocation;
}

void AElysiaTrialInteractableActor::SetIndicatorVisible(bool bVisible)
{
	if (IndicatorWidgetComponent)
	{
		IndicatorWidgetComponent->SetVisibility(bVisible);
		IndicatorWidgetComponent->SetHiddenInGame(!bVisible);
	}
}

void AElysiaTrialInteractableActor::OnTrialOfferInitialized_Implementation(AElysiaTrialEventBase* InTrialEvent)
{
}

void AElysiaTrialInteractableActor::OnTrialOfferInteracted_Implementation(AActor* InteractingActor)
{
}

void AElysiaTrialInteractableActor::OnInteractorEnteredInteractionRange_Implementation(AActor* InteractingActor)
{
}

void AElysiaTrialInteractableActor::OnInteractorLeftInteractionRange_Implementation(AActor* InteractingActor)
{
}

void AElysiaTrialInteractableActor::HandleInteractionSphereBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (TrialEvent && TrialEvent->CanBeTriggeredBy(OtherActor))
	{
		OnInteractorEnteredInteractionRange(OtherActor);
	}
}

void AElysiaTrialInteractableActor::HandleInteractionSphereEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	if (TrialEvent && TrialEvent->CanBeTriggeredBy(OtherActor))
	{
		OnInteractorLeftInteractionRange(OtherActor);
	}
}

void AElysiaTrialInteractableActor::OnRep_TrialEvent()
{
	RefreshComponentSettings();
	SetIndicatorVisible(bShowIndicatorWhileWaiting && IsValid(TrialEvent));
	InitializeIndicatorWidget();
	OnTrialOfferInitialized(TrialEvent);
}

void AElysiaTrialInteractableActor::RefreshComponentSettings()
{
	if (InteractionSphere)
	{
		InteractionSphere->SetSphereRadius(InteractionRadius);
	}

	if (IndicatorWidgetComponent)
	{
		IndicatorWidgetComponent->SetRelativeLocation(IndicatorRelativeLocation);
	}
}

void AElysiaTrialInteractableActor::InitializeIndicatorWidget()
{
	if (!IndicatorWidgetComponent)
	{
		return;
	}

	IndicatorWidgetComponent->InitWidget();
	if (UElysiaTrialIndicatorWidget* IndicatorWidget = Cast<UElysiaTrialIndicatorWidget>(IndicatorWidgetComponent->GetUserWidgetObject()))
	{
		IndicatorWidget->SetTrialOfferActor(this);
	}
}
