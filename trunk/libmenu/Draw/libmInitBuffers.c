#include "../common.h"


bool libmInitBuffers( int opt ,int sync )
{
	int unk;
	
	convert	= NULL;
	blend	= NULL;
	
	if( opt & LIBM_FMT_MASK )
	{
		int format;
		
		psx = 0;
		psy = 0;
		
		switch( opt & LIBM_FMT_MASK )
		{
			case LIBM_DRAW_INIT5650	: format = PSP_DISPLAY_PIXEL_FORMAT_565 ; break;
			case LIBM_DRAW_INIT4444	: format = PSP_DISPLAY_PIXEL_FORMAT_4444; break;
			case LIBM_DRAW_INIT5551	: format = PSP_DISPLAY_PIXEL_FORMAT_5551; break;
			default 				: format = PSP_DISPLAY_PIXEL_FORMAT_8888;
		}
		
		vinfo.format	= format;
		vinfo.lineWidth	= BUF_WIDTH;
		vinfo.buffer	= (void*) (VRAM_ADDR_TOP | (u32) sceGeEdramGetAddr());
		
		sceDisplaySetMode(0, SCR_WIDTH, SCR_HEIGHT);
		sceDisplaySetFrameBuf((void *) vinfo.buffer, BUF_WIDTH, vinfo.format, sync);
	}
	
	//LIBM_DRAW_INITÇµÇƒÇ¢ÇΩèÍçáÇ≈Ç‡ÅAé¿ç€Ç…ïœçXÇ≥ÇÍÇƒÇ¢ÇÈÇ©ÇçƒéÊìæ
	sceDisplayGetMode( &unk, &vinfo.width, &vinfo.height);
	sceDisplayGetFrameBuf( &vinfo.buffer, &vinfo.lineWidth, &vinfo.format, sync );
	
	if( !vinfo.buffer || !vinfo.lineWidth ) return false;
	
	vinfo.buffer= (void*)( (uintptr_t)vinfo.buffer | VRAM_ADDR_TOP );
	
	vinfo.pixelSize	= ( vinfo.format == PSP_DISPLAY_PIXEL_FORMAT_8888 ? 4 : 2 );
	vinfo.lineSize	= vinfo.lineWidth * vinfo.pixelSize;
	vinfo.frameSize	= vinfo.lineSize * vinfo.height;
	vinfo.opt		= opt;
	
	switch( vinfo.format )
	{
		case PSP_DISPLAY_PIXEL_FORMAT_4444:
		{
			convert = libmConvert8888_4444;
			if( opt & LIBM_DRAW_BLEND ) blend	= libmAlphaBlend4444;
			
			break;
		}
		case PSP_DISPLAY_PIXEL_FORMAT_5551:
		{
			convert = libmConvert8888_5551;
			if( opt & LIBM_DRAW_BLEND ) blend	= libmAlphaBlend5551;
			
			break;
		}
		case PSP_DISPLAY_PIXEL_FORMAT_565:
		{
			convert = libmConvert8888_5650;
			if( opt & LIBM_DRAW_BLEND ) blend	= libmAlphaBlend5650;
			
			break;
		}
		
		case PSP_DISPLAY_PIXEL_FORMAT_8888:
		{
			if( opt & LIBM_DRAW_BLEND ) blend	= libmAlphaBlend8888;
			
			break;
		}
	}
	
	if( opt & LIBM_FMT_MASK ) libmClearBuffers();
	
	return true;
}
