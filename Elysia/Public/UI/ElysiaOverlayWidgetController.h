// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ElysiaAbilitySystemComponent.h"
#include "UI/ElysiaWidgetController.h"
#include "ElysiaOverlayWidgetController.generated.h"

class AElysiaEnemy;
struct FOnAttributeChangeData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FProgressBarPercentChangedSignature, float, Percent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLevelTextChangedSignature, int32, NewLevel, bool, bLevelUp);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FScoreTextChangedSignature, int32, NewScore);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBossHealthVisibilityChangedSignature, bool, bVisible);

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

	void HandleCurrentBossChanged(AElysiaEnemy* NewBoss);
	void HandleBossHealthChanged(const FOnAttributeChangeData& Data) const;
	void HandleBossMaxHealthChanged(const FOnAttributeChangeData& Data) const;
	void BroadcastBossHealthPercent() const;
	void BindToCurrentBoss(AElysiaEnemy* NewBoss);
	void UnbindFromCurrentBoss();

	UPROPERTY(BlueprintAssignable, Category = "Overlay")
	FProgressBarPercentChangedSignature OnXPBarPercentChanged;

	UPROPERTY(BlueprintAssignable, Category = "Overlay")
	FLevelTextChangedSignature OnLevelTextChanged;

	UPROPERTY(BlueprintAssignable, Category = "Overlay")
	FScoreTextChangedSignature OnScoreTextChanged;

	UPROPERTY(BlueprintAssignable, Category = "Overlay")
	FProgressBarPercentChangedSignature OnGameProgressPercentChanged;

	UPROPERTY(BlueprintAssignable, Category = "Overlay")
	FProgressBarPercentChangedSignature OnBossHealthPercentChanged;

	UPROPERTY(BlueprintAssignable, Category = "Overlay")
	FBossHealthVisibilityChangedSignature OnBossHealthVisibilityChanged;

private:

	TWeakObjectPtr<AElysiaEnemy> CurrentBoss;
	FDelegateHandle BossHealthChangedHandle;
	FDelegateHandle BossMaxHealthChangedHandle;
};
