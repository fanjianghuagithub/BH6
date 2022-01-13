/***************************************************************************
 *
 * Copyright 2015-2019 BES.
 * All rights reserved. All unpublished rights reserved.
 *
 * No part of this work may be used or reproduced in any form or by any
 * means, or stored in a database or retrieval system, without prior written
 * permission of BES.
 *
 * Use of this work is governed by a license granted by BES.
 * This work contains confidential and proprietary information of
 * BES. which is protected by copyright, trade secret,
 * trademark and other intellectual property rights.
 *
 ****************************************************************************/

#include "cmsis.h"
#include "plat_types.h"
#include <string.h>
#include "hal_location.h"
#include "bluetooth.h"
#include "hal_trace.h"
#include "tgt_hardware.h"

extern "C"{
    typedef struct bes_bt_local_info_t{
        uint8_t bt_addr[BTIF_BD_ADDR_SIZE];
        const char *bt_name;
        uint8_t bt_len;
        uint8_t ble_addr[BTIF_BD_ADDR_SIZE];
        const char *ble_name;
        uint8_t ble_len;
    }bes_bt_local_info;
}


/*
testkey_bin

BES2000iZ
Start: 11 11 22 33 33 33
End: 111122333396
Quantity: 100
License Key: 3F0F524EA8270008A036B254
*/

extern "C" uint8_t* factory_section_get_bt_name(void);
extern "C" uint8_t* factory_section_get_ble_name(void);

const uint8_t lhdc_test_bt_addr[]  = {0x45,0x33,0x33,0x22,0x11,0x11};
const uint8_t lhdc_test_ble_addr[] = {0x45,0x33,0x33,0x22,0x11,0x11};

int bes_bt_local_info_get(bes_bt_local_info *local_info)
{
#if 1
    uint8_t *bt_name;
    uint8_t *ble_name;

    bt_name = factory_section_get_bt_name();
    ble_name = factory_section_get_ble_name();

    local_info->bt_len = strlen((char *)bt_name)+1;
    local_info->ble_len = strlen((char *)ble_name)+1;

    memcpy(local_info->bt_addr, bt_addr, BTIF_BD_ADDR_SIZE);
    memcpy((void*)local_info->bt_name, (const void*)bt_name, local_info->bt_len);
    
    memcpy(local_info->ble_addr, ble_addr, BTIF_BD_ADDR_SIZE);
    memcpy((void*)local_info->ble_name, (const void*)ble_name, local_info->ble_len);
#else
    const char *bt_name2 ="BES_LHDC_TEST";

    memcpy(local_info->bt_addr, lhdc_test_bt_addr, BTIF_BD_ADDR_SIZE);
//    local_info->bt_name = (const char *)bt_name2;
    local_info->bt_len = strlen(bt_name2)+1;
    memcpy((void *)local_info->bt_name, (void *)bt_name2, local_info->bt_len);

    memcpy(local_info->ble_addr, lhdc_test_ble_addr, BTIF_BD_ADDR_SIZE);
 //   local_info->ble_name = (const char *)bt_name2;
    local_info->ble_len = strlen(bt_name2)+1;
    memcpy((void *)local_info->ble_name, (void *)bt_name2, local_info->ble_len);
#endif
    TRACE(0,"LHDC_info_get addr");
    DUMP8("%02x ", local_info->bt_addr, BTIF_BD_ADDR_SIZE);
    TRACE(1,"LHDC_info_get name:%s", local_info->bt_name);
    TRACE(1,"LHDC_info_get len:%d", local_info->bt_len);

    return 0;
}

