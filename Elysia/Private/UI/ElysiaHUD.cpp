// Copyright GhostLazy


#include "UI/ElysiaHUD.h"
#include "Blueprint/UserWidget.h"
#include "UI/ElysiaLevelUpWidgetController.h"
#include "UI/ElysiaOverlayWidgetController.h"
#include "UI/ElysiaUserWidget.h"

void AElysiaHUD::InitOverlay(APlayerState* PS, APlayerController* PC, UAbilitySystemComponent* ASC, UAttributeSet* AS)
{
	UElysiaUserWidget* Widget = CreateWidget<UElysiaUserWidget>(GetWorld(), OverlayWidgetClass);
	
	UElysiaOverlayWidgetController* WidgetController = GetOverlayWidgetController(PS, PC, ASC, AS);
	if (LevelUpWidgetControllerClass)
	{
		GetLevelUpWidgetController(PS, PC, ASC, AS);
	}

	WidgetController->SetWidgetControllerParams(PS, PC, ASC, AS);
	WidgetController->BindCallbacksToDependencies();
	
	Widget->SetWidgetController(WidgetController);
	Widget->AddToViewport();
}

UElysiaOverlayWidgetController* AElysiaHUD::GetOverlayWidgetController(APlayerState* PS, APlayerController* PC,
                                                                       UAbilitySystemComponent* ASC, UAttributeSet* AS)
{
	if (OverlayWidgetController == nullptr)
	{
		OverlayWidgetController = NewObject<UElysiaOverlayWidgetController>(this, OverlayWidgetControllerClass);
	}

	OverlayWidgetController->SetWidgetControllerParams(PS, PC, ASC, AS);
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
