/*
 * Copyright (c) 2013 Qualcomm Atheros, Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
/*
 * Copyright (c) 2010 Atheros Communications, Inc.
 * All rights reserved.
 * 
 *
 * $ATH_LICENSE_HOSTSDK0_C$
 * 
 */
#include "bmi_msg.h"
#include "targaddrs.h"
#include "cepci.h"
#include "regtable.h"
#include "ar9888def.h"

void target_register_tbl_attach(struct hif_pci_softc *sc, u32 target_type)
{
    switch (target_type) {
    case TARGET_TYPE_AR9888:
        sc->targetdef = &ar9888_targetdef;
        break;
        break;
    default:
        break;
    }
}

void hif_register_tbl_attach(struct hif_pci_softc *sc, u32 hif_type)
{
    switch (hif_type) {
    case HIF_TYPE_AR9888:
        sc->hostdef = &ar9888_hostdef;
        break;
    default:
        break;
    }
}
