// Copyright GhostLazy


#include "UI/ElysiaLevelUpWidgetController.h"
#include "Equipment/ElysiaEquipmentComponent.h"

void UElysiaLevelUpWidgetController::BindCallbacksToDependencies()
{
	if (UElysiaEquipmentComponent* EquipmentComponent = GetEquipmentComponent())
	{
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
