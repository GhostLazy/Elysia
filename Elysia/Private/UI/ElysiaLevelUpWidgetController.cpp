// Copyright GhostLazy


#include "UI/ElysiaLevelUpWidgetController.h"
#include "Equipment/ElysiaEquipmentComponent.h"

void UElysiaLevelUpWidgetController::BindCallbacksToDependencies()
{
	if (UElysiaEquipmentComponent* EquipmentComponent = GetEquipmentComponent())
	{
		// 升级窗口只关心装备候选与库存变化
		EquipmentComponent->OnPendingChoicesChanged.RemoveAll(this);
		EquipmentComponent->OnOwnedEquipmentsChanged.RemoveAll(this);
		EquipmentComponent->OnPendingChoicesChanged.AddUObject(this, &UElysiaLevelUpWidgetController::HandleEquipmentChoicesChanged);
		EquipmentComponent->OnOwnedEquipmentsChanged.AddUObject(this, &UElysiaLevelUpWidgetController::HandleOwnedEquipmentsChanged);

		CurrentEquipmentChoices = EquipmentComponent->GetPendingChoices();
		OwnedEquipments = EquipmentComponent->GetOwnedEquipments();
	}
}

void UElysiaLevelUpWidgetController::SelectEquipmentByIndex(int32 ChoiceIndex)
{
	if (UElysiaEquipmentComponent* EquipmentComponent = GetEquipmentComponent())
	{
		// 将 UI 选择转发给装备组件，由服务端权威处理
		EquipmentComponent->SelectChoiceByIndex(ChoiceIndex);
	}
}

void UElysiaLevelUpWidgetController::HandleEquipmentChoicesChanged()
{
	if (const UElysiaEquipmentComponent* EquipmentComponent = GetEquipmentComponent())
	{
		CurrentEquipmentChoices = EquipmentComponent->GetPendingChoices();
		OnEquipmentChoicesChanged.Broadcast();
	}
}

void UElysiaLevelUpWidgetController::HandleOwnedEquipmentsChanged()
{
	if (const UElysiaEquipmentComponent* EquipmentComponent = GetEquipmentComponent())
	{
		OwnedEquipments = EquipmentComponent->GetOwnedEquipments();
		OnEquipmentInventoryChanged.Broadcast();
	}
}
