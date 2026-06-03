// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "Equipment/ElysiaEquipmentComponent.h"
#include "UI/ElysiaWidgetController.h"
#include "ElysiaLevelUpWidgetController.generated.h"

class UTexture2D;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEquipmentChoicesChangedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEquipmentInventoryChangedSignature);

USTRUCT(BlueprintType)
struct FElysiaEquipmentChoiceDisplayData
{
	GENERATED_BODY()

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

	UFUNCTION(BlueprintPure, Category = "Equipment")
	bool HasPendingEquipmentChoices() const { return CurrentEquipmentChoices.Num() > 0; }

protected:

	UPROPERTY(BlueprintReadOnly, Category = "Equipment")
	TArray<FElysiaEquipmentChoice> CurrentEquipmentChoices;

	UPROPERTY(BlueprintReadOnly, Category = "Equipment")
	TArray<FElysiaEquipmentChoiceDisplayData> CurrentEquipmentChoiceDisplays;

	UPROPERTY(BlueprintReadOnly, Category = "Equipment")
	TArray<FElysiaEquipmentEntry> OwnedEquipments;

	UPROPERTY(BlueprintAssignable)
	FEquipmentChoicesChangedSignature OnEquipmentChoicesChanged;

	UPROPERTY(BlueprintAssignable)
	FEquipmentInventoryChangedSignature OnEquipmentInventoryChanged;

	void HandleEquipmentChoicesChanged();
	void HandleOwnedEquipmentsChanged();
	void RefreshCurrentEquipmentChoiceDisplays();
	FElysiaEquipmentChoiceDisplayData MakeChoiceDisplayData(const FElysiaEquipmentChoice& Choice, int32 ChoiceIndex) const;
	const FElysiaEquipmentDefinition* FindEquipmentDefinition(FName EquipmentId) const;
};
