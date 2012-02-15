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
align (Display *display,
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

  if (ret != EXIT_FAILURE) {
    Window root;

    if (verbose) {
      fprintf (stderr, "Output: %s\n", output->name);
    }
    
    root = RootWindow (display, screen);
    ret = apply_transform (display, root, output->crtc, inputarg);
  }

  XRRFreeOutputInfo (output);
  return ret;
}

int
apply_transform (Display *display,
		 Window root,
		 RRCrtc crtcnum,
		 const char *input_name)
{
  int ret;

  ret = EXIT_SUCCESS;

  if (ret != EXIT_FAILURE) {
    XRRScreenConfiguration *sconf;
    XRRScreenResources *res;
    XRRCrtcInfo *crtc;
    XRRCrtcTransformAttributes *transform;
    Status status;
    double amx[3][3];
    double sina, cosa;
    double hscale, vscale, hoffs, voffs;
    XRRScreenSize *ssize;
    int nsizes;
    Rotation srot;
    
    res = XRRGetScreenResourcesCurrent (display, root);
    sconf = XRRGetScreenInfo (display, root);
    ssize = XRRConfigSizes(sconf, &nsizes) + XRRConfigCurrentConfiguration (sconf, &srot);
    crtc = XRRGetCrtcInfo (display, res, crtcnum);
  
    if (verbose) {
      fprintf (stderr, "Screen: (%u, %u) 0x%02x\n", ssize->width, ssize->height, srot);
      fprintf (stderr, "CRTC: (%i, %i) (%u, %u) 0x%02x\n", crtc->x, crtc->y, crtc->width, crtc->height, crtc->rotation);
    }

    switch (srot) {
    case RR_Rotate_0:
    case RR_Rotate_180:
      hscale = (double)crtc->width/(double)ssize->width;
      vscale = (double)crtc->height/(double)ssize->height;
      break;
    case RR_Rotate_90:
    case RR_Rotate_270:
      hscale = (double)crtc->width/(double)ssize->height;
      vscale = (double)crtc->height/(double)ssize->width;
      break;
    default:
      ret = EXIT_FAILURE;
      fprintf (stderr, "The screen rotation/reflection 0x%02x is not supported yet. Sorry.\n", srot);
    }

    switch (crtc->rotation) {
    case RR_Rotate_0:
      sina = 0;
      cosa = 1;
      hoffs = 0;
      voffs = 0;
      break;
    case RR_Rotate_90:
      sina = 1;
      cosa = 0;
      hoffs = 1;
      voffs = 0;
      break;
    case RR_Rotate_180:
      sina = 0;
      cosa = -1;
      hoffs = 1;
      voffs = 1;
      break;
    case RR_Rotate_270:
      sina = -1;
      cosa = 0;
      hoffs = 0;
      voffs = 1;
      break;
    default:
      ret = EXIT_FAILURE;
      fprintf (stderr, "The rotation/reflection 0x%02x is not supported yet. Sorry.\n", crtc->rotation);
    }

    if (ret != EXIT_FAILURE) {
      amx[0][0] = cosa*hscale;
      amx[0][1] = -sina*hscale;
      amx[0][2] = hoffs*hscale + crtc->x/ssize->width;
      amx[1][0] = sina*vscale;
      amx[1][1] = cosa*vscale;
      amx[1][2] = voffs*vscale + crtc->y/ssize->height;
      amx[2][0] = 0;
      amx[2][1] = 0;
      amx[2][2] = 1;

      status = XRRGetCrtcTransform (display, crtcnum, &transform);
      if (!status) {
	fprintf (stderr, "Unable to get the current transformation\n");
	ret = EXIT_FAILURE;
      }
    }

    if (ret != EXIT_FAILURE) {
      static char strmx[3][3][8];
      static const char *args[11];
      double mx[3][3];
      int i, j;

      for (j = 0; j < 3; j++) {
	for (i = 0; i < 3; i++) {
	  XFixed fv = transform->currentTransform.matrix[j][i];
          mx[j][i] = XFixedToDouble (fv);
        }
      }

      args[0] = input_name;
      args[1] = "Coordinate Transformation Matrix";
      for (j = 0; j < 3; j++) {
	for (i = 0; i < 3; i++) {
          double v = mx[0][i]*amx[j][0] + mx[1][i]*amx[j][1] + mx[2][i]*amx[j][2];
	  snprintf (strmx[j][i], 8, "%8.6f", v);
	  args[2 + i + j*3] = strmx[j][i];
	}
      }
      if (verbose) {
	fprintf (stderr, "Debug: set-float-prop");
	for (i = 0; i < 11; i++) {
	  fprintf (stderr, " %s", args[i]);
	}
	fprintf (stderr, "\n");
      }
      ret = set_float_prop(display, 11, args, "set-float-prop", ": error calling function, please report a bug.");
      XFree (transform);
    }
    XRRFreeCrtcInfo (crtc);
    XRRFreeScreenResources (res);
    XRRFreeScreenConfigInfo (sconf);
  }

  return ret;
}

/* end of align.c */
