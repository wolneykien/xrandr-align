/*
 * Copyright 2007 Peter Hutterer
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of the author shall
 * not be used in advertising or otherwise to promote the sale, use or
 * other dealings in this Software without prior written authorization
 * from the author.
 *
 */

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <X11/Xatom.h>
#include <X11/extensions/XIproto.h>

#include "xrandr-align.h"

static Atom parse_atom(Display *dpy, const char *name) {
    Bool is_atom = True;
    int i;

    for (i = 0; name[i] != '\0'; i++) {
        if (!isdigit(name[i])) {
            is_atom = False;
            break;
        }
    }

    if (is_atom)
        return atoi(name);
    else
        return XInternAtom(dpy, name, False);
}

static int
do_set_prop_xi1(Display *dpy, Atom type, int format, int argc, const char **argv, const char *n, const char *desc)
{
    XDeviceInfo  *info;
    XDevice      *dev;
    Atom          prop;
    Atom          old_type;
    const char    *name;
    int           i;
    Atom          float_atom;
    int           old_format, nelements = 0;
    unsigned long act_nitems, bytes_after;
    char         *endptr;
    union {
        unsigned char *c;
        short *s;
        long *l;
        Atom *a;
    } data;

    if (argc < 3)
    {
        fprintf(stderr, "Usage: xinput %s %s\n", n, desc);
        return EXIT_FAILURE;
    }

    info = find_device_info(dpy, argv[0], False);
    if (!info)
    {
        fprintf(stderr, "unable to find device %s\n", argv[0]);
        return EXIT_FAILURE;
    }

    dev = XOpenDevice(dpy, info->id);
    if (!dev)
    {
        fprintf(stderr, "unable to open device %s\n", argv[0]);
        return EXIT_FAILURE;
    }

    name = argv[1];

    prop = parse_atom(dpy, name);

    if (prop == None) {
        fprintf(stderr, "invalid property %s\n", name);
        return EXIT_FAILURE;
    }

    float_atom = XInternAtom(dpy, "FLOAT", False);

    nelements = argc - 2;
    if (type == None || format == 0) {
        if (XGetDeviceProperty(dpy, dev, prop, 0, 0, False, AnyPropertyType,
                               &old_type, &old_format, &act_nitems,
                               &bytes_after, &data.c) != Success) {
            fprintf(stderr, "failed to get property type and format for %s\n",
                    name);
            return EXIT_FAILURE;
        } else {
            if (type == None)
                type = old_type;
            if (format == 0)
                format = old_format;
        }

        XFree(data.c);
    }

    if (type == None) {
        fprintf(stderr, "property %s doesn't exist, you need to specify "
                "its type and format\n", name);
        return EXIT_FAILURE;
    }

    data.c = calloc(nelements, sizeof(long));

    for (i = 0; i < nelements; i++)
    {
        if (type == XA_INTEGER) {
            switch (format)
            {
                case 8:
                    data.c[i] = atoi(argv[2 + i]);
                    break;
                case 16:
                    data.s[i] = atoi(argv[2 + i]);
                    break;
                case 32:
                    data.l[i] = atoi(argv[2 + i]);
                    break;
                default:
                    fprintf(stderr, "unexpected size for property %s", name);
                    return EXIT_FAILURE;
            }
        } else if (type == float_atom) {
            if (format != 32) {
                fprintf(stderr, "unexpected format %d for property %s\n",
                        format, name);
                return EXIT_FAILURE;
            }
            *(float *)(data.l + i) = strtod(argv[2 + i], &endptr);
            if (endptr == argv[2 + i]) {
                fprintf(stderr, "argument %s could not be parsed\n", argv[2 + i]);
                return EXIT_FAILURE;
            }
        } else if (type == XA_ATOM) {
            if (format != 32) {
                fprintf(stderr, "unexpected format %d for property %s\n",
                        format, name);
                return EXIT_FAILURE;
            }
            data.a[i] = parse_atom(dpy, argv[2 + i]);
        } else {
            fprintf(stderr, "unexpected type for property %s\n", name);
            return EXIT_FAILURE;
        }
    }

    XChangeDeviceProperty(dpy, dev, prop, type, format, PropModeReplace,
                          data.c, nelements);
    free(data.c);
    XCloseDevice(dpy, dev);
    return EXIT_SUCCESS;
}

