// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "Equipment/ElysiaEquipmentComponent.h"
#include "UI/ElysiaWidgetController.h"
#include "ElysiaLevelUpWidgetController.generated.h"

class UTexture2D;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEquipmentChoicesChangedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEquipmentInventoryChangedSignature);

// 升级选择卡片的只读展示数据。
// C++ 在这里完成等级、星级与进化组合的整理，Lua/UMG 只负责按字段渲染。
USTRUCT(BlueprintType)
struct FElysiaEquipmentChoiceDisplayData
{
	GENERATED_BODY()

	// 对应 PendingChoices 中的下标，点击卡片时传回 SelectEquipmentByIndex。
	UPROPERTY(BlueprintReadOnly, Category = "Equipment|Choice")
	int32 ChoiceIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "Equipment|Choice")
	FName EquipmentId;

	UPROPERTY(BlueprintReadOnly, Category = "Equipment|Choice")
	EElysiaEquipmentType EquipmentType = EElysiaEquipmentType::Passive;

	UPROPERTY(BlueprintReadOnly, Category = "Equipment|Choice")
	FText DisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "Equipment|Choice")
	FText Description;

	UPROPERTY(BlueprintReadOnly, Category = "Equipment|Choice")
	TObjectPtr<UTexture2D> Icon = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Equipment|Choice")
	int32 CurrentLevel = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Equipment|Choice")
	int32 NextLevel = 1;

	UPROPERTY(BlueprintReadOnly, Category = "Equipment|Choice")
	int32 MaxLevel = 1;

	// 星级显示拆分：已拥有星、本次选择新增星、空星。
	UPROPERTY(BlueprintReadOnly, Category = "Equipment|Choice")
	int32 StarSlotCount = 1;

	UPROPERTY(BlueprintReadOnly, Category = "Equipment|Choice")
	int32 CurrentStarCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Equipment|Choice")
	int32 PreviewStarCount = 1;

	UPROPERTY(BlueprintReadOnly, Category = "Equipment|Choice")
	int32 SelectedStarCount = 1;

	UPROPERTY(BlueprintReadOnly, Category = "Equipment|Choice")
	int32 EmptyStarCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Equipment|Choice")
	bool bAlreadyOwned = false;

	UPROPERTY(BlueprintReadOnly, Category = "Equipment|Choice")
	bool bAtMaxLevelAfterSelection = false;

	UPROPERTY(BlueprintReadOnly, Category = "Equipment|Choice")
	bool bWillEvolve = false;

	UPROPERTY(BlueprintReadOnly, Category = "Equipment|Choice")
	bool bIsRecoveryChoice = false;

	UPROPERTY(BlueprintReadOnly, Category = "Equipment|Choice")
	float RecoveryHealth = 0.f;

	// 武器进化组合展示所需的被动装备信息。
	UPROPERTY(BlueprintReadOnly, Category = "Equipment|Evolution")
	bool bHasEvolutionCombo = false;

	UPROPERTY(BlueprintReadOnly, Category = "Equipment|Evolution")
	FName EvolutionRequiredEquipmentId;

	UPROPERTY(BlueprintReadOnly, Category = "Equipment|Evolution")
	FText EvolutionRequiredDisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "Equipment|Evolution")
	TObjectPtr<UTexture2D> EvolutionRequiredIcon = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Equipment|Evolution")
	int32 EvolutionRequiredCurrentLevel = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Equipment|Evolution")
	bool bEvolutionRequirementOwned = false;
};

UCLASS(Blueprintable)
class ELYSIA_API UElysiaLevelUpWidgetController : public UElysiaWidgetController
{
	GENERATED_BODY()

public:

	virtual void BindCallbacksToDependencies() override;

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void SelectEquipmentByIndex(int32 ChoiceIndex);

	// 是否存在待选择的升级候选项。
	UFUNCTION(BlueprintPure, Category = "Equipment")
	bool HasPendingEquipmentChoices() const { return CurrentEquipmentChoices.Num() > 0; }

protected:

	// 当前升级弹窗中展示的三选一候选项，保留原始数据以兼容现有蓝图/Lua。
	UPROPERTY(BlueprintReadOnly, Category = "Equipment")
	TArray<FElysiaEquipmentChoice> CurrentEquipmentChoices;

	// 当前升级弹窗中展示的三选一候选项，已整理成 UI 可直接使用的展示数据。
	UPROPERTY(BlueprintReadOnly, Category = "Equipment")
	TArray<FElysiaEquipmentChoiceDisplayData> CurrentEquipmentChoiceDisplays;

	// 当前已拥有的武器与被动装备。
	UPROPERTY(BlueprintReadOnly, Category = "Equipment")
	TArray<FElysiaEquipmentEntry> OwnedEquipments;

	// 升级候选变化时通知升级窗口刷新。
	UPROPERTY(BlueprintAssignable)
	FEquipmentChoicesChangedSignature OnEquipmentChoicesChanged;

	// 装备库存变化时通知升级窗口刷新。
	UPROPERTY(BlueprintAssignable)
	FEquipmentInventoryChangedSignature OnEquipmentInventoryChanged;

	void HandleEquipmentChoicesChanged();
	void HandleOwnedEquipmentsChanged();
	void RefreshCurrentEquipmentChoiceDisplays();
	FElysiaEquipmentChoiceDisplayData MakeChoiceDisplayData(const FElysiaEquipmentChoice& Choice, int32 ChoiceIndex) const;
	const FElysiaEquipmentDefinition* FindEquipmentDefinition(FName EquipmentId) const;
};
