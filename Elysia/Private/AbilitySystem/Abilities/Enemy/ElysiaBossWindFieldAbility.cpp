// Copyright GhostLazy

#include "AbilitySystem/Abilities/Enemy/ElysiaBossWindFieldAbility.h"

#include "Actor/ElysiaBossTornado.h"
#include "Character/ElysiaBossBase.h"
#include "ElysiaGameplayTags.h"

void UElysiaBossWindFieldAbility::ExecuteBossSkill()
{
	AElysiaBossBase* Boss = GetBossAvatar();
	if (!Boss || !Boss->HasAuthority() || !TornadoClass)
	{
		return;
	}

	const int32 SafeTornadoCount = FMath::Max(1, TornadoCount);
	const float StartAngle = FMath::FRandRange(0.f, 2.f * PI);
	for (int32 Index = 0; Index < SafeTornadoCount; ++Index)
	{
		const float Angle = StartAngle + 2.f * PI * static_cast<float>(Index) / static_cast<float>(SafeTornadoCount);
		const FVector SpawnOffset(FMath::Cos(Angle) * SpawnRadius, FMath::Sin(Angle) * SpawnRadius, 0.f);
		const FTransform SpawnTransform(FRotator::ZeroRotator, Boss->GetActorLocation() + SpawnOffset);

		if (AElysiaBossTornado* Tornado = Boss->GetWorld()->SpawnActorDeferred<AElysiaBossTornado>(
			TornadoClass,
			SpawnTransform,
			Boss,
			Boss,
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn))
		{
			Tornado->InitializeTornado(
				Boss,
				DamageEffectClass,
				DamageEffectLevel,
				TrackingSpeed,
				TornadoDuration,
				ContactRadius);
			Tornado->FinishSpawning(SpawnTransform);
		}
	}
}

FGameplayTag UElysiaBossWindFieldAbility::GetDefaultExecuteGameplayCueTag() const
{
	return FElysiaGameplayTags::Get().GameplayCue_Boss_WindField_Execute;
}
