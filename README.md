# AgMD2_GPM
Keysight U5309A Digitizer control and DAQ

# Prerequisites
* ROOT 6.04/08 (built with python support)
* AgMD2 driver from Keysight (version 1.6.7 for linux)
* C++11 or higher


** If installing md2 driver v1.12 on kernel 4.8+, must edit the source file:
   LinuxMD2-1.12.5/Linux/agmodinst-3.4.4/module/agmodinst-dma.c

   --> "page_cache_release" should be changed to "put_page"
   --> "get_user_pages" should be changed to "get_user_pages_remote"