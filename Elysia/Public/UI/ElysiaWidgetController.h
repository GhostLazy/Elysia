// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "ElysiaWidgetController.generated.h"

class UAbilitySystemComponent;
class UAttributeSet;
/**
 * 
 */
UCLASS()
class ELYSIA_API UElysiaWidgetController : public UObject
{
	GENERATED_BODY()
	
public:
	
	void SetWidgetControllerParams(APlayerState* PS, APlayerController* PC, UAbilitySystemComponent* ASC, UAttributeSet* AS);
	void BindCallbacksToDependencies();
	
protected:
	
	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<UAttributeSet> AttributeSet;
	
	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<APlayerController> PlayerController;
	
	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<APlayerState> PlayerState;
	
	UFUNCTION()
	void OnXPChanged(int32 NewXP);
	
	UFUNCTION(BlueprintImplementableEvent)
	void OnLevelChanged(int32 NewLevel, bool bLevelUp);
	
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateProgressPercent(const float Percent);
	
};
