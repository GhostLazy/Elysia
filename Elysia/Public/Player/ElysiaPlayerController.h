// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ElysiaPlayerController.generated.h"

struct FInputActionValue;
class AElysiaTrialInteractableActor;
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

	void Move(const FInputActionValue& InputActionValue);
	void ActivateSkill();
	void Interact();

	UFUNCTION(Server, Reliable)
	void ServerActivateSkill();

	UFUNCTION(Server, Reliable)
	void ServerInteractWithTrialOffer(AElysiaTrialInteractableActor* TrialOffer);

	void TryActivateSkill();
	AElysiaTrialInteractableActor* FindBestTrialOfferToInteract() const;
};
