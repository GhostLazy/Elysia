// Copyright GhostLazy


#include "AbilitySystem/Abilities/Character/ElysiaNormalAttack.h"
#include "AbilitySystemComponent.h"
#include "ElysiaGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/ElysiaAbilitySystemLibrary.h"
#include "AbilitySystem/ElysiaAbilitySystemComponent.h"
#include "AbilitySystem/ElysiaAttributeSet.h"
#include "Actor/ElysiaProjectile.h"
#include "Character/ElysiaCharacter.h"
#include "Equipment/ElysiaEquipmentComponent.h"

void UElysiaNormalAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                          const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                          const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor || !AvatarActor->HasAuthority())
	{
		return;
	}

	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	if (!AbilitySystemComponent)
	{
		return;
	}

	OnAttackSpeedChanged.AddDynamic(this, &UElysiaNormalAttack::ResetTimer);
	OnAttackSpeedChanged.Broadcast(AbilitySystemComponent->GetNumericAttribute(UElysiaAttributeSet::GetAttackSpeedAttribute()));

	// 当攻速属性发生变化时，重设普攻间隔
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UElysiaAttributeSet::GetAttackSpeedAttribute()).AddLambda(
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
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor || !AvatarActor->HasAuthority())
	{
		return;
	}

	if (AElysiaCharacter* ElysiaCharacter = Cast<AElysiaCharacter>(AvatarActor))
	{
		const int32 BaseProjectileCount = FMath::Max(1, GetBaseProjectileCount(ProjectileCountByLevel));
		const bool bEvolved = IsWeaponEvolved();
		const int32 ArrowsPerVolley = bEvolved ? 2 : 1;
		UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
		const bool bEnhanced = AbilitySystemComponent && AbilitySystemComponent->HasMatchingGameplayTag(FElysiaGameplayTags::Get().Status_Elysia_Enhanced);
		if (bEnhanced)
		{
			if (UElysiaAbilitySystemComponent* ElysiaASC = Cast<UElysiaAbilitySystemComponent>(AbilitySystemComponent))
			{
				ElysiaASC->ReduceCooldownRemaining(FElysiaGameplayTags::Get().Cooldown_Elysia_Skill, 0.1f);
			}
		}

		// 按武器等级执行 1/2/3/5 次连发；进化后每次连发改为并排双箭
		for (int32 VolleyIndex = 0; VolleyIndex < BaseProjectileCount; ++VolleyIndex)
		{
			const float Delay = BurstShotInterval * static_cast<float>(VolleyIndex);
			if (Delay <= 0.f)
			{
				FireProjectileVolley(ArrowsPerVolley, bEnhanced);
				continue;
			}

			FTimerDelegate VolleyDelegate = FTimerDelegate::CreateWeakLambda(this, [this, ArrowsPerVolley, bEnhanced]()
			{
				FireProjectileVolley(ArrowsPerVolley, bEnhanced);
			});
			FTimerHandle VolleyTimerHandle;
			GetWorld()->GetTimerManager().SetTimer(VolleyTimerHandle, VolleyDelegate, Delay, false);
		}
	}
}

void UElysiaNormalAttack::FindTargetAndPlayMontage()
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor || !AvatarActor->HasAuthority())
	{
		return;
	}

	if (AElysiaCharacter* ElysiaCharacter = Cast<AElysiaCharacter>(AvatarActor))
	{
		TArray<AActor*> ActorsToIgnore;
		ActorsToIgnore.Add(ElysiaCharacter);

		TArray<AActor*> OverlapActors;
		const FVector ActorLocation = ElysiaCharacter->GetActorLocation();

		// 索敌并朝向目标，随后播放普攻蒙太奇
		UElysiaAbilitySystemLibrary::GetLiveActorsWithInRadius(this, OverlapActors, ActorsToIgnore, TargetSearchRadius, ActorLocation, FName("Damageable"));
		TargetActor = UElysiaAbilitySystemLibrary::GetClosestActor(OverlapActors, ActorLocation);

		ElysiaCharacter->RotateToTarget(TargetActor);
		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, NAME_None, AttackMontage);
		MontageTask->ReadyForActivation();
	}
}

void UElysiaNormalAttack::ResetTimer(float NewAttackSpeed)
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor || !AvatarActor->HasAuthority())
	{
		return;
	}

	Interval = 1 / FMath::Clamp(NewAttackSpeed, 0.1f, 10.f);
	GetWorld()->GetTimerManager().SetTimer(SpawnProjectileTimer, this, &UElysiaNormalAttack::FindTargetAndPlayMontage, Interval, true);
}

void UElysiaNormalAttack::FireProjectileVolley(int32 ArrowsPerVolley, bool bShouldPenetrate) const
{
	AElysiaCharacter* ElysiaCharacter = Cast<AElysiaCharacter>(GetAvatarActorFromActorInfo());
	if (!ElysiaCharacter || !ElysiaCharacter->HasAuthority() || !ElysiaCharacter->GetWeapon())
	{
		return;
	}

	// 延迟连发真正执行时才读取当前枪口和目标位置，避免角色移动后仍从旧位置生成子弹。
	const FVector SpawnLocation = ElysiaCharacter->GetWeapon()->GetSocketLocation(FName("TipSocket"));
	const FVector AimDirection = IsValid(TargetActor)
		? (TargetActor->GetActorLocation() - SpawnLocation).GetSafeNormal()
		: ElysiaCharacter->GetActorForwardVector();
	const FRotator SpawnRotation = AimDirection.Rotation();
	const FVector RightVector = SpawnRotation.RotateVector(FVector::RightVector);
	const float PairHalfWidth = ArrowsPerVolley > 1 ? EvolvedPairSpacing * 0.5f : 0.f;
	const TSubclassOf<AElysiaProjectile> ProjectileClassToSpawn = bShouldPenetrate && EnhancedProjectileClass
		? EnhancedProjectileClass
		: ProjectileClass;
	if (!ProjectileClassToSpawn)
	{
		return;
	}

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
			ProjectileClassToSpawn,
			SpawnTransform,
			GetOwningActorFromActorInfo(),
			Cast<APawn>(GetOwningActorFromActorInfo()),
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		if (!Projectile)
		{
			continue;
		}

		Projectile->SetIsPenetrate(bShouldPenetrate);
		const FGameplayEffectContextHandle EffectContext = GetAbilitySystemComponentFromActorInfo()->MakeEffectContext();
		Projectile->EffectSpecHandle = GetAbilitySystemComponentFromActorInfo()->MakeOutgoingSpec(
			DamageEffectClass,
			static_cast<float>(GetWeaponEffectLevel()),
			EffectContext);
		Projectile->FinishSpawning(SpawnTransform);
	}
}
