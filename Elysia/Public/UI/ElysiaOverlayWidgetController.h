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

	UPROPERTY(BlueprintAssignable)
	FXPBarPercentChangedSignature OnXPBarPercentChanged;

	UPROPERTY(BlueprintAssignable)
	FLevelTextChangeSignature OnLevelTextChange;
};
