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

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaSeconds) override;

	void InitializeOrb(AActor* InFollowTarget, int32 InOrbIndex, int32 InOrbCount, float InOrbitRadius, float InHeightOffset, float InFollowInterpSpeed);
	void UpdateOrbLayout(int32 InOrbIndex, int32 InOrbCount, float InOrbitRadius, float InHeightOffset, float InFollowInterpSpeed);
	FVector GetMuzzleLocation() const;

protected:

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> VisualMesh;

private:

	FVector GetDesiredLocation() const;
	void RefreshFollowTickPrerequisite();

	UFUNCTION()
	void OnRep_FollowTarget();

	UFUNCTION()
	void OnRep_OrbLayout();

	UPROPERTY(ReplicatedUsing = OnRep_FollowTarget)
	TObjectPtr<AActor> FollowTarget;

	UPROPERTY(ReplicatedUsing = OnRep_OrbLayout)
	int32 OrbIndex = 0;

	UPROPERTY(ReplicatedUsing = OnRep_OrbLayout)
	int32 OrbCount = 1;

	UPROPERTY(ReplicatedUsing = OnRep_OrbLayout)
	float OrbitRadius = 120.f;

	UPROPERTY(ReplicatedUsing = OnRep_OrbLayout)
	float HeightOffset = 120.f;

	UPROPERTY(ReplicatedUsing = OnRep_OrbLayout)
	float FollowInterpSpeed = 12.f;

	TWeakObjectPtr<AActor> TickPrerequisiteTarget;
};
