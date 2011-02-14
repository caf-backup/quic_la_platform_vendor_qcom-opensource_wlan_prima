/*
 * Woodside Networks, Inc proprietary. All rights reserved.
 * This is the source code for CFG binary dump tool.
 * Author:         Kevin Nguyen    
 * Date:           03/19/02
 * History:-
 * 03/19/02        Created.
 * --------------------------------------------------------------------
 * 
 */


#include <stdlib.h>
#include <stdio.h>
#include <cfgDef.h>
#include <sirTypes.h>

static int LITTLE_ENDIAN_OUTPUT = 0;

typedef int tANI_U8;
#define TRUE    (1)
#define FALSE   (0)

/**
 * LittleEndian() 
 *
 * FUNCTION:
 * This function convert an unsigned 32-bit value to little endian format 
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param data:    input data    
 *
 * @return Data in big endian 
 */
static tANI_U32 
LittleEndian(tANI_U32 data)
{
    tANI_U32   mask;
    tANI_U32   temp; 

    if (LITTLE_ENDIAN_OUTPUT)
        return (data);

    mask = 0xFF;

    temp  = (data & mask) << 24;
    temp |= ((data >> 8) & mask) << 16;
    temp |= ((data >> 16) & mask) << 8;
    temp |= ((data >> 24) & mask);

    return (temp);
}


/**
 * checkFlag() 
 *
 * FUNCTION:
 * This function compares the 'value' against 'flag'.  If TRUE, 'true'
 * text is output.  Otherwise, 'false' text is output. 
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param value:     value to compare
 * @param flag:      flag to compare against
 * @param pTrueTxt:  pointer to 'true' text
 * @param pFalseTxt: pointer to 'false' text    
 *
 * @return None
 */
static void 
checkFlag (tANI_U32 value, tANI_U32 flag, tANI_U8 *pTrueTxt, tANI_U8 *pFalseTxt)
{
    if (((value & flag) != 0) && (pTrueTxt != NULL))
        printf("%s", pTrueTxt);
    else if (pFalseTxt != NULL)
        printf("%s", pFalseTxt);

} /*** checkFlag() ***/

 
/**
 * main() 
 *
 * FUNCTION:
 * This is the main entry point.  This is the CFG binary dump utility used
 * for testing/debugging purposes. 
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 */
int main (int argc, char *argv[])
{
    tANI_U32      i, temp, buffer, iBufCount, sBufCount;
    FILE    *pInFile;

    printf("USAGE: %s infile [-l]\n", argv[0]);
    printf("\t-l to generate little-endian output, default is big endian\n");
    if (argc < 2)
    {
        return(-1);
    }
    
    if (argc > 2)
    {
         if (strcmp(argv[2], "-l") == 0)
             LITTLE_ENDIAN_OUTPUT = 1;
         else
         {
             printf("Invalid switch : %s\n", argv[2]);
             return;
         }
    }
    if (LITTLE_ENDIAN_OUTPUT)
        printf("** Parsing little endian output\n");
    else
        printf("** Parsing big endian output\n");

    if ((pInFile = fopen(argv[1], "r")) == NULL)
    {
        printf("cfgDump:  Error opening input file:  %s\n", argv[1]);
        return (-1);
    }

    fread((void*)&temp, sizeof(tANI_U32), 1, pInFile);
    fread((void*)&temp, sizeof(tANI_U32), 1, pInFile);
    temp = LittleEndian(temp);
    printf("\nDumping %s ...\n\n", argv[1]);

    /* Get mode */
    printf("MODE:  ");
    if((temp & 0x00ff0000) != 0)
        printf("%c",temp >> 16);
    printf("%c%c\n", temp>>8, temp & 0x000000ff);

    /* Get sizes */
    fread((void*)&temp, sizeof(tANI_U32), 1, pInFile);
    temp = LittleEndian(temp);
    printf("Number of parameters:  %d\n", temp);
    fread((void*)&iBufCount, sizeof(tANI_U32), 1, pInFile);
    iBufCount = LittleEndian(iBufCount);
    printf("Integer buffer size:  %d\n", iBufCount);
    fread((void*)&sBufCount, sizeof(tANI_U32), 1, pInFile);
    sBufCount = LittleEndian(sBufCount);
    printf("String buffer size:  %d\n\n", sBufCount);

    /* Get control entries */
    for (i = 0; i < temp; i++)
    {
        fread((void*)&buffer, sizeof(tANI_U32), 1, pInFile);
        buffer = LittleEndian(buffer);

        printf("Parameter#:  %d\n", i);
        printf("Control Flags:  0x%08x\n", buffer);

        /* Read/write flag */
        printf("R/W Mode:  ");
        switch (buffer & (CFG_CTL_RE | CFG_CTL_WE))
        {
            case CFG_CTL_RE:
                printf("Read-Only\n");
                break;
            case CFG_CTL_WE:
                printf("Write-Only\n");
                break;
            case (CFG_CTL_RE | CFG_CTL_WE):
                printf("Read-Write\n");
                break;
                     
            default:
                printf("Not accessible from host\n");
        }

        /* Type */
        printf("Type:  ");
        checkFlag(buffer, CFG_CTL_INT, "Integer\t\t", "String\t\t");

        /* Buffer index */
        printf("Buffer Index:  %d\t\t", buffer & CFG_BUF_INDX_MASK);

        /* Validity flag */
        checkFlag(buffer, CFG_CTL_VALID, "VALID\n", "INVALID\n");


        /* Save flag */
        printf("Save Enable:  ");
        checkFlag(buffer, CFG_CTL_SAVE, "ON\n", "OFF\n");

        /* Notification */
        printf("Notification:  ");
        checkFlag(buffer, CFG_CTL_NTF_HDD, "HDD ",NULL);
        checkFlag(buffer, CFG_CTL_NTF_LIM, "LIM ",NULL);
        checkFlag(buffer, CFG_CTL_NTF_SCH, "SCH ",NULL);
        checkFlag(buffer, CFG_CTL_NTF_ARQ, "ARQ ",NULL);
        checkFlag(buffer, CFG_CTL_NTF_DPH, "DPH ",NULL);
        checkFlag(buffer, CFG_CTL_NTF_NIM, "NIM ",NULL);
        checkFlag(buffer, CFG_CTL_NTF_SP,  "SP ",NULL);
        checkFlag(buffer, CFG_CTL_NTF_RFP, "RFP ",NULL);
        checkFlag(buffer, CFG_CTL_NTF_RHP, "RHP ",NULL);
        checkFlag(buffer, CFG_CTL_NTF_TFP, "TFP ",NULL);
        printf("\n");           
 
        /* Semaphore index */
        printf("Semaphore Index:  %d\n", 
               (buffer & CFG_SEM_INDX_MASK) >> CFG_SEM_INDX_SHIFT);
        
        printf("\n\n");
    }
}
















