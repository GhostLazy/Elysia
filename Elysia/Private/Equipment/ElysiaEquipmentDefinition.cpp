// Copyright GhostLazy

#pragma once

#include "Equipment/ElysiaEquipmentDefinition.h"

const FElysiaEquipmentDefinition* UElysiaEquipmentPoolDataAsset::FindEquipmentById(const FName EquipmentId) const
{
	return Equipments.FindByPredicate([EquipmentId](const FElysiaEquipmentDefinition& Definition)
	{
		return Definition.EquipmentId == EquipmentId;
	});
}
