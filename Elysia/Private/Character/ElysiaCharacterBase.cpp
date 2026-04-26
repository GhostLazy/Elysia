// Copyright GhostLazy


#include "Character/ElysiaCharacterBase.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/ElysiaAbilitySystemComponent.h"
#include "AbilitySystem/ElysiaAttributeSet.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Elysia/Elysia.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UI/ElysiaUserWidget.h"

AElysiaCharacterBase::AElysiaCharacterBase()
{
	// 设置角色血条
	HealthBar = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBar"));
	HealthBar->SetupAttachment(RootComponent);
}

void AElysiaCharacterBase::InitDefaultAttributes()
{
	if (!AbilitySystemComponent || !AttributeSet)
	{
		return;
	}

	if (HasAuthority())
	{
		// 默认属性GE只允许服务端施加，避免客户端本地改权威状态
		ApplyEffectToSelf(DefaultPrimaryAttributeClass, 1.f);
		ApplyEffectToSelf(DefaultVitalAttributeClass, 1.f);
	}

	// 监听移速属性变化，并同步 CharacterMovement
	BindMoveSpeedDelegate();

	// 根据移速属性初始值，初始化角色移速
	if (const UElysiaAttributeSet* ElysiaAS = Cast<UElysiaAttributeSet>(AttributeSet))
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

void AElysiaCharacterBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 角色销毁时移除属性回调，防止旧Pawn残留委托
	UnbindAttributeDelegates();
	Super::EndPlay(EndPlayReason);
}

void AElysiaCharacterBase::InitHealthBar()
{
	if (!AbilitySystemComponent || !AttributeSet)
	{
		return;
	}

	// 绑定血量/最大血量变化，用于驱动血条和其他显示层
	BindHealthBarDelegates();
}

void AElysiaCharacterBase::ApplyEffectToSelf(const TSubclassOf<UGameplayEffect>& EffectClass, const float Level) const
{
	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	check(EffectClass);
	const FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(EffectClass, Level, EffectContext);

	AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
}

void AElysiaCharacterBase::BindMoveSpeedDelegate()
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	// 重新初始化时先移除旧回调，避免同一Pawn重复绑定
	if (MoveSpeedChangedHandle.IsValid())
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UElysiaAttributeSet::GetMoveSpeedAttribute()).Remove(MoveSpeedChangedHandle);
		MoveSpeedChangedHandle.Reset();
	}

	MoveSpeedChangedHandle = AbilitySystemComponent
		->GetGameplayAttributeValueChangeDelegate(UElysiaAttributeSet::GetMoveSpeedAttribute())
		.AddUObject(this, &AElysiaCharacterBase::HandleMoveSpeedChanged);
}

void AElysiaCharacterBase::BindHealthBarDelegates()
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	// 重新初始化时先移除旧的血量回调，避免广播重复触发
	if (HealthChangedHandle.IsValid())
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UElysiaAttributeSet::GetHealthAttribute()).Remove(HealthChangedHandle);
		HealthChangedHandle.Reset();
	}

	if (MaxHealthChangedHandle.IsValid())
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UElysiaAttributeSet::GetMaxHealthAttribute()).Remove(MaxHealthChangedHandle);
		MaxHealthChangedHandle.Reset();
	}

	HealthChangedHandle = AbilitySystemComponent
		->GetGameplayAttributeValueChangeDelegate(UElysiaAttributeSet::GetHealthAttribute())
		.AddUObject(this, &AElysiaCharacterBase::HandleHealthChanged);

	MaxHealthChangedHandle = AbilitySystemComponent
		->GetGameplayAttributeValueChangeDelegate(UElysiaAttributeSet::GetMaxHealthAttribute())
		.AddUObject(this, &AElysiaCharacterBase::HandleMaxHealthChanged);
}

void AElysiaCharacterBase::UnbindAttributeDelegates()
{
	if (AbilitySystemComponent)
	{
		// 当前项目里 ASC 生命周期稳定，直接从当前 ASC 上解绑即可
		if (MoveSpeedChangedHandle.IsValid())
		{
			AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UElysiaAttributeSet::GetMoveSpeedAttribute()).Remove(MoveSpeedChangedHandle);
		}

		if (HealthChangedHandle.IsValid())
		{
			AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UElysiaAttributeSet::GetHealthAttribute()).Remove(HealthChangedHandle);
		}

		if (MaxHealthChangedHandle.IsValid())
		{
			AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UElysiaAttributeSet::GetMaxHealthAttribute()).Remove(MaxHealthChangedHandle);
		}
	}

	MoveSpeedChangedHandle.Reset();
	HealthChangedHandle.Reset();
	MaxHealthChangedHandle.Reset();
}

void AElysiaCharacterBase::HandleMoveSpeedChanged(const FOnAttributeChangeData& Data)
{
	GetCharacterMovement()->MaxWalkSpeed = Data.NewValue;
	OnMoveSpeedChanged.Broadcast(Data.NewValue);
}

void AElysiaCharacterBase::HandleHealthChanged(const FOnAttributeChangeData& Data)
{
	OnHealthChanged.Broadcast(Data.NewValue);
}

void AElysiaCharacterBase::HandleMaxHealthChanged(const FOnAttributeChangeData& Data)
{
	OnMaxHealthChanged.Broadcast(Data.NewValue);
}

void AElysiaCharacterBase::InitCharacterAbilities() const
{
	if (!HasAuthority()) return;

	if (UElysiaAbilitySystemComponent* ElysiaASC = Cast<UElysiaAbilitySystemComponent>(AbilitySystemComponent))
	{
		ElysiaASC->AddCharacterAbilities(StartupAbilities);
	}
}
