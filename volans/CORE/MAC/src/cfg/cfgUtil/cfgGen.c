/*
 * Copyright (C) 2007-2009 Qualcomm Technologies, Inc. All rights reserved. Proprietary and Confidential.
 */

/*
 * Woodside Networks, Inc proprietary. All rights reserved.
 * This file contains the source code for CFG utility.
 * Author:         Kevin Nguyen
 * Date:           03/19/02
 * History:-
 * 03/19/02        Created.
 * --------------------------------------------------------------------
 *
 */


#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <cfgDef.h>
#include <polFile.h>

#define INCLUDE_MIN_MAX

static int generateSource = 1;

typedef int bool;

#define MAX_CFG_BIN_SIZE        20000

#define MAX_ENUMS  80

#define TRUE    (1)
#define FALSE   (0)

#define IN_FNAME      "cfg.txt"
#define OUT_FNAME     "cfg.dat"
#define SRC_STA_FNAME "cfgParamName.c"
#define SRC_AP_FNAME  "cfgDatAp.c"
#define HDR_STA_FNAME "wniCfgSta.h"
#define HDR_AP_FNAME  "wniCfgAp.h"

#define AP_DWORD      0xdeaddead
#define STA_DWORD     0xbeefbeef

#define STA_TXT_LINE1 "/*"
#define STA_TXT_LINE2 " * IMPORTANT:  This file is for system that supports STA mode ONLY."
#define STA_TXT_LINE3 " */"
#define STA_TXT_LINE4 "#if (WNI_POLARIS_FW_PRODUCT == WLAN_STA)"
#define STA_TXT_LINE5 "#endif // (WNI_POLARIS_FW_PRODUCT == WLAN_STA)"
#define AP_TXT_LINE1  "/*"
#define AP_TXT_LINE2  " * IMPORTANT:  This file is for system that supports both STA and AP mode."
#define AP_TXT_LINE3  " */"
#define AP_TXT_LINE4  "#if (WNI_POLARIS_FW_PRODUCT == AP)"
#define AP_TXT_LINE5  "#endif // (WNI_POLARIS_FW_PRODUCT == AP)"

#define TXT_DEFINE    "#define\t"
#define TXT_MIN       "_MIN"
#define TXT_MAX       "_MAX"
#define TXT_DEF       "_DEFAULT"

#define FLG_VALID     "V"
#define FLG_INVALID   "NV"
#define FLG_INT       "I"
#define FLG_STR       "S"
#define FLG_RO        "RO"
#define FLG_RW        "RW"
#define FLG_WO        "WO"
#define FLG_XX        "XX"
#define FLG_P         "P"
#define FLG_NP        "NP"
#define FLG_RESTART   "RESTART"
#define FLG_RELOAD    "RELOAD"

#define NTF_NONE      "NONE"
#define NTF_HDD       "HDD"
#define NTF_LIM       "LIM"
#define NTF_SCH       "SCH"
#define NTF_ARQ       "ARQ"
#define NTF_DPH       "DPH"
#define NTF_HAL       "HAL"
#define NTF_SP        "SP"
#define NTF_RFP       "RFP"
#define NTF_RHP       "RHP"
#define NTF_TFP       "TFP"

#define ENUM          "#ENUM"

#define TABLE         "#TABLE"
#define TABLE_END     "#END"
#define TABLE_VAL     "#ENTRY_VALUES"

#define MAX_STR_SIZE          512            /* Max string length           */
#define MAX_PARAM_ENTRY       (4*1024)       /* Max number of parameters    */
#define MAX_TABLE_ENTRY       128            /* Max number of tables        */


unsigned char srcdir[MAX_STR_SIZE];
unsigned char dstdir[MAX_STR_SIZE];
unsigned char filename[MAX_STR_SIZE];

/* Enumerated values for line processing */
enum
{
    PARAM_TABLE_INFO,
    PARAM_COMMON_INFO,
    PARAM_FLAGS_STA,
    PARAM_NTF_STA,
    PARAM_VALUES_STA,
    PARAM_FLAGS_AP,
    PARAM_NTF_AP,
    PARAM_VALUES_AP
};


/* Enumerated values for field processing */
enum
{
    FIELD_PARAM,
    FIELD_TYPE,
    FIELD_LEN,
    FIELD_SEMID,
    FIELD_VALID,
    FIELD_RO,
    FIELD_P,
    FIELD_RESTART
};


/* Processing structure */
typedef struct
{
    unsigned char    paramTxt[MAX_STR_SIZE];           /* Parameter ID text            */
    unsigned short   paramId;                          /* 16-bit parameter ID          */
    signed long   paramLen;                         /* Parameter length             */

    /* Table information */
    unsigned long   tableIndx;                        /* Index to table array         */
    unsigned long   row;                              /* Current row in table         */

    /* STA information */
    unsigned long   staControl;                       /* Control flag for STA         */
    unsigned long   staIndex;                         /* Sem index and buffer index   */
    unsigned long   staMinVal;                        /* Minimum value for STA        */
    unsigned long   staMaxVal;                        /* Maximum value for STA        */
    unsigned long   staDefVal;                        /* Default value for STA        */
    unsigned long   staLen;                           /* Length of the string */
    unsigned char    staVal[MAX_STR_SIZE];             /* Value of the string */

    /* AP information */
    unsigned long   apControl;                        /* Control flag for AP          */
    unsigned long   apIndex;                          /* Sem index and buffer index   */
    unsigned long   apMinVal;                         /* Minimum value for AP         */
    unsigned long   apMaxVal;                         /* Maximum value for AP         */
    unsigned long   apDefVal;                         /* Default value for AP         */
    unsigned long   apLen;                            /* Length of the string */
    unsigned char    apVal[MAX_STR_SIZE];              /* Value of the string */

    unsigned char    enumStr[MAX_ENUMS][MAX_STR_SIZE];
    unsigned char    enumVal[MAX_ENUMS];
    unsigned long   numEnums;
} tProcStruct;


typedef struct
{
    unsigned char    tableTxt[MAX_STR_SIZE];
    unsigned long   rowNum;
    unsigned long   colNum;
} tTableStruct;


/* Parameter storage count */
unsigned long    gIDCount;                            /* Parameter ID counter         */
unsigned long    gSemCount;                           /* Semaphore counter            */
unsigned long    gSemCountNew;                        /* temporary variable           */
unsigned long    gStaIBufCount;                       /* STA IBuf counter             */
unsigned long    gStaSBufCount;                       /* STA SBuf counter             */
unsigned long    gApIBufCount;                        /* AP IBuf counter              */
unsigned long    gApSBufCount;                        /* AP SBuf counter              */
unsigned long    gTableCount;

static bool      gVerbose  = FALSE;

/* Processing buffers */
tProcStruct   *gParamEntry;
tTableStruct   gTableEntry[MAX_TABLE_ENTRY];


static bool
CharIsValid(unsigned char input)
{
    if (((input >= 'a') && (input <= 'z')) ||
        ((input >= 'A') && (input <= 'Z')) ||
        ((input >= '0') && (input <= '9')) ||
        (input == '_') || (input == ' ') ||
        (input == '\t') || (input == '#'))
        return TRUE;
    else
        return FALSE;
}


/**
 * SwapBytes()
 *
 * FUNCTION:
 * This function convert an unsigned 32-bit value between big and little
 * endian format
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param data:    input data
 *
 * @return Data in other format
 */
static unsigned long
SwapBytes (unsigned long data)
{
    unsigned long   temp;

    temp  =  (data        & 0xff) << 24;
    temp |= ((data >> 8)  & 0xff) << 16;
    temp |= ((data >> 16) & 0xff) << 8;
    temp |= ((data >> 24) & 0xff);

    return temp;
}

