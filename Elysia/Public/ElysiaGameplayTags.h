// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"

/**
 * 存储C++原生标签的“单例”类
 */

struct FElysiaGameplayTags
{
public:
	static const FElysiaGameplayTags& Get() { return GameplayTags; }
	static void InitNativeGameplayTags();
protected:
	
private:
	static FElysiaGameplayTags GameplayTags;
};