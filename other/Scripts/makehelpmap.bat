rem @echo off
rem if "%1" == "" goto error
rem if "%2" == "" goto error

echo // Help Map file generated from application's resource.h file. > "%2"
echo. >> "%2"
echo // Commands (ID_* and IDM_*) >> "%2"
makehm /h ID_,HID_,0x00000 IDM_,HIDM_,0x10000 %1 >> %2 
echo. >> "%2" 
echo // Prompts (IDP_*) >> "%2" 
makehm /h IDP_,HIDP_,0x30000 %1 >> %2 
echo. >> "%2" 
echo // Resources (IDR_*) >> "%2" 
makehm /h IDR_,HIDR_,0x20000 %1 >> %2 
echo. >> "%2"
echo // Dialogs (IDD_*) >> "%2" 
makehm /h IDD_,HIDD_,0x20000 %1 >> %2 
echo. >> "%2" 
echo // Frame Controls (IDW_*) >> "%2" 
makehm /h IDW_,HIDW_,0x50000 %1 >> %2 
echo. >> "%2" 
echo // Controls (IDC_*) >> "%2" 
makehm /h IDC_,IDH_,0x00000 %1 >> %2 

goto end

:Error
@echo Missing parameter. Usage: makehelpmap "resource.h" "output.h"
goto end

:end