/**
 * DiscardLine()
 *
 * FUNCTION:
 * This function discards all characters until the end of the line
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param pInF:    input file pointer
 *
 * @return None
 */
static void
DiscardLine(FILE *pInF)
{
    int     input;

    input = 1;
    while ((input != '\n') && (input != EOF))
        input = fgetc(pInF);
}


/**
 * PrintHdr()
 *
 * FUNCTION:
 * This function prints the general header information for C header file.
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param pInF:    file pointer
 *
 * @return None
 */
static void
PrintHdr(FILE *pInF)
{
    time_t    now;

    now = time(NULL);

    fprintf(pInF, "/*\n");
    fprintf(pInF, " * Copyright (C) 2007-2009 Qualcomm Technologies, Inc. All rights reserved. Proprietary and Confidential.\n");
    fprintf(pInF, " */\n\n");
    fprintf(pInF, "/*\n");
    fprintf(pInF, " * DO NOT EDIT - This file is generated automaticlly\n");
    fprintf(pInF, " */\n\n");

} /*** end PrintHdr() ***/


/**
 * WriteHdrFile()
 *
 * FUNCTION:
 * This function generates the following C header files:
 *     cfgDatSta.h - Header file for STA-only
 *     cfgDatAp.h  - Header file for both STA and AP mode
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param None
 *
 * @return None
 */
static bool
WriteHdrFile(void)
{
    signed long     i, j;
    unsigned long     totalCount, rowCount, tableIndx;
    bool    procTable, tableStart;
    FILE    *pHdrSta, *pHdrAp;

    totalCount = 0;
    rowCount = 0;
    tableIndx = 0;
    procTable = 0;
    tableStart = 0;

    /* Create output header files */
    strcpy(filename, dstdir);
    strcat(filename, "/");
    strcat(filename, HDR_STA_FNAME);
    if ((pHdrSta = fopen (filename, "w")) == NULL)
    {
        printf("cfgGen:  Failed to open %s\n", filename);
        return (FALSE);
    }

    strcpy(filename, dstdir);
    strcat(filename, "/");
    strcat(filename, HDR_AP_FNAME);
    if ((pHdrAp = fopen (filename, "w")) == NULL)
    {
        printf("cfgGen:  Failed to open %s\n", filename);
        return (FALSE);
    }

    /* Prepare STA output header file */
    PrintHdr(pHdrSta);
    fprintf(pHdrSta, "%s\n", STA_TXT_LINE1);
    fprintf(pHdrSta, "%s\n", STA_TXT_LINE2);
    fprintf(pHdrSta, "%s\n", STA_TXT_LINE3);
    fprintf(pHdrSta, "\n\n");
    fprintf(pHdrSta, "#ifndef __WNICFGSTA_H\n");
    fprintf(pHdrSta, "#define __WNICFGSTA_H\n\n");
    fprintf(pHdrSta, "/*\n");
    fprintf(pHdrSta, " * Configuration Parameter ID for STA\n");
    fprintf(pHdrSta, " */\n\n");

    /* Prepare AP output header file */
    PrintHdr(pHdrAp);
    fprintf(pHdrAp, "%s\n", AP_TXT_LINE1);
    fprintf(pHdrAp, "%s\n", AP_TXT_LINE2);
    fprintf(pHdrAp, "%s\n", AP_TXT_LINE3);
    fprintf(pHdrAp, "\n\n");
    fprintf(pHdrAp, "#ifndef __WNICFGAP_H\n");
    fprintf(pHdrAp, "#define __WNICFGAP_H\n\n");
    fprintf(pHdrAp, "/*\n");
    fprintf(pHdrAp, " * Configuration Parameter ID for STA and AP\n");
    fprintf(pHdrAp, " */\n\n");

    /*
     * Output parameter definition name and ID value
     */
    for (i = 0; i < gIDCount; i++)
    {
        /* Check if this is the beginning of a table */
        if (gParamEntry[i].tableIndx != 0xffffffff) {
            tableIndx  = gParamEntry[i].tableIndx;
            procTable  = TRUE;
            tableStart = TRUE;
            totalCount = gTableEntry[tableIndx].rowNum *
                         gTableEntry[tableIndx].colNum;
        }

        /* Output parameter ID for STA only if valid */
        if ((gParamEntry[i].staControl & CFG_CTL_VALID) != 0) {
            if ((procTable) && (tableStart))
            {
                fprintf(pHdrSta, "#define %s    %d\n",
                        gTableEntry[tableIndx].tableTxt,
                        gParamEntry[i].paramId);
                fprintf(pHdrSta, "#define %s_ROW    %lu\n",
                        gTableEntry[tableIndx].tableTxt,
                        gTableEntry[tableIndx].rowNum);
                fprintf(pHdrSta, "#define %s_COL    %lu\n",
                        gTableEntry[tableIndx].tableTxt,
                        gTableEntry[tableIndx].colNum);
            }

            fprintf(pHdrSta, "#define %s    %d\n", gParamEntry[i].paramTxt,
                    gParamEntry[i].paramId);
        }

        /* Always output parameter ID for AP */
        if ((procTable) && (tableStart))
        {
            fprintf(pHdrAp, "#define %s    %d\n",
                    gTableEntry[tableIndx].tableTxt,
                    gParamEntry[i].paramId);
            fprintf(pHdrAp, "#define %s_ROW    %lu\n",
                    gTableEntry[tableIndx].tableTxt,
                    gTableEntry[tableIndx].rowNum);
            fprintf(pHdrAp, "#define %s_COL    %lu\n",
                    gTableEntry[tableIndx].tableTxt,
                    gTableEntry[tableIndx].colNum);

            /* Finished with table information */
            tableStart = FALSE;
        }

        fprintf(pHdrAp, "#define %s    %d\n", gParamEntry[i].paramTxt,
                gParamEntry[i].paramId);

        if (procTable)
            if (--totalCount == 0)
                procTable = FALSE;
    }

    /*
     * Output string length value definition
     */
    fprintf(pHdrSta, "\n/*\n");
    fprintf(pHdrSta, " * String parameter lengths \n");
    fprintf(pHdrSta, " */\n\n");
    fprintf(pHdrAp,  "\n/*\n");
    fprintf(pHdrAp, " * String parameter lengths \n");
    fprintf(pHdrAp,  " */\n\n");

    for (i = 0; i < gIDCount; i++)
    {
        /* Output string lengths for STA */
        if (((gParamEntry[i].staControl & CFG_CTL_VALID) != 0) &&
            ((gParamEntry[i].staControl & CFG_CTL_INT) == 0))
            fprintf(pHdrSta, "#define %s_LEN    %ld\n",
                    gParamEntry[i].paramTxt, gParamEntry[i].paramLen);

        /* Output string lengths for AP */
        if ((gParamEntry[i].staControl & CFG_CTL_INT) == 0)
            fprintf(pHdrAp, "#define %s_LEN    %ld\n",
                    gParamEntry[i].paramTxt, gParamEntry[i].paramLen);
    }

    /*
     * Output Min/Max/Default value definition
     */
    fprintf(pHdrSta, "\n/*\n");
    fprintf(pHdrSta, " * Integer parameter min/max/default values \n");
    fprintf(pHdrSta, " */\n\n");
    fprintf(pHdrAp,  "\n/*\n");
    fprintf(pHdrAp,  " * Integer parameter min/max/default values \n");
    fprintf(pHdrAp,  " */\n\n");

    for (i = 0; i < gIDCount; i++)
    {
        /* Check if this is the beginning of a table */
        if (gParamEntry[i].tableIndx != 0xffffffff) {
            tableIndx  = gParamEntry[i].tableIndx;
            procTable  = TRUE;
            rowCount   = 0;
            totalCount = gTableEntry[tableIndx].rowNum *
                         gTableEntry[tableIndx].colNum;
        }

        /* Output min/max/def values for STA */
        if (((gParamEntry[i].staControl & CFG_CTL_VALID) != 0) &&
            ((gParamEntry[i].staControl & CFG_CTL_INT) != 0))
        {
            if (!procTable || ((procTable) && (rowCount == 0)))
            {
                fprintf(pHdrSta, "#define %s_STAMIN    %ld\n",
                        gParamEntry[i].paramTxt, gParamEntry[i].staMinVal);
                fprintf(pHdrSta, "#define %s_STAMAX    %ld\n",
                        gParamEntry[i].paramTxt, gParamEntry[i].staMaxVal);
            }
            fprintf(pHdrSta, "#define %s_STADEF    %lu\n\n",
                    gParamEntry[i].paramTxt, gParamEntry[i].staDefVal);

            /* Output to AP header file */
            if (!procTable || ((procTable) && (rowCount == 0)))
            {
                fprintf(pHdrAp, "#define %s_STAMIN    %lu\n",
                        gParamEntry[i].paramTxt, gParamEntry[i].staMinVal);
                fprintf(pHdrAp, "#define %s_STAMAX    %lu\n",
                        gParamEntry[i].paramTxt, gParamEntry[i].staMaxVal);
            }
            fprintf(pHdrAp, "#define %s_STADEF    %lu\n\n",
                    gParamEntry[i].paramTxt, gParamEntry[i].staDefVal);
        }

        /* Output min/max/def values for AP */
        if (((gParamEntry[i].apControl & CFG_CTL_VALID) != 0) &&
            ((gParamEntry[i].apControl & CFG_CTL_INT) != 0))
        {
            if (!procTable || ((procTable) && (rowCount == 0)))
            {
                fprintf(pHdrAp, "#define %s_APMIN    %lu\n",
                        gParamEntry[i].paramTxt, gParamEntry[i].apMinVal);
                fprintf(pHdrAp, "#define %s_APMAX    %lu\n",
                        gParamEntry[i].paramTxt, gParamEntry[i].apMaxVal);
            }
            fprintf(pHdrAp, "#define %s_APDEF    %lu\n\n",
                    gParamEntry[i].paramTxt, gParamEntry[i].apDefVal);
        }

        for (j = 0; j < gParamEntry[i].numEnums; j++)
        {
            if ((gParamEntry[i].staControl & CFG_CTL_VALID) != 0)
                fprintf(pHdrSta, "#define %s_%s    %u\n",
                        gParamEntry[i].paramTxt, gParamEntry[i].enumStr[j],
                        gParamEntry[i].enumVal[j]);
            fprintf(pHdrAp, "#define %s_%s    %u\n",
                    gParamEntry[i].paramTxt, gParamEntry[i].enumStr[j],
                    gParamEntry[i].enumVal[j]);
        }
        if (j)
        {
            if ((gParamEntry[i].staControl & CFG_CTL_VALID) != 0)
                fprintf(pHdrSta, "\n");
            fprintf(pHdrAp, "\n");
        }

        if (procTable)
        {
            if (++rowCount >= gTableEntry[tableIndx].rowNum)
                rowCount = 0;

            if (--totalCount == 0)
                procTable = FALSE;
        }
    }

    fprintf(pHdrSta, "#define CFG_PARAM_MAX_NUM        %lu\n", gIDCount);
    fprintf(pHdrSta, "#define CFG_STA_IBUF_MAX_SIZE    %lu\n", gStaIBufCount);
    fprintf(pHdrSta, "#define CFG_STA_SBUF_MAX_SIZE    %lu\n", gStaSBufCount);
    fprintf(pHdrSta, "#define CFG_SEM_MAX_NUM          %lu\n\n", gSemCount);
    fprintf(pHdrSta, "#define CFG_STA_MAGIC_DWORD    0x%x\n\n", STA_DWORD);

    fprintf(pHdrAp, "#define CFG_PARAM_MAX_NUM         %lu\n", gIDCount);
    fprintf(pHdrAp, "#define CFG_AP_IBUF_MAX_SIZE      %lu\n", gApIBufCount);
    fprintf(pHdrAp, "#define CFG_AP_SBUF_MAX_SIZE      %lu\n", gApSBufCount);
    fprintf(pHdrAp, "#define CFG_STA_IBUF_MAX_SIZE     %lu\n", gStaIBufCount);
    fprintf(pHdrAp, "#define CFG_STA_SBUF_MAX_SIZE     %lu\n", gStaSBufCount);
    fprintf(pHdrAp, "#define CFG_SEM_MAX_NUM           %lu\n\n", gSemCount);
    fprintf(pHdrAp, "#define CFG_STA_MAGIC_DWORD     0x%x\n\n", STA_DWORD);
    fprintf(pHdrAp, "#define CFG_AP_MAGIC_DWORD      0x%x\n\n", AP_DWORD);

    /* Put the finishing touch on header file */
    fprintf(pHdrSta, "\n#endif\n");
    fprintf(pHdrAp, "\n#endif\n");
    fclose(pHdrSta);
    fclose(pHdrAp);
    return (TRUE);

} /*** end WriteHdrFile() ***/


