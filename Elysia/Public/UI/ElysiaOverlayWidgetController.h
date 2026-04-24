// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "UI/ElysiaWidgetController.h"
#include "ElysiaOverlayWidgetController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FXPBarPercentChangedSignature, float, Percent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLevelTextChangeSignature, int32, NewLevel, bool, bLevelUp);

UCLASS(Blueprintable)
class ELYSIA_API UElysiaOverlayWidgetController : public UElysiaWidgetController
{
	GENERATED_BODY()

public:

	virtual void BindCallbacksToDependencies() override;

protected:

	UFUNCTION()
	void OnXPChanged(int32 NewXP) const;

	UFUNCTION()
	void HandleLevelChanged(int32 NewLevel, bool bLevelUp);

	// 常驻 HUD：经验条百分比
	UPROPERTY(BlueprintAssignable)
	FXPBarPercentChangedSignature OnXPBarPercentChanged;
	
	// 常驻 HUD：等级文本变化
	UPROPERTY(BlueprintAssignable)
	FLevelTextChangeSignature OnLevelTextChange;
};
