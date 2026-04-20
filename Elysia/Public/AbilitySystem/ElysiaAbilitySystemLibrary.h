// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ElysiaAbilitySystemLibrary.generated.h"

/**
 * 
 */
UCLASS()
class ELYSIA_API UElysiaAbilitySystemLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	
	// 获取半径内存活的指定类型Actor
	UFUNCTION(BlueprintCallable)
	static void GetLiveActorsWithInRadius(const UObject* WorldContextObject, TArray<AActor*>& OutActors,
	                                  const TArray<AActor*>& ActorsToIgnore, const float Radius, const FVector& Origin, const FName ActorTag);
	
	// 获取Actor列表中距离最近的Actor
	UFUNCTION(BlueprintCallable)
	static AActor* GetClosestActor(const TArray<AActor*>& Actors, const FVector& Origin);
	
};
