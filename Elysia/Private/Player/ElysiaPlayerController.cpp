// Copyright GhostLazy


#include "Player/ElysiaPlayerController.h"
#include "AbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "Character/ElysiaCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "ElysiaGameplayTags.h"
#include "GameFramework/Character.h"
#include "Trial/ElysiaTrialInteractionComponent.h"

AElysiaPlayerController::AElysiaPlayerController()
{
	bReplicates = true;
	TrialInteractionComponent = CreateDefaultSubobject<UElysiaTrialInteractionComponent>(TEXT("TrialInteractionComponent"));
}

void AElysiaPlayerController::BeginPlay()
{
	Super::BeginPlay();
	check(ElysiaIMC);
	
	if (UEnhancedInputLocalPlayerSubsystem* SubSystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		SubSystem->AddMappingContext(ElysiaIMC, 0);
	}
	
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;
}

void AElysiaPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	if (MoveAction)
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AElysiaPlayerController::Move);
	}
	if (SkillAction)
	{
		EnhancedInputComponent->BindAction(SkillAction, ETriggerEvent::Started, this, &AElysiaPlayerController::ActivateSkill);
	}
	if (InteractAction)
	{
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AElysiaPlayerController::Interact);
	}
}

void AElysiaPlayerController::Move(const FInputActionValue& InputActionValue)
{
	const FVector2D InputVector = InputActionValue.Get<FVector2D>();
	if (AElysiaCharacter* ElysiaCharacter = Cast<AElysiaCharacter>(GetCharacter()))
	{
		// 移动朝向取决于相机，便于实现角色面向敌人走A
		const FRotator Rotation = ElysiaCharacter->GetCamera()->GetComponentRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	
		ElysiaCharacter->AddMovementInput(ForwardDirection, InputVector.Y);
		ElysiaCharacter->AddMovementInput(RightDirection, InputVector.X);
	}
}

void AElysiaPlayerController::ActivateSkill()
{
	if (HasAuthority())
	{
		TryActivateSkill();
		return;
	}

	ServerActivateSkill();
}

void AElysiaPlayerController::Interact()
{
	if (TrialInteractionComponent)
	{
		TrialInteractionComponent->TryInteractWithCurrentTrialOffer();
	}
}

void AElysiaPlayerController::ServerActivateSkill_Implementation()
{
	TryActivateSkill();
}

void AElysiaPlayerController::TryActivateSkill()
{
	if (AElysiaCharacter* ElysiaCharacter = Cast<AElysiaCharacter>(GetCharacter()))
	{
		if (UAbilitySystemComponent* AbilitySystemComponent = ElysiaCharacter->GetAbilitySystemComponent())
		{
			FGameplayTagContainer SkillTags;
			SkillTags.AddTag(FElysiaGameplayTags::Get().Ability_Elysia_Skill);
			AbilitySystemComponent->TryActivateAbilitiesByTag(SkillTags);
		}
	}
}
