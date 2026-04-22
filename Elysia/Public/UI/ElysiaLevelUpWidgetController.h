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

	UFUNCTION(BlueprintPure, Category = "Equipment")
	bool HasPendingEquipmentChoices() const { return CurrentEquipmentChoices.Num() > 0; }

protected:

	UPROPERTY(BlueprintReadOnly, Category = "Equipment")
	TArray<FElysiaEquipmentChoice> CurrentEquipmentChoices;

	UPROPERTY(BlueprintReadOnly, Category = "Equipment")
	TArray<FElysiaEquipmentEntry> OwnedEquipments;

	UPROPERTY(BlueprintAssignable)
	FEquipmentChoicesChangedSignature OnEquipmentChoicesChanged;

	UPROPERTY(BlueprintAssignable)
	FEquipmentInventoryChangedSignature OnEquipmentInventoryChanged;

	void HandleEquipmentChoicesChanged();
	void HandleOwnedEquipmentsChanged();
};
