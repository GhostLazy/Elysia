// Copyright GhostLazy


#include "Actor/ElysiaRunePickup.h"

#include "Character/ElysiaCharacter.h"
#include "Equipment/ElysiaEquipmentComponent.h"
#include "Player/ElysiaPlayerState.h"

void AElysiaRunePickup::HandlePickedBy(AElysiaCharacter* Character)
{
	if (!Character)
	{
		return;
	}

	if (AElysiaPlayerState* PlayerState = Character->GetPlayerState<AElysiaPlayerState>())
	{
		if (UElysiaEquipmentComponent* EquipmentComponent = PlayerState->GetEquipmentComponent())
		{
			EquipmentComponent->QueueRuneSelection();
		}
	}
}
