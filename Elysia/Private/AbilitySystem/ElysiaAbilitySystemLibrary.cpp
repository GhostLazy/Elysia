// Copyright GhostLazy


#include "AbilitySystem/ElysiaAbilitySystemLibrary.h"
#include "Engine/OverlapResult.h"
#include "Interface/CombatInterface.h"

void UElysiaAbilitySystemLibrary::GetActorsWithInRadius(const UObject* WorldContextObject, TArray<AActor*>& OutActors,
                                                        const TArray<AActor*>& ActorsToIgnore, float Radius, const FVector& Origin)
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
			if (Overlap.GetActor()->Implements<UCombatInterface>() && Overlap.GetActor()->ActorHasTag(FName("Enemy")))
			{
				OutActors.AddUnique(Overlap.GetActor());
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
