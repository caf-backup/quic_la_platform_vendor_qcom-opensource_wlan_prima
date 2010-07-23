#ifndef QWLANFW_VERSION_H
#define QWLANFW_VERSION_H
/*===========================================================================

FILE: 
   qwlanfw_version.h

BRIEF DESCRIPTION:
   WLAN Firmware Version file.
   Build number automaticly updated by build scripts.


                Copyright (c) 2008-2010 QUALCOMM Incorporated.
                All Right Reserved.
                Qualcomm Confidential and Proprietary
===========================================================================*/

#define WLANFW_VERSION_MAJOR            1
#define WLANFW_VERSION_MINOR            0
#define WLANFW_VERSION_PATCH            0
#define WLANFW_VERSION_EXTRA            ""
#define WLANFW_VERSION_BUILD            1


#define WLANFW_VERSION \
    WLANFW_VERSION_MAKE( \
        WLANFW_VERSION_MAJOR, \
        WLANFW_VERSION_MINOR, \
        WLANFW_VERSION_PATCH)

#define WLANFW_VERSIONSTR               "1.0.0.1"

/*---------------------------------------------------------------------------
 * Encoding of version information
 *
 * Major version : 16 bits
 * Minor version : 8 bits
 * Patch level : 8 bits
 *-------------------------------------------------------------------------*/

#define WLANFW_VERSION_MAKE(nMajor, nMinor, nPatch) \
    (((nMajor) << 16) | ((nMinor) << 8) | ((nPatch) << 0))

#endif /* WLANFW_VERSION_H */
