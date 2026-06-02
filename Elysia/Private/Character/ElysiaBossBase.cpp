// Copyright GhostLazy


#include "Character/ElysiaBossBase.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Abilities/ElysiaBossGameplayAbility.h"
#include "AI/AIController/ElysiaBossAIController.h"
#include "Components/CapsuleComponent.h"
#include "Elysia/Elysia.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Interface/CombatInterface.h"
#include "NavigationSystem.h"

AElysiaBossBase::AElysiaBossBase()
{
	EnemyType = EElysiaEnemyType::Boss;
	AIControllerClass = AElysiaBossAIController::StaticClass();
	
	GetCapsuleComponent()->SetCollisionObjectType(ECC_Boss);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Player, ECR_Overlap);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Minion, ECR_Overlap);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Projectile, ECR_Overlap);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	GetMesh()->SetCollisionResponseToAllChannels(ECR_Ignore);
}

void AElysiaBossBase::BeginPlay()
{
	Super::BeginPlay();
	GrantBossAbilities();
}

void AElysiaBossBase::Die()
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->CancelAllAbilities();
	}

	ActiveBossAbilityCount = 0;
	Super::Die();
}

void AElysiaBossBase::SetCombatTarget(AActor* InTargetActor)
{
	CombatTarget = InTargetActor;
}

bool AElysiaBossBase::HasValidCombatTarget() const
{
	if (AActor* TargetActor = CombatTarget.Get())
	{
		const ICombatInterface* CombatInterface = Cast<ICombatInterface>(TargetActor);
		return CombatInterface && CombatInterface->IsPlayer() && !CombatInterface->IsDead();
	}

	return false;
}

float AElysiaBossBase::GetDistanceToCombatTarget2D() const
{
	if (const AActor* TargetActor = CombatTarget.Get())
	{
		return FVector::Dist2D(GetActorLocation(), TargetActor->GetActorLocation());
	}

	return FLT_MAX;
}

bool AElysiaBossBase::TryActivateBestBossAbility()
{
	if (!HasAuthority() || IsUsingBossAbility() || !AbilitySystemComponent || !HasValidCombatTarget())
	{
		return false;
	}

	const FGameplayAbilityActorInfo* ActorInfo = AbilitySystemComponent->AbilityActorInfo.Get();
	if (!ActorInfo)
	{
		return false;
	}

	struct FWeightedBossAbility
	{
		FGameplayAbilitySpecHandle Handle;
		float Weight = 0.f;
	};

	TArray<FWeightedBossAbility> CandidateAbilities;
	float TotalWeight = 0.f;

	for (const FGameplayAbilitySpec& AbilitySpec : AbilitySystemComponent->GetActivatableAbilities())
	{
		const UElysiaBossGameplayAbility* BossAbility = Cast<UElysiaBossGameplayAbility>(AbilitySpec.Ability);
		if (!BossAbility || BossAbility->GetSelectionWeight() <= 0.f)
		{
			continue;
		}

		if (!BossAbility->CanActivateAbility(AbilitySpec.Handle, ActorInfo))
		{
			continue;
		}

		CandidateAbilities.Add({ AbilitySpec.Handle, BossAbility->GetSelectionWeight() });
		TotalWeight += BossAbility->GetSelectionWeight();
	}

	if (CandidateAbilities.IsEmpty() || TotalWeight <= 0.f)
	{
		return false;
	}

	float RemainingWeight = FMath::FRandRange(0.f, TotalWeight);
	for (const FWeightedBossAbility& CandidateAbility : CandidateAbilities)
	{
		RemainingWeight -= CandidateAbility.Weight;
		if (RemainingWeight <= 0.f)
		{
			return AbilitySystemComponent->TryActivateAbility(CandidateAbility.Handle);
		}
	}

	return AbilitySystemComponent->TryActivateAbility(CandidateAbilities[0].Handle);
}

