

#ifndef SYS_API_H
#define SYS_API_H

//include files that have required api calls
#include <ani_assert.h>
#include <halPhyApi.h>
#include <phyApi.h>
          
#include <phyDebug.h>

#ifdef VERIFY_HALPHY_SIMV_MODEL
#undef FUNC_HPHY_INIT
#undef DATA_HPHY_INIT
#undef DATA_HPHYCALBMAP_INIT
#undef DATA_HPHYPM_INIT
#define FUNC_HPHY_INIT
#define DATA_HPHY_INIT
#define DATA_HPHYCALBMAP_INIT
#define DATA_HPHYPM_INIT
#else
#define hv_printLog(v,...)
#endif

#endif
