// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ElysiaSmartBulletOrb.generated.h"

class UStaticMeshComponent;

UCLASS()
class ELYSIA_API AElysiaSmartBulletOrb : public AActor
{
	GENERATED_BODY()

public:

	AElysiaSmartBulletOrb();

	virtual void Tick(float DeltaSeconds) override;

	void InitializeOrb(AActor* InFollowTarget, int32 InOrbIndex, int32 InOrbCount, float InOrbitRadius, float InHeightOffset, float InFollowInterpSpeed);
	void UpdateOrbLayout(int32 InOrbIndex, int32 InOrbCount, float InOrbitRadius, float InHeightOffset, float InFollowInterpSpeed);
	FVector GetMuzzleLocation() const;

protected:

	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> VisualMesh;

private:

	FVector GetDesiredLocation() const;

	UPROPERTY()
	TObjectPtr<AActor> FollowTarget;

	int32 OrbIndex = 0;
	int32 OrbCount = 1;
	float OrbitRadius = 120.f;
	float HeightOffset = 120.f;
	float FollowInterpSpeed = 12.f;
};
