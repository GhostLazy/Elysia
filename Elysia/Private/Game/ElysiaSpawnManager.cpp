// Copyright GhostLazy


#include "Game/ElysiaSpawnManager.h"
#include "AI/NavigationSystemBase.h"
#include "Actor/ElysiaTreasureChest.h"
#include "Character/ElysiaCharacterBase.h"
#include "Character/ElysiaEnemy.h"
#include "Components/CapsuleComponent.h"
#include "EngineUtils.h"
#include "Elysia/Elysia.h"
#include "GameFramework/Character.h"
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
		if (AElysiaEnemy* SpawnedEnemy = GetWorld()->SpawnActor<AElysiaEnemy>(EnemyClassToSpawn, SpawnLocation, FRotator::ZeroRotator, SpawnParameters))
		{
			SpawnedEnemy->SetLevel(NormalEnemyLevel);
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

	GetWorldTimerManager().ClearTimer(EliteSpawnTimerHandle);
	GetWorldTimerManager().SetTimer(EliteSpawnTimerHandle, this, &AElysiaSpawnManager::HandleEliteSpawnTick, EliteSpawnInterval, false);
}

void AElysiaSpawnManager::StopEliteSpawn()
{
	GetWorldTimerManager().ClearTimer(EliteSpawnTimerHandle);
}

void AElysiaSpawnManager::StartTreasureChestSpawn()
{
	if (!HasAuthority() || !TreasureChestClass)
	{
		return;
	}

	bTreasureChestSpawnEnabled = true;
	NextTreasureChestSpawnTime = 0.f;
	if (!GetWorldTimerManager().IsTimerActive(TreasureChestSpawnTimerHandle))
	{
		GetWorldTimerManager().SetTimer(
			TreasureChestSpawnTimerHandle,
			this,
			&AElysiaSpawnManager::HandleTreasureChestSpawnTick,
			TreasureChestSpawnInterval,
			true);
	}

	HandleTreasureChestSpawnTick();
}

void AElysiaSpawnManager::StopTreasureChestSpawn()
{
	bTreasureChestSpawnEnabled = false;
	GetWorldTimerManager().ClearTimer(TreasureChestSpawnTimerHandle);
}

void AElysiaSpawnManager::SetNormalEnemyLevel(int32 InLevel)
{
	NormalEnemyLevel = FMath::Max(1, InLevel);
}

void AElysiaSpawnManager::SetNormalPhaseIndex(int32 InPhaseIndex)
{
	NormalPhaseIndex = FMath::Max(1, InPhaseIndex);
	if (ActiveTreasureChest.IsValid() && !ActiveTreasureChest->IsOpened())
	{
		ActiveTreasureChest->SetRewardPhaseIndex(NormalPhaseIndex);
	}
}

AElysiaEnemy* AElysiaSpawnManager::SpawnSpecialEnemy(TSubclassOf<AElysiaEnemy> EnemyClass, int32 EnemyLevel)
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

	float CapsuleRadius = 0.f;
	float CapsuleHalfHeight = 0.f;
	if (!GetEnemyCapsuleSize(EnemyClass, CapsuleRadius, CapsuleHalfHeight))
	{
		return nullptr;
	}

	for (int32 AttemptIndex = 0; AttemptIndex < BossMaxSpawnAttempts; ++AttemptIndex)
	{
		FVector SpawnLocation;
		if (!TryFindGroundedBossSpawnLocation(PlayerPawn->GetActorLocation(), EnemyClass, SpawnLocation))
		{
			continue;
		}

		const FTransform SpawnTransform(FRotator::ZeroRotator, SpawnLocation);
		AElysiaEnemy* SpawnedEnemy = GetWorld()->SpawnActorDeferred<AElysiaEnemy>(
			EnemyClass,
			SpawnTransform,
			this,
			nullptr,
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding);
		if (!SpawnedEnemy)
		{
			continue;
		}

		// Boss 等级必须在 BeginPlay 初始化属性和授予技能前写入。
		SpawnedEnemy->SetLevel(EnemyLevel);
		SpawnedEnemy->FinishSpawning(SpawnTransform);

		if (HasGroundBelowBoss(SpawnedEnemy->GetActorLocation(), CapsuleHalfHeight, SpawnedEnemy))
		{
			return SpawnedEnemy;
		}

		SpawnedEnemy->Destroy();
	}

	return nullptr;
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

