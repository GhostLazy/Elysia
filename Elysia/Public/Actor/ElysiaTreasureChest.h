// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Actor.h"
#include "Interface/CombatInterface.h"
#include "ElysiaTreasureChest.generated.h"

class AElysiaBombPickup;
class AElysiaHealthPickup;
class AElysiaMagnetPickup;
class AElysiaXPBall;
class UElysiaAbilitySystemComponent;
class UAbilitySystemComponent;
class UAttributeSet;
class USphereComponent;
class UStaticMeshComponent;
struct FOnAttributeChangeData;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnTreasureChestOpenedSignature, AElysiaTreasureChest*);

UENUM(BlueprintType)
enum class EElysiaTreasureChestRewardType : uint8
{
	Magnet,
	Bomb,
	Health,
	XPBall
};

UCLASS()
class ELYSIA_API AElysiaTreasureChest : public AActor, public IAbilitySystemInterface, public ICombatInterface
{
	GENERATED_BODY()

public:

	AElysiaTreasureChest();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	virtual void Die() override;
	virtual bool IsDead() const override { return bOpened; }
	virtual bool IsPlayer() const override { return false; }
	virtual bool IsEnemy() const override { return false; }
	virtual bool HasTag(const FName Tag) const override { return ActorHasTag(Tag); }

	UFUNCTION(BlueprintCallable, Category = "Treasure Chest")
	void OpenChest(AActor* Opener);

	bool IsOpened() const { return bOpened; }

	FOnTreasureChestOpenedSignature OnTreasureChestOpened;

protected:

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UElysiaAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Treasure Chest|Attributes", meta = (ClampMin = "1.0"))
	float ChestHealth = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Treasure Chest|Rewards")
	TSubclassOf<AElysiaMagnetPickup> MagnetPickupClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Treasure Chest|Rewards")
	TSubclassOf<AElysiaBombPickup> BombPickupClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Treasure Chest|Rewards")
	TSubclassOf<AElysiaHealthPickup> HealthPickupClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Treasure Chest|Rewards")
	TSubclassOf<AElysiaXPBall> XPBallClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Treasure Chest|Rewards", meta = (ClampMin = "0.0"))
	float MagnetRewardWeight = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Treasure Chest|Rewards", meta = (ClampMin = "0.0"))
	float BombRewardWeight = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Treasure Chest|Rewards", meta = (ClampMin = "0.0"))
	float HealthRewardWeight = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Treasure Chest|Rewards", meta = (ClampMin = "0.0"))
	float XPBallRewardWeight = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Treasure Chest|Rewards", meta = (ClampMin = "0"))
	int32 XPBallRewardValue = 20;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Treasure Chest|Rewards", meta = (ClampMin = "1"))
	int32 XPBallRewardLevel = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Treasure Chest|Rewards", meta = (ClampMin = "0.0"))
	float RewardSpawnRadius = 80.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Treasure Chest|Rewards")
	float RewardSpawnHeightOffset = 20.f;

private:

	void InitializeAttributes() const;
	void SpawnRandomReward(AActor* Opener);
	EElysiaTreasureChestRewardType ChooseRewardType() const;
	float GetRewardWeight(EElysiaTreasureChestRewardType RewardType) const;
	bool HasRewardClass(EElysiaTreasureChestRewardType RewardType) const;
	FVector GetRewardSpawnLocation() const;
	void HandleHealthChanged(const FOnAttributeChangeData& Data);

	FDelegateHandle HealthChangedHandle;
	bool bOpened = false;
};
