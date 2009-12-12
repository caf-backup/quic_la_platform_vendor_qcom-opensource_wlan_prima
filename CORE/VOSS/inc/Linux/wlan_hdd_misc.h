#ifndef WLAN_HDD_MISC_H
#define WLAN_HDD_MISC_H

VOS_STATUS hdd_get_fw_binary(v_VOID_t *pCtx,v_VOID_t **ppfw_data, v_SIZE_t *pSize);

VOS_STATUS hdd_release_fw_binary(v_VOID_t *pCtx);

VOS_STATUS hdd_get_cfg_file_size(v_VOID_t *pCtx, char *pFileName, v_SIZE_t *pBufSize);

VOS_STATUS hdd_read_cfg_file(v_VOID_t *pCtx, char *pFileName, v_VOID_t *pBuffer, v_SIZE_t *pBufSize);

#endif /* WLAN_SAL_MISC_H */

