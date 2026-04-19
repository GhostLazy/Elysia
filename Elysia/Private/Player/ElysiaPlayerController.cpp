// Copyright GhostLazy


#include "Player/ElysiaPlayerController.h"
#include "GameFramework/Character.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "Character/ElysiaCharacter.h"

AElysiaPlayerController::AElysiaPlayerController()
{
	bReplicates = true;
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
	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AElysiaPlayerController::Move);
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
