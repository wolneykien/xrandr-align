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

#include "xrandr-align.h"
#include <string.h>

#define INVALID_EVENT_TYPE	-1

static int           motion_type = INVALID_EVENT_TYPE;
static int           button_press_type = INVALID_EVENT_TYPE;
static int           button_release_type = INVALID_EVENT_TYPE;
static int           key_press_type = INVALID_EVENT_TYPE;
static int           key_release_type = INVALID_EVENT_TYPE;
static int           proximity_in_type = INVALID_EVENT_TYPE;
static int           proximity_out_type = INVALID_EVENT_TYPE;

static int
register_events(Display		*dpy,
		XDeviceInfo	*info,
		const char	*dev_name,
		Bool		handle_proximity)
{
    int			number = 0;	/* number of events registered */
    XEventClass		event_list[7];
    int			i;
    XDevice		*device;
    Window		root_win;
    unsigned long	screen;
    XInputClassInfo	*ip;

    screen = DefaultScreen(dpy);
    root_win = RootWindow(dpy, screen);

    device = XOpenDevice(dpy, info->id);

    if (!device) {
	fprintf(stderr, "unable to open device %s\n", dev_name);
	return 0;
    }

    if (device->num_classes > 0) {
	for (ip = device->classes, i=0; i<info->num_classes; ip++, i++) {
	    switch (ip->input_class) {
/*
	    case KeyClass:
		DeviceKeyPress(device, key_press_type, event_list[number]); number++;
		DeviceKeyRelease(device, key_release_type, event_list[number]); number++;
		break;

	    case ButtonClass:
		DeviceButtonPress(device, button_press_type, event_list[number]); number++;
		DeviceButtonRelease(device, button_release_type, event_list[number]); number++;
		break;
*/
	    case ValuatorClass:
		DeviceMotionNotify(device, motion_type, event_list[number]); number++;
		if (handle_proximity) {
		    ProximityIn(device, proximity_in_type, event_list[number]); number++;
		    ProximityOut(device, proximity_out_type, event_list[number]); number++;
		}
		break;
/*
	    default:
		fprintf(stderr, "unknown class\n");
		break;
*/
	    }
	}

	if (XSelectExtensionEvent(dpy, root_win, event_list, number)) {
	    fprintf(stderr, "error selecting extended events\n");
	    return 0;
	}
    }
    return number;
}

int
align_crtc (Display *display,
	    Window root,
	    RRCrtc crtcnum,
	    Rotation rot)
{
  int ret;
  XRRScreenResources *res;
  XRRCrtcInfo *crtc;
  Status status;

  res = XRRGetScreenResourcesCurrent (display, root);
  crtc = XRRGetCrtcInfo (display, res, crtcnum);

  status = XRRSetCrtcConfig (display, res, crtcnum, CurrentTime, crtc->x, crtc->y, crtc->mode, rot, crtc->outputs, crtc->noutput);
  if (status) {
    ret = EXIT_FAILURE;
  } else {
    ret = EXIT_SUCCESS;
  }

  XRRFreeCrtcInfo (crtc);
  XRRFreeScreenResources (res);

  return ret;
}

Rotation
current_rotation (Display *display,
		  Window root,
		  RRCrtc crtcnum)
{
  Rotation rot;
  XRRScreenResources *res;
  XRRCrtcInfo *crtc;

  res = XRRGetScreenResourcesCurrent (display, root);
  crtc = XRRGetCrtcInfo (display, res, crtcnum);

  rot = crtc->rotation;

  XRRFreeCrtcInfo (crtc);
  XRRFreeScreenResources (res);

  return rot;
}

int
read_events (Display *display,
	     Window root,
	     const XRROutputInfo *output,
	     double tratio)
{
  XEvent e;
  Rotation crot;

  crot = current_rotation (display, root, output->crtc);
  if (verbose) {
    fprintf (stderr, "Current orientation of %s: %u\n", output->name, (unsigned int) crot);
  }

  while (1) {
    XNextEvent(display, &e);

    if (e.type == motion_type) {
      XDeviceMotionEvent *m = (XDeviceMotionEvent *) &e;

      if (m->axes_count > 1) {
	Rotation rot;
	double ratio;
	double x, y;
	int ret;

	x = m->axis_data[m->first_axis];
	y = m->axis_data[m->first_axis + 1];
	if (verbose) {
	  fprintf (stderr, "X: %f, Y: %f\n", x, y);
	}
	ratio = y/x;
	if (ratio >= tratio || ratio <= -tratio) {
	  if (y < 0) {
	    rot = RR_Rotate_180;
	    if (verbose) {
	      fprintf (stderr, "Orientation: inverted\n");
	    }
	  } else {
	    rot = RR_Rotate_0;
	    if (verbose) {
	      fprintf (stderr, "Orientation: normal\n");
	    }
	  }
	} else {
	  if (x < 0) {
	    rot = RR_Rotate_90;
	    if (verbose) {
	      fprintf (stderr, "Orientation: left\n");
	    }
	  } else {
	    rot = RR_Rotate_270;
	    if (verbose) {
	      fprintf (stderr, "Orientation: right\n");
	    }
	  }
	}
	if (rot != crot) {
	  if (verbose) {
	    fprintf (stderr, "Orientation changed: %u\n", (unsigned int) rot);
	  }
	  ret = align_crtc (display, root, output->crtc, rot);
	  if (ret == EXIT_FAILURE) {
	    fprintf (stderr, "Unable to set the CRTC configuration for output %s\n", output->name);
	    return ret;
	  }
	  crot = rot;
	}
      }
    }
  }

  return EXIT_SUCCESS;
}

int
gravitate (Display *display,
	   int argc,
	   const char *argv[],
	   const char *funcname,
	   const char *usage)
{
  RROutput outputid;
  XRROutputInfo *output;
  int ret;
  const char *inputarg;
  XDeviceInfo *input;
  int screen;
  int event_base, error_base;
  double ratio;
  const char *ratioarg;
  char *ratioend;

  ret = get_screen (display, argc, argv, funcname, usage, &screen);
  if (ret == EXIT_FAILURE) {
    return ret;
  }

  ret = get_argval (argc, argv, "ratio", funcname, usage, "2.0", &ratioarg);
  if (ret == EXIT_FAILURE) {
    return ret;
  } else {
    ratio = strtod (ratioarg, &ratioend);
    if (ratioend != NULL && strlen (ratioend) > 0) {
      fprintf (stderr, "Invalid number: %s\n", ratioarg);
      return EXIT_FAILURE;
    }
  }

  ret = get_argval (argc, argv, "input", funcname, usage, "Virtual core pointer", &inputarg);
  if (ret == EXIT_FAILURE) {
    return ret;
  }
  input = find_device_info(display, inputarg, False);
  if (!input) {
    fprintf(stderr, "Unable to find device: %s\n", inputarg);
    ret = EXIT_FAILURE;
    return ret;
  }

  ret = get_output (display, argc, argv, funcname, usage, &outputid, &output);

  if (ret != EXIT_FAILURE) {
    Window root;

    root = RootWindow (display, screen);
    if (register_events(display, input, inputarg, False)) {
      ret = read_events (display, root, output, ratio);
    } else {
      fprintf(stderr, "Unable to register for input events.\n");
      ret = EXIT_FAILURE;
    }
  }

  XRRFreeOutputInfo (output);
  return ret;
}

/* end of gravitate.c */
