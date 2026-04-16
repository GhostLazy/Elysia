// Copyright GhostLazy


#include "AbilitySystem/Abilities/ElysiaNormalAttack.h"
#include "AbilitySystemComponent.h"
#include "Actor/ElysiaProjectile.h"

void UElysiaNormalAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                          const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                          const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{			
		constexpr bool bReplicateEndAbility = true;
		constexpr bool bWasCancelled = true;
		EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	}
	
	K2_ActivateAbility();
}

void UElysiaNormalAttack::SpawnProjectile(const AActor* TargetActor) const
{
	// 客户端无权限执行生成子弹操作
	if (!GetAvatarActorFromActorInfo()->HasAuthority()) return;
	
	// 设置子弹生成位置与方向
	FTransform SpawnTransform;
	const FVector SpawnLocation = GetAvatarActorFromActorInfo()->GetActorLocation();
	const FVector TargetLocation = TargetActor ? TargetActor->GetActorLocation() : FVector(0, 0, SpawnLocation.Z);
	SpawnTransform.SetLocation(SpawnLocation);
	const FRotator SpawnRotation = (TargetLocation - SpawnLocation).Rotation();
	SpawnTransform.SetRotation(SpawnRotation.Quaternion());
	
	// 设置子弹伤害参数
	AElysiaProjectile* Projectile = GetWorld()->SpawnActorDeferred<AElysiaProjectile>(
		ProjectileClass, SpawnTransform, GetOwningActorFromActorInfo(), 
		Cast<APawn>(GetOwningActorFromActorInfo()), ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	const FGameplayEffectContextHandle EffectContext = GetAbilitySystemComponentFromActorInfo()->MakeEffectContext();
	Projectile->EffectSpecHandle = GetAbilitySystemComponentFromActorInfo()->MakeOutgoingSpec(DamageEffectClass, 1.f, EffectContext);
	
	// 生成子弹
	Projectile->FinishSpawning(SpawnTransform);
}
