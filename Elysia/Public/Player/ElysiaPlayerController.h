// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ElysiaPlayerController.generated.h"

struct FInputActionValue;
class UElysiaTrialInteractionComponent;
class UInputAction;
class UInputMappingContext;
/**
 * 
 */
UCLASS()
class ELYSIA_API AElysiaPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	
	AElysiaPlayerController();

	UFUNCTION(BlueprintPure, Category = "Trial|Interaction")
	UElysiaTrialInteractionComponent* GetTrialInteractionComponent() const { return TrialInteractionComponent; }

	UFUNCTION(BlueprintCallable, Category = "LevelUp")
	bool TrySetLevelUpPaused(bool bPaused);
	
protected:
	
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	
private:
	
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputMappingContext> ElysiaIMC;
	
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> SkillAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> InteractAction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trial|Interaction", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UElysiaTrialInteractionComponent> TrialInteractionComponent;

	void Move(const FInputActionValue& InputActionValue);
	void ActivateSkill();
	void Interact();

	UFUNCTION(Server, Reliable)
	void ServerActivateSkill();

	UFUNCTION(Server, Reliable)
	void ServerSetLevelUpPaused(bool bPaused);

	bool TryActivateSkill();
	bool IsSinglePlayerSessionForPause() const;
	bool ApplyLevelUpPauseState(bool bPaused);
	void ClearLevelUpPauseStateLocally() const;
};
