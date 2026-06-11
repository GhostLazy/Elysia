// Copyright GhostLazy

#include "AbilitySystem/Abilities/Character/ElysiaSwordSlash.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/ElysiaAbilitySystemLibrary.h"
#include "AbilitySystem/ElysiaAttributeSet.h"
#include "ElysiaGameplayTags.h"
#include "Engine/World.h"

void UElysiaSwordSlash::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	const AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor || !AvatarActor->HasAuthority())
	{
		return;
	}

	BindAttackSpeedChanged();
}

void UElysiaSwordSlash::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	UnbindAttackSpeedChanged();
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SlashTimerHandle);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UElysiaSwordSlash::ExecuteSlashVolley()
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	UAbilitySystemComponent* SourceAbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	if (!AvatarActor || !AvatarActor->HasAuthority() || !SourceAbilitySystemComponent || !DamageEffectClass)
	{
		return;
	}

	FGameplayEffectContextHandle EffectContext = SourceAbilitySystemComponent->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	const FGameplayEffectSpecHandle DamageSpecHandle = SourceAbilitySystemComponent->MakeOutgoingSpec(
		DamageEffectClass,
		static_cast<float>(GetWeaponAbilityLevel()),
		EffectContext);
	if (!DamageSpecHandle.IsValid())
	{
		return;
	}

	DamageSpecHandle.Data->SetSetByCallerMagnitude(
		FElysiaGameplayTags::Get().Data_DamageMultiplier,
		GetCurrentDamageMultiplier());

	const FVector AvatarLocation = AvatarActor->GetActorLocation();
	const float DamageRadius = GetCurrentDamageRadius();
	TSet<TWeakObjectPtr<AActor>> DamagedActors;

	for (int32 SlashIndex = 0; SlashIndex < GetCurrentSlashCount(); ++SlashIndex)
	{
		const FVector SlashCenter = MakeRandomSlashCenter(AvatarLocation);
		ExecuteSlashGameplayCue(SlashCenter, DamageRadius);

		TArray<AActor*> ActorsToIgnore;
		ActorsToIgnore.Add(AvatarActor);

		TArray<AActor*> OverlapActors;
		UElysiaAbilitySystemLibrary::GetLiveActorsWithInRadius(
			this,
			OverlapActors,
			ActorsToIgnore,
			DamageRadius,
			SlashCenter,
			FName("Damageable"));

		for (AActor* TargetActor : OverlapActors)
		{
			const TWeakObjectPtr<AActor> WeakTarget(TargetActor);
			if (!IsValid(TargetActor) || DamagedActors.Contains(WeakTarget))
			{
				continue;
			}

			if (UAbilitySystemComponent* TargetAbilitySystemComponent =
				UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor))
			{
				TargetAbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*DamageSpecHandle.Data.Get());
				DamagedActors.Add(WeakTarget);
			}
		}
	}
}

void UElysiaSwordSlash::BindAttackSpeedChanged()
{
	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	if (!AbilitySystemComponent)
	{
		ResetAttackTimer(1.f);
		return;
	}

	UnbindAttackSpeedChanged();
	AttackSpeedChangedHandle = AbilitySystemComponent
		->GetGameplayAttributeValueChangeDelegate(UElysiaAttributeSet::GetAttackSpeedAttribute())
		.AddUObject(this, &UElysiaSwordSlash::HandleAttackSpeedChanged);

	ResetAttackTimer(AbilitySystemComponent->GetNumericAttribute(UElysiaAttributeSet::GetAttackSpeedAttribute()));
}

void UElysiaSwordSlash::UnbindAttackSpeedChanged()
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

void UElysiaSwordSlash::HandleAttackSpeedChanged(const FOnAttributeChangeData& Data)
{
	ResetAttackTimer(Data.NewValue);
}