/**
 * WriteSrcFile()
 *
 * FUNCTION:
 * This function generates the following C source files:
 *     cfgDatSta.c - C source file for STA-only.  This contains memory
 *                   allocation for valid parameter in STA mode only.
 *
 *     cfgDatAp.c  - C source file for both STA and AP mode.  This contains
 *                   memory allocation that can support both STA and AP.
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param None
 *
 * @return None
 */
static bool
WriteSrcFile(void)
{
    int i;
    FILE    *pSrcSta;
    /* FILE    *pSrcAp; */

    /* Create output source files */
    filename[0] = '\0';
    if (strcmp (dstdir, ".") != 0)
        strcpy(filename, "../");
    strcat(filename, SRC_STA_FNAME);
    if ((pSrcSta = fopen (filename, "w")) == NULL)
    {
        printf("cfgGen:  Failed to open %s\n", filename);
        return (FALSE);
    }

    /* Write output source file for STA */
    PrintHdr(pSrcSta);
    fprintf(pSrcSta, "%s\n", STA_TXT_LINE1);
    fprintf(pSrcSta, "%s\n", STA_TXT_LINE2);
    fprintf(pSrcSta, "%s\n", STA_TXT_LINE3);
    fprintf(pSrcSta, "#include \"cfgPriv.h\"\n");
    fprintf(pSrcSta, "\nunsigned char *gCfgParamName[] = {\n");
    for (i = 0; i < gIDCount; i++)
    {
        fprintf(pSrcSta, "\t(unsigned char *)\"%s\",\n", &gParamEntry[i].paramTxt[8]);
    }
    fprintf(pSrcSta, "};\n\n");
    fprintf(pSrcSta, "\n\n");
    fclose(pSrcSta);

    /* Write output source file for AP */
    /*
    strcpy(filename, dstdir);
    strcat(filename, "/");
    strcat(filename, SRC_AP_FNAME);
    if ((pSrcAp = fopen (filename, "w")) == NULL)
    {
        printf("cfgGen:  Failed to open %s\n", filename);
        return (FALSE);
    }

    PrintHdr(pSrcAp);
    fprintf(pSrcAp, "\n\n%s\n", AP_TXT_LINE1);
    fprintf(pSrcAp, "%s\n", AP_TXT_LINE2);
    fprintf(pSrcAp, "%s\n", AP_TXT_LINE3);
    fprintf(pSrcAp, "\n\n");

    fclose(pSrcAp);
    */
    return (TRUE);

} /*** end WriteSrcFile() ***/


/**
 * OpenBinaryOutput ()
 *
 * FUNCTION:
 * This function opens a binary file for writing.
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param None
 *
 * @return None
 */

