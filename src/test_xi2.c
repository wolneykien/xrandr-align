/*
 * Copyright © 2009 Red Hat, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */


#include "xinput.h"


#define BitIsOn(ptr, bit) (((BYTE *) (ptr))[(bit)>>3] & (1 << ((bit) & 7)))

static Window create_win(Display *dpy)
{
    Window win = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), 0, 0, 200,
            200, 0, 0, WhitePixel(dpy, 0));

    XMapWindow(dpy, win);
    XFlush(dpy);
    return win;
}

int
test_xi2(Display	*display,
         int	argc,
         char	*argv[],
         char	*name,
         char	*desc)
{
    int i;
    XIDeviceEventMask mask;
    Window win;

    list(display, argc, argv, name, desc);
    win = create_win(display);

    /* Select for motion events */
    mask.deviceid = AllDevices;
    mask.mask_len = 1;
    mask.mask = calloc(1, sizeof(char));
    mask.mask[0] = XI_ButtonPressMask | XI_ButtonReleaseMask | XI_MotionMask |
        XI_KeyPressMask | XI_KeyReleaseMask;
    XISelectEvent(display, win, &mask, 1);
    free(mask.mask);

    while(1)
    {
        XIEvent ev;
        XNextEvent(display, (XEvent*)&ev);
        if (ev.type == GenericEvent)
        {
            XIDeviceEvent *event = (XIDeviceEvent*)&ev;
            double *val;

            printf("EVENT type %d\n", event->evtype);
            printf("    device: %d (%d)\n", event->deviceid, event->sourceid);
            printf("    detail: %d\n", event->detail);
            printf("    buttons:");
            for (i = 0; i < event->buttons->mask_len * 8; i++)
                if (BitIsOn(event->buttons->mask, i))
                    printf(" %d", i);
            printf("\n");

            printf("    modifiers: locked 0x%x latched 0x%x base 0x%x\n",
                    event->mods->locked, event->mods->latched,
                    event->mods->base);
            printf("    group: locked 0x%x latched 0x%x base 0x%x\n",
                    event->group->locked, event->group->latched,
                    event->group->base);
            printf("    valuators:");

            val = event->valuators->values;
            for (i = 0; i < event->valuators->mask_len * 8; i++)
                if (BitIsOn(event->valuators->mask, i))
                    printf(" %.2f", *val++);
            printf("\n");

            printf("    windows: root 0x%lx event 0x%lx child 0x%ld\n",
                    event->root, event->event, event->child);
        }

        XIFreeEventData(&ev);
    }

    return EXIT_SUCCESS;
}
