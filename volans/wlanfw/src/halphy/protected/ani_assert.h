#ifndef ANI_ASSERT_H
#define ANI_ASSERT_H

#ifdef FEATURE_WLANFW_PHY_DEBUG
#define assert(x) COREX_ASSERT(WLANFW_MODULE_BASE, WLANFW_ASSERT_HIGH, x)
#else
#define assert(x) 
#endif
#endif

