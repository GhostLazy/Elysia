// Copyright GhostLazy


#include "AbilitySystem/Abilities/ElysiaSmartBulletAssistant.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/ElysiaAbilitySystemLibrary.h"
#include "AbilitySystem/ElysiaAttributeSet.h"
#include "Actor/ElysiaExplosiveProjectile.h"
#include "Actor/ElysiaProjectile.h"
#include "Actor/ElysiaSmartBulletOrb.h"

void UElysiaSmartBulletAssistant::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor || !AvatarActor->HasAuthority())
	{
		return;
	}

	SpawnOrUpdateOrbs();
	BindAttackSpeedChanged();
}

void UElysiaSmartBulletAssistant::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	UnbindAttackSpeedChanged();
	DestroyOrbs();
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UElysiaSmartBulletAssistant::SpawnOrUpdateOrbs()
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor || !AvatarActor->HasAuthority() || !OrbClass)
	{
		return;
	}

	const int32 DesiredOrbCount = GetDesiredOrbCount();
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (int32 Index = ActiveOrbs.Num() - 1; Index >= 0; --Index)
	{
		if (!IsValid(ActiveOrbs[Index]))
		{
			ActiveOrbs.RemoveAt(Index);
			if (FiredBulletCountsPerOrb.IsValidIndex(Index))
			{
				FiredBulletCountsPerOrb.RemoveAt(Index);
			}
		}
	}

	while (ActiveOrbs.Num() < DesiredOrbCount)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = AvatarActor;
		SpawnParameters.Instigator = Cast<APawn>(AvatarActor);
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AElysiaSmartBulletOrb* NewOrb = World->SpawnActor<AElysiaSmartBulletOrb>(
			OrbClass,
			AvatarActor->GetActorLocation(),
			FRotator::ZeroRotator,
			SpawnParameters);
		if (!NewOrb)
		{
			break;
		}

		ActiveOrbs.Add(NewOrb);
		FiredBulletCountsPerOrb.Add(0);
	}

	while (ActiveOrbs.Num() > DesiredOrbCount)
	{
		if (AElysiaSmartBulletOrb* ExtraOrb = ActiveOrbs.Pop())
		{
			ExtraOrb->Destroy();
		}

		if (FiredBulletCountsPerOrb.Num() > ActiveOrbs.Num())
		{
			FiredBulletCountsPerOrb.RemoveAt(ActiveOrbs.Num());
		}
	}

	FiredBulletCountsPerOrb.SetNum(ActiveOrbs.Num());

	for (int32 Index = 0; Index < ActiveOrbs.Num(); ++Index)
	{
		if (AElysiaSmartBulletOrb* Orb = ActiveOrbs[Index])
		{
			Orb->InitializeOrb(AvatarActor, Index, ActiveOrbs.Num(), OrbOrbitRadius, OrbHeightOffset, OrbFollowInterpSpeed);
		}
	}
}

void UElysiaSmartBulletAssistant::DestroyOrbs()
{
	for (AElysiaSmartBulletOrb* Orb : ActiveOrbs)
	{
		if (IsValid(Orb))
		{
			Orb->Destroy();
		}
	}

	ActiveOrbs.Empty();
	FiredBulletCountsPerOrb.Empty();
}

void UElysiaSmartBulletAssistant::FireFromOrbs()
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor || !AvatarActor->HasAuthority())
	{
		return;
	}

	SpawnOrUpdateOrbs();

	for (int32 OrbIndex = 0; OrbIndex < ActiveOrbs.Num(); ++OrbIndex)
	{
		AElysiaSmartBulletOrb* Orb = ActiveOrbs[OrbIndex];
		if (!IsValid(Orb))
		{
			continue;
		}

		for (int32 BulletIndex = 0; BulletIndex < BulletsPerOrb; ++BulletIndex)
		{
			FireBulletFromOrb(Orb, OrbIndex, BulletIndex, BulletsPerOrb);
		}
	}
}

