// Copyright GhostLazy


#include "Character/ElysiaCharacterBase.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/ElysiaAbilitySystemComponent.h"
#include "AbilitySystem/ElysiaAttributeSet.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UI/ElysiaUserWidget.h"
#include "Components/CapsuleComponent.h"
#include "Elysia/Elysia.h"

AElysiaCharacterBase::AElysiaCharacterBase()
{
	// 设置角色血条
	HealthBar = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBar"));
	HealthBar->SetupAttachment(RootComponent);
}

void AElysiaCharacterBase::InitDefaultAttributes() const
{
	ApplyEffectToSelf(DefaultPrimaryAttributeClass, 1.f);
	ApplyEffectToSelf(DefaultVitalAttributeClass, 1.f);
	
	// 当移速属性变化时，改变角色移速
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UElysiaAttributeSet::GetMoveSpeedAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
	{
		GetCharacterMovement()->MaxWalkSpeed = Data.NewValue;
	});
	
	// 根据移速属性初始值，初始化角色移速
	if (UElysiaAttributeSet* ElysiaAS = Cast<UElysiaAttributeSet>(AttributeSet))
	{
		GetCharacterMovement()->MaxWalkSpeed = ElysiaAS->GetMoveSpeed();
	}
}

void AElysiaCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
	if (UElysiaUserWidget* ElysiaUserWidget = Cast<UElysiaUserWidget>(HealthBar->GetUserWidgetObject()))
	{
		ElysiaUserWidget->SetWidgetController(this);
	}
}

void AElysiaCharacterBase::InitHealthBar()
{
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UElysiaAttributeSet::GetHealthAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
	{
		OnHealthChanged.Broadcast(Data.NewValue);
	});
	
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UElysiaAttributeSet::GetMaxHealthAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
	{
		OnMaxHealthChanged.Broadcast(Data.NewValue);
	});
}

void AElysiaCharacterBase::ApplyEffectToSelf(const TSubclassOf<UGameplayEffect>& EffectClass, const float Level) const
{
	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	EffectContext.AddSourceObject(this);
	
	check(EffectClass);
	const FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(EffectClass, Level, EffectContext);
	
	AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
}

void AElysiaCharacterBase::AddCharacterAbilities() const
{
	if (!HasAuthority()) return;
	
	if (UElysiaAbilitySystemComponent* ElysiaASC = Cast<UElysiaAbilitySystemComponent>(AbilitySystemComponent))
	{
		ElysiaASC->AddCharacterAbilities(StartupAbilities);
		ElysiaASC->AddCharacterPassiveAbilities(StartupPassiveAbilities);
	}
}