static FILE *
OpenBinaryOutput (char *dir, char *name)
{
    FILE *      fp;

    strcpy (filename, dir);
    strcat (filename, "/");
    strcat (filename, name);
    if ((fp = fopen (filename, "wb")) == NULL)
    {
        printf("cfgGen:  Failed to open %s\n", filename);
        exit (1);
    }
    return fp;
}

/**
 * ValArray
 *
 * FUNCTION:
 * These functions provide a array of unsigned longs.
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param None
 *
 * @return None
 */

typedef unsigned int    ValueU32;

typedef struct
{
    ValueU32            data[MAX_CFG_BIN_SIZE];
    int                 dataUsed;

} ValArray;

static void
ValArray_Init (ValArray *array)
{
    array->dataUsed = 0;
}

static int
ValArray_GetSize (ValArray *array)
{
    return array->dataUsed * sizeof (ValueU32);
}

static void
ValArray_PutValue (ValArray *array, ValueU32 val)
{
    array->data[array->dataUsed++] = val;
}

static void
ValArray_PutArray (ValArray *dest, ValArray *src)
{
    memcpy (&dest->data[dest->dataUsed],
            src->data,
            src->dataUsed * sizeof (ValueU32));
    dest->dataUsed += src->dataUsed;
}

static void
ValArray_Write (ValArray *array, bool swap, FILE *fp)
{
    int                 i;
    ValueU32            tmp;

    if (!swap)
        fwrite (array->data, sizeof (ValueU32), array->dataUsed, fp);
    else
    {
        for (i = 0; i < array->dataUsed; ++i)
        {
            tmp = SwapBytes (array->data[i]);
            fwrite (&tmp, sizeof (ValueU32), 1, fp);
        }
    }
}

/**
 * SetStaData
 *
 * FUNCTION:
 * This function setups the array of longs to be written to wniCfgSta.bin
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param None
 *
 * @return None
 */

static void
SetStaData (ValArray *data)
{
    int                 i;
    int                 j;
    unsigned long       staStrLen = 0;

    for (i = 0; i < gIDCount; i++)
    {
        if (((gParamEntry[i].staControl & CFG_CTL_VALID) != 0) &&
            ((gParamEntry[i].staControl & CFG_CTL_INT) == 0))
            staStrLen += 4 + (((gParamEntry[i].staLen + 3) >> 2) << 2);
    }

    /* Write STA header */
    ValArray_PutValue (data, STA_DWORD);
    ValArray_PutValue (data, STA_DWORD);
    ValArray_PutValue (data, gIDCount);
    ValArray_PutValue (data, gStaIBufCount);
    ValArray_PutValue (data, staStrLen);

    /* Write STA control flags to STA binary file */
    for (i = 0; i < gIDCount; i++)
    {
        ValArray_PutValue (data, gParamEntry[i].staControl |
                                  gParamEntry[i].staIndex);
    }

    /* Write STA default values */
    for (i = 0; i < gIDCount; i++)
    {
        /* Write to output only if parameter is valid and integer */
        if (((gParamEntry[i].staControl & CFG_CTL_VALID) != 0) &&
            ((gParamEntry[i].staControl & CFG_CTL_INT) != 0))
        {
            ValArray_PutValue (data, gParamEntry[i].staDefVal);
        }
    }

#ifdef INCLUDE_MIN_MAX
    /* Write STA min values */
    for (i = 0; i < gIDCount; i++)
    {
        /* Write to output only if parameter is valid and integer */
        if (((gParamEntry[i].staControl & CFG_CTL_VALID) != 0) &&
            ((gParamEntry[i].staControl & CFG_CTL_INT) != 0))
        {
            ValArray_PutValue (data, gParamEntry[i].staMinVal);
        }
    }
    /* Write STA max values */
    for (i = 0; i < gIDCount; i++)
    {
        /* Write to output only if parameter is valid and integer */
        if (((gParamEntry[i].staControl & CFG_CTL_VALID) != 0) &&
            ((gParamEntry[i].staControl & CFG_CTL_INT) != 0))
        {
            ValArray_PutValue (data, gParamEntry[i].staMaxVal);
        }
    }
#endif

    /* Write STA default str values */
    for (i = 0; i < gIDCount; i++)
    {
        /* Write to output only if parameter is valid and string */
        if (((gParamEntry[i].staControl & CFG_CTL_VALID) != 0) &&
            ((gParamEntry[i].staControl & CFG_CTL_INT) == 0))
        {
            /* TYPE,Length */
            ValArray_PutValue (data, (i << 16) | gParamEntry[i].staLen);
            /* Value */
            for (j=0; j < ((gParamEntry[i].staLen + 3) >> 2); j++)
            {
                ValArray_PutValue (data,
                                 gParamEntry[i].staVal[4*j] << 24 |
                                 gParamEntry[i].staVal[4*j+1] << 16 |
                                 gParamEntry[i].staVal[4*j+2] << 8 |
                                 gParamEntry[i].staVal[4*j+3]);
            }
        }
    }
}

/**
 * SetApData
 *
 * FUNCTION:
 * This function setups the array of longs to be written to wniCfgAp.bin
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param None
 *
 * @return None
 */

static void
SetApData (ValArray *data)
{
    int                 i;
    int                 j;
    unsigned long       apStrLen = 0;

    for (i = 0; i < gIDCount; i++)
    {
        if (((gParamEntry[i].apControl & CFG_CTL_VALID) != 0) &&
            ((gParamEntry[i].apControl & CFG_CTL_INT) == 0))
            apStrLen += 4 + (((gParamEntry[i].apLen + 3) >> 2) << 2);
    }

    /* Write AP header */
    ValArray_PutValue (data, AP_DWORD);
    ValArray_PutValue (data, AP_DWORD);
    ValArray_PutValue (data, gIDCount);
    ValArray_PutValue (data, gApIBufCount);
    ValArray_PutValue (data, apStrLen);

    /* Write control flags for AP binary file */
    for (i = 0; i < gIDCount; i++)
    {
        ValArray_PutValue (data, gParamEntry[i].apControl |
                                        gParamEntry[i].apIndex);
    }

    /* Write AP default values */
    for (i = 0; i < gIDCount; i++)
    {
        if ((gParamEntry[i].staControl & CFG_CTL_INT) != 0)
        {
            ValArray_PutValue (data, gParamEntry[i].apDefVal);
        }
    }

#ifdef INCLUDE_MIN_MAX
    /* Write AP min values */
    for (i = 0; i < gIDCount; i++)
    {
        if ((gParamEntry[i].staControl & CFG_CTL_INT) != 0)
        {
            ValArray_PutValue (data, gParamEntry[i].apMinVal);
        }
    }

    /* Write AP max values */
    for (i = 0; i < gIDCount; i++)
    {
        if ((gParamEntry[i].staControl & CFG_CTL_INT) != 0)
        {
            ValArray_PutValue (data, gParamEntry[i].apMaxVal);
        }
    }
#endif

    /* Write AP default str values */
    for (i = 0; i < gIDCount; i++)
    {
        /* Write to output only if parameter is valid and string */
        if (((gParamEntry[i].apControl & CFG_CTL_VALID) != 0) &&
            ((gParamEntry[i].apControl & CFG_CTL_INT) == 0))
        {
            /* TYPE,Length */
            ValArray_PutValue (data, (i << 16) | gParamEntry[i].apLen);
            /* Value */
            for (j=0; j < ((gParamEntry[i].apLen + 3) >> 2); j++)
            {
                ValArray_PutValue (data,
                                 gParamEntry[i].apVal[4*j] << 24 |
                                 gParamEntry[i].apVal[4*j+1] << 16 |
                                 gParamEntry[i].apVal[4*j+2] << 8 |
                                 gParamEntry[i].apVal[4*j+3]);
            }
        }
    }
}

