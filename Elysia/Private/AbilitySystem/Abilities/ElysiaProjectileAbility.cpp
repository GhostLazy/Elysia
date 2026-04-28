// Copyright GhostLazy


#include "AbilitySystem/Abilities/ElysiaProjectileAbility.h"

void UElysiaProjectileAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	
	GetWorld()->GetTimerManager().ClearTimer(SpawnProjectileTimer);
}

int32 UElysiaProjectileAbility::GetBaseProjectileCount(TArray<int32>& ProjectileCountByLevel) const
{
	const int32 WeaponLevel = GetWeaponAbilityLevel();
	const int32 LevelIndex = FMath::Clamp(WeaponLevel - 1, 0, ProjectileCountByLevel.Num() - 1);
	return ProjectileCountByLevel.IsValidIndex(LevelIndex) ? ProjectileCountByLevel[LevelIndex] : 1;
}
