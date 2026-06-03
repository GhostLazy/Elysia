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
	DisplayData.bIsRecoveryChoice = Choice.bIsRecoveryChoice;
	DisplayData.RecoveryHealth = Choice.RecoveryHealth;

	if (Choice.bIsRecoveryChoice)
	{
		DisplayData.CurrentLevel = 0;
		DisplayData.NextLevel = 0;
		DisplayData.MaxLevel = 0;
		DisplayData.StarSlotCount = 0;
		DisplayData.CurrentStarCount = 0;
		DisplayData.PreviewStarCount = 0;
		DisplayData.SelectedStarCount = 0;
		DisplayData.EmptyStarCount = 0;
		return DisplayData;
	}

	DisplayData.EquipmentId = Choice.Equipment.EquipmentId;
	DisplayData.EquipmentType = Choice.EquipmentType;
	DisplayData.DisplayName = Choice.Equipment.DisplayName;
	DisplayData.Description = Choice.Equipment.Description;
	DisplayData.Icon = Choice.Equipment.Icon;
	DisplayData.MaxLevel = FMath::Max(1, Choice.MaxLevel);
	DisplayData.CurrentLevel = FMath::Clamp(Choice.CurrentLevel, 0, DisplayData.MaxLevel);
	DisplayData.NextLevel = FMath::Clamp(Choice.NextLevel, 1, DisplayData.MaxLevel);
	DisplayData.StarSlotCount = DisplayData.MaxLevel;
	DisplayData.CurrentStarCount = FMath::Clamp(DisplayData.CurrentLevel, 0, DisplayData.StarSlotCount);
	DisplayData.SelectedStarCount = FMath::Clamp(DisplayData.NextLevel, 0, DisplayData.StarSlotCount);
	DisplayData.PreviewStarCount = FMath::Max(0, DisplayData.SelectedStarCount - DisplayData.CurrentStarCount);
	DisplayData.EmptyStarCount = FMath::Max(0, DisplayData.StarSlotCount - DisplayData.SelectedStarCount);
	DisplayData.bAlreadyOwned = Choice.bAlreadyOwned;
	DisplayData.bAtMaxLevelAfterSelection = DisplayData.NextLevel >= DisplayData.MaxLevel;
	DisplayData.bWillEvolve = Choice.bWillEvolve;

	DisplayData.EvolutionRequiredEquipmentId = Choice.Equipment.RequiredPassiveEquipmentId;
	DisplayData.bHasEvolutionCombo = Choice.EquipmentType == EElysiaEquipmentType::Weapon
		&& !DisplayData.EvolutionRequiredEquipmentId.IsNone();

	if (DisplayData.bHasEvolutionCombo)
	{
		if (const FElysiaEquipmentDefinition* RequiredDefinition = FindEquipmentDefinition(DisplayData.EvolutionRequiredEquipmentId))
		{
			DisplayData.EvolutionRequiredDisplayName = RequiredDefinition->DisplayName;
			DisplayData.EvolutionRequiredIcon = RequiredDefinition->Icon;
		}

		if (const UElysiaEquipmentComponent* EquipmentComponent = GetEquipmentComponent())
		{
			DisplayData.EvolutionRequiredCurrentLevel = EquipmentComponent->GetEquipmentLevelById(DisplayData.EvolutionRequiredEquipmentId);
		}

		DisplayData.bEvolutionRequirementOwned = DisplayData.EvolutionRequiredCurrentLevel > 0;
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
