#ifndef RF_GEMINI_DUMP_COMMANDS_H
#define RF_GEMINI_DUMP_COMMANDS_H

char *dump_rf_menu(tHalHandle hHal, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p);
char *dump_rf_fields(tHalHandle hHal, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p);
char *set_rf_field(tHalHandle hHal, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p);
char *dump_rf_gains(tHalHandle hHal, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p);
char *set_rf_dco(tHalHandle hHal, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p);
char *dump_rf_dco(tHalHandle hHal, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p);
char *dumpRfCalValues(tHalHandle hHal, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p);

#endif
