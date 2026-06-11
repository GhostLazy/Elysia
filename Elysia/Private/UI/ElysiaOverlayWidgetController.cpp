// Copyright GhostLazy


#include "UI/ElysiaOverlayWidgetController.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/ElysiaAttributeSet.h"
#include "Character/ElysiaEnemy.h"
#include "ElysiaGameplayTags.h"
#include "Game/ElysiaGameState.h"
#include "Player/ElysiaPlayerState.h"

void UElysiaOverlayWidgetController::BindCallbacksToDependencies()
{
	if (!GameState && PlayerController && PlayerController->GetWorld())
	{
		GameState = PlayerController->GetWorld()->GetGameState();
	}

	if (AElysiaPlayerState* ElysiaPS = Cast<AElysiaPlayerState>(PlayerState))
	{
		// Overlay 只关心常驻 HUD 数据：经验与等级
		ElysiaPS->OnXPChanged.RemoveAll(this);
		ElysiaPS->OnLevelChanged.RemoveAll(this);
		ElysiaPS->OnXPChanged.AddUObject(this, &UElysiaOverlayWidgetController::HandleXPChanged);
		ElysiaPS->OnLevelChanged.AddUObject(this, &UElysiaOverlayWidgetController::HandleLevelChanged);

		HandleXPChanged(ElysiaPS->GetXP());
		HandleLevelChanged(ElysiaPS->GetPlayerLevel(), false);
	}
	
	if (AElysiaGameState* ElysiaGameState = Cast<AElysiaGameState>(GameState))
	{
		ElysiaGameState->OnTotalScoreChanged.RemoveAll(this);
		ElysiaGameState->OnNormalPhaseTotalSecondsChanged.RemoveAll(this);
		ElysiaGameState->OnCurrentBossChanged.RemoveAll(this);
		ElysiaGameState->OnTotalScoreChanged.AddUObject(this, &UElysiaOverlayWidgetController::HandleTotalScoreChanged);
		ElysiaGameState->OnNormalPhaseTotalSecondsChanged.AddUObject(this, &UElysiaOverlayWidgetController::HandleGameProgressPercentChanged);
		ElysiaGameState->OnCurrentBossChanged.AddUObject(this, &UElysiaOverlayWidgetController::HandleCurrentBossChanged);
		
		HandleTotalScoreChanged(ElysiaGameState->GetTotalScore());
		HandleGameProgressPercentChanged(ElysiaGameState->GetNormalPhaseTotalSeconds());
		HandleCurrentBossChanged(ElysiaGameState->GetCurrentBoss());
	}
}

FElysiaCooldownInfo UElysiaOverlayWidgetController::GetSkillCooldownInfo() const
{
	const UElysiaAbilitySystemComponent* ElysiaASC = Cast<UElysiaAbilitySystemComponent>(AbilitySystemComponent);
	if (!ElysiaASC)
	{
		return FElysiaCooldownInfo();
	}

	return ElysiaASC->GetCooldownInfo(FElysiaGameplayTags::Get().Cooldown_Elysia_Skill);
}

void UElysiaOverlayWidgetController::HandleXPChanged(int32 NewXP) const
{
	if (const AElysiaPlayerState* ElysiaPS = Cast<AElysiaPlayerState>(PlayerState))
	{
		const FScalableFloat LevelUpRequirement = ElysiaPS->GetLevelUpRequirement();

		int32 Level = 1;
		for (int32 i = 2; i <= ElysiaPS->GetMaxLevel(); ++i)
		{
			if (NewXP >= LevelUpRequirement.GetValueAtLevel(i - 1)) Level = i;
			else break;
		}

		float Percent = 1.f;
		if (Level == 1)
		{
			const float RequiredXP = LevelUpRequirement.GetValueAtLevel(Level);
			Percent = RequiredXP > 0.f ? static_cast<float>(NewXP) / RequiredXP : 1.f;
		}
		else if (NewXP <= LevelUpRequirement.GetValueAtLevel(ElysiaPS->GetMaxLevel()))
		{
			// 非 1 级时，按当前等级区间换算经验条百分比
			const float CurrentLevelXP = LevelUpRequirement.GetValueAtLevel(Level);
			const float PreviousLevelXP = LevelUpRequirement.GetValueAtLevel(Level - 1);
			const float LevelRange = CurrentLevelXP - PreviousLevelXP;
			Percent = LevelRange > 0.f ? (static_cast<float>(NewXP) - PreviousLevelXP) / LevelRange : 1.f;
		}

		OnXPBarPercentChanged.Broadcast(FMath::Clamp(Percent, 0.f, 1.f));
	}
}

void UElysiaOverlayWidgetController::HandleLevelChanged(int32 NewLevel, bool bLevelUp) const
{
	OnLevelTextChanged.Broadcast(NewLevel, bLevelUp);
}

