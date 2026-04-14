// Copyright GhostLazy

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "ElysiaAssetManager.generated.h"

/**
 * 自定义资产管理器单例类，用于确保C++原生标签在开始初始化加载时，完成初始化
 * 
 * 确保在DefaultEngine.ini设置资产管理器配置，具体操作为：
 * 在[/Script/Engine.Engine]部分添加如下内容——
 * AssetManagerClassName=/Script/Elysia.ElysiaAssetManager
 */
UCLASS()
class ELYSIA_API UElysiaAssetManager : public UAssetManager
{
	GENERATED_BODY()
	
public:
	
	static UElysiaAssetManager& Get();
	
protected:
	
	virtual void StartInitialLoading() override;
	
};
