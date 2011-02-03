	.set noreorder

#include "pspstub.s"

	STUB_START	"sceCtrl_driver",0x40090000,0x00030005
	STUB_FUNC	0x18654FC0, sceCtrlPeekBufferPositive
	STUB_FUNC	0x855C255D, sceCtrlSetSamplingCycle
	STUB_FUNC	0x6CB49301, sceCtrlSetSamplingMode
	STUB_END

