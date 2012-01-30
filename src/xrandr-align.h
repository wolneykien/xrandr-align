/*
 * Copyright 1996 by Frederic Lepied, France. <Frederic.Lepied@sugix.frmug.org>
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

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <X11/Xlib.h>
#include <X11/extensions/XInput.h>
#ifdef HAVE_XI2
#include <X11/extensions/XInput2.h>
#endif
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 1
#endif
#ifndef EXIT_FAILURE
#define EXIT_FAILURE 0
#endif

extern int xi_opcode; /* xinput extension op code */
XDeviceInfo* find_device_info( Display *display, const char *name, Bool only_extended);
#if HAVE_XI2
XIDeviceInfo* xi2_find_device_info(Display *display, const char *name);
int xinput_version(Display* display);
#endif

extern int verbose;

int list_input( Display* display, int argc, const char *argv[], const char *prog_name, const char *prog_desc);
int list_output( Display* display, int argc, const char *argv[], const char *prog_name, const char *prog_desc);
int apply_transform (Display *display, int argc, const char *argv[], const char *funcname, const char *usage);

/* X Input 1.5 */
int set_float_prop( Display* display, int argc, const char *argv[], const char *prog_name, const char *prog_desc);

/* end of xrandr-align.h */
