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

#include "xinput.h"
#include <string.h>
#include <X11/extensions/XIproto.h> /* for XI_Device***ChangedNotify */

static void
print_info(Display* dpy, XDeviceInfo	*info, Bool shortformat)
{
    int			i,j;
    XAnyClassPtr	any;
    XKeyInfoPtr		k;
    XButtonInfoPtr	b;
    XValuatorInfoPtr	v;
    XAxisInfoPtr	a;

    printf("\"%s\"\tid=%ld\t[", info->name, info->id);

    switch (info->use) {
    case IsXPointer:
       printf("XPointer");
       break;
    case IsXKeyboard:
       printf("XKeyboard");
       break;
    case IsXExtensionDevice:
       printf("XExtensionDevice");
       break;
    case IsXExtensionKeyboard:
       printf("XExtensionKeyboard");
       break;
    case IsXExtensionPointer:
       printf("XExtensionPointer");
       break;
    default:
       printf("Unknown class");
       break;
    }
    printf("]\n");

    if (shortformat)
        return;

    if(info->type != None)
	printf("\tType is %s\n", XGetAtomName(dpy, info->type));

    if (info->num_classes > 0) {
	any = (XAnyClassPtr) (info->inputclassinfo);
	for (i=0; i<info->num_classes; i++) {
	    switch (any->class) {
	    case KeyClass:
		k = (XKeyInfoPtr) any;
		printf("\tNum_keys is %d\n", k->num_keys);
		printf("\tMin_keycode is %d\n", k->min_keycode);
		printf("\tMax_keycode is %d\n", k->max_keycode);
		break;

	    case ButtonClass:
		b = (XButtonInfoPtr) any;
		printf("\tNum_buttons is %d\n", b->num_buttons);
		break;

	    case ValuatorClass:
		v = (XValuatorInfoPtr) any;
		a = (XAxisInfoPtr) ((char *) v +
				    sizeof (XValuatorInfo));
		printf("\tNum_axes is %d\n", v->num_axes);
		printf("\tMode is %s\n", (v->mode == Absolute) ? "Absolute" : "Relative");
		printf("\tMotion_buffer is %ld\n", v->motion_buffer);
		for (j=0; j<v->num_axes; j++, a++) {
		    printf("\tAxis %d :\n", j);
		    printf("\t\tMin_value is %d\n", a->min_value);
		    printf("\t\tMax_value is %d\n", a->max_value);
		    printf ("\t\tResolution is %d\n", a->resolution);
		}
		break;
	    default:
		printf ("unknown class\n");
	    }
	    any = (XAnyClassPtr) ((char *) any + any->length);
	}
    }
}

static int list_xi1(Display     *display,
                    int	        argc,
                    char        *argv[],
                    char        *name,
                    char        *desc)
{
    XDeviceInfo		*info;
    int			loop;
    int                 shortformat = False;
    int                 daemon = False;

    shortformat = (argc == 1 && strcmp(argv[0], "--short") == 0);
    daemon = (argc == 1 && strcmp(argv[0], "--loop") == 0);

    if (argc == 0 || shortformat || daemon) {
	int		num_devices;

        do {
            info = XListInputDevices(display, &num_devices);
            for(loop=0; loop<num_devices; loop++) {
                print_info(display, info+loop, shortformat);
            }
        } while(daemon);
    } else {
	int	ret = EXIT_SUCCESS;

	for(loop=0; loop<argc; loop++) {
	    info = find_device_info(display, argv[loop], False);

	    if (!info) {
		fprintf(stderr, "unable to find device %s\n", argv[loop]);
		ret = EXIT_FAILURE;
	    } else {
		print_info(display, info, shortformat);
	    }
	}
	return ret;
    }
    return EXIT_SUCCESS;
}

