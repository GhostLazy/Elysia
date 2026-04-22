// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "ElysiaWidgetController.generated.h"

class UAbilitySystemComponent;
class UAttributeSet;
class UElysiaEquipmentComponent;

UCLASS(Blueprintable)
class ELYSIA_API UElysiaWidgetController : public UObject
{
	GENERATED_BODY()

public:

	virtual void SetWidgetControllerParams(APlayerState* PS, APlayerController* PC, UAbilitySystemComponent* ASC, UAttributeSet* AS);
	virtual void BindCallbacksToDependencies();

protected:

	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<UAttributeSet> AttributeSet;

	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<APlayerController> PlayerController;

	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<APlayerState> PlayerState;

	UElysiaEquipmentComponent* GetEquipmentComponent() const;
};
