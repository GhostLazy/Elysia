// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "Character/ElysiaCharacterBase.h"
#include "ElysiaCharacter.generated.h"

class UWidgetComponent;
/**
 * 
 */
UCLASS()
class ELYSIA_API AElysiaCharacter : public AElysiaCharacterBase
{
	GENERATED_BODY()

public:
	
	AElysiaCharacter();
	
	// 确保服务器端角色的PC和PS已设置后，初始化ASC信息
	virtual void PossessedBy(AController* NewController) override;
	// 确保客户端角色的PS已设置后，初始化ASC信息
	virtual void OnRep_PlayerState() override;
	
protected:
	
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, Category = "Component")
	TObjectPtr<USkeletalMeshComponent> Weapon;
	
	UPROPERTY(VisibleAnywhere, Category = "Component")
	TObjectPtr<USpringArmComponent> SpringArm;
	
	UPROPERTY(VisibleAnywhere, Category = "Component")
	TObjectPtr<UCameraComponent> Camera;
	
	UPROPERTY(VisibleAnywhere, Category = "Component")
	TObjectPtr<UWidgetComponent> HealthBar;
	
	UPROPERTY(BlueprintAssignable)
	FOnAttributeChangeSignature OnHealthChanged;
	
	UPROPERTY(BlueprintAssignable)
	FOnAttributeChangeSignature OnMaxHealthChanged;
	
private:
	
	void InitAbilityActorInfo();
	void InitHealthBar();
	
};
