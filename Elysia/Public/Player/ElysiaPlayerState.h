// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "ScalableFloat.h"
#include "GameFramework/PlayerState.h"
#include "ElysiaPlayerState.generated.h"

class UAttributeSet;
class UAbilitySystemComponent;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnLevelValueChange, int32 /*StatValue*/, bool /*bLevelUp*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnXPValueChange, int32 /*StatValue*/);

/**
 * 
 */
UCLASS()
class ELYSIA_API AElysiaPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	
	AElysiaPlayerState();
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }
	UAttributeSet* GetAttributeSet() const { return AttributeSet; }
	
	FOnLevelValueChange OnLevelChanged;
	FOnXPValueChange OnXPChanged;
	
	FORCEINLINE int32 GetPlayerLevel() const { return Level; }
	FORCEINLINE int32 GetMaxLevel() const { return MaxLevel; }
	FORCEINLINE int32 GetXP() const { return XP; }
	FORCEINLINE FScalableFloat GetLevelUpRequirement() const { return LevelUpRequirement; }
	
	void AddToLevel(int32 InLevel);
	void AddToXP(int32 InXP);
	
protected:
	
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;
	
	UPROPERTY(EditDefaultsOnly)
	FScalableFloat LevelUpRequirement = FScalableFloat();
	
private:
	
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_Level)
	int32 Level = 1;
	
	UPROPERTY(VisibleAnywhere, Replicated)
	int32 MaxLevel = 15;
	
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_XP)
	int32 XP = 0;
	
	UFUNCTION()
	void OnRep_Level() const;
	
	UFUNCTION()
	void OnRep_XP() const;
	
};