#if HAVE_XI2
static int
do_set_prop_xi2(Display *dpy, Atom type, int format, int argc, const char **argv, const char *n, const char *desc)
{
    XIDeviceInfo *info;
    Atom          prop;
    Atom          old_type;
    const char    *name;
    int           i;
    Atom          float_atom;
    int           old_format, nelements = 0;
    unsigned long act_nitems, bytes_after;
    char         *endptr;
    union {
        unsigned char *c;
        int16_t *s;
        int32_t *l;
    } data;

    if (argc < 3)
    {
        fprintf(stderr, "Usage: xinput %s %s\n", n, desc);
        return EXIT_FAILURE;
    }

    info = xi2_find_device_info(dpy, argv[0]);
    if (!info)
    {
        fprintf(stderr, "unable to find device %s\n", argv[0]);
        return EXIT_FAILURE;
    }

    name = argv[1];

    prop = parse_atom(dpy, name);

    if (prop == None) {
        fprintf(stderr, "invalid property %s\n", name);
        return EXIT_FAILURE;
    }

    float_atom = XInternAtom(dpy, "FLOAT", False);

    nelements = argc - 2;
    if (type == None || format == 0) {
        if (XIGetProperty(dpy, info->deviceid, prop, 0, 0, False,
                          AnyPropertyType, &old_type, &old_format, &act_nitems,
                          &bytes_after, &data.c) != Success) {
            fprintf(stderr, "failed to get property type and format for %s\n",
                    name);
            return EXIT_FAILURE;
        } else {
            if (type == None)
                type = old_type;
            if (format == 0)
                format = old_format;
        }

        XFree(data.c);
    }

    if (type == None) {
        fprintf(stderr, "property %s doesn't exist, you need to specify "
                "its type and format\n", name);
        return EXIT_FAILURE;
    }

    data.c = calloc(nelements, sizeof(int32_t));

    for (i = 0; i < nelements; i++)
    {
        if (type == XA_INTEGER) {
            switch (format)
            {
                case 8:
                    data.c[i] = atoi(argv[2 + i]);
                    break;
                case 16:
                    data.s[i] = atoi(argv[2 + i]);
                    break;
                case 32:
                    data.l[i] = atoi(argv[2 + i]);
                    break;
                default:
                    fprintf(stderr, "unexpected size for property %s", name);
                    return EXIT_FAILURE;
            }
        } else if (type == float_atom) {
            if (format != 32) {
                fprintf(stderr, "unexpected format %d for property %s\n",
                        format, name);
                return EXIT_FAILURE;
            }
            *(float *)(data.l + i) = strtod(argv[2 + i], &endptr);
            if (endptr == argv[2 + i]) {
                fprintf(stderr, "argument %s could not be parsed\n", argv[2 + i]);
                return EXIT_FAILURE;
            }
        } else if (type == XA_ATOM) {
            if (format != 32) {
                fprintf(stderr, "unexpected format %d for property %s\n",
                        format, name);
                return EXIT_FAILURE;
            }
            data.l[i] = parse_atom(dpy, argv[2 + i]);
        } else {
            fprintf(stderr, "unexpected type for property %s\n", name);
            return EXIT_FAILURE;
        }
    }

    XIChangeProperty(dpy, info->deviceid, prop, type, format, PropModeReplace,
                          data.c, nelements);
    free(data.c);
    return EXIT_SUCCESS;
}
#endif

static int
do_set_prop(Display *display, Atom type, int format, int argc, const char *argv[], const char *name, const char *desc)
{
#ifdef HAVE_XI2
    if (xinput_version(display) == XI_2_Major)
        return do_set_prop_xi2(display, type, format, argc, argv, name, desc);
#endif
    return do_set_prop_xi1(display, type, format, argc, argv, name, desc);
}

int
set_float_prop(Display *dpy, int argc, const char** argv, const char* n, const char *desc)
{
    Atom float_atom = XInternAtom(dpy, "FLOAT", False);

    if (sizeof(float) != 4)
    {
	fprintf(stderr, "sane FP required\n");
	return EXIT_FAILURE;
    }

    return do_set_prop(dpy, float_atom, 32, argc, argv, n, desc);
}
