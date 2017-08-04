# Overview
Like famous Detours in Windows, AndDetours intercepts native functions by re-writing the in-memory code for target functions.   
However, it is extreme lightweight, suboptimal and mainly used for Reverse engineering.   

# Usage
```C++
#include <Extreme/AndDetours.h>
#include <sys/system_properties.h>

static void test() 
{
//	static {
		cpu_insns::init();
//	}
	
	static detours<__func_type(__system_property_get)> *sys_prop = nullptr;
	sys_prop = sys_prop->hook(__func_cast(__system_property_get), [](const char *name, char *value)->int {
		// invokes the original __system_property_get
		return sys_prop->invoke(name, value);
	});
}
```