bool AElysiaBossBase::TryTeleportNearCombatTargetIfTooFar()
{
	if (!HasAuthority() || !bTeleportNearTargetWhenTooFar || IsUsingBossAbility() || !HasValidCombatTarget())
	{
		return false;
	}

	if (GetDistanceToCombatTarget2D() <= TeleportTriggerDistance)
	{
		return false;
	}

	FVector TeleportLocation;
	if (!TryFindTeleportLocationNearCombatTarget(TeleportLocation))
	{
		return false;
	}

	const AActor* TargetActor = CombatTarget.Get();
	const FVector FacingDirection = TargetActor
		? (TargetActor->GetActorLocation() - TeleportLocation).GetSafeNormal2D()
		: GetActorForwardVector().GetSafeNormal2D();
	const FRotator TeleportRotation = FacingDirection.IsNearlyZero()
		? GetActorRotation()
		: FacingDirection.Rotation();

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->StopMovementImmediately();
	}

	return TeleportTo(TeleportLocation, TeleportRotation, false, false);
}

bool AElysiaBossBase::ApplyBossDamageToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> DamageEffectClass, float DamageEffectLevel) const
{
	if (!IsValid(TargetActor) || !AbilitySystemComponent || !DamageEffectClass)
	{
		return false;
	}

	const ICombatInterface* CombatInterface = Cast<ICombatInterface>(TargetActor);
	if (!CombatInterface || !CombatInterface->IsPlayer() || CombatInterface->IsDead())
	{
		return false;
	}

	if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor))
	{
		FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
		EffectContext.AddSourceObject(this);

		const FGameplayEffectSpecHandle EffectSpecHandle = AbilitySystemComponent->MakeOutgoingSpec(
			DamageEffectClass,
			FMath::Max(1.f, DamageEffectLevel),
			EffectContext);

		if (EffectSpecHandle.IsValid())
		{
			TargetASC->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
			return true;
		}
	}

	return false;
}

void AElysiaBossBase::NotifyBossAbilityStarted(const UElysiaBossGameplayAbility* BossAbility)
{
	++ActiveBossAbilityCount;
}

void AElysiaBossBase::NotifyBossAbilityEnded(const UElysiaBossGameplayAbility* BossAbility)
{
	ActiveBossAbilityCount = FMath::Max(0, ActiveBossAbilityCount - 1);
	if (ActiveBossAbilityCount == 0)
	{
		OnBossAbilityFinished.Broadcast();
	}
}

void AElysiaBossBase::GrantBossAbilities()
{
	if (!HasAuthority() || !AbilitySystemComponent)
	{
		return;
	}

	for (const TSubclassOf<UElysiaBossGameplayAbility>& AbilityClass : BossAbilityClasses)
	{
		if (!AbilityClass)
		{
			continue;
		}

		bool bAlreadyGranted = false;
		for (const FGameplayAbilitySpec& ExistingSpec : AbilitySystemComponent->GetActivatableAbilities())
		{
			if (ExistingSpec.Ability && ExistingSpec.Ability->GetClass() == AbilityClass)
			{
				bAlreadyGranted = true;
				break;
			}
		}

		if (!bAlreadyGranted)
		{
			AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AbilityClass, FMath::Max(1, GetLevel())));
		}
	}
}

