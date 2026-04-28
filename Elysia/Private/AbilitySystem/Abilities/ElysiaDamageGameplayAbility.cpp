// Copyright GhostLazy


#include "AbilitySystem/Abilities/ElysiaDamageGameplayAbility.h"
#include "Equipment/ElysiaEquipmentComponent.h"
#include "Character/ElysiaCharacter.h"
#include "Player/ElysiaPlayerState.h"

UElysiaDamageGameplayAbility::UElysiaDamageGameplayAbility()
{
	// 确保客户端可激活技能
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
}

void UElysiaDamageGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		constexpr bool bReplicateEndAbility = true;
		constexpr bool bWasCancelled = true;
		EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	}
}

int32 UElysiaDamageGameplayAbility::GetWeaponAbilityLevel() const
{
	const int32 AbilityLevel = GetAbilityLevel();
	if (AbilityLevel > 0)
	{
		return AbilityLevel;
	}

	if (const UElysiaEquipmentComponent* EquipmentComponent = GetEquipmentComponent())
	{
		return FMath::Max(1, EquipmentComponent->GetEquipmentLevelByAbilityClass(GetClass()));
	}

	return 1;
}

bool UElysiaDamageGameplayAbility::IsWeaponEvolved() const
{
	if (const UElysiaEquipmentComponent* EquipmentComponent = GetEquipmentComponent())
	{
		return EquipmentComponent->IsEquipmentEvolvedByAbilityClass(GetClass());
	}

	return false;
}

UElysiaEquipmentComponent* UElysiaDamageGameplayAbility::GetEquipmentComponent() const
{
	if (const AElysiaCharacter* ElysiaCharacter = Cast<AElysiaCharacter>(GetAvatarActorFromActorInfo()))
	{
		if (const AElysiaPlayerState* ElysiaPlayerState = ElysiaCharacter->GetPlayerState<AElysiaPlayerState>())
		{
			return ElysiaPlayerState->GetEquipmentComponent();
		}
	}

	return nullptr;
}