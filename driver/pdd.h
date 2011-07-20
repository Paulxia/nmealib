#ifndef __PDD_H__
#define __PDD_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
 * @doc HWINTERNAL
 * @struct SER_INFO | Private structure.
 */
    
    typedef struct __SER_INFO
    {
         // Put lib struct first so we can easily cast pointers
        VGD_INFO ar_info;
        
         // now hardware specific goodies
        PUCHAR		pBaseAddress;		// @field Start of serial registers
        DWORD       dwMemBase;       // @field Base Address - unmapped
        DWORD       dwMemLen;        // @field Length
        DWORD       dwSysIntr;       // @field System Interrupt number for this peripheral

        UINT8       cOpenCount;     // @field Count of concurrent opens
        COMMPROP	CommProp;	    // @field Pointer to CommProp structure.
        PVOID		pMddHead;		// @field First arg to mdd callbacks.
        BOOL		fIRMode;		// @field Boolean, are we running in IR mode?
        PHWOBJ      pHWObj;         // @field Pointer to PDDs HWObj structure
    } SER_INFO, *PSER_INFO;


#ifdef __cplusplus
}
#endif


#endif __PDD_H__
