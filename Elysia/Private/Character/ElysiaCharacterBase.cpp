// Copyright GhostLazy


#include "Character/ElysiaCharacterBase.h"
#include "AbilitySystemComponent.h"

void AElysiaCharacterBase::ApplyEffectToSelf(const TSubclassOf<UGameplayEffect>& EffectClass, const float Level) const
{
	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	EffectContext.AddSourceObject(this);
	
	check(DefaultPrimaryAttributeClass);
	const FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(EffectClass, Level, EffectContext);
	
	AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
}

void AElysiaCharacterBase::InitDefaultAttributes() const
{
	ApplyEffectToSelf(DefaultPrimaryAttributeClass, 1.f);
	ApplyEffectToSelf(DefaultVitalAttributeClass, 1.f);
}

void AElysiaCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
}
