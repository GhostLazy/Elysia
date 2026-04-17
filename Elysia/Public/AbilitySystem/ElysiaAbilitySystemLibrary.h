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
	
	// 获取半径内的Actor
	UFUNCTION(BlueprintCallable)
	static void GetActorsWithInRadius(const UObject* WorldContextObject, TArray<AActor*>& OutActors,
	                                  const TArray<AActor*>& ActorsToIgnore, float Radius, const FVector& Origin);
	
	// 获取Actor列表中距离最近的Actor
	UFUNCTION(BlueprintCallable)
	static AActor* GetClosestActor(const TArray<AActor*>& Actors, const FVector& Origin);
	
};
