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
	// 基类不绑定任何业务事件，交由子类实现
}

UElysiaEquipmentComponent* UElysiaWidgetController::GetEquipmentComponent() const
{
	if (const AElysiaPlayerState* ElysiaPS = Cast<AElysiaPlayerState>(PlayerState))
	{
		return ElysiaPS->GetEquipmentComponent();
	}

	return nullptr;
}
