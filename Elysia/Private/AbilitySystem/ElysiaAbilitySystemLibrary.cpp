// Copyright GhostLazy


#include "AbilitySystem/ElysiaAbilitySystemLibrary.h"

#include "Character/ElysiaEnemy.h"
#include "Engine/OverlapResult.h"
#include "Interface/CombatInterface.h"

void UElysiaAbilitySystemLibrary::GetLiveActorsWithInRadius(const UObject* WorldContextObject, TArray<AActor*>& OutActors,
                                                        const TArray<AActor*>& ActorsToIgnore, const float Radius, const FVector& Origin, const FName ActorTag)
{
	FCollisionQueryParams SphereParams;
	SphereParams.AddIgnoredActors(ActorsToIgnore);
	
	if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		TArray<FOverlapResult> Overlaps;
		World->OverlapMultiByObjectType(Overlaps, Origin, FQuat::Identity,
		                                FCollisionObjectQueryParams::InitType::AllDynamicObjects,
		                                FCollisionShape::MakeSphere(Radius), SphereParams);
		
		for (FOverlapResult& Overlap : Overlaps)
		{
			if (const ICombatInterface* CombatInterface = Cast<ICombatInterface>(Overlap.GetActor()))
			{
				if (!CombatInterface->IsDead() && CombatInterface->HasTag(ActorTag))
				{
					OutActors.AddUnique(Overlap.GetActor());
				}
			}
		}
	}
}

AActor* UElysiaAbilitySystemLibrary::GetClosestActor(const TArray<AActor*>& Actors, const FVector& Origin)
{
	AActor* OutActor = nullptr;
	float MinDistance = FLT_MAX;
	
	for (AActor* Actor : Actors)
	{
		const float Distance = (Origin - Actor->GetActorLocation()).Size();
		if (Distance < MinDistance)
		{
			MinDistance = Distance;
			OutActor = Actor;
		}
	}
	return OutActor;
}
