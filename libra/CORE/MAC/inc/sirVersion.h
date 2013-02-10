//==================================================================
//
//  File:         sirVersion.h
//
//  Description:  Driver version information
//
//  Copyright 2008,  Qualcomm Technologies, Inc.  All rights reserved.
//
//  Change History:
//  06/24/2008 - STSAO  - Dervied from hddVersion.h
//
//===================================================================

#ifndef __SIR_VERSION_H__
#define __SIR_VERSION_H__

// force string expansion from chars                               
#define strEXPAND(x)  #x        
#define strSTRING(x)  strEXPAND(x)
#define strVERSION( _mj, _mn, _sfx, _build ) strSTRING(_mj) "." strSTRING(_mn) "." strSTRING(_sfx) "." strSTRING(_build)


#if defined( BLD_REL )
#define WNI_DRIVER_MAJOR_VERSION BLD_REL
#else 
#define WNI_DRIVER_MAJOR_VERSION             0   
#endif

#if defined( BLD_VER )
#define WNI_DRIVER_MINOR_VERSION BLD_VER
#else 
#define WNI_DRIVER_MINOR_VERSION             0
#endif

#if defined( BLD_SFX )
#define WNI_DRIVER_SUFFIX BLD_SFX 
#else
#define WNI_DRIVER_SUFFIX                    0
#endif 

#if defined( BLD_NUM )
#define WNI_DRIVER_BUILD BLD_NUM
#else 
#define WNI_DRIVER_BUILD                     0000  
#endif 

#define WNI_DRIVER_VERSION WNI_DRIVER_MAJOR_VERSION,WNI_DRIVER_MINOR_VERSION

#define WNI_DRIVER_VERSION_STR strVERSION( WNI_DRIVER_MAJOR_VERSION, WNI_DRIVER_MINOR_VERSION, WNI_DRIVER_SUFFIX, WNI_DRIVER_BUILD )
                                            

#endif  // __SIR_VERSION_H__


