// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "ElysiaHUD.generated.h"

class UElysiaUserWidget;
class UElysiaWidgetController;
class UElysiaOverlayWidgetController;
class UElysiaLevelUpWidgetController;
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
	UElysiaOverlayWidgetController* GetOverlayWidgetController(APlayerState* PS, APlayerController* PC, UAbilitySystemComponent* ASC, UAttributeSet* AS);
	UElysiaLevelUpWidgetController* GetLevelUpWidgetController(APlayerState* PS, APlayerController* PC, UAbilitySystemComponent* ASC, UAttributeSet* AS);
	bool ShowRunResult(int32 FinalScore);
	
protected:
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UElysiaUserWidget> OverlayWidgetClass;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UElysiaUserWidget> LevelUpWidgetClass;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UElysiaOverlayWidgetController> OverlayWidgetControllerClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UElysiaLevelUpWidgetController> LevelUpWidgetControllerClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UElysiaUserWidget> RunResultWidgetClass;

	UPROPERTY()
	TObjectPtr<UElysiaOverlayWidgetController> OverlayWidgetController;

	UPROPERTY()
	TObjectPtr<UElysiaLevelUpWidgetController> LevelUpWidgetController;

	UPROPERTY()
	TObjectPtr<UElysiaUserWidget> RunResultWidget;
	
};
