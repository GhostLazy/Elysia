// Copyright GhostLazy


#include "UI/ElysiaHUD.h"
#include "Blueprint/UserWidget.h"
#include "UI/ElysiaLevelUpWidgetController.h"
#include "UI/ElysiaOverlayWidgetController.h"
#include "UI/ElysiaUserWidget.h"
#include "Game/ElysiaGameState.h"

void AElysiaHUD::InitOverlay(APlayerState* PS, APlayerController* PC, UAbilitySystemComponent* ASC, UAttributeSet* AS)
{
	UElysiaUserWidget* OverlayWidget = CreateWidget<UElysiaUserWidget>(GetWorld(), OverlayWidgetClass);
	UElysiaUserWidget* LevelUpWidget = CreateWidget<UElysiaUserWidget>(GetWorld(), LevelUpWidgetClass);
	
	OverlayWidget->SetWidgetController(GetOverlayWidgetController(PS, PC, ASC, AS));
	LevelUpWidget->SetWidgetController(GetLevelUpWidgetController(PS, PC, ASC, AS));
	OverlayWidget->AddToViewport();
	LevelUpWidget->AddToViewport();
	LevelUpWidget->SetVisibility(ESlateVisibility::Collapsed);
}

UElysiaOverlayWidgetController* AElysiaHUD::GetOverlayWidgetController(APlayerState* PS, APlayerController* PC,
                                                                       UAbilitySystemComponent* ASC, UAttributeSet* AS)
{
	if (OverlayWidgetController == nullptr)
	{
		OverlayWidgetController = NewObject<UElysiaOverlayWidgetController>(this, OverlayWidgetControllerClass);
	}

	if (OverlayWidgetController)
	{
		OverlayWidgetController->SetWidgetControllerParams(PS, PC, ASC, AS);
		OverlayWidgetController->BindCallbacksToDependencies();
	}
	
	return OverlayWidgetController;
}

UElysiaLevelUpWidgetController* AElysiaHUD::GetLevelUpWidgetController(APlayerState* PS, APlayerController* PC,
                                                                       UAbilitySystemComponent* ASC, UAttributeSet* AS)
{
	if (LevelUpWidgetController == nullptr && LevelUpWidgetControllerClass)
	{
		LevelUpWidgetController = NewObject<UElysiaLevelUpWidgetController>(this, LevelUpWidgetControllerClass);
	}

	if (LevelUpWidgetController)
	{
		LevelUpWidgetController->SetWidgetControllerParams(PS, PC, ASC, AS);
		LevelUpWidgetController->BindCallbacksToDependencies();
	}

	return LevelUpWidgetController;
}

bool AElysiaHUD::ShowRunResult(const int32 FinalScore)
{
	if (!IsValid(RunResultWidget))
	{
		APlayerController* OwningPlayerController = GetOwningPlayerController();
		if (!IsValid(OwningPlayerController) || !RunResultWidgetClass)
		{
			return false;
		}

		RunResultWidget = CreateWidget<UElysiaUserWidget>(OwningPlayerController, RunResultWidgetClass);
		if (!IsValid(RunResultWidget))
		{
			return false;
		}

		if (AElysiaGameState* ElysiaGameState = GetWorld()->GetGameState<AElysiaGameState>())
		{
			RunResultWidget->SetWidgetController(ElysiaGameState);
		}

		RunResultWidget->AddToViewport(1000);
	}

	RunResultWidget->SetVisibility(ESlateVisibility::Visible);
	return true;
}
