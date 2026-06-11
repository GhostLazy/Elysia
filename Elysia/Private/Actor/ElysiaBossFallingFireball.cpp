// Copyright GhostLazy

#include "Actor/ElysiaBossFallingFireball.h"

#include "AbilitySystemComponent.h"
#include "Character/ElysiaBossBase.h"
#include "Components/SceneComponent.h"
#include "Engine/OverlapResult.h"
#include "ElysiaGameplayTags.h"
#include "Elysia/Elysia.h"
#include "GameplayEffect.h"
#include "Net/UnrealNetwork.h"

AElysiaBossFallingFireball::AElysiaBossFallingFireball()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicatingMovement(true);
	SetNetUpdateFrequency(30.f);
	SetMinNetUpdateFrequency(15.f);

	SceneRoot = CreateDefaultSubobject<USceneComponent>("SceneRoot");
	SetRootComponent(SceneRoot);
}

void AElysiaBossFallingFireball::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AElysiaBossFallingFireball, TargetLocation);
	DOREPLIFETIME(AElysiaBossFallingFireball, FallDuration);
	DOREPLIFETIME(AElysiaBossFallingFireball, ImpactRadius);
}

void AElysiaBossFallingFireball::InitializeFireball(
	AElysiaBossBase* InSourceBoss,
	const FVector& InTargetLocation,
	TSubclassOf<UGameplayEffect> InDamageEffectClass,
	float InDamageEffectLevel,
	float InFallDuration,
	float InImpactRadius)
{
	SourceBoss = InSourceBoss;
	TargetLocation = InTargetLocation;
	DamageEffectClass = InDamageEffectClass;
	DamageEffectLevel = FMath::Max(1.f, InDamageEffectLevel);
	FallDuration = FMath::Max(0.01f, InFallDuration);
	ImpactRadius = FMath::Max(0.f, InImpactRadius);
	StartLocation = GetActorLocation();
	bInitialized = true;
}

void AElysiaBossFallingFireball::BeginPlay()
{
	Super::BeginPlay();

	StartLocation = GetActorLocation();
	if (HasAuthority() && !bInitialized)
	{
		Destroy();
	}
}

void AElysiaBossFallingFireball::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!HasAuthority() || !bInitialized || bImpacted)
	{
		return;
	}

	ElapsedFallTime += DeltaSeconds;
	const float Alpha = FMath::Clamp(ElapsedFallTime / FallDuration, 0.f, 1.f);
	SetActorLocation(FMath::Lerp(StartLocation, TargetLocation, Alpha));
	if (Alpha >= 1.f)
	{
		Impact();
	}
}

void AElysiaBossFallingFireball::Impact()
{
	if (bImpacted)
	{
		return;
	}
	bImpacted = true;

	// 落地表现独立于是否命中角色，确保空砸时客户端仍能看到冲击效果。
	if (SourceBoss)
	{
		UAbilitySystemComponent* AbilitySystemComponent = SourceBoss->GetAbilitySystemComponent();
		const FGameplayTag ImpactCueTag = FElysiaGameplayTags::Get().GameplayCue_Boss_Fireball_Impact;
		if (AbilitySystemComponent && ImpactCueTag.IsValid())
		{
			FGameplayCueParameters CueParameters;
			CueParameters.Location = TargetLocation;
			CueParameters.Normal = FVector::UpVector;
			CueParameters.RawMagnitude = ImpactRadius;
			CueParameters.AbilityLevel = FMath::Max(1, SourceBoss->GetEnemyLevel());
			CueParameters.GameplayEffectLevel = DamageEffectLevel;
			CueParameters.Instigator = SourceBoss;
			CueParameters.EffectCauser = SourceBoss;
			AbilitySystemComponent->ExecuteGameplayCue(ImpactCueTag, CueParameters);
		}
	}

	if (SourceBoss && DamageEffectClass && ImpactRadius > 0.f)
	{
		TArray<FOverlapResult> Overlaps;
		FCollisionObjectQueryParams ObjectQueryParams;
		ObjectQueryParams.AddObjectTypesToQuery(ECC_Player);

		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ElysiaBossFireballImpact), false);
		QueryParams.AddIgnoredActor(this);
		QueryParams.AddIgnoredActor(SourceBoss);

		if (GetWorld()->OverlapMultiByObjectType(
			Overlaps,
			TargetLocation,
			FQuat::Identity,
			ObjectQueryParams,
			FCollisionShape::MakeSphere(ImpactRadius),
			QueryParams))
		{
			for (const FOverlapResult& Overlap : Overlaps)
			{
				SourceBoss->ApplyBossDamageToTarget(Overlap.GetActor(), DamageEffectClass, DamageEffectLevel);
			}
		}
	}

	Destroy();
}
