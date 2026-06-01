// Copyright GhostLazy


#include "UI/ElysiaOverlayWidgetController.h"

#include "ElysiaGameplayTags.h"
#include "Game/ElysiaGameState.h"
#include "Player/ElysiaPlayerState.h"

void UElysiaOverlayWidgetController::BindCallbacksToDependencies()
{
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
		ElysiaGameState->OnTotalScoreChanged.AddUObject(this, &UElysiaOverlayWidgetController::HandleTotalScoreChanged);
		ElysiaGameState->OnNormalPhaseTotalSecondsChanged.AddUObject(this, &UElysiaOverlayWidgetController::HandleGameProgressPercentChanged);
		
		HandleTotalScoreChanged(ElysiaGameState->GetTotalScore());
		HandleGameProgressPercentChanged(ElysiaGameState->GetNormalPhaseTotalSeconds());
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
		if (NormalPhaseTotalDuration <= 0)
		{
			NormalPhaseTotalDuration = ElysiaGameState->GetNormalPhaseTotalDuration();
		}
		
		if (NormalPhaseTotalDuration > 0)
		{
			const float Percent = FMath::Clamp(1.0 * NewTotalSecond / NormalPhaseTotalDuration, 0.f, 1.f);
			OnGameProgressPercentChanged.Broadcast(Percent);
		}
	}
}
