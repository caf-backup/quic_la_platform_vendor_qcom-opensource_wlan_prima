
#ifndef WLAN_HDD_MISC_H
#define WLAN_HDD_MISC_H

#define LIBRA_CFG_FILE      "wlan/cfg.dat"
#define LIBRA_FW_FILE       "wlan/qcom_fw.bin"
#define LIBRA_NV_FILE       "wlan/qcom_wlan_nv.bin"
#define LIBRA_COUNTRY_INFO_FILE     "wlan_country_info.dat"
#define LIBRA_HO_CFG_FILE   "wlan_ho_config"

VOS_STATUS hdd_request_firmware(char *pfileName,v_VOID_t *pCtx,v_VOID_t **ppfw_data, v_SIZE_t *pSize);

VOS_STATUS hdd_release_firmware(char *pFileName,v_VOID_t *pCtx);

VOS_STATUS hdd_get_cfg_file_size(v_VOID_t *pCtx, char *pFileName, v_SIZE_t *pBufSize);

VOS_STATUS hdd_read_cfg_file(v_VOID_t *pCtx, char *pFileName, v_VOID_t *pBuffer, v_SIZE_t *pBufSize);
#if 0
VOS_STATUS hdd_get_fw_binary(v_VOID_t *pCtx,v_VOID_t **ppfw_data, v_SIZE_t *pSize);

VOS_STATUS hdd_release_fw_binary(v_VOID_t *pCtx);

VOS_STATUS hdd_get_cfg_file_size(v_VOID_t *pCtx, char *pFileName, v_SIZE_t *pBufSize);

VOS_STATUS hdd_read_cfg_file(v_VOID_t *pCtx, char *pFileName, v_VOID_t *pBuffer, v_SIZE_t *pBufSize);

#endif

#ifdef WLAN_SOFTAP_FEATURE

VOS_CON_MODE hdd_get_conparam ( void );
#endif

#endif /* WLAN_SAL_MISC_H */

