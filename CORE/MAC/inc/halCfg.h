/*
 * =====================================================================================
 * 
 *       Filename:  halCfg.h
 * 
 *    Description:  HAL configuration related definitions shared by HDD
 * 
 *        Version:  1.0
 *        Created:  03/09/2007 05:17:03 PM PST
 *       Revision:  none
 *       Compiler:  gcc
 * 
 *         Author:  Dinesh Upadhyay
 *        Company:  Qualcomm, Inc.
 * 
 * =====================================================================================
 */

#ifndef _HALCFG_H_
#define _HALCFG_H_

#include "halHddApis.h"

#define BA_DEFAULT_RX_BUFFER_SIZE 64
#define BA_DEFAULT_TX_BUFFER_SIZE 64
#define BA_MAX_SESSIONS 8
#define BA_SESSION_ID_INVALID 0xFFFF

#define BA_INVALID_OPCODE_BAR   254

#endif // _HALCFG_H_
