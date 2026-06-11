// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "Actor/ElysiaProjectile.h"
#include "ElysiaBossProjectile.generated.h"

/**
 * Boss 使用的投射物，仅响应玩家碰撞。
 */
UCLASS()
class ELYSIA_API AElysiaBossProjectile : public AElysiaProjectile
{
	GENERATED_BODY()

public:

	AElysiaBossProjectile();
};
