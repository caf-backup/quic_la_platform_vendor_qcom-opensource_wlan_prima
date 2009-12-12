#ifndef PHY_DUMP_COMMANDS_H
#define PHY_DUMP_COMMANDS_H
char *dump_phy_menu(tHalHandle hHal, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p);
char *dump_phy_set_channel(tHalHandle hHal, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p);
char *dump_phy_get_channel(tHalHandle hHal, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p);
char *dump_phy_set_chains(tHalHandle hHal, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p);
char *perform_init_cal(tHalHandle hHal, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p);
char *perform_periodic_cal(tHalHandle hHal, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p);
char *enable_periodic_cal(tHalHandle hHal, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p);
char *set_phy_rate_power_group(tHalHandle hHal, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p);
char *dump_phy_rate_power_table(tHalHandle hHal, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p);
char *add_watchlist_reg(tHalHandle hHal, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p);
char *clear_reg_watchlist(tHalHandle hHal, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p);
char *dump_reg_watchlist(tHalHandle hHal, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p);
char *dump_phy_diagnostics(tHalHandle hHal, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p);
char *dump_all_regs(tHalHandle hHal, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p);
char *override_tx_gain(tHalHandle hHal, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p);
char *get_dco_samples(tHalHandle hHal, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p);
char *dump_grab_ram(tHalHandle hHal, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p);
char *read_deinterlaced_memory(tHalHandle hHal, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p);
char *dump_fill_memory(tHalHandle hHal, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p);
char *dump_tpc_luts(tHalHandle hHal, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p);
char *dump_mW_conversion(tHalHandle hHal, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p);
char *testSetRxIQMemory(tHalHandle hHal, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p);
char *testGetRxIQMemory(tHalHandle hHal, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p);
char *testTxLoFFT(tHalHandle hHal, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p);
char *testMeasureTxLoSuppression(tHalHandle hHal, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p);
char *dumpPhyCalibrationValues(tHalHandle hHal, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p);
char *dumpPhyDbgMem(tHalHandle hHal, tANI_U32 arg1, tANI_U32 arg2, tANI_U32 arg3, tANI_U32 arg4, char *p);

#endif
