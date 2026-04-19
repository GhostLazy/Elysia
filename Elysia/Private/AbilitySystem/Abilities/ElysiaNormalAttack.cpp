// Copyright GhostLazy


#include "AbilitySystem/Abilities/ElysiaNormalAttack.h"
#include "AbilitySystemComponent.h"
#include "ElysiaGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/ElysiaAbilitySystemLibrary.h"
#include "AbilitySystem/ElysiaAttributeSet.h"
#include "Actor/ElysiaProjectile.h"
#include "Character/ElysiaCharacter.h"

UElysiaNormalAttack::UElysiaNormalAttack()
{
	// 确保客户端可激活技能
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
}

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
	
	OnAttackSpeedChanged.AddDynamic(this, &UElysiaNormalAttack::ResetTimer);
	OnAttackSpeedChanged.Broadcast(GetAbilitySystemComponentFromActorInfo()->GetNumericAttribute(UElysiaAttributeSet::GetAttackSpeedAttribute()));
	
	GetAbilitySystemComponentFromActorInfo()->GetGameplayAttributeValueChangeDelegate(UElysiaAttributeSet::GetAttackSpeedAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
	{
		OnAttackSpeedChanged.Broadcast(Data.NewValue);
	});
	
	UAbilityTask_WaitGameplayEvent* EventAttack = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FElysiaGameplayTags::Get().Event_Montage_Elysia_NormalAttack);
	EventAttack->EventReceived.AddDynamic(this, &UElysiaNormalAttack::SpawnProjectile);
	EventAttack->ReadyForActivation();
}

void UElysiaNormalAttack::SpawnProjectile(FGameplayEventData Payload)
{
	// 客户端无权限执行生成子弹操作
	if (!GetAvatarActorFromActorInfo()->HasAuthority()) return;
	
	if (AElysiaCharacter* ElysiaCharacter = Cast<AElysiaCharacter>(GetAvatarActorFromActorInfo()))
	{
		// 设置子弹生成位置（武器尖端）与方向（瞄准怪物）
		FTransform SpawnTransform;
		const FVector SpawnLocation = ElysiaCharacter->GetWeapon()->GetSocketLocation(FName("TipSocket"));
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
}

void UElysiaNormalAttack::PlayAttackMontage()
{
	if (AElysiaCharacter* ElysiaCharacter = Cast<AElysiaCharacter>(GetAvatarActorFromActorInfo()))
	{
		TArray<AActor*> ActorsToIgnore;
		ActorsToIgnore.Add(ElysiaCharacter);
	
		TArray<AActor*> OverlapActors;
		const FVector ActorLocation = ElysiaCharacter->GetActorLocation();
		
		UElysiaAbilitySystemLibrary::GetActorsWithInRadius(this, OverlapActors, ActorsToIgnore, 800, ActorLocation);
		TargetActor = UElysiaAbilitySystemLibrary::GetClosestActor(OverlapActors, ActorLocation);
		
		ElysiaCharacter->RotateToTarget(TargetActor);
		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, NAME_None, AttackMontage);
		MontageTask->ReadyForActivation();
	}
}

void UElysiaNormalAttack::ResetTimer(float NewAttackSpeed)
{
	Interval = 1 / FMath::Clamp(NewAttackSpeed, 0.1f, 10.f);
	GetWorld()->GetTimerManager().SetTimer(SpawnProjectileTimer, this, &UElysiaNormalAttack::PlayAttackMontage, Interval, true);
}

