/** ------------------------------------------------------------------------- * 
    ------------------------------------------------------------------------- *  

  
    \file aniPciIds.h
  
    \brief Airgo Networks PCI Device Ids
  
    $Id$ 
  
    Copyright (C) 2006 Airgo Networks, Incorporated
  
  
   ========================================================================== */
#ifndef _ANIPCIIDS_H_
#define _ANIPCIIDS_H_

/*
 * note that most of the following definitions also appear in modified
 * versions of the Linux header file pci_ids.h that were previously
 * distributed to our customers.  We have relocated the definitions to
 * this file so that our customers do not have to modify their version
 * of pci_ids.h.  However, note that the PCI_* versions may not use
 * parenthesis since the previously published versions did not, and hence
 * a macro redefinition warning/error may result if parenthesis are used
 * and a previously distributed pci_ids.h is encountered.
 */

#define PCI_VENDOR_ID_ANI		        0x17CB
#define PCI_DEVICE_ID_ANI_POLARIS		0x0001
#define PCI_DEVICE_ID_ANI_TITAN			0x0002
#define PCI_DEVICE_ID_ANI_POLARIS_REV3	0x0003
#define PCI_DEVICE_ID_ANI_POLARIS_OLD	0x802B
#define PCI_DEVICE_ID_ANI_TAURUS_PCI	0x0005
#define PCI_DEVICE_ID_ANI_TAURUS_PCIE	0x0006
#define PCI_DEVICE_ID_ANI_VIRGO_PCI	    0x0007
#define PCI_DEVICE_ID_ANI_VIRGO_PCIE	0x0008
#define PCI_DEVICE_ID_ANI_SPICA_PCIE	0x0009

#define PCI_DEVICE_ID_ANI_TAURUS_PRE_A4	0
#define PCI_DEVICE_ID_ANI_TAURUS_A4	    0x02

#define ANI_PCI_VENDOR_ID		        ( PCI_VENDOR_ID_ANI              )
#define ANI_PCI_DEVICE_ID_POLARIS		( PCI_DEVICE_ID_ANI_POLARIS      )
#define ANI_PCI_DEVICE_ID_TITAN			( PCI_DEVICE_ID_ANI_TITAN        )
#define ANI_PCI_DEVICE_ID_POLARIS_REV3	( PCI_DEVICE_ID_ANI_POLARIS_REV3 )
#define ANI_PCI_DEVICE_ID_POLARIS_OLD	( PCI_DEVICE_ID_ANI_POLARIS_OLD  )
#define ANI_PCI_DEVICE_ID_TAURUS_PCI	( PCI_DEVICE_ID_ANI_TAURUS_PCI   )
#define ANI_PCI_DEVICE_ID_TAURUS_PCIE	( PCI_DEVICE_ID_ANI_TAURUS_PCIE  )
#define ANI_PCI_DEVICE_ID_VIRGO_PCI	    ( PCI_DEVICE_ID_ANI_VIRGO_PCI   )
#define ANI_PCI_DEVICE_ID_VIRGO_PCIE	( PCI_DEVICE_ID_ANI_VIRGO_PCIE  )
#define ANI_PCI_DEVICE_ID_SPICA_PCIE	( PCI_DEVICE_ID_ANI_SPICA_PCIE  )



#endif // _ANIPCIIDS_H_