bool AElysiaBossBase::TryFindTeleportLocationNearCombatTarget(FVector& OutTeleportLocation) const
{
	const AActor* TargetActor = CombatTarget.Get();
	const UCapsuleComponent* BossCapsuleComponent = GetCapsuleComponent();
	if (!TargetActor || !BossCapsuleComponent || TeleportTargetMaxRadius <= 0.f || TeleportMaxAttempts <= 0)
	{
		return false;
	}

	const float CapsuleRadius = BossCapsuleComponent->GetScaledCapsuleRadius();
	const float CapsuleHalfHeight = BossCapsuleComponent->GetScaledCapsuleHalfHeight();
	if (CapsuleRadius <= 0.f || CapsuleHalfHeight <= 0.f)
	{
		return false;
	}

	UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (!NavSystem)
	{
		return false;
	}

	const float MaxRadius = FMath::Max(0.f, TeleportTargetMaxRadius);
	const float MinRadius = FMath::Clamp(TeleportTargetMinRadius, 0.f, MaxRadius);
	const float MinRadiusSquared = FMath::Square(MinRadius);
	const float MaxRadiusSquared = FMath::Square(MaxRadius);
	const FVector TargetLocation = TargetActor->GetActorLocation();

	for (int32 AttemptIndex = 0; AttemptIndex < TeleportMaxAttempts; ++AttemptIndex)
	{
		const float Radius = FMath::Sqrt(FMath::FRandRange(MinRadiusSquared, MaxRadiusSquared));
		const float AngleRadians = FMath::FRandRange(0.f, 2.f * PI);
		const FVector CandidateLocation = TargetLocation + FVector(FMath::Cos(AngleRadians) * Radius, FMath::Sin(AngleRadians) * Radius, 0.f);

		FNavLocation NavLocation;
		if (!NavSystem->ProjectPointToNavigation(CandidateLocation, NavLocation, TeleportNavProjectExtent))
		{
			continue;
		}

		FVector GroundedTeleportLocation;
		if (TryProjectTeleportCandidateToGround(NavLocation.Location, CapsuleHalfHeight, GroundedTeleportLocation)
			&& IsTeleportLocationClear(GroundedTeleportLocation, CapsuleRadius, CapsuleHalfHeight, TargetActor))
		{
			OutTeleportLocation = GroundedTeleportLocation;
			return true;
		}
	}

	return false;
}

bool AElysiaBossBase::TryProjectTeleportCandidateToGround(const FVector& CandidateLocation, float CapsuleHalfHeight, FVector& OutTeleportLocation) const
{
	if (!GetWorld())
	{
		return false;
	}

	FHitResult GroundHit;
	const FVector TraceStart = CandidateLocation + FVector::UpVector * TeleportGroundTraceUpDistance;
	const FVector TraceEnd = CandidateLocation - FVector::UpVector * TeleportGroundTraceDownDistance;

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ElysiaBossTeleportGroundTrace), false);
	QueryParams.AddIgnoredActor(this);

	if (!GetWorld()->LineTraceSingleByObjectType(GroundHit, TraceStart, TraceEnd, ObjectQueryParams, QueryParams)
		|| !GroundHit.bBlockingHit)
	{
		return false;
	}

	OutTeleportLocation = GroundHit.ImpactPoint + FVector::UpVector * (CapsuleHalfHeight + TeleportGroundClearance);
	return true;
}

bool AElysiaBossBase::IsTeleportLocationClear(const FVector& TeleportLocation, float CapsuleRadius, float CapsuleHalfHeight, const AActor* TargetActor) const
{
	if (!GetWorld())
	{
		return false;
	}

	const float MinAllowedRadius = FMath::Clamp(TeleportTargetMinRadius, 0.f, FMath::Max(0.f, TeleportTargetMaxRadius));
	if (TargetActor && FVector::DistSquared2D(TeleportLocation, TargetActor->GetActorLocation()) < FMath::Square(MinAllowedRadius))
	{
		return false;
	}

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Player);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Minion);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Boss);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ElysiaBossTeleportOverlap), false);
	QueryParams.AddIgnoredActor(this);
	if (TargetActor)
	{
		QueryParams.AddIgnoredActor(TargetActor);
	}

	const float TestHalfHeight = FMath::Max(1.f, CapsuleHalfHeight - TeleportGroundClearance);
	return !GetWorld()->OverlapAnyTestByObjectType(
		TeleportLocation,
		FQuat::Identity,
		ObjectQueryParams,
		FCollisionShape::MakeCapsule(CapsuleRadius, TestHalfHeight),
		QueryParams);
}
