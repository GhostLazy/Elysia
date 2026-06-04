// Copyright GhostLazy


#include "Actor/ElysiaTreasureChest.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/ElysiaAbilitySystemComponent.h"
#include "AbilitySystem/ElysiaAttributeSet.h"
#include "Actor/ElysiaBombPickup.h"
#include "Actor/ElysiaHealthPickup.h"
#include "Actor/ElysiaMagnetPickup.h"
#include "Actor/ElysiaXPBall.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Elysia/Elysia.h"

AElysiaTreasureChest::AElysiaTreasureChest()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	SetRootComponent(CollisionSphere);
	CollisionSphere->InitSphereRadius(80.f);
	CollisionSphere->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionSphere->SetCollisionResponseToChannel(ECC_Projectile, ECR_Overlap);
	CollisionSphere->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	AbilitySystemComponent = CreateDefaultSubobject<UElysiaAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	AttributeSet = CreateDefaultSubobject<UElysiaAttributeSet>(TEXT("AttributeSet"));

	Tags.Add(FName("Chest"));
	Tags.Add(FName("Damageable"));
}

UAbilitySystemComponent* AElysiaTreasureChest::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AElysiaTreasureChest::BeginPlay()
{
	Super::BeginPlay();

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
		HealthChangedHandle = AbilitySystemComponent
			->GetGameplayAttributeValueChangeDelegate(UElysiaAttributeSet::GetHealthAttribute())
			.AddUObject(this, &AElysiaTreasureChest::HandleHealthChanged);
	}

	if (HasAuthority())
	{
		InitializeAttributes();
	}
}

void AElysiaTreasureChest::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (AbilitySystemComponent && HealthChangedHandle.IsValid())
	{
		AbilitySystemComponent
			->GetGameplayAttributeValueChangeDelegate(UElysiaAttributeSet::GetHealthAttribute())
			.Remove(HealthChangedHandle);
	}

	HealthChangedHandle.Reset();
	Super::EndPlay(EndPlayReason);
}

void AElysiaTreasureChest::Die()
{
	OpenChest(nullptr);
}

void AElysiaTreasureChest::OpenChest(AActor* Opener)
{
	if (!HasAuthority() || bOpened)
	{
		return;
	}

	bOpened = true;
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	SpawnRandomReward(Opener);
	OnTreasureChestOpened.Broadcast(this);
	Destroy();
}

void AElysiaTreasureChest::SetRewardPhaseIndex(int32 InPhaseIndex)
{
	RewardPhaseIndex = FMath::Max(1, InPhaseIndex);
}

void AElysiaTreasureChest::InitializeAttributes() const
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	const float ClampedHealth = FMath::Max(1.f, ChestHealth);
	AbilitySystemComponent->SetNumericAttributeBase(UElysiaAttributeSet::GetMaxHealthAttribute(), ClampedHealth);
	AbilitySystemComponent->SetNumericAttributeBase(UElysiaAttributeSet::GetHealthAttribute(), ClampedHealth);
}

void AElysiaTreasureChest::SpawnRandomReward(AActor* Opener)
{
	const EElysiaTreasureChestRewardType RewardType = ChooseRewardType();
	const FVector SpawnLocation = GetRewardSpawnLocation();
	const FRotator SpawnRotation = FRotator::ZeroRotator;

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.Instigator = Cast<APawn>(Opener);
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	switch (RewardType)
	{
	case EElysiaTreasureChestRewardType::Magnet:
		if (MagnetPickupClass)
		{
			GetWorld()->SpawnActor<AElysiaMagnetPickup>(MagnetPickupClass, SpawnLocation, SpawnRotation, SpawnParameters);
		}
		break;
	case EElysiaTreasureChestRewardType::Bomb:
		if (BombPickupClass)
		{
			GetWorld()->SpawnActor<AElysiaBombPickup>(BombPickupClass, SpawnLocation, SpawnRotation, SpawnParameters);
		}
		break;
	case EElysiaTreasureChestRewardType::Health:
		if (HealthPickupClass)
		{
			GetWorld()->SpawnActor<AElysiaHealthPickup>(HealthPickupClass, SpawnLocation, SpawnRotation, SpawnParameters);
		}
		break;
	case EElysiaTreasureChestRewardType::XPBall:
	default:
		SpawnXPBallRewards(Opener);
		break;
	}
}

