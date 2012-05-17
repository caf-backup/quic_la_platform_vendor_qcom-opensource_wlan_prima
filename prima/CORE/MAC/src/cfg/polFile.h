//==================================================================
//
//  File:         polFile.h
//
//  Description:  Structures that define the firmware file format.                
//
//  Author:       Larry Cawley
// 
//  Copyright (c) 2011 Qualcomm Atheros, Inc.
//  All Rights Reserved. 
//  Qualcomm Atheros Confidential and Proprietary
//
//  Copyright 2002, Woodside Networks, Inc.  All rights reserved.
//
//  Change History:
//  04/09/2002 - LAC - Initial version.
//
//===================================================================
#if !defined( __polFile_h__ )
#define __polFile_h__



   
// File format
//
//  byte 0        1        2       3
//
// +---------+---------+--------+-------+                        <----+
// | Major   | Minor   |        |       |                             |
// | Version | Version | Suffix | Build |   FileVersion               |
// +---------+---------+--------+-------+                             |
// | Major   | Minor   |        |       |                             |
// | Version | Version | Suffix | Build |   HwCapabilities            | tPolFileHeader
// +---------+---------+--------+-------+                             |
// |                                    |                             |
// | FileLength                         |   FileLength                |
// +------------------------------------+                             |
// |                                    |                             |
// | Number of Directory Entries        |   NumDirectoryEntries       |
// +------------------------------------+                        <----+
// |                                    |                             |
// | Directory Entry 1 Type             |   DirEntryType              |
// +------------------------------------+                             |
// |                                    |                             | tPolFileDirEntry 1
// | Directory Entry 1 File Offset      |   DirEntryFileOffset        |
// +------------------------------------+                             |
// |                                    |                             |
// | Directory Entry 1 Length           |   DirEntryLength            |
// +------------------------------------+                        <----+
// |            . . .                   |                        . . . 
// +------------------------------------+                        <----+
// |                                    |                             |
// | Directory Entry n Type             |                             |
// +------------------------------------+                             |
// |                                    |                             | tpolFileDirEntry n
// | Directory Entry n File Offset      |                             |
// +------------------------------------+                             |
// |                                    |                             |
// | Directory Entry n Length           |                             |
// +------------------------------------+                        <----+
// |                                    |
// |                                    |
// | File data described by             |
// | directory entry 1                  |
// |                                    |
// |                                    |
// +------------------------------------+
// |            . . .                   |
// +------------------------------------+
// |                                    |
// |                                    |
// | File data described by             |
// | directory entry n                  |
// |                                    |
// |                                    |
// +---------+---------+----------------+
// |                   |
// | File Checksum     |
// +---------+---------+
//
//
//         
//


#pragma pack( push )
#pragma pack( 1 )

typedef struct sPolFileVersion {

  unsigned char  MajorVersion;
  unsigned char  MinorVersion;
  unsigned char  Suffix;
  unsigned char  Build;

} tPolFileVersion;


typedef struct sPolFileHeader {

  tPolFileVersion FileVersion;
  tPolFileVersion HWCapabilities;
  unsigned long   FileLength;
  unsigned long   NumDirectoryEntries;

} tPolFileHeader;


typedef enum ePolFileDirTypes {

  ePOL_DIR_TYPE_BOOTLOADER = 0,
  ePOL_DIR_TYPE_STA_FIRMWARE,
  ePOL_DIR_TYPE_AP_FIRMWARE,
  ePOL_DIR_TYPE_DIAG_FIRMWARE,
  ePOL_DIR_TYPE_STA_CONFIG,
  ePOL_DIR_TYPE_AP_CONFIG

} tPolFileDirTypes;


typedef struct sPolFileDirEntry {

  unsigned long DirEntryType;
  unsigned long DirEntryFileOffset;
  unsigned long DirEntryLength;

} tPolFileDirEntry;


#pragma pack( pop )


__inline unsigned short polFileChkSum( unsigned short *FileData, unsigned long NumWords )
{
  unsigned long Sum;

  for ( Sum = 0; NumWords > 0; NumWords-- ) {

    Sum += *FileData++;
  }

  Sum  = (Sum >> 16) + (Sum & 0xffff); // add carry
  Sum += (Sum >> 16);                  // maybe last unsigned short

  return( (unsigned short)( ~Sum ) );
}



#endif

