// Copyright GhostLazy


#include "UI/ElysiaWidgetController.h"
#include "Equipment/ElysiaEquipmentComponent.h"
#include "Player/ElysiaPlayerState.h"

void UElysiaWidgetController::SetWidgetControllerParams(APlayerState* PS, APlayerController* PC,
                                                        UAbilitySystemComponent* ASC, UAttributeSet* AS)
{
	PlayerState = PS;
	PlayerController = PC;
	AbilitySystemComponent = ASC;
	AttributeSet = AS;
}

void UElysiaWidgetController::BindCallbacksToDependencies()
{
}

UElysiaEquipmentComponent* UElysiaWidgetController::GetEquipmentComponent() const
{
	if (const AElysiaPlayerState* ElysiaPS = Cast<AElysiaPlayerState>(PlayerState))
	{
		return ElysiaPS->GetEquipmentComponent();
	}

	return nullptr;
}