void AElysiaTreasureChest::SpawnXPBallRewards(AActor* Opener)
{
	if (!XPBallClass)
	{
		return;
	}

	const int32 RewardCount = FMath::Max(0, XPBallRewardCount);
	const int32 RewardValue = FMath::Max(0, XPBallRewardValue);
	const int32 RewardLevel = GetXPBallRewardLevel();
	for (int32 Index = 0; Index < RewardCount; ++Index)
	{
		FTransform SpawnTransform(FRotator::ZeroRotator, GetRewardSpawnLocation());
		if (AElysiaXPBall* XPBall = GetWorld()->SpawnActorDeferred<AElysiaXPBall>(
			XPBallClass,
			SpawnTransform,
			this,
			Cast<APawn>(Opener),
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn))
		{
			XPBall->SetXPValue(RewardValue);
			XPBall->SetColorByLevel(RewardLevel);
			XPBall->FinishSpawning(SpawnTransform);
		}
	}
}

EElysiaTreasureChestRewardType AElysiaTreasureChest::ChooseRewardType() const
{
	static constexpr EElysiaTreasureChestRewardType RewardTypes[] =
	{
		EElysiaTreasureChestRewardType::Magnet,
		EElysiaTreasureChestRewardType::Bomb,
		EElysiaTreasureChestRewardType::Health,
		EElysiaTreasureChestRewardType::XPBall
	};

	float TotalWeight = 0.f;
	for (const EElysiaTreasureChestRewardType RewardType : RewardTypes)
	{
		if (HasRewardClass(RewardType))
		{
			TotalWeight += GetRewardWeight(RewardType);
		}
	}

	if (TotalWeight <= 0.f)
	{
		return EElysiaTreasureChestRewardType::XPBall;
	}

	float RemainingWeight = FMath::FRandRange(0.f, TotalWeight);
	for (const EElysiaTreasureChestRewardType RewardType : RewardTypes)
	{
		if (!HasRewardClass(RewardType))
		{
			continue;
		}

		RemainingWeight -= GetRewardWeight(RewardType);
		if (RemainingWeight <= 0.f)
		{
			return RewardType;
		}
	}

	return EElysiaTreasureChestRewardType::XPBall;
}

float AElysiaTreasureChest::GetRewardWeight(EElysiaTreasureChestRewardType RewardType) const
{
	switch (RewardType)
	{
	case EElysiaTreasureChestRewardType::Magnet:
		return FMath::Max(0.f, MagnetRewardWeight);
	case EElysiaTreasureChestRewardType::Bomb:
		return FMath::Max(0.f, BombRewardWeight);
	case EElysiaTreasureChestRewardType::Health:
		return FMath::Max(0.f, HealthRewardWeight);
	case EElysiaTreasureChestRewardType::XPBall:
	default:
		return FMath::Max(0.f, XPBallRewardWeight);
	}
}

bool AElysiaTreasureChest::HasRewardClass(EElysiaTreasureChestRewardType RewardType) const
{
	switch (RewardType)
	{
	case EElysiaTreasureChestRewardType::Magnet:
		return MagnetPickupClass != nullptr;
	case EElysiaTreasureChestRewardType::Bomb:
		return BombPickupClass != nullptr;
	case EElysiaTreasureChestRewardType::Health:
		return HealthPickupClass != nullptr;
	case EElysiaTreasureChestRewardType::XPBall:
	default:
		return XPBallClass != nullptr;
	}
}

int32 AElysiaTreasureChest::GetXPBallRewardLevel() const
{
	if (RewardPhaseIndex <= 0)
	{
		return FMath::Max(1, XPBallRewardLevel);
	}

	return RewardPhaseIndex <= 2 ? 1 : 2;
}

FVector AElysiaTreasureChest::GetRewardSpawnLocation() const
{
	const float AngleRadians = FMath::FRandRange(0.f, 2.f * PI);
	const float Radius = FMath::FRandRange(0.f, RewardSpawnRadius);
	const FVector Offset = FVector(FMath::Cos(AngleRadians), FMath::Sin(AngleRadians), 0.f) * Radius;
	return GetActorLocation() + Offset + FVector::UpVector * RewardSpawnHeightOffset;
}

void AElysiaTreasureChest::HandleHealthChanged(const FOnAttributeChangeData& Data)
{
	if (HasAuthority() && Data.NewValue < Data.OldValue)
	{
		OpenChest(nullptr);
	}
}
