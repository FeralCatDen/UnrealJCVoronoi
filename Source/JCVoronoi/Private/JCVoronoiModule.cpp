// Copyright Feral Cat Den, LLC. All Rights Reserved.

#include "Core.h"
#include "Modules/ModuleManager.h"
#include "Modules/ModuleInterface.h"

#define LOCTEXT_NAMESPACE "FJCVoronoiModule"

class FJCVoronoiModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override {}
	virtual void ShutdownModule() override {}

	static inline FJCVoronoiModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FJCVoronoiModule>("JCVoronoi");
	}

	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("JCVoronoi");
	}
};

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FJCVoronoiModule, JCVoronoi)
