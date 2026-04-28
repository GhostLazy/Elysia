// Copyright GhostLazy


#include "AbilitySystem/Abilities/ElysiaChalkLauncher.h"
#include "Actor/ElysiaProjectile.h"
#include "AbilitySystemComponent.h"

void UElysiaChalkLauncher::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                           const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                           const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	GetWorld()->GetTimerManager().SetTimer(SpawnProjectileTimer, this, &UElysiaChalkLauncher::SpawnProjectile, Interval, true);
}

void UElysiaChalkLauncher::SpawnProjectile()
{
	const FVector SpawnLocation = GetAvatarActorFromActorInfo()->GetActorLocation();
	const FRotator SpawnRotationBase = GetAvatarActorFromActorInfo()->GetActorRotation();
	const int32 NumChalks = IsWeaponEvolved() ? EvolvedProjectileCount : GetBaseProjectileCount(ProjectileCountByLevel);
	const float Offset = 360 / NumChalks;
	for (int32 i = 0; i < NumChalks; ++i)
	{
		FTransform SpawnTransform;
		SpawnTransform.SetLocation(SpawnLocation);
		FRotator SpawnRotation = SpawnRotationBase + FRotator(0, Offset * i, 0);
		SpawnTransform.SetRotation(SpawnRotation.Quaternion());

		// 设置粉笔头伤害参数并生成粉笔头
		AElysiaProjectile* Projectile = GetWorld()->SpawnActorDeferred<AElysiaProjectile>(
			ProjectileClass,
			SpawnTransform,
			GetOwningActorFromActorInfo(),
			Cast<APawn>(GetOwningActorFromActorInfo()),
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		Projectile->SetIsPenetrate(true);
		Projectile->SetMovementSpeed(MovementSpeed);
		
		const FGameplayEffectContextHandle EffectContext = GetAbilitySystemComponentFromActorInfo()->MakeEffectContext();
		Projectile->EffectSpecHandle = GetAbilitySystemComponentFromActorInfo()->MakeOutgoingSpec(
			DamageEffectClass,
			static_cast<float>(GetWeaponAbilityLevel()),
			EffectContext);
		Projectile->FinishSpawning(SpawnTransform);
	}
}