#ifdef HAVE_XI2
/* also used from test_xi2.c */
void
print_classes_xi2(Display* display, XIAnyClassInfo **classes,
                  int num_classes)
{
    int i;

    printf("\tReporting %d classes:\n", num_classes);
    for (i = 0; i < num_classes; i++)
    {
        switch(classes[i]->type)
        {
            case ButtonClass:
                {
                    XIButtonClassInfo *b = (XIButtonClassInfo*)classes[i];
                    printf("\t\tButtons supported: %d\n", b->num_buttons);

                }
                break;
            case KeyClass:
                {
                    XIKeyClassInfo *k = (XIKeyClassInfo*)classes[i];
                    printf("\t\tKeycodes supported: %d\n", k->num_keycodes);
                }
                break;
            case ValuatorClass:
                {
                    XIValuatorClassInfo *v = (XIValuatorClassInfo*)classes[i];
                    printf("\t\tDetail for Valuator %d:\n", v->number);
                    printf("\t\t  Name: %s\n", XGetAtomName(display, v->name));
                    printf("\t\t  Range: %f - %f\n", v->min, v->max);
                    printf("\t\t  Resolution: %d units/m\n", v->resolution);
                    printf("\t\t  Mode: %s\n", v->mode == Absolute ? "absolute" :
                            "relative");
                }
                break;
        }
    }

    printf("\n");
}

static void
print_info_xi2(Display* display, XIDeviceInfo *dev, Bool shortformat)
{
    printf("%-40s\tid=%d\t[", dev->name, dev->deviceid);
    switch(dev->use)
    {
        case XIMasterPointer:
            printf("master pointer  (%d)]\n", dev->attachment);
            break;
        case XIMasterKeyboard:
            printf("master keyboard (%d)]\n", dev->attachment);
            break;
        case XISlavePointer:
            printf("slave  pointer  (%d)]\n", dev->attachment);
            break;
        case XISlaveKeyboard:
            printf("slave  keyboard (%d)]\n", dev->attachment);
            break;
        case XIFloatingSlave:
            printf("floating slave]\n");
            break;
    }

    if (shortformat)
        return;

    if (!dev->enabled)
        printf("\tThis device is disabled\n");

    print_classes_xi2(display, dev->classes, dev->num_classes);
}


int
list_xi2(Display	*display,
         int	argc,
         char	*argv[],
         char	*name,
         char	*desc)
{
    int major = XI_2_Major,
        minor = XI_2_Minor;
    int ndevices;
    int i, j, shortformat;
    XIDeviceInfo *info, *dev;

    shortformat = (argc == 1 && strcmp(argv[0], "--short") == 0);

    if (XIQueryVersion(display, &major, &minor) != Success ||
        (major * 1000 + minor) < (XI_2_Major * 1000 + XI_2_Minor))
    {
        fprintf(stderr, "XI2 not supported.\n");
        return EXIT_FAILURE;
    }

    info = XIQueryDevice(display, XIAllDevices, &ndevices);
    dev = info;

    for(i = 0; i < ndevices; i++)
    {
        dev = &info[i];
        if (dev->use == XIMasterPointer || dev->use == XIMasterKeyboard)
        {
            if (dev->use == XIMasterPointer)
                printf("⎡ ");
            else
                printf("⎣ ");

            print_info_xi2(display, dev, shortformat);
            for (j = 0; j < ndevices; j++)
            {
                XIDeviceInfo* sd = &info[j];

                if ((sd->use == XISlavePointer || sd->use == XISlaveKeyboard) &&
                     (sd->attachment == dev->deviceid))
                {
                    printf("%s   ↳ ", dev->use == XIMasterPointer ? "⎜" : " ");
                    print_info_xi2(display, sd, shortformat);
                }
            }
        }
    }


    XIFreeDeviceInfo(info);
    return EXIT_SUCCESS;
}
#endif

int
list(Display	*display,
     int	argc,
     char	*argv[],
     char	*name,
     char	*desc)
{
#ifdef HAVE_XI2
    if (xinput_version(display) == XI_2_Major)
        return list_xi2(display, argc, argv, name, desc);
#endif
    return list_xi1(display, argc, argv, name, desc);
}

/* end of list.c */