void UElysiaSmartBulletAssistant::FireBulletFromOrb(AElysiaSmartBulletOrb* Orb, int32 OrbIndex, int32 BulletIndex, int32 BulletCount)
{
	if (!Orb || !GetWorld())
	{
		return;
	}

	const bool bExplosionShot = ShouldFireExplosionShot(OrbIndex);
	const TSubclassOf<AElysiaProjectile> ProjectileClassToSpawn = bExplosionShot && ExplosionProjectileClass
		? ExplosionProjectileClass
		: ProjectileClass;
	const TSubclassOf<UGameplayEffect> DamageEffectClassToUse = bExplosionShot && ExplosionDamageEffectClass
		? ExplosionDamageEffectClass
		: DamageEffectClass;
	if (!ProjectileClassToSpawn || !DamageEffectClassToUse)
	{
		return;
	}

	const FVector SpawnLocation = Orb->GetMuzzleLocation();
	const AActor* TargetActor = FindTarget(SpawnLocation);
	const FVector AimDirection = TargetActor
		? (TargetActor->GetActorLocation() - SpawnLocation).GetSafeNormal()
		: GetAvatarActorFromActorInfo()->GetActorForwardVector();
	const FRotator BaseRotation = AimDirection.Rotation();
	const float SpreadAlpha = BulletCount > 1
		? static_cast<float>(BulletIndex) / static_cast<float>(BulletCount - 1) - 0.5f
		: 0.f;
	const FRotator SpawnRotation = BaseRotation + FRotator(0.f, VolleySpreadAngle * SpreadAlpha, 0.f);

	FTransform SpawnTransform;
	SpawnTransform.SetLocation(SpawnLocation);
	SpawnTransform.SetRotation(SpawnRotation.Quaternion());

	AElysiaProjectile* Projectile = GetWorld()->SpawnActorDeferred<AElysiaProjectile>(
		ProjectileClassToSpawn,
		SpawnTransform,
		GetOwningActorFromActorInfo(),
		Cast<APawn>(GetOwningActorFromActorInfo()),
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (!Projectile)
	{
		return;
	}

	Projectile->SetMovementSpeed(ProjectileMovementSpeed);
	if (bExplosionShot)
	{
		if (AElysiaExplosiveProjectile* ExplosiveProjectile = Cast<AElysiaExplosiveProjectile>(Projectile))
		{
			ExplosiveProjectile->SetExplosionRadius(ExplosionRadius);
		}
	}

	const FGameplayEffectContextHandle EffectContext = GetAbilitySystemComponentFromActorInfo()->MakeEffectContext();
	Projectile->EffectSpecHandle = GetAbilitySystemComponentFromActorInfo()->MakeOutgoingSpec(
		DamageEffectClassToUse,
		static_cast<float>(GetWeaponAbilityLevel()),
		EffectContext);
	Projectile->FinishSpawning(SpawnTransform);
}

AActor* UElysiaSmartBulletAssistant::FindTarget(const FVector& Origin) const
{
	const AActor* AvatarActor = GetAvatarActorFromActorInfo();
	TArray<AActor*> ActorsToIgnore;
	if (AvatarActor)
	{
		ActorsToIgnore.Add(const_cast<AActor*>(AvatarActor));
	}

	TArray<AActor*> OverlapActors;
	UElysiaAbilitySystemLibrary::GetLiveActorsWithInRadius(this, OverlapActors, ActorsToIgnore, TargetSearchRadius, Origin, FName("Enemy"));
	return UElysiaAbilitySystemLibrary::GetClosestActor(OverlapActors, Origin);
}

void UElysiaSmartBulletAssistant::BindAttackSpeedChanged()
{
	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	if (!AbilitySystemComponent)
	{
		ResetFireTimerForAttackSpeed(1.f);
		return;
	}

	UnbindAttackSpeedChanged();
	AttackSpeedChangedHandle = AbilitySystemComponent
		->GetGameplayAttributeValueChangeDelegate(UElysiaAttributeSet::GetAttackSpeedAttribute())
		.AddUObject(this, &UElysiaSmartBulletAssistant::HandleAttackSpeedChanged);

	ResetFireTimerForAttackSpeed(AbilitySystemComponent->GetNumericAttribute(UElysiaAttributeSet::GetAttackSpeedAttribute()));
}

void UElysiaSmartBulletAssistant::UnbindAttackSpeedChanged()
{
	if (!AttackSpeedChangedHandle.IsValid())
	{
		return;
	}

	const FGameplayAbilityActorInfo* ActorInfo = CurrentActorInfo;
	UAbilitySystemComponent* AbilitySystemComponent = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent
			->GetGameplayAttributeValueChangeDelegate(UElysiaAttributeSet::GetAttackSpeedAttribute())
			.Remove(AttackSpeedChangedHandle);
	}

	AttackSpeedChangedHandle.Reset();
}

void UElysiaSmartBulletAssistant::HandleAttackSpeedChanged(const FOnAttributeChangeData& Data)
{
	ResetFireTimerForAttackSpeed(Data.NewValue);
}

void UElysiaSmartBulletAssistant::ResetFireTimerForAttackSpeed(float AttackSpeed)
{
	if (!GetWorld())
	{
		return;
	}

	GetWorld()->GetTimerManager().SetTimer(
		SpawnProjectileTimer,
		this,
		&UElysiaSmartBulletAssistant::FireFromOrbs,
		GetFireIntervalWithAttackSpeed(AttackSpeed),
		true);
}

float UElysiaSmartBulletAssistant::GetFireIntervalWithAttackSpeed(float AttackSpeed) const
{
	return FMath::Max(0.01f, FireInterval / FMath::Clamp(AttackSpeed, 0.1f, 10.f));
}

int32 UElysiaSmartBulletAssistant::GetDesiredOrbCount() const
{
	return FMath::Max(1, BaseOrbCount + (IsWeaponEvolved() ? EvolvedBonusOrbCount : 0));
}

int32 UElysiaSmartBulletAssistant::GetCurrentExplosionShotInterval() const
{
	return IsWeaponEvolved()
		? FMath::Max(1, EvolvedExplosionShotInterval)
		: FMath::Max(1, ExplosionShotInterval);
}

bool UElysiaSmartBulletAssistant::ShouldFireExplosionShot(int32 OrbIndex)
{
	if (OrbIndex < 0)
	{
		return false;
	}

	if (!FiredBulletCountsPerOrb.IsValidIndex(OrbIndex))
	{
		FiredBulletCountsPerOrb.SetNum(OrbIndex + 1);
	}

	++FiredBulletCountsPerOrb[OrbIndex];

	if (GetWeaponAbilityLevel() < ExplosionUnlockLevel)
	{
		return false;
	}

	return FiredBulletCountsPerOrb[OrbIndex] % GetCurrentExplosionShotInterval() == 0;
}