/**
 * SetMergeData
 *
 * FUNCTION:
 * This function sets an array of longs representing the config data
 *
 * Replaces the os/WinHost/MergeBin utility.
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param None
 *
 * @return None
 */

static void
SetMergeData (ValArray *staData,
              ValArray *apData,
              ValArray *cfgBinData,
              bool swapBytes,
              unsigned short *chkSum)
{
    int                 i;
    int                 numFiles;
    int                 offset;
    int                 checkSumSize;
    int                 staFileSize;
    int                 apFileSize;
    int                 totalFileSize;

    numFiles      = 2;
    checkSumSize  = sizeof (unsigned short);
    staFileSize   = ValArray_GetSize (staData);
    apFileSize    = ValArray_GetSize (apData);
    offset        = (4 + numFiles * 3) * sizeof (ValueU32);
    totalFileSize = offset           /* header size */
                  + staFileSize      /* wniCfgSta.bin size */
                  + apFileSize       /* wniCfgAp.bin size */
                  + checkSumSize;    /* check sum appended at end */

    ValArray_PutValue (cfgBinData, 0x0b030c01); /* FileVersion */
    ValArray_PutValue (cfgBinData, 0x01020001); /* HwCapabilities */
    ValArray_PutValue (cfgBinData, totalFileSize);
    ValArray_PutValue (cfgBinData, numFiles);

    ValArray_PutValue (cfgBinData, ePOL_DIR_TYPE_STA_CONFIG);
    ValArray_PutValue (cfgBinData, offset);
    ValArray_PutValue (cfgBinData, staFileSize);
    offset += staFileSize;

    ValArray_PutValue (cfgBinData, ePOL_DIR_TYPE_AP_CONFIG);
    ValArray_PutValue (cfgBinData, offset);
    ValArray_PutValue (cfgBinData, apFileSize);

    ValArray_PutArray (cfgBinData, staData);
    ValArray_PutArray (cfgBinData, apData);

                                /*
                                 * Historically MergeBin reswapped
                                 * bytes read from wniCfgSta.bin and
                                 * wniCfgAp.bin that is why we have
                                 * to swap bytes if we are running on
                                 * a big endian system.
                                 */
    if (!swapBytes)
    {
        for (i = 0; i < cfgBinData->dataUsed; ++i)
            cfgBinData->data[i] = SwapBytes (cfgBinData->data[i]);
    }

    *chkSum = polFileChkSum ((unsigned short *) cfgBinData->data,
                                                cfgBinData->dataUsed * 2);
    if (!swapBytes)
    {
        *chkSum = ((*chkSum & 0xff00) >> 8) |
                  ((*chkSum & 0x00ff) << 8);
    }
}

/**
 * WriteBinFiles()
 *
 * FUNCTION:
 * This function generates the following binary files:
 *     sta.bin       - Binary image containing control values as well as
 *                     default values for STA only.
 *
 *     ap.bin        - Binary image containing control values as well as
 *                     default values for AP only.
 *
 *     cfg.bin       - Binary merge of the sta.bin and ap.bin
 *     (aka cfg.dat in WM)
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param None
 *
 * @return None
 */

static void
WriteBinFiles(void)
{
    FILE *              fp;
    bool                swapBytes;
    ValArray            staData;
    ValArray            apData;
    ValArray            cfgBinData;
    unsigned short      chkSum;
    unsigned long       byteOrder = 0x12345678;
    unsigned char *     p = (unsigned char *) &byteOrder;

                                /*
                                 * Should we swap bytes written to wni*.bin
                                 */
    swapBytes = (p[0] == 0x12) ? 0 : 1;

    ValArray_Init (&staData);
    ValArray_Init (&apData);
    ValArray_Init (&cfgBinData);

                                /* Set up binary file values */
    SetStaData (&staData);
    SetApData  (&apData);
    SetMergeData (&staData, &apData, &cfgBinData, swapBytes, &chkSum);

                                /* Write STA bin file */
    fp = OpenBinaryOutput (dstdir, "wniCfgSta.bin");
    ValArray_Write (&staData, swapBytes, fp);
    fclose (fp);

                                /* Write AP bin file */
    fp = OpenBinaryOutput (dstdir, "wniCfgAp.bin");
    ValArray_Write (&apData, swapBytes, fp);
    fclose (fp);

                                /* Write cfg.bin file */
    fp = OpenBinaryOutput (dstdir, OUT_FNAME);
    ValArray_Write (&cfgBinData, FALSE, fp);
    fwrite (&chkSum, sizeof (unsigned short), 1, fp);
    fclose (fp);

} /*** WriteBinFile() ***/


/**
 * GetLine()
 *
 * FUNCTION:
 * This function reads the next line from the input stream and copies to
 * the specifies buffer.
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param pInF:  input file pointer
 * @param pBuf:  data buffer
 *
 * @return None
 */
static int
GetLine(FILE *pInF, char *pBuf)
{
    int     len;
    int     input;
    bool    validChar;

    len   = 0;
    validChar = FALSE;

    /* Skip all white spaces */
    while (!validChar)
    {
        /* Check for EOF */
        input = fgetc(pInF);
        if (input == EOF)
            return -1;
        else
            *pBuf = (char)input;

        /* Check for comment */
        if (*pBuf == '*')
            DiscardLine(pInF);
        else if ((*pBuf != ' ') && (*pBuf != '\r') &&
                 (*pBuf != '\t') && (*pBuf != '\n'))
            validChar = TRUE;
    }

    /* Read until the end of the line */
    while (CharIsValid(*pBuf))
    {
        pBuf++;
        len++;
        input = fgetc(pInF);
        *pBuf = (char)input;
        if (*pBuf == '*')
        {
            *pBuf = 0;
            return len;
        }
    }
    *pBuf = 0;
    return len;

} /*** end GetLine() ***/


/**
 * GetNext()
 *
 * FUNCTION:
 * This function reads the next token in the specified buffer and copies
 * to the data buffer.
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param pInput:  input buffer
 * @param pBuf:    data buffer
 * @param ppInput: address of variable to return advanced pointer
 *
 * @return None
 */
static int
GetNext(unsigned char *pInput, unsigned char *pBuf, unsigned char **ppInput)
{
    int len;

    len   = 0;

    /* Discard all white spaces */
    while ((*pInput == ' ') || (*pInput == '\t'))
        pInput++;

    /* Get next token */
    while ((*pInput != ' ') && (*pInput != '\t') &&
           (*pInput != '*') && (*pInput != 0))
    {
        *pBuf++ = *pInput++;
        len++;
    }

    *pBuf = 0;

    /* Advance pointer if we need to */
    if (ppInput != 0)
        *ppInput = pInput;

    return len;
}


/*
 * ProcessCommonInfo()
 *
 *FUNCTION:
 * This function processes parameter common information.
 *
 *LOGIC:
 *
 *ASSUMPTIONS:
 *
 *NOTE:
 * @param pBuf:       input buffer
 * @param pCtl:       address of control structure entry
 * @param pSemCount:  address of semaphore counter
 *
 * @return None
 */
static void
ProcessCommonInfo(unsigned char *pBuf, tProcStruct *pCtl, unsigned long *pSemCount)
{
    unsigned long   semId;
    unsigned long   field;
    unsigned char    buf[MAX_STR_SIZE];

    field = FIELD_PARAM;
    while (GetNext(pBuf, buf, &pBuf) > 0)
    {
        switch (field++)
        {
            case FIELD_PARAM:
                /* Copy parameter name */
                strcpy(pCtl->paramTxt, buf);
                break;

            case FIELD_TYPE:
                /* Set parameter type */
                pCtl->staControl = 0;
                if (!strcmp(buf, FLG_INT))
                    pCtl->staControl |= CFG_CTL_INT;
                break;

            case FIELD_LEN:
                /* Set parameter length */
                pCtl->paramLen = strtol(buf, 0, 0);
                break;

            case FIELD_SEMID:
                /* Set SEM_INDX */
                semId = strtol(buf, 0, 0);
                if (semId > 0)
                {
                    pCtl->staControl |= ((semId << CFG_SEM_INDX_SHIFT) &
                                         CFG_SEM_INDX_MASK);
                    *pSemCount = semId;
                }
                break;

            default:
                pCtl->apControl = pCtl->staControl;
                return;
        }
    }

    /* Copy flags for AP control */
    pCtl->apControl = pCtl->staControl;

} /*** end ProcessFlags() ***/


