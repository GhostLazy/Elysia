// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "ElysiaHUD.generated.h"

class UElysiaUserWidget;
class UElysiaWidgetController;
class UAttributeSet;
class UAbilitySystemComponent;
/**
 * 
 */
UCLASS()
class ELYSIA_API AElysiaHUD : public AHUD
{
	GENERATED_BODY()
	
public:
	
	void InitOverlay(APlayerState* PS, APlayerController* PC, UAbilitySystemComponent* ASC, UAttributeSet* AS);
	
protected:
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UElysiaUserWidget> OverlayWidgetClass;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UElysiaWidgetController> OverlayWidgetControllerClass;
	
};
