// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "Equipment/ElysiaEquipmentComponent.h"
#include "UI/ElysiaWidgetController.h"
#include "ElysiaLevelUpWidgetController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEquipmentChoicesChangedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEquipmentInventoryChangedSignature);

UCLASS(Blueprintable)
class ELYSIA_API UElysiaLevelUpWidgetController : public UElysiaWidgetController
{
	GENERATED_BODY()

public:

	virtual void BindCallbacksToDependencies() override;

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void SelectEquipmentByIndex(int32 ChoiceIndex);

	// 是否存在待选择的升级候选项
	UFUNCTION(BlueprintPure, Category = "Equipment")
	bool HasPendingEquipmentChoices() const { return CurrentEquipmentChoices.Num() > 0; }

protected:

	// 当前升级弹窗中展示的三选一候选项
	UPROPERTY(BlueprintReadOnly, Category = "Equipment")
	TArray<FElysiaEquipmentChoice> CurrentEquipmentChoices;

	// 当前已拥有的武器与被动
	UPROPERTY(BlueprintReadOnly, Category = "Equipment")
	TArray<FElysiaEquipmentEntry> OwnedEquipments;

	// 升级候选变化时通知升级窗口刷新
	UPROPERTY(BlueprintAssignable)
	FEquipmentChoicesChangedSignature OnEquipmentChoicesChanged;

	// 装备库存变化时通知升级窗口刷新
	UPROPERTY(BlueprintAssignable)
	FEquipmentInventoryChangedSignature OnEquipmentInventoryChanged;

	void HandleEquipmentChoicesChanged();
	void HandleOwnedEquipmentsChanged();
};