/*
 * ProcessFlags()
 *
 * FUNCTION:
 * This function processes an input line and sets the corresponding flags in
 * the control field.  If buffer is required, the buffer counter (I or S)
 * will be incremented.
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 * For STA only, the chkValid flag will be TRUE so that no buffer will be
 * allocated if a parameter is invalid in STA mode.  For AP/STA, this flag
 * will be FALSE so that buffer will always be allocated.
 *
 * @param pBuf:      input buffer
 * @param pControl:  address to write control field
 * @param pBufIdx:   address of buffer index variable
 * @param paramLen:  parameter length
 * @param pICount:   address of IBuffer counter
 * @param pSCount:   address of SBuffer counter
 * @param chkValid:  flag to enable validity check
 *
 * @return    None
 */
static void
ProcessFlags(unsigned char *pBuf, unsigned long *pControl, unsigned long *pBufIdx, unsigned long paramLen,
             unsigned long *pICount, unsigned long *pSCount, bool chkValid)
{
    unsigned long   len;
    unsigned long   field;
    unsigned char    buf[MAX_STR_SIZE];

    field = FIELD_VALID;
    while ((len = GetNext(pBuf, buf, &pBuf)) > 0)
    {
        switch (field++)
        {
            case FIELD_VALID:
                /* Set valid flag */
                if (!(strcmp(buf, FLG_VALID)))
                    *pControl |= CFG_CTL_VALID;

                /* Allocate buffer if valid or no check required */
                if (((*pControl & CFG_CTL_VALID) != 0) || !chkValid)
                {
                    if (!(*pControl & CFG_CTL_INT))
                    {
                        *pBufIdx = (*pSCount) & CFG_BUF_INDX_MASK;
                        *pSCount += (paramLen + 1 + 1);
                    }
                    else
                    {
                        *pBufIdx = (*pICount) & CFG_BUF_INDX_MASK;
                        (*pICount)++;
                    }
                }
                break;

            case FIELD_RO:
                /* Set RO flag */
                if (!(strcmp(buf, FLG_RO)))
                    *pControl |= CFG_CTL_RE;
                else if (!(strcmp(buf, FLG_RW)))
                    *pControl |= (CFG_CTL_RE | CFG_CTL_WE);
                else if (!(strcmp(buf, FLG_WO)))
                    *pControl |= CFG_CTL_WE;

                break;

            case FIELD_P:
                /* Set persistent flag */
                if (!(strcmp(buf, FLG_P)))
                    *pControl |= CFG_CTL_SAVE;
                break;

            case FIELD_RESTART:
                if (!strcmp(buf, FLG_RESTART))
                    *pControl |= CFG_CTL_RESTART;
                else if (!strcmp(buf, FLG_RELOAD))
                    *pControl |= CFG_CTL_RELOAD;
                else
                {
                    printf("ERROR : invalid flag %s\n", buf);
                    exit(1);
                }
                break;

            default:
                return;
        }
    }
}


/*
 * ProcessNtf()
 *
 * FUNCTION:
 * This function processes the notification flags.
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param pBuf:      input buffer
 * @param pControl:  address to write control field
 *
 * @return    None
 */
static void
ProcessNtf(unsigned char *pBuf, unsigned long *pControl)
{
    unsigned char        buf[MAX_STR_SIZE];
    signed long        len;

    /* Process till the end of the line */
    while ((len = GetNext(pBuf, buf, &pBuf)) > 0)
    {
        if (!strcmp(buf, NTF_NONE))
            return;
        else if (!strcmp(buf, NTF_HDD))
            *pControl |= CFG_CTL_NTF_HDD;
        else if (!strcmp(buf, NTF_LIM))
            *pControl |= CFG_CTL_NTF_LIM;
        else if (!strcmp(buf, NTF_SCH))
            *pControl |= CFG_CTL_NTF_SCH;
        else if (!strcmp(buf, NTF_ARQ))
            *pControl |= CFG_CTL_NTF_ARQ;
        else if (!strcmp(buf, NTF_DPH))
            *pControl |= CFG_CTL_NTF_DPH;
        else if (!strcmp(buf, NTF_HAL))
            *pControl |= CFG_CTL_NTF_HAL;
        else if (!strcmp(buf, NTF_SP))
            *pControl |= CFG_CTL_NTF_SP;
        else if (!strcmp(buf, NTF_RFP))
            *pControl |= CFG_CTL_NTF_RFP;
        else if (!strcmp(buf, NTF_RHP))
            *pControl |= CFG_CTL_NTF_RHP;
        else if (!strcmp(buf, NTF_TFP))
            *pControl |= CFG_CTL_NTF_TFP;
    }

} /*** end ProcessNtf() ***/


/*
 * ProcessValues()
 *
 * FUNCTION:
 * This function processes Min/Max/Default values.
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param pBuf:  input buffer
 * @param pMin:  address to write min value
 * @param pMax:  address to write max value
 * @param pDef:  address to write default value
 *
 * @return    None
 */
static void
ProcessValues(unsigned char *pBuf, unsigned long *pMin, unsigned long *pMax, unsigned long *pDef)
{
    unsigned char        buf[MAX_STR_SIZE];

    /* Get minimum value */
    GetNext(pBuf, buf, &pBuf);
    *pMin = strtol(buf, 0, 0);

    /* Get maximum value */
    GetNext(pBuf, buf, &pBuf);
    *pMax = strtol(buf, 0, 0);

    /* Get default value */
    GetNext(pBuf, buf, &pBuf);
    *pDef = strtol(buf, 0, 0);

} /*** end ProcessValues() ***/


/*
 * ProcessStringValues()
 *
 * FUNCTION:
 * This function processes Min/Max/Default values.
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param pBuf:  input buffer
 * @param pMin:  address to write min value
 * @param pMax:  address to write max value
 * @param pDef:  address to write default value
 *
 * @return    None
 */
static void
ProcessStringValues(unsigned char *pBuf, unsigned long *pLen, unsigned char *val)
{
    unsigned char        buf[MAX_STR_SIZE];
    unsigned long       i;

    GetNext(pBuf, buf, &pBuf);
    *pLen = strtol(buf, 0, 0);
    if (*pLen > MAX_STR_SIZE)
    {
        printf("ERROR : String length > MAX_STR_SIZE (%d)\n", MAX_STR_SIZE);
        *pLen = MAX_STR_SIZE;
    }
    for (i=0; i<*pLen; i++)
    {
        GetNext(pBuf, buf, &pBuf);
        val[i] = strtol(buf, 0, 0);
    }

} /*** end ProcessStringValues() ***/


/*
 * ReplicateEntry()
 *
 * FUNCTION:
 * This function replicates the specified entry n times.  This is called
 * when processing a table.  One entry will be replicated n time, with n
 * being the number of rows.
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param startIndx:  index of entry to replicate
 * @param count:      number of time to replicate
 *
 * @return None
 */
