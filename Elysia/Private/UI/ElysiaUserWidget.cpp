// Copyright GhostLazy


#include "UI/ElysiaUserWidget.h"

void UElysiaUserWidget::SetWidgetController(UObject* InWidgetController)
{
	WidgetController = InWidgetController;
	WidgetControllerSet();
}