void UElysiaOverlayWidgetController::HandleTotalScoreChanged(int32 NewTotalScore) const
{
	OnScoreTextChanged.Broadcast(NewTotalScore);
}

void UElysiaOverlayWidgetController::HandleGameProgressPercentChanged(int32 NewTotalSecond)
{
	if (const AElysiaGameState* ElysiaGameState = Cast<AElysiaGameState>(GameState))
	{
		const int32 TotalDuration = ElysiaGameState->GetNormalPhaseTotalDuration();
		const float Percent = TotalDuration > 0
			? FMath::Clamp(static_cast<float>(NewTotalSecond) / static_cast<float>(TotalDuration), 0.f, 1.f)
			: 0.f;
		OnGameProgressPercentChanged.Broadcast(Percent);
	}
}

void UElysiaOverlayWidgetController::HandleCurrentBossChanged(AElysiaEnemy* NewBoss)
{
	UnbindFromCurrentBoss();
	BindToCurrentBoss(NewBoss);
}

void UElysiaOverlayWidgetController::HandleBossHealthChanged(const FOnAttributeChangeData& Data) const
{
	BroadcastBossHealthPercent();
}

void UElysiaOverlayWidgetController::HandleBossMaxHealthChanged(const FOnAttributeChangeData& Data) const
{
	BroadcastBossHealthPercent();
}

void UElysiaOverlayWidgetController::BroadcastBossHealthPercent() const
{
	const AElysiaEnemy* Boss = CurrentBoss.Get();
	const UElysiaAttributeSet* BossAttributeSet = Boss ? Cast<UElysiaAttributeSet>(Boss->GetAttributeSet()) : nullptr;
	if (!Boss || Boss->IsDead() || !BossAttributeSet)
	{
		OnBossHealthPercentChanged.Broadcast(0.f);
		OnBossHealthVisibilityChanged.Broadcast(false);
		return;
	}

	const float MaxHealth = BossAttributeSet->GetMaxHealth();
	const float HealthPercent = MaxHealth > 0.f
		? FMath::Clamp(BossAttributeSet->GetHealth() / MaxHealth, 0.f, 1.f)
		: 0.f;
	OnBossHealthPercentChanged.Broadcast(HealthPercent);
}

void UElysiaOverlayWidgetController::BindToCurrentBoss(AElysiaEnemy* NewBoss)
{
	if (!IsValid(NewBoss) || NewBoss->IsDead())
	{
		OnBossHealthPercentChanged.Broadcast(0.f);
		OnBossHealthVisibilityChanged.Broadcast(false);
		return;
	}

	UAbilitySystemComponent* BossAbilitySystemComponent = NewBoss->GetAbilitySystemComponent();
	const UElysiaAttributeSet* BossAttributeSet = Cast<UElysiaAttributeSet>(NewBoss->GetAttributeSet());
	if (!BossAbilitySystemComponent || !BossAttributeSet)
	{
		OnBossHealthPercentChanged.Broadcast(0.f);
		OnBossHealthVisibilityChanged.Broadcast(false);
		return;
	}

	CurrentBoss = NewBoss;
	BossHealthChangedHandle = BossAbilitySystemComponent
		->GetGameplayAttributeValueChangeDelegate(UElysiaAttributeSet::GetHealthAttribute())
		.AddUObject(this, &UElysiaOverlayWidgetController::HandleBossHealthChanged);
	BossMaxHealthChangedHandle = BossAbilitySystemComponent
		->GetGameplayAttributeValueChangeDelegate(UElysiaAttributeSet::GetMaxHealthAttribute())
		.AddUObject(this, &UElysiaOverlayWidgetController::HandleBossMaxHealthChanged);

	OnBossHealthVisibilityChanged.Broadcast(true);
	BroadcastBossHealthPercent();
}

void UElysiaOverlayWidgetController::UnbindFromCurrentBoss()
{
	if (AElysiaEnemy* Boss = CurrentBoss.Get())
	{
		if (UAbilitySystemComponent* BossAbilitySystemComponent = Boss->GetAbilitySystemComponent())
		{
			if (BossHealthChangedHandle.IsValid())
			{
				BossAbilitySystemComponent
					->GetGameplayAttributeValueChangeDelegate(UElysiaAttributeSet::GetHealthAttribute())
					.Remove(BossHealthChangedHandle);
			}

			if (BossMaxHealthChangedHandle.IsValid())
			{
				BossAbilitySystemComponent
					->GetGameplayAttributeValueChangeDelegate(UElysiaAttributeSet::GetMaxHealthAttribute())
					.Remove(BossMaxHealthChangedHandle);
			}
		}
	}

	CurrentBoss.Reset();
	BossHealthChangedHandle.Reset();
	BossMaxHealthChangedHandle.Reset();
}
