/*
 * Copyright (C) 2016 Rockchip Electronics Co.Ltd
 * Authors:
 *	Zhiqin Wei <wzq@rock-chips.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */

#define LOG_NDEBUG 0
#define LOG_TAG "RockchipRgaStereo"

#include <stdint.h>
#include <sys/types.h>
#include <math.h>
#include <fcntl.h>
#include <utils/misc.h>
#include <signal.h>
#include <time.h>

#include <cutils/properties.h>

#include <binder/IPCThreadState.h>
#include <utils/Atomic.h>
#include <utils/Errors.h>
#include <utils/Log.h>

#include <ui/PixelFormat.h>
#include <ui/Rect.h>
#include <ui/Region.h>
#include <ui/DisplayInfo.h>
#include <ui/GraphicBufferMapper.h>

#include <gui/ISurfaceComposer.h>
#include <gui/Surface.h>
#include <gui/SurfaceComposerClient.h>
#include <RockchipRga.h>

#include <GLES/gl.h>
#include <GLES/glext.h>
#include <EGL/eglext.h>

#include <stdint.h>
#include <sys/types.h>

#include <system/window.h>

#include <utils/Thread.h>

#include <EGL/egl.h>
#include <GLES/gl.h>

///////////////////////////////////////////////////////
//#include "../drmrga.h"
#include <hardware/hardware.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>

#include <sys/mman.h>
#include <linux/stddef.h>
#include "RockchipFileOps.h"
///////////////////////////////////////////////////////

using namespace android;

int main()
{
    int ret = 0;
    int srcWidth,srcHeight,srcFormat;
    int dstWidth,dstHeight,dstFormat;

    srcWidth = 1536;
    srcHeight = 2048;
    srcFormat = HAL_PIXEL_FORMAT_RGBA_8888;

    dstWidth = 1536;
    dstHeight = 2048;
    dstFormat = HAL_PIXEL_FORMAT_RGBX_8888;

    RockchipRga& rkRga(RockchipRga::get());

    GraphicBufferMapper &mgbMapper = GraphicBufferMapper::get();


    char* buf = NULL;
    sp<GraphicBuffer> gbs(new GraphicBuffer(srcWidth,srcHeight,srcFormat,
                         	          GRALLOC_USAGE_SW_WRITE_OFTEN));

    if (gbs->initCheck()) {
        printf("GraphicBuffer error : %s\n",strerror(errno));
        return ret;
    } else
        printf("GraphicBuffer ok : %s\n","*************************************");
        

    sp<GraphicBuffer> gbd(new GraphicBuffer(dstWidth,dstHeight,dstFormat,
                         	          GRALLOC_USAGE_SW_WRITE_OFTEN));

    if (gbd->initCheck()) {
        printf("GraphicBuffer error : %s\n",strerror(errno));
        return ret;
    } else
        printf("GraphicBuffer ok : %s\n","*************************************");


    mgbMapper.registerBuffer(gbs->handle);
    mgbMapper.registerBuffer(gbd->handle);

    ret = gbs->lock(GRALLOC_USAGE_SW_WRITE_OFTEN, (void**)&buf);
    
    if (ret) {
        printf("lock buffer error : %s\n",strerror(errno));
        return ret;
    } else 
        printf("lock buffer ok : %s\n","**************************************");

#if 1
    get_buf_from_file(buf, srcFormat, srcWidth, srcHeight, 0);
#else
    memset(buf,0x55,4*1200*1920);
#endif
    ret = gbs->unlock();

    ret = gbd->lock(GRALLOC_USAGE_SW_WRITE_OFTEN, (void**)&buf);

    if (ret) {
        printf("lock buffer error : %s\n",strerror(errno));
        return ret;
    } else
        printf("lock buffer ok : %s\n","**************************************");


    {
#if 1
        get_buf_from_file(buf, srcFormat, srcWidth, srcHeight, 1);
#else
        memset(buf,0x55,4*1200*1920);
#endif
    }
    ret = gbd->unlock();

    ret = gbs->lock(GRALLOC_USAGE_SW_WRITE_OFTEN, (void**)&buf);
    if (ret) {
        printf("unlock buffer error : %s\n",strerror(errno));
        return ret;
    } else 
        printf("unlock buffer ok : %s\n","*************************************");

    while(1) {
    	rga_info_t src;
    	rga_info_t dst;
    	memset(&src, 0, sizeof(rga_info_t));
    	src.fd = -1;
    	src.blend = 0xFF0105;
    	memset(&dst, 0, sizeof(rga_info_t));
    	dst.fd = -1;
    	src.hnd = gbs->handle;
    	dst.hnd = gbd->handle;
        ret = rkRga.RkRgaBlit(&src, &dst, NULL);

        if (ret) {
            printf("rgaFillColor error : %s,hnd=%p\n",
                                            strerror(errno),(void*)(gbd->handle));
            ALOGD("rgaFillColor error : %s,hnd=%p\n",
                                            strerror(errno),(void*)(gbd->handle));
        }

        {
            char* dstbuf = NULL;
            ret = gbd->lock(GRALLOC_USAGE_SW_WRITE_OFTEN, (void**)&dstbuf);
            output_buf_data_to_file(dstbuf, dstFormat, dstWidth, dstHeight, 0);
	    ret = gbd->unlock();
	    ret = gbd->lock(GRALLOC_USAGE_SW_WRITE_OFTEN, (void**)&dstbuf);
	    
	    get_buf_from_file(buf, srcFormat, srcWidth, srcHeight, 2);
	    ret = gbs->unlock();
	    ret = gbs->lock(GRALLOC_USAGE_SW_WRITE_OFTEN, (void**)&buf); 
	    
	    int size = dstWidth * dstHeight * 4;
            //for(int i =0; i < mHeight * 1.5; i++)
            //    memcpy(dstbuf + i * 2400,buf + i * 3000,2400);
            //output_buf_data_to_file(dstbuf, dstFormat, dstWidth, dstHeight, 0);
            //ret = gbd->unlock();
            //ret = gbd->lock(GRALLOC_USAGE_SW_WRITE_OFTEN, (void**)&dstbuf);
	    unsigned int *pstd = (unsigned int *)buf;
	    unsigned int *pnow = (unsigned int *)dstbuf;
	    int errCount = 0;
	    int rightCount = 0;
            for (int i = 0; i < size / 4; i++) {
		if (*pstd != *pnow) {
			printf("i = %8d,[0x%x,0x%x];", i, *pstd ,*pnow);
			if (i % 16 == 0)
				printf("\n");
			errCount ++;

		} else {
			if (i % (640*1024) == 0)
				printf("pass i=%8d[0x%x,0x%x]\n",i,*pstd,*pnow);
			rightCount++;	
		}
		pstd++;
		pnow++;
		if (errCount > 64)
			break;
	    }
	    ret = gbs->unlock();
            ret = gbd->unlock();
	    printf("errCount=%d,rightCount=%d\n", errCount, rightCount);
        }
	break;
    }
    return 0;
}
