// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ElysiaBossLaserActor.generated.h"

class AElysiaBossBase;
class UNiagaraComponent;
class UNiagaraSystem;
class USceneComponent;
class USoundBase;
class UGameplayEffect;

UCLASS()
class ELYSIA_API AElysiaBossLaserActor : public AActor
{
	GENERATED_BODY()

public:
	AElysiaBossLaserActor();

	void InitializeLaser(
		AElysiaBossBase* InSourceBoss,
		const FVector& InOrigin,
		const FVector& InDirection,
		TSubclassOf<UGameplayEffect> InDamageEffectClass,
		float InDamageEffectLevel,
		float InLaserLength,
		float InLaserWidth,
		float InLaserDuration,
		float InLaserTickInterval);

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintNativeEvent, Category = "Boss|Laser")
	void OnLaserInitialized(FVector InOrigin, FVector InDirection);

	virtual void OnLaserInitialized_Implementation(FVector InOrigin, FVector InDirection);

	UFUNCTION(BlueprintNativeEvent, Category = "Boss|Laser")
	void OnLaserFinished();

	virtual void OnLaserFinished_Implementation();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Laser")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Laser|VFX")
	TObjectPtr<UNiagaraSystem> LaserEffect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Laser|VFX")
	TObjectPtr<USoundBase> LaserStartSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Laser|VFX")
	TObjectPtr<USoundBase> LaserEndSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Laser|VFX")
	FName NiagaraLengthParameterName = FName("User.Length");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Laser|VFX")
	FName NiagaraWidthParameterName = FName("User.Width");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Laser|VFX")
	FName NiagaraDurationParameterName = FName("User.Duration");

private:
	void StartLaser();
	void ApplyLaserEffectParameters(UNiagaraComponent* NiagaraComponent) const;
	void HandleDamageTick();
	void FinishLaser();

	UPROPERTY(BlueprintReadOnly, Category = "Boss|Laser", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AElysiaBossBase> SourceBoss;

	UPROPERTY(BlueprintReadOnly, Category = "Boss|Laser", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY(BlueprintReadOnly, Category = "Boss|Laser", meta = (AllowPrivateAccess = "true"))
	float DamageEffectLevel = 1.f;

	UPROPERTY(BlueprintReadOnly, Category = "Boss|Laser", meta = (AllowPrivateAccess = "true"))
	float LaserLength = 1200.f;

	UPROPERTY(BlueprintReadOnly, Category = "Boss|Laser", meta = (AllowPrivateAccess = "true"))
	float LaserWidth = 180.f;

	UPROPERTY(BlueprintReadOnly, Category = "Boss|Laser", meta = (AllowPrivateAccess = "true"))
	float LaserDuration = 1.5f;

	UPROPERTY(BlueprintReadOnly, Category = "Boss|Laser", meta = (AllowPrivateAccess = "true"))
	float LaserTickInterval = 0.2f;

	UPROPERTY(BlueprintReadOnly, Category = "Boss|Laser", meta = (AllowPrivateAccess = "true"))
	FVector Origin = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Boss|Laser", meta = (AllowPrivateAccess = "true"))
	FVector Direction = FVector::ForwardVector;

	UPROPERTY()
	TObjectPtr<UNiagaraComponent> LaserEffectComponent;

	FTimerHandle DamageTickTimerHandle;
	FTimerHandle FinishTimerHandle;
	bool bLaserInitialized = false;
	bool bLaserStarted = false;
};
