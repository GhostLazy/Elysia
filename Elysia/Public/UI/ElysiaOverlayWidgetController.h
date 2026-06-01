// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ElysiaAbilitySystemComponent.h"
#include "UI/ElysiaWidgetController.h"
#include "ElysiaOverlayWidgetController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FProgressBarPercentChangedSignature, float, Percent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLevelTextChangedSignature, int32, NewLevel, bool, bLevelUp);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FScoreTextChangedSignature, int32, NewScore);

UCLASS(Blueprintable)
class ELYSIA_API UElysiaOverlayWidgetController : public UElysiaWidgetController
{
	GENERATED_BODY()

public:

	virtual void BindCallbacksToDependencies() override;

	UFUNCTION(BlueprintPure, Category = "Cooldown")
	FElysiaCooldownInfo GetSkillCooldownInfo() const;

protected:

	UFUNCTION()
	void HandleXPChanged(int32 NewXP) const;

	UFUNCTION()
	void HandleLevelChanged(int32 NewLevel, bool bLevelUp) const;
	
	UFUNCTION()
	void HandleTotalScoreChanged(int32 NewTotalScore) const;
	
	UFUNCTION()
	void HandleGameProgressPercentChanged(int32 NewTotalSecond);
	
	// 常驻 HUD：经验条百分比
	UPROPERTY(BlueprintAssignable)
	FProgressBarPercentChangedSignature OnXPBarPercentChanged;
	
	// 常驻 HUD：等级文本变化
	UPROPERTY(BlueprintAssignable)
	FLevelTextChangedSignature OnLevelTextChanged;
	
	// 常驻 HUD：分数文本变化
	UPROPERTY(BlueprintAssignable)
	FScoreTextChangedSignature OnScoreTextChanged;
	
	// 常驻 HUD：游戏进度条百分比
	UPROPERTY(BlueprintAssignable)
	FProgressBarPercentChangedSignature OnGameProgressPercentChanged;
	
private:
	
	int32 NormalPhaseTotalDuration = 0;
	
};
