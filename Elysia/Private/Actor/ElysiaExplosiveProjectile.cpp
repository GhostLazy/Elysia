// Copyright GhostLazy


#include "Actor/ElysiaExplosiveProjectile.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/ElysiaAbilitySystemLibrary.h"
#include "ElysiaGameplayTags.h"
#include "Math/UnrealMathUtility.h"

void AElysiaExplosiveProjectile::SetExplosionRadius(float InExplosionRadius)
{
	ExplosionRadius = FMath::Max(0.f, InExplosionRadius);
}

void AElysiaExplosiveProjectile::HandleEnemyHit(AActor* OtherActor)
{
	if (bHasExploded)
	{
		return;
	}

	bHasExploded = true;
	ExecuteExplosionGameplayCue();

	TArray<AActor*> ActorsToIgnore;
	TArray<AActor*> OverlapActors;
	UElysiaAbilitySystemLibrary::GetLiveActorsWithInRadius(
		this,
		OverlapActors,
		ActorsToIgnore,
		ExplosionRadius,
		GetActorLocation(),
		FName("Damageable"));

	bool bAppliedDamage = false;
	for (AActor* OverlapActor : OverlapActors)
	{
		bAppliedDamage |= ApplyDamageToActor(OverlapActor);
	}

	if (!bAppliedDamage)
	{
		ApplyDamageToActor(OtherActor);
	}

	Destroy();
}

void AElysiaExplosiveProjectile::ExecuteExplosionGameplayCue() const
{
	const FGameplayTag CueTag = ExplosionGameplayCueTag.IsValid()
		? ExplosionGameplayCueTag
		: FElysiaGameplayTags::Get().GameplayCue_Weapon_SmartBullet_Explosion;
	if (!CueTag.IsValid())
	{
		return;
	}

	const FGameplayEffectContextHandle EffectContext = EffectSpecHandle.IsValid()
		? EffectSpecHandle.Data->GetContext()
		: FGameplayEffectContextHandle();

	UAbilitySystemComponent* SourceAbilitySystemComponent =
		EffectContext.GetOriginalInstigatorAbilitySystemComponent();
	if (!SourceAbilitySystemComponent)
	{
		SourceAbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
	}
	if (!SourceAbilitySystemComponent)
	{
		SourceAbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetInstigator());
	}
	if (!SourceAbilitySystemComponent)
	{
		return;
	}

	AActor* OriginalInstigator = EffectContext.GetOriginalInstigator();
	AActor* EffectCauser = EffectContext.GetEffectCauser();

	FGameplayCueParameters CueParameters;
	CueParameters.Location = GetActorLocation();
	CueParameters.Normal = FVector::UpVector;
	CueParameters.RawMagnitude = ExplosionRadius;
	CueParameters.GameplayEffectLevel = EffectSpecHandle.IsValid()
		? EffectSpecHandle.Data->GetLevel()
		: 1.f;
	CueParameters.Instigator = OriginalInstigator ? OriginalInstigator : GetOwner();
	CueParameters.EffectCauser = EffectCauser ? EffectCauser : GetOwner();

	SourceAbilitySystemComponent->ExecuteGameplayCue(CueTag, CueParameters);
}
