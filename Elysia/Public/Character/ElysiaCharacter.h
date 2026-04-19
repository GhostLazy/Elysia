// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "Character/ElysiaCharacterBase.h"
#include "ElysiaCharacter.generated.h"

class UMotionWarpingComponent;
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
	// 执行平滑转向
	void RotateToTarget(const AActor* TargetActor) const;
	
	// 获取角色武器
	USkeletalMeshComponent* GetWeapon() { return Weapon; }
	// 获取角色相机
	UCameraComponent* GetCamera() { return Camera; }
	
protected:
	
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, Category = "Component")
	TObjectPtr<USkeletalMeshComponent> Weapon;
	
	UPROPERTY(VisibleAnywhere, Category = "Component")
	TObjectPtr<USpringArmComponent> SpringArm;
	
	UPROPERTY(VisibleAnywhere, Category = "Component")
	TObjectPtr<UCameraComponent> Camera;
	
	UPROPERTY(VisibleAnywhere, Category = "Component")
	TObjectPtr<UMotionWarpingComponent> MotionWarpingComponent;

private:
	
	// 角色ASC初始化
	void InitAbilityActorInfo();
	
};