static void
ReplicateEntry(unsigned long startIndx, unsigned long count)
{
    unsigned char    buf[4];
    unsigned long   i, row;

    row = gParamEntry[startIndx].row + 1;

    for (i = startIndx + 1; i <= (startIndx + count); i++)
    {
        strcpy(gParamEntry[i].paramTxt, gParamEntry[startIndx].paramTxt);
        gParamEntry[i].paramId    = ++gIDCount;
        gParamEntry[i].paramLen   = gParamEntry[startIndx].paramLen;
        gParamEntry[i].tableIndx  = 0xffffffff;
        gParamEntry[i].row        = row++;
        gParamEntry[i].staControl = gParamEntry[startIndx].staControl;
        gParamEntry[i].staMinVal  = gParamEntry[startIndx].staMinVal;
        gParamEntry[i].staMaxVal  = gParamEntry[startIndx].staMaxVal;
        gParamEntry[i].staDefVal  = gParamEntry[startIndx].staDefVal;
        gParamEntry[i].apControl  = gParamEntry[startIndx].apControl;
        gParamEntry[i].apMinVal   = gParamEntry[startIndx].apMinVal;
        gParamEntry[i].apMaxVal   = gParamEntry[startIndx].apMaxVal;
        gParamEntry[i].apDefVal   = gParamEntry[startIndx].apDefVal;

        /* Allocate buffer for STA only if valid */
        if ((gParamEntry[i].staControl & CFG_CTL_VALID) != 0)
        {
            if (!(gParamEntry[i].staControl & CFG_CTL_INT))
            {
                gParamEntry[i].staIndex = gStaSBufCount & CFG_BUF_INDX_MASK;
                gStaSBufCount += (gParamEntry[i].paramLen + 1 + 1);
            }
            else
            {
                gParamEntry[i].staIndex = gStaIBufCount & CFG_BUF_INDX_MASK;
                gStaIBufCount++;
            }
        }

        /* Allocate buffer for AP */
        if (!(gParamEntry[i].apControl & CFG_CTL_INT))
        {
            gParamEntry[i].apIndex = gApSBufCount & CFG_BUF_INDX_MASK;
            gApSBufCount += (gParamEntry[i].paramLen + 1 + 1);
        }
        else
        {
            gParamEntry[i].apIndex = gApIBufCount & CFG_BUF_INDX_MASK;
            gApIBufCount++;
        }
    }

    /* Insert row number into parameter text string */
    row = 1;
    for (i = startIndx; i < (startIndx + count + 1); i++)
    {
        sprintf(buf, "_%lu", row++);
        strcat(gParamEntry[i].paramTxt, buf);

    }

} /*** end ReplicateEntry() ***/


/*
 * Main()
 *
 * FUNCTION:
 * This is the main entry.
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param None
 *
 * @return  0 - Successful
 * @return -1 - Failed
 */
