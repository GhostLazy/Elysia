// Copyright GhostLazy


#include "Actor/ElysiaBombPickup.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/ElysiaAbilitySystemLibrary.h"
#include "Character/ElysiaCharacter.h"

void AElysiaBombPickup::HandlePickedBy(AElysiaCharacter* Character)
{
	if (!Character)
	{
		return;
	}

	UAbilitySystemComponent* SourceASC = Character->GetAbilitySystemComponent();
	if (!SourceASC)
	{
		return;
	}

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(Character);

	TArray<AActor*> OverlapActors;
	UElysiaAbilitySystemLibrary::GetLiveActorsWithInRadius(
		this,
		OverlapActors,
		ActorsToIgnore,
		KillRadius,
		GetActorLocation(),
		FName("Minion"));

	for (AActor* OverlapActor : OverlapActors)
	{
		ApplyInstantKillEffectToTarget(OverlapActor, SourceASC);
	}
}

bool AElysiaBombPickup::ApplyInstantKillEffectToTarget(AActor* TargetActor, UAbilitySystemComponent* SourceASC) const
{
	if (!IsValid(TargetActor) || !SourceASC || !InstantKillEffectClass)
	{
		return false;
	}

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (!TargetASC)
	{
		return false;
	}

	FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	const FGameplayEffectSpecHandle EffectSpecHandle = SourceASC->MakeOutgoingSpec(
		InstantKillEffectClass,
		1.f,
		EffectContext);
	if (!EffectSpecHandle.IsValid())
	{
		return false;
	}

	TargetASC->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
	return true;
}
