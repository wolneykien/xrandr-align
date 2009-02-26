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
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 1
#endif
#ifndef EXIT_FAILURE
#define EXIT_FAILURE 0
#endif

XDeviceInfo*
find_device_info(
		 Display	*display,
		 char		*name,
		 Bool		only_extended
		 );


#define DECLARE(name) \
    int (name) ( \
		 Display*	display, \
		 int		argc, \
		 char		*argv[], \
		 char		*prog_name, \
		 char		*prog_desc \
)

DECLARE(get_feedbacks);
DECLARE(set_ptr_feedback);
DECLARE(get_button_map);
DECLARE(set_button_map);
DECLARE(set_pointer);
DECLARE(set_mode);
DECLARE(list);
DECLARE(test);
DECLARE(version);
DECLARE(set_integer_feedback);
DECLARE(query_state);
DECLARE(create_master);
DECLARE(remove_master);
DECLARE(change_attachment);
DECLARE(float_device);
DECLARE(set_clientpointer);
DECLARE(list_props);
DECLARE(set_int_prop);
DECLARE(set_float_prop);
DECLARE(set_atom_prop);
DECLARE(watch_props);
DECLARE(delete_prop);
/* end of xinput.h */