int main(int argc, char *argv[])
{
    unsigned char      inBuf[MAX_STR_SIZE];
    unsigned char      tmpBuf[MAX_STR_SIZE];
    unsigned char      *pTmp;
    signed long     inLen, currLine;
    unsigned long     i, rowCount, tmpIDCount;
    bool    procTable = FALSE;
    FILE    *pInFile;

    rowCount = 0;
    tmpIDCount = 0;

    srcdir[0] = dstdir[0] = '.';
    srcdir[1] = dstdir[1] = 0;
    for (i=1; i<argc; i++)
    {
        if (strcmp(argv[i], "-b") == 0)
           generateSource = 0;
        else if (strcmp(argv[i], "-s") == 0)
        {
            i++;
            if (i >= argc)
            {
                printf("Too few arguments to -s\n");
                exit (1);
            }
            strcpy(srcdir, argv[i]);
        }
        else if (strcmp(argv[i], "-d") == 0)
        {
            i++;
            if (i >= argc)
            {
                printf("Too few arguments to -d\n");
                exit (1);
            }
            strcpy(dstdir, argv[i]);
        }
        else if (strcmp (argv[i], "-v") == 0)
        {
            gVerbose = TRUE;
        }
        else
        {
            printf("Invalid switch : %s\n\n", argv[1]);

            printf("Usage : %s [-b] [-v] [-s src dir] [-d dst dir]\n", argv[0]);
            printf("\t-b to generate only binaries\n");
            printf("\t-s to specify source directory (for cfg.txt)\n");
            printf("\t-d to specify destination directory (for outputting bin files)\n");
            printf("\t-v verbose\n");
            exit (1);
        }
    }

    if (gVerbose)
    {
        printf("** Generating big endian output\n");
        printf("SRC : %s DST : %s\n", srcdir, dstdir);
    }

    gParamEntry = (tProcStruct*)malloc(sizeof(tProcStruct) * 1024);

    if (gParamEntry == NULL) {
	    printf("Malloc Failed\n");
	    return -1;
    }

    /* Open input data file */
    strcpy(filename, srcdir);
    strcat(filename, "/");
    strcat(filename, IN_FNAME);
    if ((pInFile = fopen (filename, "r")) == NULL)
    {
        printf("cfgGen:  Failed to open %s\n", filename);
        return -1;
    }

    gIDCount      = 0;
    gSemCount     = 0;
    gStaIBufCount = 0;
    gStaSBufCount = 0;
    gApIBufCount  = 0;
    gApSBufCount  = 0;
    gTableCount   = 0;

    /* Process each line until EOF is reached */
    currLine   = PARAM_TABLE_INFO;
    while (TRUE)
    {
        inLen = GetLine(pInFile, inBuf);

        /* Check for EOF */
        if (inLen == -1)
            break;

        switch (currLine)
        {
            case PARAM_TABLE_INFO:
                /* Check if this is the beginning of the table */
                GetNext(inBuf, tmpBuf, NULL);

                /* Check for begin of table */
                if (!(strcmp(tmpBuf, TABLE)))
                {
                    procTable = TRUE;
                    pTmp = inBuf;

                    /* Save table text */
                    GetNext(pTmp, tmpBuf, &pTmp);
                    GetNext(pTmp, tmpBuf, &pTmp);
                    strcpy(gTableEntry[gTableCount].tableTxt, tmpBuf);

                    /* Get number of rows */
                    GetNext(pTmp, tmpBuf, &pTmp);
                    rowCount = strtol(tmpBuf, 0, 0);

                    /* Save to table entry */
                    gTableEntry[gTableCount].rowNum = rowCount;
                    gTableEntry[gTableCount].colNum = 0;

                    gParamEntry[gIDCount].tableIndx = gTableCount;

                    currLine++;
                    break;
                }
                /* Check for end of table */
                else if (!(strcmp(tmpBuf, TABLE_END)))
                {
                    if (!procTable)
                    {
                        printf("cfgGen:  Error processing table\n");
                        return(-1);
                    }
                    procTable = FALSE;
                    gTableCount++;
                    break;
                }
                /* Check for entry initialization values */
                else if (!(strcmp(tmpBuf, TABLE_VAL)))
                {
                    pTmp = inBuf;
                    GetNext(pTmp, tmpBuf, &pTmp);
                    GetNext(pTmp, tmpBuf, &pTmp);
                    i = strtol(tmpBuf, 0, 0) - 1;

                    /* Process STA default values */
                    GetLine(pInFile, inBuf);
                    ProcessValues(inBuf,
                                  &gParamEntry[tmpIDCount + i].staMinVal,
                                  &gParamEntry[tmpIDCount + i].staMaxVal,
                                  &gParamEntry[tmpIDCount + i].staDefVal);

                    /* Process AP default values */
                    GetLine(pInFile, inBuf);
                    ProcessValues(inBuf,
                                  &gParamEntry[tmpIDCount + i].apMinVal,
                                  &gParamEntry[tmpIDCount + i].apMaxVal,
                                  &gParamEntry[tmpIDCount + i].apDefVal);

                    break;
                }
                else if (!(strcmp(tmpBuf, ENUM)))
                {
                    unsigned long index;
                    if (gIDCount < 1)
                    {
                        printf("Parse error : gIDCount = %lu\n", gIDCount);
                        exit(1);
                    }
                    index = gParamEntry[gIDCount-1].numEnums;
                    if (index >= MAX_ENUMS)
                    {
                        printf("Parse error : Too many enums %lu\n", index);
                        exit(1);
                    }

                    pTmp = inBuf;
                    GetNext(pTmp, tmpBuf, &pTmp);
                    GetNext(pTmp, tmpBuf, &pTmp);
                    strcpy(gParamEntry[gIDCount-1].enumStr[index], tmpBuf);
                    if (gVerbose)
                    {
                        printf("#define %s_%s ", gParamEntry[gIDCount-1].paramTxt, tmpBuf);
                    }
                    GetNext(pTmp, tmpBuf, &pTmp);
                    i = strtol(tmpBuf, 0, 0);
                    if ((i < gParamEntry[gIDCount-1].staMinVal ||
                         i > gParamEntry[gIDCount-1].staMaxVal) ||
                        (i < gParamEntry[gIDCount-1].apMinVal ||
                         i > gParamEntry[gIDCount-1].apMaxVal))
                    {
                        printf("Parse error : value %lu out of valid range\n", i);
                        exit(1);
                    }
                    gParamEntry[gIDCount-1].enumVal[index] = i;
                    if (gVerbose)
                        printf("%lu\n", i);

                    gParamEntry[gIDCount-1].numEnums++;
                    break;
                }

                /* Not table, proceed with common info processing */
                gParamEntry[gIDCount].tableIndx = 0xffffffff;
                currLine = PARAM_COMMON_INFO;

            case PARAM_COMMON_INFO:
                /* Process common information */

                /* If we are processing table, increment column # */
                if (procTable)
                    gTableEntry[gTableCount].colNum++;

                ProcessCommonInfo(inBuf, &gParamEntry[gIDCount], &gSemCountNew);
                if (gSemCountNew > gSemCount)
                    gSemCount = gSemCountNew;
                if (gVerbose)
                    printf("%s ",gParamEntry[gIDCount].paramTxt);
                gParamEntry[gIDCount].paramId = gIDCount;
                gParamEntry[gIDCount].numEnums = 0;
                currLine++;
                break;

            case PARAM_FLAGS_STA:
                /* Process control flags for STA */
                ProcessFlags(inBuf, &gParamEntry[gIDCount].staControl,
                             &gParamEntry[gIDCount].staIndex,
                             gParamEntry[gIDCount].paramLen, &gStaIBufCount,
                             &gStaSBufCount, TRUE);
                currLine++;
                break;

            case PARAM_NTF_STA:
                /* Process notification flags */
                ProcessNtf(inBuf, &gParamEntry[gIDCount].staControl);
                currLine++;
                break;

            case PARAM_VALUES_STA:
                if (gParamEntry[gIDCount].staControl & CFG_CTL_INT)
                {
                    /* Get min/max/default values */
                    ProcessValues(inBuf, &gParamEntry[gIDCount].staMinVal,
                                  &gParamEntry[gIDCount].staMaxVal,
                                  &gParamEntry[gIDCount].staDefVal);
                    if (gVerbose)
                        printf("** Integer %lu [%lu,%lu]\n",
                           gParamEntry[gIDCount].staDefVal,
                           gParamEntry[gIDCount].staMinVal,
                           gParamEntry[gIDCount].staMaxVal);

                    if (gParamEntry[gIDCount].staDefVal < gParamEntry[gIDCount].staMinVal)
                    {
                        printf("ERROR : id %lu STA default val %lu < min %lu, using min\n", gIDCount,
                               gParamEntry[gIDCount].staDefVal, gParamEntry[gIDCount].staMinVal);
                        gParamEntry[gIDCount].staDefVal = gParamEntry[gIDCount].staMinVal;
                    }
                    if (gParamEntry[gIDCount].staDefVal > gParamEntry[gIDCount].staMaxVal)
                    {
                        printf("ERROR : id %lu STA default val %lu > max %lu, using max\n",  gIDCount,
                               gParamEntry[gIDCount].staDefVal, gParamEntry[gIDCount].staMaxVal);
                        gParamEntry[gIDCount].staDefVal = gParamEntry[gIDCount].staMaxVal;
                    }
                }
                else
                {
                    unsigned long i;
                    ProcessStringValues(inBuf, &gParamEntry[gIDCount].staLen,
                                        gParamEntry[gIDCount].staVal);
                    if (gVerbose)
                    {
                        printf("** String ");
                        for (i=0; i<gParamEntry[gIDCount].staLen; i++)
                            printf("%d ", gParamEntry[gIDCount].staVal[i]);
                        printf("\n");
                    }
                }
                currLine++;
                break;

            case PARAM_FLAGS_AP:
                /* Process control flags for AP */
                ProcessFlags(inBuf, &gParamEntry[gIDCount].apControl,
                             &gParamEntry[gIDCount].apIndex,
                             gParamEntry[gIDCount].paramLen, &gApIBufCount,
                             &gApSBufCount, FALSE);
                currLine++;
                break;

            case PARAM_NTF_AP:
                /* Process notification flags */
                ProcessNtf(inBuf, &gParamEntry[gIDCount].apControl);
                currLine++;
                break;

            case PARAM_VALUES_AP:
                if (gParamEntry[gIDCount].staControl & CFG_CTL_INT)
                {
                    /* Get min/max/default values */
                    ProcessValues(inBuf, &gParamEntry[gIDCount].apMinVal,
                                  &gParamEntry[gIDCount].apMaxVal,
                                  &gParamEntry[gIDCount].apDefVal);

                    if (gParamEntry[gIDCount].apDefVal < gParamEntry[gIDCount].apMinVal)
                    {
                        printf("ERROR : id %lu AP default val %lu < min %lu, using min\n",  gIDCount,
                               gParamEntry[gIDCount].apDefVal, gParamEntry[gIDCount].apMinVal);
                        gParamEntry[gIDCount].apDefVal = gParamEntry[gIDCount].apMinVal;
                    }
                    if (gParamEntry[gIDCount].apDefVal > gParamEntry[gIDCount].apMaxVal)
                    {
                        printf("ERROR : id %lu AP default val %lu > max %lu, using max\n",  gIDCount,
                               gParamEntry[gIDCount].apDefVal, gParamEntry[gIDCount].apMaxVal);
                        gParamEntry[gIDCount].apDefVal = gParamEntry[gIDCount].apMaxVal;
                    }
                }
                else
                {
                    ProcessStringValues(inBuf, &gParamEntry[gIDCount].apLen,
                                        gParamEntry[gIDCount].apVal);
                }

                /* Replicate row# time this parameter */
                if (procTable)
                {
                    tmpIDCount = gIDCount;
                    ReplicateEntry(gIDCount, rowCount - 1);
                }

                if (gVerbose)
                {
                    printf("\t");
                    if (gParamEntry[gIDCount].staControl & CFG_CTL_RESTART)
                        printf("sta RESTART ");
                    if (gParamEntry[gIDCount].staControl & CFG_CTL_RELOAD)
                        printf("sta RELOAD ");
                    if (gParamEntry[gIDCount].apControl & CFG_CTL_RESTART)
                        printf("ap RESTART ");
                    if (gParamEntry[gIDCount].apControl & CFG_CTL_RELOAD)
                        printf("ap RELOAD ");
                    printf("\n");
                }

                /* Process next parameter entry */
                currLine = PARAM_TABLE_INFO;
                gIDCount++;
                break;

            default:
                printf("ERROR parsing currLine = %lu\n", currLine);
                exit(1);
        }

    }

    if (generateSource)
    {
        if (!WriteHdrFile())
            return (-1);

        if (!WriteSrcFile())
            return (-1);
    }

    WriteBinFiles();

    if (gVerbose)
        printf("Total Entries = %lu\n", gIDCount);

    /* Close all open files */
    fclose(pInFile);

    return 0;
}

