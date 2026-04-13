// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ElysiaUserWidget.generated.h"

class UElysiaWidgetController;
/**
 * 
 */
UCLASS()
class ELYSIA_API UElysiaUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	
	UFUNCTION(BlueprintCallable)
	void SetWidgetController(UObject* InWidgetController);
	
protected:
	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UObject> WidgetController;
	
	UFUNCTION(BlueprintImplementableEvent)
	void WidgetControllerSet();
	
};
