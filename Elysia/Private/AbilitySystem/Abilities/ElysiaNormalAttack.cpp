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
	if (!GetAvatarActorFromActorInfo()->HasAuthority())
	{
		return;
	}

	if (AElysiaCharacter* ElysiaCharacter = Cast<AElysiaCharacter>(GetAvatarActorFromActorInfo()))
	{
		const FVector SpawnLocation = ElysiaCharacter->GetWeapon()->GetSocketLocation(FName("TipSocket"));
		const FVector AimDirection = TargetActor
			? (TargetActor->GetActorLocation() - SpawnLocation).GetSafeNormal()
			: ElysiaCharacter->GetActorForwardVector();
		const FRotator BaseRotation = AimDirection.Rotation();
		const int32 BaseProjectileCount = FMath::Max(1, GetBaseProjectileCount());
		const bool bEvolved = IsWeaponEvolved();
		const int32 ArrowsPerVolley = bEvolved ? 2 : 1;

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

void UElysiaNormalAttack::PlayAttackMontage()
{
	if (AElysiaCharacter* ElysiaCharacter = Cast<AElysiaCharacter>(GetAvatarActorFromActorInfo()))
	{
		TArray<AActor*> ActorsToIgnore;
		ActorsToIgnore.Add(ElysiaCharacter);

		TArray<AActor*> OverlapActors;
		const FVector ActorLocation = ElysiaCharacter->GetActorLocation();

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
	GetWorld()->GetTimerManager().SetTimer(SpawnProjectileTimer, this, &UElysiaNormalAttack::PlayAttackMontage, Interval, true);
}

void UElysiaNormalAttack::FireProjectileVolley(const FVector& SpawnLocation, const FRotator& SpawnRotation, int32 ArrowsPerVolley) const
{
	const FVector RightVector = SpawnRotation.RotateVector(FVector::RightVector);
	const float PairHalfWidth = ArrowsPerVolley > 1 ? EvolvedPairSpacing * 0.5f : 0.f;

	for (int32 ArrowIndex = 0; ArrowIndex < ArrowsPerVolley; ++ArrowIndex)
	{
		const float PairOffset = ArrowsPerVolley > 1
			? (ArrowIndex == 0 ? -PairHalfWidth : PairHalfWidth)
			: 0.f;
		const FVector FinalSpawnLocation = SpawnLocation + RightVector * PairOffset;

		FTransform SpawnTransform;
		SpawnTransform.SetLocation(FinalSpawnLocation);
		SpawnTransform.SetRotation(SpawnRotation.Quaternion());

		AElysiaProjectile* Projectile = GetWorld()->SpawnActorDeferred<AElysiaProjectile>(
			ProjectileClass,
			SpawnTransform,
			GetOwningActorFromActorInfo(),
			Cast<APawn>(GetOwningActorFromActorInfo()),
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		const FGameplayEffectContextHandle EffectContext = GetAbilitySystemComponentFromActorInfo()->MakeEffectContext();
		Projectile->EffectSpecHandle = GetAbilitySystemComponentFromActorInfo()->MakeOutgoingSpec(DamageEffectClass, 1.f, EffectContext);
		Projectile->FinishSpawning(SpawnTransform);
	}
}

int32 UElysiaNormalAttack::GetBaseProjectileCount() const
{
	int32 WeaponLevel = 1;
	if (const UElysiaEquipmentComponent* EquipmentComponent = GetEquipmentComponent())
	{
		WeaponLevel = FMath::Max(1, EquipmentComponent->GetEquipmentLevelByAbilityClass(GetClass()));
	}

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