void UElysiaSwordSlash::ResetAttackTimer(float AttackSpeed)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const float Interval = BaseAttackInterval / FMath::Clamp(AttackSpeed, 0.1f, 10.f);
	World->GetTimerManager().SetTimer(
		SlashTimerHandle,
		this,
		&UElysiaSwordSlash::ExecuteSlashVolley,
		FMath::Max(0.01f, Interval),
		true);
}

FVector UElysiaSwordSlash::MakeRandomSlashCenter(const FVector& AvatarLocation) const
{
	const float RandomAngle = FMath::FRandRange(0.f, 2.f * PI);
	const FVector RingOffset(
		FMath::Cos(RandomAngle) * SpawnRingRadius,
		FMath::Sin(RandomAngle) * SpawnRingRadius,
		0.f);
	const FVector DesiredCenter = AvatarLocation + RingOffset;

	const UWorld* World = GetWorld();
	const AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!World || GroundTraceVerticalExtent <= 0.f)
	{
		return DesiredCenter;
	}

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ElysiaSwordSlashGroundTrace), false);
	QueryParams.AddIgnoredActor(AvatarActor);

	FHitResult GroundHit;
	const FVector TraceStart = DesiredCenter + FVector::UpVector * GroundTraceVerticalExtent;
	const FVector TraceEnd = DesiredCenter - FVector::UpVector * GroundTraceVerticalExtent;
	if (World->LineTraceSingleByChannel(GroundHit, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
	{
		return GroundHit.ImpactPoint + GroundHit.ImpactNormal * GroundEffectOffset;
	}

	return DesiredCenter;
}

void UElysiaSwordSlash::ExecuteSlashGameplayCue(const FVector& Center, float DamageRadius) const
{
	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	const FGameplayTag CueTag = SwordSlashGameplayCueTag.IsValid()
		? SwordSlashGameplayCueTag
		: FElysiaGameplayTags::Get().GameplayCue_Weapon_SwordSlash;
	if (!AbilitySystemComponent || !CueTag.IsValid())
	{
		return;
	}

	FGameplayCueParameters CueParameters;
	CueParameters.Location = Center;
	CueParameters.Normal = FVector::UpVector;
	CueParameters.RawMagnitude = DamageRadius;
	CueParameters.GameplayEffectLevel = IsWeaponEvolved()
		? static_cast<float>(GetWeaponAbilityLevel() + 1)
		: static_cast<float>(GetWeaponAbilityLevel());
	CueParameters.Instigator = GetAvatarActorFromActorInfo();
	CueParameters.EffectCauser = GetAvatarActorFromActorInfo();
	CueParameters.SourceObject = const_cast<UElysiaSwordSlash*>(this);
	AbilitySystemComponent->ExecuteGameplayCue(CueTag, CueParameters);
}

int32 UElysiaSwordSlash::GetCurrentSlashCount() const
{
	if (SlashCountByLevel.IsEmpty())
	{
		return 1;
	}

	const int32 LevelIndex = FMath::Clamp(GetWeaponAbilityLevel() - 1, 0, SlashCountByLevel.Num() - 1);
	return FMath::Max(1, SlashCountByLevel[LevelIndex]);
}

float UElysiaSwordSlash::GetCurrentDamageMultiplier() const
{
	if (IsWeaponEvolved())
	{
		return FMath::Max(0.f, EvolvedDamageMultiplier);
	}

	if (DamageMultiplierByLevel.IsEmpty())
	{
		return 0.f;
	}

	const int32 LevelIndex = FMath::Clamp(GetWeaponAbilityLevel() - 1, 0, DamageMultiplierByLevel.Num() - 1);
	return FMath::Max(0.f, DamageMultiplierByLevel[LevelIndex]);
}

float UElysiaSwordSlash::GetCurrentDamageRadius() const
{
	const float RangeMultiplier = IsWeaponEvolved() ? EvolvedRangeMultiplier : 1.f;
	return FMath::Max(0.f, BaseDamageRadius * FMath::Max(0.f, RangeMultiplier));
}
