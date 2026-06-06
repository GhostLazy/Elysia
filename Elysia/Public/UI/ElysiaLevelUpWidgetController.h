// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "Equipment/ElysiaEquipmentComponent.h"
#include "UI/ElysiaWidgetController.h"
#include "ElysiaLevelUpWidgetController.generated.h"

class UTexture2D;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEquipmentChoicesChangedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEquipmentInventoryChangedSignature);

UENUM(BlueprintType)
enum class EElysiaEquipmentChoiceDisplayType : uint8
{
	Weapon,
	AttributeBonus,
	Evolution,
	Recovery
};

UENUM(BlueprintType)
enum class EElysiaEquipmentChoiceCardStyle : uint8
{
	Weapon,
	AttributeBonus,
	Evolution
};

// 升级选择卡片的只读展示数据。
// C++ 在这里完成候选项类型、卡面样式与基础展示内容整理，Lua/UMG 只负责按字段渲染。
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

	// 用于 UI 区分普通装备、属性加成、进化卡、恢复卡。
	UPROPERTY(BlueprintReadOnly, Category = "Equipment|Choice")
	EElysiaEquipmentChoiceDisplayType DisplayType = EElysiaEquipmentChoiceDisplayType::AttributeBonus;

	UPROPERTY(BlueprintReadOnly, Category = "Equipment|Choice")
	FText TypeDisplayName;

	// 用于 UI 从三种背景中选择卡片样式。
	UPROPERTY(BlueprintReadOnly, Category = "Equipment|Choice")
	EElysiaEquipmentChoiceCardStyle CardStyle = EElysiaEquipmentChoiceCardStyle::AttributeBonus;

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

	UPROPERTY(BlueprintReadOnly, Category = "Equipment|Choice")
	bool bAlreadyOwned = false;

	UPROPERTY(BlueprintReadOnly, Category = "Equipment|Choice")
	bool bShowLevelText = true;

	// 装备系统已经整理好的进化需求；UI 只负责展示。
	UPROPERTY(BlueprintReadOnly, Category = "Equipment|Choice")
	bool bHasEvolutionRequirement = false;

	UPROPERTY(BlueprintReadOnly, Category = "Equipment|Choice")
	FText EvolutionRequiredDisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "Equipment|Choice")
	TObjectPtr<UTexture2D> EvolutionRequiredIcon = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Equipment|Choice")
	int32 EvolutionRequiredCurrentLevel = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Equipment|Choice")
	bool bEvolutionRequirementMet = false;

	UPROPERTY(BlueprintReadOnly, Category = "Equipment|Choice")
	float RecoveryHealth = 0.f;
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
};
