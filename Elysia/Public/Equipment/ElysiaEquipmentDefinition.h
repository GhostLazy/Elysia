// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ElysiaEquipmentDefinition.generated.h"

class UGameplayAbility;
class UGameplayEffect;
class UTexture2D;

UENUM(BlueprintType)
enum class EElysiaEquipmentType : uint8
{
	Weapon,
	Passive
};

USTRUCT(BlueprintType)
struct FElysiaEquipmentDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
	FName EquipmentId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment", meta = (MultiLine = true))
	FText Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
	TObjectPtr<UTexture2D> Icon = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
	EElysiaEquipmentType EquipmentType = EElysiaEquipmentType::Passive;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
	TSubclassOf<UGameplayEffect> GrantedEffectClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
	TSubclassOf<UGameplayAbility> GrantedAbilityClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment", meta = (ClampMin = "1"))
	int32 MaxLevel = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment|Evolution")
	FName RequiredPassiveEquipmentId;
};

UCLASS(BlueprintType)
class ELYSIA_API UElysiaEquipmentPoolDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment")
	TArray<FElysiaEquipmentDefinition> Equipments;
};
