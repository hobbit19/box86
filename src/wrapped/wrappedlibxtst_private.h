#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))
#error Meh....
#endif

GO(XRecordAllocRange, pFv)
GO(XRecordCreateContext, LFpipipi)
//GO(XRecordDisableContext, 
//GO(XRecordEnableContext, 
//GO(XRecordEnableContextAsync, 
GO(XRecordFreeContext, iFpL)
//GO(XRecordFreeData, 
GO(XRecordFreeState, vFp)
GO(XRecordGetContext, iFpLp)
GO(XRecordIdBaseMask, uFp)
//GO(XRecordProcessReplies, 
GO(XRecordQueryVersion, iFpii)
GO(XRecordRegisterClients, iFpLipipi)
GO(XRecordUnregisterClients, iFpLpi)
GO(XTestCompareCurrentCursorWithWindow, iFpp)
GO(XTestCompareCursorWithWindow, iFppp)
GO(XTestDiscard, iFp)
GO(XTestFakeButtonEvent, iFpuiL)
GO(XTestFakeDeviceButtonEvent, iFppuipiL)
GO(XTestFakeDeviceKeyEvent, iFppuipiL)
GO(XTestFakeDeviceMotionEvent, iFppiipiL)
GO(XTestFakeKeyEvent, iFpuiL)
GO(XTestFakeMotionEvent, iFpiiiL)
GO(XTestFakeProximityEvent, iFppipiL)
GO(XTestFakeRelativeMotionEvent, iFpiiiL)
GO(XTestGrabControl, iFpi)
GO(XTestQueryExtension, iFppppp)
GO(XTestSetGContextOfGC, vFpp)
GO(XTestSetVisualIDOfVisual, vFpp)