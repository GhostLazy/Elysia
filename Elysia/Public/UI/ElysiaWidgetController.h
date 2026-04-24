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

	// 注入 WidgetController 需要使用的公共上下文
	virtual void SetWidgetControllerParams(APlayerState* PS, APlayerController* PC, UAbilitySystemComponent* ASC, UAttributeSet* AS);
	// 由子类重写，绑定各自需要监听的依赖
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

	// 常用辅助：从 PlayerState 上取装备组件
	UElysiaEquipmentComponent* GetEquipmentComponent() const;
};
