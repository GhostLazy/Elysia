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
#include "Equipment/ElysiaEquipmentComponent.h"
#include "Player/ElysiaPlayerState.h"

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

	// 当攻速属性发生变化时，重设普攻间隔
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
	if (!GetAvatarActorFromActorInfo()->HasAuthority())
	{
		return;
	}

	if (AElysiaCharacter* ElysiaCharacter = Cast<AElysiaCharacter>(GetAvatarActorFromActorInfo()))
	{
		// 计算发射原点与朝向；所有连发都沿同一条发射线前进
		const FVector SpawnLocation = ElysiaCharacter->GetWeapon()->GetSocketLocation(FName("TipSocket"));
		const FVector AimDirection = TargetActor
			? (TargetActor->GetActorLocation() - SpawnLocation).GetSafeNormal()
			: ElysiaCharacter->GetActorForwardVector();
		const FRotator BaseRotation = AimDirection.Rotation();
		const int32 BaseProjectileCount = FMath::Max(1, GetBaseProjectileCount());
		const bool bEvolved = IsWeaponEvolved();
		const int32 ArrowsPerVolley = bEvolved ? 2 : 1;

		// 按武器等级执行 1/2/3/5 次连发；进化后每次连发改为并排双箭
		for (int32 VolleyIndex = 0; VolleyIndex < BaseProjectileCount; ++VolleyIndex)
		{
			const float Delay = BurstShotInterval * static_cast<float>(VolleyIndex);
			if (Delay <= 0.f)
			{
				FireProjectileVolley(SpawnLocation, BaseRotation, ArrowsPerVolley);
				continue;
			}

			FTimerDelegate VolleyDelegate = FTimerDelegate::CreateWeakLambda(this, [this, SpawnLocation, BaseRotation, ArrowsPerVolley]()
			{
				FireProjectileVolley(SpawnLocation, BaseRotation, ArrowsPerVolley);
			});
			FTimerHandle VolleyTimerHandle;
			GetWorld()->GetTimerManager().SetTimer(VolleyTimerHandle, VolleyDelegate, Delay, false);
		}
	}
}

void UElysiaNormalAttack::FindTargetAndPlayMontage()
{
	if (AElysiaCharacter* ElysiaCharacter = Cast<AElysiaCharacter>(GetAvatarActorFromActorInfo()))
	{
		TArray<AActor*> ActorsToIgnore;
		ActorsToIgnore.Add(ElysiaCharacter);

		TArray<AActor*> OverlapActors;
		const FVector ActorLocation = ElysiaCharacter->GetActorLocation();

		// 索敌并朝向目标，随后播放普攻蒙太奇
		UElysiaAbilitySystemLibrary::GetLiveActorsWithInRadius(this, OverlapActors, ActorsToIgnore, 800, ActorLocation, FName("Enemy"));
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
	GetWorld()->GetTimerManager().SetTimer(SpawnProjectileTimer, this, &UElysiaNormalAttack::FindTargetAndPlayMontage, Interval, true);
}

void UElysiaNormalAttack::FireProjectileVolley(const FVector& SpawnLocation, const FRotator& SpawnRotation, int32 ArrowsPerVolley) const
{
	const FVector RightVector = SpawnRotation.RotateVector(FVector::RightVector);
	const float PairHalfWidth = ArrowsPerVolley > 1 ? EvolvedPairSpacing * 0.5f : 0.f;

	// 进化前只发 1 支；进化后同一轮改为并排双箭
	for (int32 ArrowIndex = 0; ArrowIndex < ArrowsPerVolley; ++ArrowIndex)
	{
		const float PairOffset = ArrowsPerVolley > 1
			? (ArrowIndex == 0 ? -PairHalfWidth : PairHalfWidth)
			: 0.f;
		const FVector FinalSpawnLocation = SpawnLocation + RightVector * PairOffset;

		FTransform SpawnTransform;
		SpawnTransform.SetLocation(FinalSpawnLocation);
		SpawnTransform.SetRotation(SpawnRotation.Quaternion());

		// 设置箭矢伤害参数并生成箭矢
		AElysiaProjectile* Projectile = GetWorld()->SpawnActorDeferred<AElysiaProjectile>(
			ProjectileClass,
			SpawnTransform,
			GetOwningActorFromActorInfo(),
			Cast<APawn>(GetOwningActorFromActorInfo()),
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		const FGameplayEffectContextHandle EffectContext = GetAbilitySystemComponentFromActorInfo()->MakeEffectContext();
		Projectile->EffectSpecHandle = GetAbilitySystemComponentFromActorInfo()->MakeOutgoingSpec(
			DamageEffectClass,
			static_cast<float>(GetWeaponAbilityLevel()),
			EffectContext);
		Projectile->FinishSpawning(SpawnTransform);
	}
}

int32 UElysiaNormalAttack::GetWeaponAbilityLevel() const
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

int32 UElysiaNormalAttack::GetBaseProjectileCount() const
{
	const int32 WeaponLevel = GetWeaponAbilityLevel();
	const int32 LevelIndex = FMath::Clamp(WeaponLevel - 1, 0, ProjectileCountByLevel.Num() - 1);
	return ProjectileCountByLevel.IsValidIndex(LevelIndex) ? ProjectileCountByLevel[LevelIndex] : 1;
}

int32 UElysiaNormalAttack::GetProjectileCount() const
{
	const int32 BaseProjectileCount = GetBaseProjectileCount();
	return BaseProjectileCount * (IsWeaponEvolved() ? 2 : 1);
}

bool UElysiaNormalAttack::IsWeaponEvolved() const
{
	if (const UElysiaEquipmentComponent* EquipmentComponent = GetEquipmentComponent())
	{
		return EquipmentComponent->IsEquipmentEvolvedByAbilityClass(GetClass());
	}

	return false;
}

UElysiaEquipmentComponent* UElysiaNormalAttack::GetEquipmentComponent() const
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
