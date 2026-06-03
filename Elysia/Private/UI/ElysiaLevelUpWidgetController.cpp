// Copyright GhostLazy


#include "UI/ElysiaLevelUpWidgetController.h"
#include "Equipment/ElysiaEquipmentComponent.h"

namespace
{
	FText GetChoiceTypeDisplayName(EElysiaEquipmentChoiceDisplayType DisplayType)
	{
		switch (DisplayType)
		{
		case EElysiaEquipmentChoiceDisplayType::Weapon:
			return NSLOCTEXT("ElysiaEquipmentChoice", "Weapon", "装备");
		case EElysiaEquipmentChoiceDisplayType::AttributeBonus:
			return NSLOCTEXT("ElysiaEquipmentChoice", "AttributeBonus", "属性加成");
		case EElysiaEquipmentChoiceDisplayType::Evolution:
			return NSLOCTEXT("ElysiaEquipmentChoice", "Evolution", "进化");
		case EElysiaEquipmentChoiceDisplayType::Recovery:
			return NSLOCTEXT("ElysiaEquipmentChoice", "Recovery", "恢复");
		default:
			return FText::GetEmpty();
		}
	}
}

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
		RefreshCurrentEquipmentChoiceDisplays();
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
		RefreshCurrentEquipmentChoiceDisplays();
		OnEquipmentChoicesChanged.Broadcast();
	}
}

void UElysiaLevelUpWidgetController::HandleOwnedEquipmentsChanged()
{
	if (const UElysiaEquipmentComponent* EquipmentComponent = GetEquipmentComponent())
	{
		OwnedEquipments = EquipmentComponent->GetOwnedEquipments();
		RefreshCurrentEquipmentChoiceDisplays();
		OnEquipmentInventoryChanged.Broadcast();
	}
}

void UElysiaLevelUpWidgetController::RefreshCurrentEquipmentChoiceDisplays()
{
	CurrentEquipmentChoiceDisplays.Reset(CurrentEquipmentChoices.Num());

	for (int32 ChoiceIndex = 0; ChoiceIndex < CurrentEquipmentChoices.Num(); ++ChoiceIndex)
	{
		CurrentEquipmentChoiceDisplays.Add(MakeChoiceDisplayData(CurrentEquipmentChoices[ChoiceIndex], ChoiceIndex));
	}
}

FElysiaEquipmentChoiceDisplayData UElysiaLevelUpWidgetController::MakeChoiceDisplayData(
	const FElysiaEquipmentChoice& Choice,
	int32 ChoiceIndex) const
{
	FElysiaEquipmentChoiceDisplayData DisplayData;
	DisplayData.ChoiceIndex = ChoiceIndex;
	DisplayData.RecoveryHealth = Choice.RecoveryHealth;

	if (Choice.bIsRecoveryChoice)
	{
		DisplayData.DisplayType = EElysiaEquipmentChoiceDisplayType::Recovery;
		DisplayData.TypeDisplayName = GetChoiceTypeDisplayName(DisplayData.DisplayType);
		DisplayData.CardStyle = EElysiaEquipmentChoiceCardStyle::AttributeBonus;
		DisplayData.CurrentLevel = 0;
		DisplayData.NextLevel = 0;
		DisplayData.MaxLevel = 0;
		DisplayData.bShowLevelText = false;
		return DisplayData;
	}

	DisplayData.EquipmentId = Choice.Equipment.EquipmentId;
	DisplayData.EquipmentType = Choice.EquipmentType;
	const bool bDisplaysAsEvolution = Choice.bWillEvolve;
	const bool bDisplaysAsWeapon = Choice.EquipmentType == EElysiaEquipmentType::Weapon;
	DisplayData.DisplayType = bDisplaysAsEvolution
		? EElysiaEquipmentChoiceDisplayType::Evolution
		: (bDisplaysAsWeapon
			? EElysiaEquipmentChoiceDisplayType::Weapon
			: EElysiaEquipmentChoiceDisplayType::AttributeBonus);
	DisplayData.TypeDisplayName = GetChoiceTypeDisplayName(DisplayData.DisplayType);
	DisplayData.CardStyle = bDisplaysAsEvolution
		? EElysiaEquipmentChoiceCardStyle::Evolution
		: (bDisplaysAsWeapon
			? EElysiaEquipmentChoiceCardStyle::Weapon
			: EElysiaEquipmentChoiceCardStyle::AttributeBonus);
	DisplayData.DisplayName = bDisplaysAsEvolution && !Choice.Equipment.EvolvedDisplayName.IsEmpty()
		? Choice.Equipment.EvolvedDisplayName
		: Choice.Equipment.DisplayName;
	DisplayData.Description = bDisplaysAsEvolution && !Choice.Equipment.EvolvedDescription.IsEmpty()
		? Choice.Equipment.EvolvedDescription
		: Choice.Equipment.Description;
	DisplayData.Icon = bDisplaysAsEvolution && Choice.Equipment.EvolvedIcon
		? Choice.Equipment.EvolvedIcon
		: Choice.Equipment.Icon;
	DisplayData.MaxLevel = FMath::Max(1, Choice.MaxLevel);
	DisplayData.CurrentLevel = FMath::Clamp(Choice.CurrentLevel, 0, DisplayData.MaxLevel);
	DisplayData.NextLevel = FMath::Clamp(Choice.NextLevel, 1, DisplayData.MaxLevel);
	DisplayData.bAlreadyOwned = Choice.bAlreadyOwned;
	DisplayData.bShowLevelText = !bDisplaysAsEvolution;
	const FName EvolutionRequiredEquipmentId = Choice.Equipment.RequiredPassiveEquipmentId;
	DisplayData.bHasEvolutionCombo = Choice.EquipmentType == EElysiaEquipmentType::Weapon
		&& !EvolutionRequiredEquipmentId.IsNone()
		&& !bDisplaysAsEvolution;

	if (DisplayData.bHasEvolutionCombo)
	{
		if (const FElysiaEquipmentDefinition* RequiredDefinition = FindEquipmentDefinition(EvolutionRequiredEquipmentId))
		{
			DisplayData.EvolutionRequiredDisplayName = RequiredDefinition->DisplayName;
		}

		if (const UElysiaEquipmentComponent* EquipmentComponent = GetEquipmentComponent())
		{
			DisplayData.bEvolutionRequirementOwned = EquipmentComponent->GetEquipmentLevelById(EvolutionRequiredEquipmentId) > 0;
		}
	}

	return DisplayData;
}

const FElysiaEquipmentDefinition* UElysiaLevelUpWidgetController::FindEquipmentDefinition(FName EquipmentId) const
{
	if (EquipmentId.IsNone())
	{
		return nullptr;
	}

	if (const UElysiaEquipmentComponent* EquipmentComponent = GetEquipmentComponent())
	{
		if (const UElysiaEquipmentPoolDataAsset* EquipmentPool = EquipmentComponent->GetEquipmentPool())
		{
			if (const FElysiaEquipmentDefinition* EquipmentDefinition = EquipmentPool->FindEquipmentById(EquipmentId))
			{
				return EquipmentDefinition;
			}
		}
	}

	for (const FElysiaEquipmentEntry& OwnedEquipment : OwnedEquipments)
	{
		if (OwnedEquipment.Equipment.EquipmentId == EquipmentId)
		{
			return &OwnedEquipment.Equipment;
		}
	}

	return nullptr;
}
