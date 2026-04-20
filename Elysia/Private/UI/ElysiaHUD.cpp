// Copyright GhostLazy


#include "UI/ElysiaHUD.h"
#include "Blueprint/UserWidget.h"
#include "UI/ElysiaUserWidget.h"
#include "UI/ElysiaWidgetController.h"

void AElysiaHUD::InitOverlay(APlayerState* PS, APlayerController* PC, UAbilitySystemComponent* ASC, UAttributeSet* AS)
{
	UElysiaUserWidget* Widget = CreateWidget<UElysiaUserWidget>(GetWorld(), OverlayWidgetClass);
	
	UElysiaWidgetController* WidgetController = NewObject<UElysiaWidgetController>(this, OverlayWidgetControllerClass);
	WidgetController->SetWidgetControllerParams(PS, PC, ASC, AS);
	WidgetController->BindCallbacksToDependencies();
	
	Widget->SetWidgetController(WidgetController);
	Widget->AddToViewport();
}