bool AElysiaSpawnManager::TryFindGroundedBossSpawnLocation(const FVector& PlayerLocation, TSubclassOf<AElysiaEnemy> EnemyClass, FVector& OutSpawnLocation) const
{
	float CapsuleRadius = 0.f;
	float CapsuleHalfHeight = 0.f;
	if (!GetEnemyCapsuleSize(EnemyClass, CapsuleRadius, CapsuleHalfHeight))
	{
		return false;
	}

	if (UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld()))
	{
		const FVector CandidateLocation = PlayerLocation + GenerateSpawnOffsetInBand();
		FNavLocation NavLocation;
		if (NavSystem->ProjectPointToNavigation(CandidateLocation, NavLocation, NavProjectExtent))
		{
			FVector GroundedSpawnLocation;
			if (TryProjectBossCandidateToGround(NavLocation.Location, CapsuleHalfHeight, GroundedSpawnLocation)
				&& IsBossSpawnLocationClear(GroundedSpawnLocation, CapsuleRadius, CapsuleHalfHeight, FindSpawnTargetPlayer()))
			{
				OutSpawnLocation = GroundedSpawnLocation;
				return true;
			}
		}
	}

	return false;
}

bool AElysiaSpawnManager::TryProjectBossCandidateToGround(const FVector& CandidateLocation, float CapsuleHalfHeight, FVector& OutSpawnLocation) const
{
	FHitResult GroundHit;
	const FVector TraceStart = CandidateLocation + FVector(0.f, 0.f, BossGroundTraceUpDistance);
	const FVector TraceEnd = CandidateLocation - FVector(0.f, 0.f, BossGroundTraceDownDistance);

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ElysiaBossGroundTrace), false);
	QueryParams.AddIgnoredActor(this);

	if (!GetWorld()->LineTraceSingleByObjectType(GroundHit, TraceStart, TraceEnd, ObjectQueryParams, QueryParams)
		|| !GroundHit.bBlockingHit)
	{
		return false;
	}

	OutSpawnLocation = GroundHit.ImpactPoint + FVector::UpVector * (CapsuleHalfHeight + BossGroundClearance);
	return true;
}

bool AElysiaSpawnManager::IsBossSpawnLocationClear(const FVector& SpawnLocation, float CapsuleRadius, float CapsuleHalfHeight, const AActor* PlayerActor) const
{
	if (PlayerActor && FVector::DistSquared2D(SpawnLocation, PlayerActor->GetActorLocation()) < FMath::Square(PlayerSafeRadius))
	{
		return false;
	}

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Player);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Minion);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Boss);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ElysiaBossSpawnOverlap), false);
	QueryParams.AddIgnoredActor(this);
	if (PlayerActor)
	{
		QueryParams.AddIgnoredActor(PlayerActor);
	}

	const float TestHalfHeight = FMath::Max(1.f, CapsuleHalfHeight - BossGroundClearance);
	return !GetWorld()->OverlapAnyTestByObjectType(
		SpawnLocation,
		FQuat::Identity,
		ObjectQueryParams,
		FCollisionShape::MakeCapsule(CapsuleRadius, TestHalfHeight),
		QueryParams);
}

bool AElysiaSpawnManager::HasGroundBelowBoss(const FVector& BossLocation, float CapsuleHalfHeight, const AActor* IgnoredActor) const
{
	FHitResult GroundHit;
	const FVector TraceStart = BossLocation;
	const FVector TraceEnd = BossLocation - FVector::UpVector * (CapsuleHalfHeight + BossGroundClearance + BossPostSpawnGroundCheckDistance);

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ElysiaBossPostSpawnGroundTrace), false);
	QueryParams.AddIgnoredActor(this);
	if (IgnoredActor)
	{
		QueryParams.AddIgnoredActor(IgnoredActor);
	}

	return GetWorld()->LineTraceSingleByObjectType(GroundHit, TraceStart, TraceEnd, ObjectQueryParams, QueryParams)
		&& GroundHit.bBlockingHit;
}

