// Copyright GhostLazy


#include "ElysiaAssetManager.h"
#include "ElysiaGameplayTags.h"

UElysiaAssetManager& UElysiaAssetManager::Get()
{
	return *Cast<UElysiaAssetManager>(GEngine->AssetManager);
}

void UElysiaAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();
	
	// 预加载期初始化C++原生标签
	FElysiaGameplayTags::InitNativeGameplayTags();
}
