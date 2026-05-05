// Copyright GhostLazy


#include "Game/ElysiaSpawnManager.h"

#include "AI/NavigationSystemBase.h"
#include "Character/ElysiaCharacterBase.h"
#include "Character/ElysiaEnemy.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "NavigationSystem.h"

AElysiaSpawnManager::AElysiaSpawnManager()
{
	// 自定义定时器
	PrimaryActorTick.bCanEverTick = false;
	// 刷怪机仅存在于服务器（隶属于GameMode）
	bReplicates = false;
}

void AElysiaSpawnManager::BeginPlay()
{
	Super::BeginPlay();
}

void AElysiaSpawnManager::HandleSpawnTick()
{
	if (!HasAuthority() || !bNormalSpawnEnabled || SpawnPool.IsEmpty())
	{
		return;
	}

	APawn* PlayerPawn = FindSpawnTargetPlayer();
	if (!PlayerPawn)
	{
		return;
	}

	const int32 AliveMinionCount = CountAliveMinions();
	const int32 MissingMinions = FMath::Max(0, MaxAliveMinions - AliveMinionCount);
	if (MissingMinions <= 0)
	{
		return;
	}

	const int32 NumToSpawn = FMath::Min(SpawnBatchSize, MissingMinions);
	int32 SpawnedCount = 0;
	int32 AttemptCount = 0;
	const FVector PlayerLocation = PlayerPawn->GetActorLocation();

	while (SpawnedCount < NumToSpawn && AttemptCount < MaxSpawnAttemptsPerTick)
	{
		++AttemptCount;

		const TSubclassOf<AElysiaEnemy> EnemyClassToSpawn = ChooseEnemyClassToSpawn();
		if (!EnemyClassToSpawn)
		{
			continue;
		}

		FVector SpawnLocation;
		if (!TryFindSpawnLocation(PlayerLocation, SpawnLocation))
		{
			continue;
		}

		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
		if (GetWorld()->SpawnActor<AElysiaEnemy>(EnemyClassToSpawn, SpawnLocation, FRotator::ZeroRotator, SpawnParameters))
		{
			++SpawnedCount;
		}
	}
}

void AElysiaSpawnManager::StartNormalSpawn()
{
	if (!HasAuthority() || SpawnPool.IsEmpty())
	{
		return;
	}

	bNormalSpawnEnabled = true;
	if (!GetWorldTimerManager().IsTimerActive(SpawnTimerHandle))
	{
		GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AElysiaSpawnManager::HandleSpawnTick, SpawnInterval, true);
	}
}

void AElysiaSpawnManager::StopNormalSpawn()
{
	bNormalSpawnEnabled = false;
	GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
}

void AElysiaSpawnManager::StartEliteSpawn()
{
	if (!HasAuthority() || EliteSpawnPool.IsEmpty())
	{
		return;
	}

	if (!GetWorldTimerManager().IsTimerActive(EliteSpawnTimerHandle))
	{
		GetWorldTimerManager().SetTimer(EliteSpawnTimerHandle, this, &AElysiaSpawnManager::HandleEliteSpawnTick, EliteSpawnInterval, true);
	}
}

void AElysiaSpawnManager::StopEliteSpawn()
{
	GetWorldTimerManager().ClearTimer(EliteSpawnTimerHandle);
}

AElysiaEnemy* AElysiaSpawnManager::SpawnSpecialEnemy(TSubclassOf<AElysiaEnemy> EnemyClass)
{
	return SpawnEnemyOfClass(EnemyClass);
}

int32 AElysiaSpawnManager::CountAliveMinions() const
{
	int32 AliveCount = 0;
	for (TActorIterator<AElysiaEnemy> It(GetWorld()); It; ++It)
	{
		if (IsValid(*It) && !It->IsDead() && It->GetEnemyType() == EElysiaEnemyType::Minion)
		{
			++AliveCount;
		}
	}

	return AliveCount;
}

TSubclassOf<AElysiaEnemy> AElysiaSpawnManager::ChooseEnemyClassToSpawn() const
{
	float TotalWeight = 0.f;
	for (const FElysiaSpawnEntry& SpawnEntry : SpawnPool)
	{
		if (SpawnEntry.EnemyClass && SpawnEntry.Weight > 0.f)
		{
			TotalWeight += SpawnEntry.Weight;
		}
	}

	if (TotalWeight <= 0.f)
	{
		return nullptr;
	}

	float RemainingWeight = FMath::FRandRange(0.f, TotalWeight);
	for (const FElysiaSpawnEntry& SpawnEntry : SpawnPool)
	{
		if (!SpawnEntry.EnemyClass || SpawnEntry.Weight <= 0.f)
		{
			continue;
		}

		RemainingWeight -= SpawnEntry.Weight;
		if (RemainingWeight <= 0.f)
		{
			return SpawnEntry.EnemyClass;
		}
	}

	for (const FElysiaSpawnEntry& SpawnEntry : SpawnPool)
	{
		if (SpawnEntry.EnemyClass && SpawnEntry.Weight > 0.f)
		{
			return SpawnEntry.EnemyClass;
		}
	}

	return nullptr;
}