bool AElysiaSpawnManager::GetEnemyCapsuleSize(TSubclassOf<AElysiaEnemy> EnemyClass, float& OutCapsuleRadius, float& OutCapsuleHalfHeight) const
{
	const AElysiaEnemy* DefaultEnemy = EnemyClass ? EnemyClass->GetDefaultObject<AElysiaEnemy>() : nullptr;
	const UCapsuleComponent* CapsuleComponent = DefaultEnemy ? DefaultEnemy->GetCapsuleComponent() : nullptr;
	if (!CapsuleComponent)
	{
		return false;
	}

	OutCapsuleRadius = CapsuleComponent->GetScaledCapsuleRadius();
	OutCapsuleHalfHeight = CapsuleComponent->GetScaledCapsuleHalfHeight();
	return OutCapsuleRadius > 0.f && OutCapsuleHalfHeight > 0.f;
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

void AElysiaSpawnManager::HandleTreasureChestSpawnTick()
{
	if (!HasAuthority() || !bTreasureChestSpawnEnabled || !TreasureChestClass)
	{
		return;
	}

	if (FindExistingTreasureChest())
	{
		return;
	}

	if (GetWorld()->GetTimeSeconds() < NextTreasureChestSpawnTime)
	{
		return;
	}

	APawn* PlayerPawn = FindSpawnTargetPlayer();
	if (!PlayerPawn)
	{
		return;
	}

	for (int32 AttemptIndex = 0; AttemptIndex < TreasureChestMaxSpawnAttempts; ++AttemptIndex)
	{
		FVector SpawnLocation;
		if (TryFindTreasureChestSpawnLocation(PlayerPawn->GetActorLocation(), SpawnLocation))
		{
			SpawnTreasureChest(SpawnLocation);
			return;
		}
	}
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
	AElysiaEnemy* SpawnedEnemy = GetWorld()->SpawnActor<AElysiaEnemy>(EnemyClass, SpawnLocation, FRotator::ZeroRotator, SpawnParameters);
	if (SpawnedEnemy)
	{
		SpawnedEnemy->SetLevel(NormalEnemyLevel);
	}

	return SpawnedEnemy;
}

AElysiaTreasureChest* AElysiaSpawnManager::FindExistingTreasureChest()
{
	if (ActiveTreasureChest.IsValid() && !ActiveTreasureChest->IsOpened())
	{
		return ActiveTreasureChest.Get();
	}

	ActiveTreasureChest = nullptr;
	for (TActorIterator<AElysiaTreasureChest> It(GetWorld()); It; ++It)
	{
		if (!IsValid(*It) || It->IsOpened())
		{
			continue;
		}

		ActiveTreasureChest = *It;
		It->SetRewardPhaseIndex(NormalPhaseIndex);
		It->OnTreasureChestOpened.RemoveAll(this);
		It->OnTreasureChestOpened.AddUObject(this, &AElysiaSpawnManager::HandleTreasureChestOpened);
		return *It;
	}

	return nullptr;
}

AElysiaTreasureChest* AElysiaSpawnManager::SpawnTreasureChest(const FVector& SpawnLocation)
{
	if (!TreasureChestClass)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

	const FRotator SpawnRotation(0.f, FMath::FRandRange(0.f, 360.f), 0.f);
	AElysiaTreasureChest* SpawnedChest = GetWorld()->SpawnActor<AElysiaTreasureChest>(
		TreasureChestClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParameters);
	if (SpawnedChest)
	{
		SpawnedChest->SetRewardPhaseIndex(NormalPhaseIndex);
		ActiveTreasureChest = SpawnedChest;
		SpawnedChest->OnTreasureChestOpened.RemoveAll(this);
		SpawnedChest->OnTreasureChestOpened.AddUObject(this, &AElysiaSpawnManager::HandleTreasureChestOpened);
	}

	return SpawnedChest;
}

bool AElysiaSpawnManager::TryFindTreasureChestSpawnLocation(const FVector& PlayerLocation, FVector& OutSpawnLocation) const
{
	UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (!NavSystem)
	{
		return false;
	}

	const FVector CandidateLocation = PlayerLocation + GenerateTreasureChestSpawnOffset();
	FNavLocation NavLocation;
	if (!NavSystem->ProjectPointToNavigation(CandidateLocation, NavLocation, TreasureChestNavProjectExtent))
	{
		return false;
	}

	if (!IsTreasureChestSpawnLocationClear(NavLocation.Location, FindSpawnTargetPlayer()))
	{
		return false;
	}

	OutSpawnLocation = NavLocation.Location;
	return true;
}

bool AElysiaSpawnManager::IsTreasureChestSpawnLocationClear(const FVector& SpawnLocation, const AActor* PlayerActor) const
{
	if (PlayerActor && FVector::DistSquared2D(SpawnLocation, PlayerActor->GetActorLocation()) < FMath::Square(TreasureChestMinSpawnDistance))
	{
		return false;
	}

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Player);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Minion);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Boss);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ElysiaTreasureChestSpawnOverlap), false);
	QueryParams.AddIgnoredActor(this);
	if (PlayerActor)
	{
		QueryParams.AddIgnoredActor(PlayerActor);
	}

	return !GetWorld()->OverlapAnyTestByObjectType(
		SpawnLocation,
		FQuat::Identity,
		ObjectQueryParams,
		FCollisionShape::MakeSphere(TreasureChestSpawnClearanceRadius),
		QueryParams);
}

FVector AElysiaSpawnManager::GenerateTreasureChestSpawnOffset() const
{
	const float MinRadius = FMath::Max(0.f, TreasureChestMinSpawnDistance);
	const float MaxRadius = FMath::Max(MinRadius, TreasureChestMaxSpawnDistance);
	const float Radius = FMath::Sqrt(FMath::FRandRange(FMath::Square(MinRadius), FMath::Square(MaxRadius)));
	const float AngleRadians = FMath::FRandRange(0.f, 2.f * PI);
	return FVector(FMath::Cos(AngleRadians) * Radius, FMath::Sin(AngleRadians) * Radius, 0.f);
}

void AElysiaSpawnManager::HandleTreasureChestOpened(AElysiaTreasureChest* OpenedChest)
{
	if (ActiveTreasureChest.Get() == OpenedChest)
	{
		ActiveTreasureChest = nullptr;
	}

	NextTreasureChestSpawnTime = GetWorld()->GetTimeSeconds() + TreasureChestRespawnDelay;
}
