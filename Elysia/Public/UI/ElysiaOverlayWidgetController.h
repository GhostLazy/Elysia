// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ElysiaAbilitySystemComponent.h"
#include "UI/ElysiaWidgetController.h"
#include "ElysiaOverlayWidgetController.generated.h"

class AElysiaEnemy;
class AElysiaGameState;
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
	virtual void BeginDestroy() override;

	// Widget 完成委托绑定后调用，补发当前 UI 快照，避免客户端错过早于订阅发生的复制通知。
	UFUNCTION(BlueprintCallable, Category = "Overlay")
	void BroadcastInitialValues();

	// 周期性读取客户端已经复制到本地的状态，处理 GameState/Boss 晚于 HUD 到达的情况。
	UFUNCTION(BlueprintCallable, Category = "Overlay")
	void RefreshReplicatedState();

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
	void ResolveAndBindGameState();
	void BindToGameState(AElysiaGameState* NewGameState);
	void UnbindFromGameState();
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
	TWeakObjectPtr<AElysiaGameState> BoundGameState;
	FDelegateHandle BossHealthChangedHandle;
	FDelegateHandle BossMaxHealthChangedHandle;
	FTimerHandle ReplicatedStateRefreshTimerHandle;
};
