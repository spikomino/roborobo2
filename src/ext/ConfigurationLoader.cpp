#include "Config/ConfigurationLoader.h"
#include <string.h>

#include "Config/BasicProjectConfigurationLoader.h"
#include "Config/DemoMedeaConfigurationLoader.h"
#include "Config/SandboxConfigurationLoader.h"
#include "Config/TestConfigurationLoader.h"
#include "Config/TopEDOConfigurationLoader.h"
#include "Config/neatestConfigurationLoader.h"
#include "Config/TestMedeaConfigurationLoader.h"
#include "Config/SimpleConfigurationLoader.h"
#include "Config/esMedeaConfigurationLoader.h"
//###DO-NOT-DELETE-THIS-LINE###TAG:INCLUDE###//


ConfigurationLoader::ConfigurationLoader()
{
	//nothing to do
}

ConfigurationLoader::~ConfigurationLoader()
{
	//nothing to do
}

ConfigurationLoader* ConfigurationLoader::make_ConfigurationLoader (std::string configurationLoaderObjectName)
{
	if (0)
	{
		// >>> Never reached
	}
#if defined PRJ_BASICPROJECT || !defined MODULAR
    else if (configurationLoaderObjectName == "BasicProjectConfigurationLoader" )
    {
        return new BasicProjectConfigurationLoader();
    }
#endif
#if defined PRJ_DEMOMEDEA || !defined MODULAR
	else if (configurationLoaderObjectName == "DemoMedeaConfigurationLoader" )
	{
	  return new DemoMedeaConfigurationLoader();
	}
#endif
#if defined PRJ_SANDBOX || !defined MODULAR
	else if (configurationLoaderObjectName == "SandboxConfigurationLoader" )
	{
		return new SandboxConfigurationLoader();
	}
#endif
#if defined PRJ_TEST || !defined MODULAR
	else if (configurationLoaderObjectName == "TestConfigurationLoader" )
	{
	  return new TestConfigurationLoader();
	}
#endif
#if defined PRJ_TOPEDO || !defined MODULAR
	else if (configurationLoaderObjectName == "TopEDOConfigurationLoader" )
	{
		return new TopEDOConfigurationLoader();
	}
#endif
#if defined PRJ_NEATEST || !defined MODULAR
	else if (configurationLoaderObjectName == "neatestConfigurationLoader" )
	{
		return new neatestConfigurationLoader();
	}
#endif
#if defined PRJ_TESTMEDEA || !defined MODULAR
	else if (configurationLoaderObjectName == "TestMedeaConfigurationLoader" )
	{
		return new TestMedeaConfigurationLoader();
	}
#endif
#if defined PRJ_SIMPLE || !defined MODULAR
	else if (configurationLoaderObjectName == "SimpleConfigurationLoader" )
	{
		return new SimpleConfigurationLoader();
	}
#endif
#if defined PRJ_ESMEDEA || !defined MODULAR
	else if (configurationLoaderObjectName == "esMedeaConfigurationLoader" )
	{
		return new esMedeaConfigurationLoader();
	}
#endif
    //###DO-NOT-DELETE-THIS-LINE###TAG:SWITCH###//
	else
	{
		return NULL;
	}

}
