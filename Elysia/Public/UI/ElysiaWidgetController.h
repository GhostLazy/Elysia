// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "ElysiaWidgetController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FXPBarPercentChangedSignature, float, Percent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLevelTextChangeSignature, int32, NewLevel, bool, bLevelUp);

class UAbilitySystemComponent;
class UAttributeSet;
/**
 * 
 */
UCLASS(Blueprintable)
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
	void OnXPChanged(int32 NewXP) const;
	
	UPROPERTY(BlueprintAssignable)
	FXPBarPercentChangedSignature OnXPBarPercentChanged;
	
	UPROPERTY(BlueprintAssignable)
	FLevelTextChangeSignature OnLevelTextChange;
	
};
