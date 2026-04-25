// Copyright GhostLazy


#include "UI/ElysiaHUD.h"
#include "Blueprint/UserWidget.h"
#include "UI/ElysiaLevelUpWidgetController.h"
#include "UI/ElysiaOverlayWidgetController.h"
#include "UI/ElysiaUserWidget.h"

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
