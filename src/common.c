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

#include "string.h"
#include "common.h"

int
get_argval (int argc,
	    const char *argv[],
	    const char *argname,
	    const char *funcname,
	    const char *usage,
	    const char *defval,
	    const char **outval)
{
  int i;

  for (i = 0; i < argc; i++) {
    if (strlen (argv[i]) > 2 &&
	strncmp (argv[i], "--", 2) == 0 &&
	strncmp (argv[i] + 2, argname, strlen(argname)) == 0) {
      char *eq;
      if (!(eq = strchr (argv[i], '=')) ||
	  strlen (eq) < 2)
	{
	  fprintf (stderr, "Usage: %s %s\n", funcname, usage);
	  return EXIT_FAILURE;
	}
      else
	{
	  *outval = eq + 1;
	  return EXIT_SUCCESS;
	}
    }
  }

  *outval = defval;
  return EXIT_SUCCESS;
}

int
get_screen (Display *display,
	    int	argc,
	    const char *argv[],
	    const char *funcname,
	    const char *usage,
	    int *retscreen)
{
  long screen;
  const char *screenarg;
  int ret;

  ret = get_argval (argc, argv, "screen", funcname, usage, "-1", &screenarg);
  if (ret != EXIT_FAILURE) {
    char *endptr;
    screen = strtol (screenarg, &endptr, 0);
    if (endptr == screenarg) {
      fprintf (stderr, "Invalid number: %s\n", screenarg);
      ret = EXIT_FAILURE;
    }
  }

  if (ret != EXIT_FAILURE) {
    if (screen < 0) {
      screen = DefaultScreen (display);
    } else if (screen >= ScreenCount (display)) {
      fprintf (stderr, "Invalid screen number %ld (display has %d)\n",
	       screen, ScreenCount (display));
      ret = EXIT_FAILURE;
    }
  }

  if (ret != EXIT_FAILURE) {
    *retscreen = (int)screen;
  }
  return ret;
}

int
check_output (XRRScreenResources *res,
	      int outid)
{
  int o;

  for (o = 0; o < res->noutput; o++) {
    if (res->outputs[o] == outid) {
      return 1;
    }
  }

  return 0;
}

int
get_output (Display *display,
	    int	argc,
	    const char *argv[],
	    const char *funcname,
	    const char *usage,
	    RROutput *retoutputid,
	    XRROutputInfo **retoutput)
{
  int ret;
  int screen;
  const char *outname;

  ret = get_argval (argc, argv, "output", funcname, usage, "", &outname);
  if (ret == EXIT_FAILURE) {
    return ret;
  }

  ret = get_screen (display, argc, argv, funcname, usage, &screen);
  if (ret != EXIT_FAILURE) {
    XRRScreenResources *res;
    Window root;
    int outnum;

    root = RootWindow (display, screen);
    res = XRRGetScreenResourcesCurrent (display, root);

    *retoutput = NULL;
    if (strlen (outname) == 0) {
      outnum = XRRGetOutputPrimary (display, root);
      if (!check_output (res, outnum)) {
	outnum = res->outputs[0];
      }
      *retoutput = XRRGetOutputInfo (display, res, outnum);
      *retoutputid = outnum;
    } else {
      char *endptr;
      outnum = (int) strtol (outname, &endptr, 0);
      if (endptr != NULL && strlen (endptr) != 0) {
	int o;
	ret = EXIT_FAILURE;
	for (o = 0; o < res->noutput; o++) {
	  XRROutputInfo *out = XRRGetOutputInfo (display, res, res->outputs[o]);
	  if (strncmp (out->name, outname, 256) == 0) {
	    *retoutput = out;
      	    *retoutputid = res->outputs[o];
	    ret = EXIT_SUCCESS;
	  } else {
	    XRRFreeOutputInfo (out);
	  }
	}
	if (*retoutput == NULL) {
	  fprintf (stderr, "Output '%s' not found\n", outname);
	  ret = EXIT_FAILURE;
	}
      } else {
	if (check_output (res, outnum)) {
	  *retoutput = XRRGetOutputInfo (display, res, outnum);
	  *retoutputid = outnum;
	} else {
	  fprintf (stderr, "Output with id=%i not found\n", outnum);
	  ret = EXIT_FAILURE;
	}
      }
    }
    XRRFreeScreenResources (res);
  }

  return ret;
}