TSubclassOf<AElysiaEnemy> AElysiaSpawnManager::ChooseEliteClassToSpawn() const
{
	float TotalWeight = 0.f;
	for (const FElysiaSpawnEntry& SpawnEntry : EliteSpawnPool)
	{
		if (SpawnEntry.EnemyClass && SpawnEntry.Weight > 0.f)
		{
			TotalWeight += SpawnEntry.Weight;
		}
	}

	if (TotalWeight <= 0.f)
	{
		return nullptr;
	}

	float RemainingWeight = FMath::FRandRange(0.f, TotalWeight);
	for (const FElysiaSpawnEntry& SpawnEntry : EliteSpawnPool)
	{
		if (!SpawnEntry.EnemyClass || SpawnEntry.Weight <= 0.f)
		{
			continue;
		}

		RemainingWeight -= SpawnEntry.Weight;
		if (RemainingWeight <= 0.f)
		{
			return SpawnEntry.EnemyClass;
		}
	}

	for (const FElysiaSpawnEntry& SpawnEntry : EliteSpawnPool)
	{
		if (SpawnEntry.EnemyClass && SpawnEntry.Weight > 0.f)
		{
			return SpawnEntry.EnemyClass;
		}
	}

	return nullptr;
}

APawn* AElysiaSpawnManager::FindSpawnTargetPlayer() const
{
	if (APawn* PlayerPawn = GetWorld()->GetFirstPlayerController() ? GetWorld()->GetFirstPlayerController()->GetPawn() : nullptr)
	{
		if (const AElysiaCharacterBase* CharacterBase = Cast<AElysiaCharacterBase>(PlayerPawn))
		{
			if (!CharacterBase->IsDead())
			{
				return PlayerPawn;
			}
		}
		else
		{
			return PlayerPawn;
		}
	}

	for (TActorIterator<APawn> It(GetWorld()); It; ++It)
	{
		if (!It->ActorHasTag(FName("Player")))
		{
			continue;
		}

		if (const AElysiaCharacterBase* CharacterBase = Cast<AElysiaCharacterBase>(*It))
		{
			if (!CharacterBase->IsDead())
			{
				return *It;
			}
		}
		else
		{
			return *It;
		}
	}

	return nullptr;
}

bool AElysiaSpawnManager::TryFindSpawnLocation(const FVector& PlayerLocation, FVector& OutSpawnLocation) const
{
	if (UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld()))
	{
		const FVector CandidateLocation = PlayerLocation + GenerateSpawnOffsetInBand();
		FNavLocation NavLocation;
		if (NavSystem->ProjectPointToNavigation(CandidateLocation, NavLocation, NavProjectExtent)
			&& IsSpawnLocationAvailable(NavLocation.Location, FindSpawnTargetPlayer()))
		{
			OutSpawnLocation = NavLocation.Location;
			return true;
		}
	}

	return false;
}

bool AElysiaSpawnManager::IsSpawnLocationAvailable(const FVector& SpawnLocation, const AActor* PlayerActor) const
{
	if (PlayerActor && FVector::DistSquared2D(SpawnLocation, PlayerActor->GetActorLocation()) < FMath::Square(PlayerSafeRadius))
	{
		return false;
	}

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ElysiaSpawnManagerOverlap), false);
	QueryParams.AddIgnoredActor(this);
	if (PlayerActor)
	{
		QueryParams.AddIgnoredActor(PlayerActor);
	}

	return !GetWorld()->OverlapAnyTestByObjectType(
		SpawnLocation,
		FQuat::Identity,
		ObjectQueryParams,
		FCollisionShape::MakeSphere(100.f),
		QueryParams);
}

FVector AElysiaSpawnManager::GenerateSpawnOffsetInBand() const
{
	const float OuterHalfX = VisibleHalfExtent.X + SpawnBandThickness;
	const float OuterHalfY = VisibleHalfExtent.Y + SpawnBandThickness;
	const int32 SideIndex = FMath::RandRange(0, 3);

	switch (SideIndex)
	{
	case 0:
		return FVector(
			FMath::FRandRange(-OuterHalfX, -VisibleHalfExtent.X),
			FMath::FRandRange(-OuterHalfY, OuterHalfY),
			0.f);
	case 1:
		return FVector(
			FMath::FRandRange(VisibleHalfExtent.X, OuterHalfX),
			FMath::FRandRange(-OuterHalfY, OuterHalfY),
			0.f);
	case 2:
		return FVector(
			FMath::FRandRange(-OuterHalfX, OuterHalfX),
			FMath::FRandRange(VisibleHalfExtent.Y, OuterHalfY),
			0.f);
	default:
		return FVector(
			FMath::FRandRange(-OuterHalfX, OuterHalfX),
			FMath::FRandRange(-OuterHalfY, -VisibleHalfExtent.Y),
			0.f);
	}
}

void AElysiaSpawnManager::HandleEliteSpawnTick()
{
	if (!HasAuthority() || EliteSpawnPool.IsEmpty())
	{
		return;
	}

	SpawnEnemyOfClass(ChooseEliteClassToSpawn());
}

AElysiaEnemy* AElysiaSpawnManager::SpawnEnemyOfClass(TSubclassOf<AElysiaEnemy> EnemyClass)
{
	if (!HasAuthority() || !EnemyClass)
	{
		return nullptr;
	}

	APawn* PlayerPawn = FindSpawnTargetPlayer();
	if (!PlayerPawn)
	{
		return nullptr;
	}

	FVector SpawnLocation;
	if (!TryFindSpawnLocation(PlayerPawn->GetActorLocation(), SpawnLocation))
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
	return GetWorld()->SpawnActor<AElysiaEnemy>(EnemyClass, SpawnLocation, FRotator::ZeroRotator, SpawnParameters);
}
