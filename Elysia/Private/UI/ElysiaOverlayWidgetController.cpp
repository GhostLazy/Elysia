// Copyright GhostLazy


#include "UI/ElysiaOverlayWidgetController.h"
#include "Player/ElysiaPlayerState.h"

void UElysiaOverlayWidgetController::BindCallbacksToDependencies()
{
	if (AElysiaPlayerState* ElysiaPS = Cast<AElysiaPlayerState>(PlayerState))
	{
		// Overlay 只关心常驻 HUD 数据：经验与等级
		ElysiaPS->OnXPChanged.RemoveAll(this);
		ElysiaPS->OnLevelChanged.RemoveAll(this);
		ElysiaPS->OnXPChanged.AddUObject(this, &UElysiaOverlayWidgetController::OnXPChanged);
		ElysiaPS->OnLevelChanged.AddUObject(this, &UElysiaOverlayWidgetController::HandleLevelChanged);

		OnXPChanged(ElysiaPS->GetXP());
		OnLevelTextChange.Broadcast(ElysiaPS->GetPlayerLevel(), false);
	}
}

void UElysiaOverlayWidgetController::OnXPChanged(int32 NewXP) const
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

void UElysiaOverlayWidgetController::HandleLevelChanged(int32 NewLevel, bool bLevelUp)
{
	OnLevelTextChange.Broadcast(NewLevel, bLevelUp);
}
