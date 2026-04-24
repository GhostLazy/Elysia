// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayEffectTypes.h"
#include "Equipment/ElysiaEquipmentDefinition.h"
#include "ElysiaEquipmentComponent.generated.h"

class UAbilitySystemComponent;
class UGameplayAbility;
class UElysiaEquipmentPoolDataAsset;

USTRUCT(BlueprintType)
struct FElysiaEquipmentEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Equipment")
	FElysiaEquipmentDefinition Equipment;

	UPROPERTY(BlueprintReadOnly, Category = "Equipment")
	int32 Level = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Equipment")
	bool bEvolved = false;
};

USTRUCT(BlueprintType)
struct FElysiaEquipmentChoice
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Equipment")
	FElysiaEquipmentDefinition Equipment;

	UPROPERTY(BlueprintReadOnly, Category = "Equipment")
	EElysiaEquipmentType EquipmentType = EElysiaEquipmentType::Passive;

	UPROPERTY(BlueprintReadOnly, Category = "Equipment")
	int32 CurrentLevel = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Equipment")
	int32 NextLevel = 1;

	UPROPERTY(BlueprintReadOnly, Category = "Equipment")
	int32 MaxLevel = 1;

	UPROPERTY(BlueprintReadOnly, Category = "Equipment")
	bool bAlreadyOwned = false;

	UPROPERTY(BlueprintReadOnly, Category = "Equipment")
	bool bWillEvolve = false;
};

DECLARE_MULTICAST_DELEGATE(FOnEquipmentCollectionChanged);
DECLARE_MULTICAST_DELEGATE(FOnEquipmentChoicesChanged);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ELYSIA_API UElysiaEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UElysiaEquipmentComponent();
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	const TArray<FElysiaEquipmentEntry>& GetOwnedEquipments() const { return OwnedEquipments; }
	const TArray<FElysiaEquipmentChoice>& GetPendingChoices() const { return PendingChoices; }
	bool HasPendingChoices() const { return PendingChoices.Num() > 0; }
	int32 GetEquipmentLevelById(FName EquipmentId) const;
	bool IsEquipmentEvolvedById(FName EquipmentId) const;
	int32 GetEquipmentLevelByAbilityClass(TSubclassOf<UGameplayAbility> AbilityClass) const;
	bool IsEquipmentEvolvedByAbilityClass(TSubclassOf<UGameplayAbility> AbilityClass) const;

	// 接口：每当角色升1级，执行一次装备选择
	void QueueLevelUpSelections(int32 NumSelections);

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void SelectChoiceByIndex(int32 ChoiceIndex);

	FOnEquipmentCollectionChanged OnOwnedEquipmentsChanged;
	FOnEquipmentChoicesChanged OnPendingChoicesChanged;

protected:

	UPROPERTY(EditDefaultsOnly, Category = "Equipment")
	TObjectPtr<UElysiaEquipmentPoolDataAsset> EquipmentPool;

	UPROPERTY(EditDefaultsOnly, Category = "Equipment", meta = (ClampMin = "1"))
	int32 ChoiceCountPerLevel = 3;

private:

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_OwnedEquipments, Category = "Equipment")
	TArray<FElysiaEquipmentEntry> OwnedEquipments;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_PendingChoices, Category = "Equipment")
	TArray<FElysiaEquipmentChoice> PendingChoices;

	UPROPERTY(VisibleAnywhere, Replicated, Category = "Equipment")
	int32 PendingSelectionCount = 0;

	UFUNCTION()
	void OnRep_OwnedEquipments();

	UFUNCTION()
	void OnRep_PendingChoices();

	UFUNCTION(Server, Reliable)
	void ServerSelectChoiceByIndex(int32 ChoiceIndex);

	void GrantEquipment(const FElysiaEquipmentDefinition& EquipmentDefinition);
	void RollNextChoices();
	void ApplyEquipmentEffects(const FElysiaEquipmentEntry& EquipmentEntry);
	void EnsureWeaponAbilityGranted(const FElysiaEquipmentDefinition& EquipmentDefinition);
	void UpdateWeaponEvolutionStates();
	bool CanEvolve(const FElysiaEquipmentEntry& EquipmentEntry) const;
	FElysiaEquipmentEntry* FindOwnedEquipment(FName EquipmentId);
	const FElysiaEquipmentEntry* FindOwnedEquipmentById(FName EquipmentId) const;
	const FElysiaEquipmentEntry* FindOwnedEquipmentByAbilityClass(TSubclassOf<UGameplayAbility> AbilityClass) const;
	int32 FindOwnedEquipmentIndex(FName EquipmentId) const;
	bool CanOfferEquipment(const FElysiaEquipmentDefinition& EquipmentDefinition) const;
	UAbilitySystemComponent* GetAbilitySystemComponent() const;

	// 用于被动升级时，执行GE更新（删除&新增）操作
	TMap<FName, FActiveGameplayEffectHandle> ActiveEffectHandles;
};
