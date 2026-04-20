// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CombatInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UCombatInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ELYSIA_API ICombatInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	
	virtual void Die() = 0;
	virtual bool IsDead() const = 0;
	virtual bool IsPlayer() const = 0;
	virtual bool IsEnemy() const = 0;
	virtual bool HasTag(const FName Tag) const = 0;
	
};
