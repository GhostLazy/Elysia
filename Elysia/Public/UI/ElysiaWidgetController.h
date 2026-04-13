// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "ElysiaWidgetController.generated.h"

class AElysiaPlayerState;
class AElysiaPlayerController;
class UElysiaAttributeSet;
class UElysiaAbilitySystemComponent;
/**
 * 
 */
UCLASS()
class ELYSIA_API UElysiaWidgetController : public UObject
{
	GENERATED_BODY()
	
protected:
	
	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<UElysiaAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<UElysiaAttributeSet> AttributeSet;
	
	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<AElysiaPlayerController> PlayerController;
	
	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<AElysiaPlayerState> PlayerState;
	
	virtual void BindCallbacksToDependencies();
	
};
