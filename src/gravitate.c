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
#include <time.h>

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
align_screen (Display *display,
	    Window root,
	    Rotation rot)
{
  int ret;
  XRRScreenConfiguration *sconf;
  SizeID ssize;
  Rotation crot;
  Status status;

  sconf = XRRGetScreenInfo (display, root);
  ssize = XRRConfigCurrentConfiguration (sconf, &crot);

  ret = EXIT_SUCCESS;
  if (rot != crot) {
    status = XRRSetScreenConfig (display, sconf, root, ssize, rot, CurrentTime);
    if (status != RRSetConfigSuccess) {
      ret = EXIT_FAILURE;
    }
  }

  XRRFreeScreenConfigInfo (sconf);

  return ret;
}

Rotation
current_rotation (Display *display,
		  Window root)
                  /* RRCrtc crtcnum)*/
{
  Rotation rot;
  XRRScreenConfiguration *sconf;
  SizeID ssize;

  sconf = XRRGetScreenInfo (display, root);
  ssize = XRRConfigCurrentConfiguration (sconf, &rot);

  XRRFreeScreenConfigInfo (sconf);

  return rot;
}

int
get_thresholds (XDeviceInfo *info,
		double thr,
		double *xthr,
		double *ythr)
{
    XAnyClassPtr any;
    XValuatorInfoPtr v;
    XAxisInfoPtr a;
    int i, j;

    if (info->num_classes > 0) {
        any = (XAnyClassPtr) (info->inputclassinfo);
	for (i = 0; i < info->num_classes; i++) {
	    if (any->class == ValuatorClass) {
		v = (XValuatorInfoPtr) any;

		if (v->num_axes < 2) {
		    continue;
		}

		a = (XAxisInfoPtr) ((char *) v + sizeof (XValuatorInfo));
		*xthr = (a->max_value - a->min_value) * thr;
		a++;
		*ythr = (a->max_value - a->min_value) * thr;

		return EXIT_SUCCESS;
	    }
	}
    }

    return EXIT_FAILURE;
}

int
read_events (Display *display,
	     Window root,
	     /*	const XRROutputInfo *output,*/
	     XDeviceInfo *input,
	     double tratio,
	     double thr)
{
  XEvent e;
  Rotation crot;
  time_t rtime;
  double xthr, ythr;
  int ret;
  int asleep = 0;

  if (! register_events(display, input, "", False)) {
    fprintf(stderr, "Unable to register for input events.\n");
    return EXIT_FAILURE;
  }

  ret = get_thresholds (input, thr, &xthr, &ythr);
  if (ret != EXIT_SUCCESS) {
    fprintf (stderr, "Unable to calculate the threshold values for the axes\n");
    return ret;
  }

  crot = current_rotation (display, root);
  rtime = time (NULL);

/*
  if (verbose) {
    fprintf (stderr, "Current orientation of %s: %u\n", output->name, (unsigned int) crot);
  }
*/

  while (1) {
    XNextEvent(display, &e);

    if (e.type == motion_type) {
      if (asleep) {
        if ((time (NULL) - rtime) < 1) {
          continue;
        } else {
	  asleep = 0;
	  if (verbose) {
            fprintf (stderr, "...Woken up.\n");
	  }
        }
      }
      XDeviceMotionEvent *m = (XDeviceMotionEvent *) &e;

      if (m->axes_count > 1) {
	Rotation rot = 0;
	double ratio;
	double x, y;
	int ret;

	x = m->axis_data[m->first_axis];
	y = m->axis_data[m->first_axis + 1];
	ratio = y/x;
	if (ratio >= tratio || ratio <= -tratio) {
	  if (y < -ythr) {
	    rot = RR_Rotate_180;
	  } else if (y > ythr) {
	    rot = RR_Rotate_0;
	  }
	} else {
	  if (x < -xthr) {
	    rot = RR_Rotate_90;
	  } else if (x > xthr) {
	    rot = RR_Rotate_270;
	  }
	}
	if (rot && rot != crot) {
	  if (verbose) {
	    fprintf (stderr, "X: %f, Y: %f\n", x, y);
	    fprintf (stderr, "Orientation changed: %u\n", (unsigned int) rot);
	  }
	  ret = align_screen (display, root, rot);
	  if (ret == EXIT_FAILURE) {
	    fprintf (stderr, "Unable to set the screen configuration\n");
	    return ret;
	  }
	  crot = rot;
	  if (verbose) {
            fprintf (stderr, "Enter sleep...\n");
	  }
	  rtime = time (NULL);
	  asleep = 1;
	}
      }
    }
  }

  return ret;
}

int
gravitate (Display *display,
	   int argc,
	   const char *argv[],
	   const char *funcname,
	   const char *usage)
{
  /*  RROutput outputid;
      XRROutputInfo *output;*/
  int ret;
  const char *inputarg;
  XDeviceInfo *input;
  int screen;
  int event_base, error_base;
  double ratio;
  const char *ratioarg;
  char *ratioend;
  double thr;
  const char *thrarg;
  char *thrend;

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

  ret = get_argval (argc, argv, "threshold", funcname, usage, "0.12", &thrarg);
  if (ret == EXIT_FAILURE) {
    return ret;
  } else {
    thr = strtod (thrarg, &thrend);
    if (thrend != NULL && strlen (thrend) > 0) {
      fprintf (stderr, "Invalid number: %s\n", thrarg);
      return EXIT_FAILURE;
    }
  }

  ret = get_argval (argc, argv, "input", funcname, usage, "Virtual core pointer", &inputarg);
  if (ret == EXIT_FAILURE) {
    return ret;
  }
  input = find_device_info_ext (display, inputarg, False, Absolute, 2, True);
  if (!input) {
    fprintf(stderr, "Unable to find device: %s\n", inputarg);
    ret = EXIT_FAILURE;
    return ret;
  }

  /* ret = get_output (display, argc, argv, funcname, usage, &outputid, &output); */

  if (ret != EXIT_FAILURE) {
    Window root;

    root = RootWindow (display, screen);
    ret = read_events (display, root, input, ratio, thr);
  }

  /*  XRRFreeOutputInfo (output); */
  return ret;
}

/* end of gravitate.c */
