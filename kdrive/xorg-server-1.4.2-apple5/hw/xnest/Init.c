/*

Copyright 1993 by Davor Matic

Permission to use, copy, modify, distribute, and sell this software
and its documentation for any purpose is hereby granted without fee,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation.  Davor Matic makes no representations about
the suitability of this software for any purpose.  It is provided "as
is" without express or implied warranty.

*/

#ifdef HAVE_XNEST_CONFIG_H
#include <xnest-config.h>
#endif

#include <X11/X.h>
#include <X11/Xproto.h>
#include "screenint.h"
#include "input.h"
#include "misc.h"
#include "scrnintstr.h"
#include "windowstr.h"
#include "servermd.h"
#include "mi.h"
#include <X11/fonts/fontstruct.h>

#include "Xnest.h"

#include "Display.h"
#include "Screen.h"
#include "Pointer.h"
#include "Keyboard.h"
#include "Handlers.h"
#include "Init.h"
#include "Args.h"
#include "Drawable.h"
#include "XNGC.h"
#include "XNFont.h"
#ifdef DPMSExtension
#include "dpmsproc.h"
#endif

Bool xnestDoFullGeneration = True;

xEvent *xnestEvents = NULL;

void
InitOutput(ScreenInfo *screenInfo, int argc, char *argv[])
{
  int i, j;

  xnestOpenDisplay(argc, argv);
  
  screenInfo->imageByteOrder = ImageByteOrder(xnestDisplay);
  screenInfo->bitmapScanlineUnit = BitmapUnit(xnestDisplay);
  screenInfo->bitmapScanlinePad = BitmapPad(xnestDisplay);
  screenInfo->bitmapBitOrder = BitmapBitOrder(xnestDisplay);
  
  screenInfo->numPixmapFormats = 0;
  for (i = 0; i < xnestNumPixmapFormats; i++) 
    for (j = 0; j < xnestNumDepths; j++)
      if ((xnestPixmapFormats[i].depth == 1) ||
          (xnestPixmapFormats[i].depth == xnestDepths[j])) {
	screenInfo->formats[screenInfo->numPixmapFormats].depth = 
	  xnestPixmapFormats[i].depth;
	screenInfo->formats[screenInfo->numPixmapFormats].bitsPerPixel = 
	  xnestPixmapFormats[i].bits_per_pixel;
	screenInfo->formats[screenInfo->numPixmapFormats].scanlinePad = 
	  xnestPixmapFormats[i].scanline_pad;
	screenInfo->numPixmapFormats++;
	break;
      }
  
  xnestWindowPrivateIndex = AllocateWindowPrivateIndex();
  xnestGCPrivateIndex = AllocateGCPrivateIndex();
  xnestFontPrivateIndex = AllocateFontPrivateIndex();
  
  if (!xnestNumScreens) xnestNumScreens = 1;

  for (i = 0; i < xnestNumScreens; i++)
    AddScreen(xnestOpenScreen, argc, argv);

  xnestNumScreens = screenInfo->numScreens;

  xnestDoFullGeneration = xnestFullGeneration;
}

void
InitInput(int argc, char *argv[])
{
  xnestPointerDevice = AddInputDevice(xnestPointerProc, TRUE);
  xnestKeyboardDevice = AddInputDevice(xnestKeyboardProc, TRUE);

  if (!xnestEvents)
      xnestEvents = (xEvent *) xcalloc(sizeof(xEvent), GetMaximumEventsNum());
  if (!xnestEvents)
      FatalError("couldn't allocate room for events\n");

  RegisterPointerDevice(xnestPointerDevice);
  RegisterKeyboardDevice(xnestKeyboardDevice);

  mieqInit();

  AddEnabledDevice(XConnectionNumber(xnestDisplay));

  RegisterBlockAndWakeupHandlers(xnestBlockHandler, xnestWakeupHandler, NULL);
}

/*
 * DDX - specific abort routine.  Called by AbortServer().
 */
void AbortDDX()
{
  xnestDoFullGeneration = True;
  xnestCloseDisplay();
}

/* Called by GiveUp(). */
void ddxGiveUp()
{
  AbortDDX();
}

#ifdef __APPLE__
void
DarwinHandleGUI(int argc, char *argv[])
{
}

void GlxExtensionInit();
void GlxWrapInitVisuals(void *procPtr);

void
DarwinGlxExtensionInit()
{
    GlxExtensionInit();
}

void
DarwinGlxWrapInitVisuals(
    void *procPtr)
{
    GlxWrapInitVisuals(procPtr);
}
#endif

void OsVendorInit()
{
    return;
}

void OsVendorFatalError()
{
    return;
}

void ddxBeforeReset(void)
{
    return;
}

/* this is just to get the server to link on AIX */
#ifdef AIXV3
int SelectWaitTime = 10000; /* usec */
#endif
