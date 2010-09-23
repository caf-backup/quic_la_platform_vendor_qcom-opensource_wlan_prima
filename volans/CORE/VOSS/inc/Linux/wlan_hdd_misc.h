#ifndef WLAN_HDD_MISC_H
#define WLAN_HDD_MISC_H

#define WCN1314_CFG_FILE      "wlan/volans/WCN1314_cfg.dat"
#define WCN1314_FW_FILE       "wlan/volans/WCN1314_qcom_fw.bin"
#define WCN1314_NV_FILE       "wlan/volans/WCN1314_qcom_wlan_nv.bin"
#define WCN1314_CFG_INI_FILE  "wlan/volans/WCN1314_qcom_cfg.ini"
#define WCN1314_COUNTRY_INFO_FILE     "wlan_country_info.dat"
#define WCN1314_HO_CFG_FILE   "wlan_ho_config"


VOS_STATUS hdd_request_firmware(char *pfileName,v_VOID_t *pCtx,v_VOID_t **ppfw_data, v_SIZE_t *pSize);

VOS_STATUS hdd_release_firmware(char *pFileName,v_VOID_t *pCtx);

VOS_STATUS hdd_get_cfg_file_size(v_VOID_t *pCtx, char *pFileName, v_SIZE_t *pBufSize);

VOS_STATUS hdd_read_cfg_file(v_VOID_t *pCtx, char *pFileName, v_VOID_t *pBuffer, v_SIZE_t *pBufSize);

#endif /* WLAN_SAL_MISC_H */

