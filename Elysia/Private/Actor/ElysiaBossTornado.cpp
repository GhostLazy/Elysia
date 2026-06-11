// Copyright GhostLazy

#include "Actor/ElysiaBossTornado.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Character/ElysiaBossBase.h"
#include "Components/SphereComponent.h"
#include "EngineUtils.h"
#include "Elysia/Elysia.h"
#include "GameFramework/Pawn.h"
#include "Interface/CombatInterface.h"

AElysiaBossTornado::AElysiaBossTornado()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);
	SetNetUpdateFrequency(30.f);
	SetMinNetUpdateFrequency(15.f);

	CollisionSphere = CreateDefaultSubobject<USphereComponent>("CollisionSphere");
	SetRootComponent(CollisionSphere);
	CollisionSphere->SetSphereRadius(ContactRadius);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionSphere->SetCollisionResponseToChannel(ECC_Player, ECR_Overlap);
}

void AElysiaBossTornado::InitializeTornado(
	AElysiaBossBase* InSourceBoss,
	TSubclassOf<UGameplayEffect> InDamageEffectClass,
	float InDamageEffectLevel,
	float InTrackingSpeed,
	float InDuration,
	float InContactRadius)
{
	SourceBoss = InSourceBoss;
	DamageEffectClass = InDamageEffectClass;
	DamageEffectLevel = FMath::Max(1.f, InDamageEffectLevel);
	TrackingSpeed = FMath::Max(0.f, InTrackingSpeed);
	Duration = FMath::Max(0.01f, InDuration);
	ContactRadius = FMath::Max(0.f, InContactRadius);
	CollisionSphere->SetSphereRadius(ContactRadius);
	bInitialized = true;
}

void AElysiaBossTornado::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
	{
		return;
	}

	if (!bInitialized || !SourceBoss)
	{
		Destroy();
		return;
	}

	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AElysiaBossTornado::HandleOverlapBegin);
	CollisionSphere->OnComponentEndOverlap.AddDynamic(this, &AElysiaBossTornado::HandleOverlapEnd);
	SetLifeSpan(Duration);
}

void AElysiaBossTornado::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HasAuthority())
	{
		ClearContactDamageEffects();
	}

	Super::EndPlay(EndPlayReason);
}

void AElysiaBossTornado::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!HasAuthority() || !bInitialized)
	{
		return;
	}

	AActor* TargetActor = FindClosestPlayer();
	if (!TargetActor)
	{
		return;
	}

	const FVector CurrentLocation = GetActorLocation();
	FVector Direction = TargetActor->GetActorLocation() - CurrentLocation;
	Direction.Z = 0.f;
	Direction = Direction.GetSafeNormal();
	if (Direction.IsNearlyZero())
	{
		return;
	}

	SetActorLocation(CurrentLocation + Direction * TrackingSpeed * DeltaSeconds, true);
	SetActorRotation(Direction.Rotation());
}

void AElysiaBossTornado::HandleOverlapBegin(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	ApplyContactDamage(OtherActor);
}

void AElysiaBossTornado::HandleOverlapEnd(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	RemoveContactDamage(OtherActor);
}

AActor* AElysiaBossTornado::FindClosestPlayer() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	AActor* ClosestPlayer = nullptr;
	float ClosestDistanceSquared = TNumericLimits<float>::Max();
	for (TActorIterator<APawn> It(World); It; ++It)
	{
		APawn* Candidate = *It;
		if (!IsValidPlayerTarget(Candidate))
		{
			continue;
		}

		const float DistanceSquared = FVector::DistSquared2D(GetActorLocation(), Candidate->GetActorLocation());
		if (DistanceSquared < ClosestDistanceSquared)
		{
			ClosestDistanceSquared = DistanceSquared;
			ClosestPlayer = Candidate;
		}
	}

	return ClosestPlayer;
}

void AElysiaBossTornado::ApplyContactDamage(AActor* TargetActor)
{
	if (!HasAuthority() || !SourceBoss || !DamageEffectClass || !IsValidPlayerTarget(TargetActor))
	{
		return;
	}

	const TWeakObjectPtr<AActor> WeakTarget(TargetActor);
	if (const FActiveGameplayEffectHandle* ExistingHandle = ActiveContactDamageEffects.Find(WeakTarget);
		ExistingHandle && ExistingHandle->IsValid())
	{
		return;
	}

	UAbilitySystemComponent* SourceASC = SourceBoss->GetAbilitySystemComponent();
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (!SourceASC || !TargetASC)
	{
		return;
	}

	FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
	EffectContext.AddSourceObject(this);
	const FGameplayEffectSpecHandle EffectSpecHandle = SourceASC->MakeOutgoingSpec(
		DamageEffectClass,
		DamageEffectLevel,
		EffectContext);
	if (!EffectSpecHandle.IsValid())
	{
		return;
	}

	const FActiveGameplayEffectHandle AppliedHandle = TargetASC->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
	if (AppliedHandle.IsValid())
	{
		ActiveContactDamageEffects.FindOrAdd(WeakTarget) = AppliedHandle;
	}
}

void AElysiaBossTornado::RemoveContactDamage(AActor* TargetActor)
{
	if (!HasAuthority() || !TargetActor)
	{
		return;
	}

	const TWeakObjectPtr<AActor> WeakTarget(TargetActor);
	const FActiveGameplayEffectHandle ActiveHandle = ActiveContactDamageEffects.FindRef(WeakTarget);
	if (ActiveHandle.IsValid())
	{
		if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor))
		{
			TargetASC->RemoveActiveGameplayEffect(ActiveHandle);
		}
	}
	ActiveContactDamageEffects.Remove(WeakTarget);
}

void AElysiaBossTornado::ClearContactDamageEffects()
{
	TArray<TWeakObjectPtr<AActor>> ActiveTargets;
	ActiveContactDamageEffects.GetKeys(ActiveTargets);
	for (const TWeakObjectPtr<AActor>& WeakTarget : ActiveTargets)
	{
		RemoveContactDamage(WeakTarget.Get());
	}
}

bool AElysiaBossTornado::IsValidPlayerTarget(AActor* TargetActor)
{
	if (!IsValid(TargetActor))
	{
		return false;
	}

	const ICombatInterface* CombatInterface = Cast<ICombatInterface>(TargetActor);
	return CombatInterface && CombatInterface->IsPlayer() && !CombatInterface->IsDead();
}
