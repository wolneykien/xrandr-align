/*
 * Original xinput:
 * Copyright 1996 by Frederic Lepied, France. <Frederic.Lepied@sugix.frmug.org>
 *
 * Original xrandr:
 * Copyright © 2001 Keith Packard, member of The XFree86 Project, Inc.
 * Copyright © 2002 Hewlett Packard Company, Inc.
 * Copyright © 2006 Intel Corporation
 *
 * xrandr-align:
 *
 * Copyright © 2012 Paul Wolneykien <manowar@altlinux.org>, ALT Linux Ltd.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is  hereby granted without fee, provided that
 * the  above copyright   notice appear  in   all  copies and  that both  that
 * copyright  notice   and   this  permission   notice  appear  in  supporting
 * documentation, and that   the  name of  the authors  not  be  used  in
 * advertising or publicity pertaining to distribution of the software without
 * specific,  written      prior  permission.     The authors  make  no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * THE AUTHORS DISCLAIM ALL   WARRANTIES WITH REGARD  TO  THIS SOFTWARE,
 * INCLUDING ALL IMPLIED   WARRANTIES OF MERCHANTABILITY  AND   FITNESS, IN NO
 * EVENT  SHALL THE AUTHORS  BE   LIABLE   FOR ANY  SPECIAL, INDIRECT   OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA  OR PROFITS, WHETHER  IN  AN ACTION OF  CONTRACT,  NEGLIGENCE OR OTHER
 * TORTIOUS  ACTION, ARISING    OUT OF OR   IN  CONNECTION  WITH THE USE    OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 */

#include "common.h"
#include "xrandr-align.h"
#include <string.h>
#include <X11/extensions/Xrandr.h>

int
monitor (Display *display,
	 int argc,
	 const char *argv[],
	 const char *funcname,
	 const char *usage)
{
  RROutput outputid;
  XRROutputInfo *output;
  int ret;
  const char *inputarg;
  int screen;
  int event_base, error_base;

  ret = get_screen (display, argc, argv, funcname, usage, &screen);
  if (ret == EXIT_FAILURE) {
    return ret;
  }

  ret = get_argval (argc, argv, "input", funcname, usage, "Virtual core pointer", &inputarg);
  if (ret == EXIT_FAILURE) {
    return ret;
  }

  ret = get_output (display, argc, argv, funcname, usage, &outputid, &output);
  if (ret == EXIT_FAILURE) {
    return ret;
  }

  if (!XRRQueryExtension (display, &event_base, &error_base)) {
    fprintf (stderr, "RandR extension missing\n");
    ret = EXIT_FAILURE;
  }

  if (ret != EXIT_FAILURE) {
    Window root;

    if (verbose) {
      fprintf (stderr, "Monitoring the output: %s id=%u\n", output->name, (unsigned int)outputid);
    }
    
    root = RootWindow (display, screen);
    XRRSelectInput (display, root, RRScreenChangeNotifyMask | RROutputChangeNotifyMask | RRCrtcChangeNotifyMask);
    
    while (ret != EXIT_FAILURE) {
      XEvent event;
      XRRScreenChangeNotifyEvent *sce;
      XRRNotifyEvent *ne;
      XRROutputChangeNotifyEvent *oce;
      XRRCrtcChangeNotifyEvent *cce;
      int escreen;

      XNextEvent(display, &event);

      switch (event.type - event_base) {
      case RRScreenChangeNotify:
	sce = (XRRScreenChangeNotifyEvent *) &event;
	if (verbose) {
	  fprintf (stderr, "Get a RRScreenChangeNotifyEvent: (%u, %u) 0x%02x\n", sce->width, sce->height, sce->rotation);
	}
	escreen = XRRRootToScreen (sce->display, sce->root);
	if (escreen == screen) {
	  ret = apply_transform (sce->display, sce->root, output->crtc, inputarg);
	  if (ret != EXIT_FAILURE) {
	    root = sce->root;
	    XRRFreeOutputInfo (output);
	    ret = get_output (display, argc, argv, funcname, usage, &outputid, &output);
	    if (verbose) {
	      fprintf (stderr, "Monitoring the output: %s id=%u\n", output->name, (unsigned int)outputid);
	    }
	  }
	} else if (verbose) {
	  fprintf (stderr, "Skip this event due to another screen number: %i\n", escreen);
	}
	break;
      case RRNotify:
	ne = (XRRNotifyEvent *) &event;
	switch (ne->subtype) {
	case RRNotify_OutputChange:
	  oce = (XRROutputChangeNotifyEvent *) ne;
	  if (verbose) {
	    fprintf (stderr, "Get a RROutputChangeNotifyEvent: %u %u 0x%02x\n", (unsigned int)oce->output, (unsigned int)oce->crtc, oce->rotation);
	  }
	  if (oce->output == outputid) {
	    if (oce->crtc) {
	      ret = apply_transform (oce->display, root, oce->crtc, inputarg);
	    } else {
	      fprintf (stderr, "Output is disconnected: skip this event\n");
	    }
	  } else if (verbose) {
	    fprintf (stderr, "Skip this event due to another output ID: %u\n", (unsigned int)oce->output);
	  }
	  break;
	case RRNotify_CrtcChange:
	  cce = (XRRCrtcChangeNotifyEvent *) ne;
	  if (verbose) {
	    fprintf (stderr, "Get a RRCrtcChangeNotifyEvent: (%i, %i) (%u, %u) 0x%02x\n", cce->x, cce->y, cce->width, cce->height, cce->rotation);
	  }
	  if (output->crtc == cce->crtc) {
	    ret = apply_transform (cce->display, root, cce->crtc, inputarg);
	  } else if (verbose) {
	    fprintf (stderr, "Skip this event due to another CRTC ID: %u\n", (unsigned int)cce->crtc);
	  }
	  break;
	}
	break;
      }
    }
  }

  XRRFreeOutputInfo (output);
  return ret;
}

/* end of monitor.c */
