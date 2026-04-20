// Copyright GhostLazy


#include "UI/ElysiaWidgetController.h"
#include "Player/ElysiaPlayerState.h"

void UElysiaWidgetController::SetWidgetControllerParams(APlayerState* PS, APlayerController* PC,
                                                        UAbilitySystemComponent* ASC, UAttributeSet* AS)
{
	PlayerState = PS;
	PlayerController = PC;
	AbilitySystemComponent = ASC;
	AttributeSet = AS;
}

void UElysiaWidgetController::BindCallbacksToDependencies()
{
	if (AElysiaPlayerState* ElysiaPS = Cast<AElysiaPlayerState>(PlayerState))
	{
		ElysiaPS->OnXPChanged.AddUObject(this, &UElysiaWidgetController::OnXPChanged);
		ElysiaPS->OnLevelChanged.AddUObject(this, &UElysiaWidgetController::OnLevelChanged);
	}
}

void UElysiaWidgetController::OnXPChanged(int32 NewXP)
{
	if (const AElysiaPlayerState* ElysiaPS = Cast<AElysiaPlayerState>(PlayerState))
	{
		const FScalableFloat LevelUpRequirement = ElysiaPS->GetLevelUpRequirement();
		
		int32 Level = 1; 
		for (; Level <= ElysiaPS->GetMaxLevel(); ++Level)
		{
			if (NewXP < LevelUpRequirement.GetValueAtLevel(Level)) break;
		}
		
		float Percent = 1.f;
		if (Level == 1)
		{
			Percent = NewXP / LevelUpRequirement.GetValueAtLevel(Level);
		}
		else if (Level <= ElysiaPS->GetMaxLevel())
		{
			Percent = (NewXP - LevelUpRequirement.GetValueAtLevel(Level - 1)) / LevelUpRequirement.GetValueAtLevel(Level);
		}
		
		UpdateProgressPercent(Percent);
	}
}
