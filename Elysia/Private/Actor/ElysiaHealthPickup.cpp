// Copyright GhostLazy


#include "Actor/ElysiaHealthPickup.h"

#include "AbilitySystemComponent.h"
#include "Character/ElysiaCharacter.h"

void AElysiaHealthPickup::HandlePickedBy(AElysiaCharacter* Character)
{
	if (!Character || !HealEffectClass)
	{
		return;
	}

	UAbilitySystemComponent* AbilitySystemComponent = Character->GetAbilitySystemComponent();
	if (!AbilitySystemComponent)
	{
		return;
	}

	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	const FGameplayEffectSpecHandle EffectSpecHandle = AbilitySystemComponent->MakeOutgoingSpec(
		HealEffectClass,
		1.f,
		EffectContext);
	if (!EffectSpecHandle.IsValid())
	{
		return;
	}

	AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
}
