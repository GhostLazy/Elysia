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
		ElysiaPS->OnLevelChanged.AddLambda([this](int32 NewLevel, bool bLevelUp)
		{
			OnLevelTextChange.Broadcast(NewLevel, bLevelUp);
		});
	}
}

void UElysiaWidgetController::OnXPChanged(int32 NewXP) const
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
			Percent = NewXP / LevelUpRequirement.GetValueAtLevel(Level);
		}
		else if (NewXP <= LevelUpRequirement.GetValueAtLevel(ElysiaPS->GetMaxLevel()))
		{
			Percent = (NewXP - LevelUpRequirement.GetValueAtLevel(Level - 1)) / (LevelUpRequirement.GetValueAtLevel(Level) - LevelUpRequirement.GetValueAtLevel(Level - 1));
		}
		
		OnXPBarPercentChanged.Broadcast(Percent);
	}
}
