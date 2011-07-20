#ifndef __CONFIG_H__
#define __CONFIG_H__

///////////////////////////////////////////////////////////
// driver flags
///////////////////////////////////////////////////////////

#define VGD_LOG_MODE_NONE               0
#define VGD_LOG_MODE_SL                 1
#define VGD_LOG_MODE_FULL               2

#define VGD_VERSION                     ("1.3.5")

#define VGD_MULTIOPEN                   1
#define VGD_THREAD_PRIORITY             103

#define VGD_ABOUT_ENABLE                1
#define VGD_ABOUT_STRING                ("$$ Virtual GPS Driver, v 1.3.5 (jet)\r\n")

#define VGD_USE_LOG                     VGD_LOG_MODE_NONE
#define VGD_LOG_FILE                    (_T("/My Documents/vgd_drv.log"))

#define VGD_POSTINIT_EXEC               0
#define VGD_PIEXEC_APP                  (_T(""))

#define VGD_TAP_FILE                    (_T("/vgd_data.txt"))

#define VGD_AC_CONFIGURATOR             (0x0001L)

///////////////////////////////////////////////////////////
// build flags
///////////////////////////////////////////////////////////

#define VGD_DEFAULT_DATATIMER           (1000)

#define VGD_READ_TIMEOUT                (250)
#define VGD_READ_TIMEOUT_MULTIPLIER     (10)
#define VGD_READ_TIMEOUT_CONSTANT       (100)
#define VGD_WRITE_TIMEOUT_MULTIPLIER    (0)
#define VGD_WRITE_TIMEOUT_CONSTANT      (1000)

#define VGD_DEFAULT_CE_THREAD_PRIORITY  (103)
						
#define VGD_BUFFER_SIZE                 (4096)

#define VGD_WRITE_DEF_BLOCK             (64)
#define VGD_WRITE_NTRY                  (3)

#endif // __CONFIG_H__
