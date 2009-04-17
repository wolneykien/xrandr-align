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
#include <string.h>

extern void print_classes_xi2(Display*, XIAnyClassInfo **classes,
                              int num_classes);

#define BitIsOn(ptr, bit) (((unsigned char *) (ptr))[(bit)>>3] & (1 << ((bit) & 7)))
#define SetBit(ptr, bit)  (((unsigned char *) (ptr))[(bit)>>3] |= (1 << ((bit) & 7)))

static Window create_win(Display *dpy)
{
    Window win = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), 0, 0, 200,
            200, 0, 0, WhitePixel(dpy, 0));
    Window subwindow = XCreateSimpleWindow(dpy, win, 50, 50, 50, 50, 0, 0,
            BlackPixel(dpy, 0));

    XMapWindow(dpy, subwindow);
    XMapWindow(dpy, win);
    XFlush(dpy);
    return win;
}

static void print_deviceevent(XIDeviceEvent* event)
{
    double *val;
    int i;

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

static void print_devicechangedevent(Display *dpy, XIDeviceChangedEvent *event)
{
    printf("    device: %d (%d)\n", event->deviceid, event->sourceid);
    printf("    reason: %s\n", (event->reason == SlaveSwitch) ? "SlaveSwitch" :
                                "DeviceChanged");
    print_classes_xi2(dpy, event->classes, event->num_classes);
}

static void print_hierarchychangedevent(XIDeviceHierarchyEvent *event)
{
    int i;
    printf("    Changes happened: %s %s %s %s %s %s %s\n",
            (event->flags & HF_MasterAdded) ? "[new master]" : "",
            (event->flags & HF_MasterRemoved) ? "[master removed]" : "",
            (event->flags & HF_SlaveAdded) ? "[new slave]" : "",
            (event->flags & HF_SlaveRemoved) ? "[slave removed]" : "",
            (event->flags & HF_SlaveAttached) ? "[slave attached]" : "",
            (event->flags & HF_DeviceEnabled) ? "[device enabled]" : "",
            (event->flags & HF_DeviceDisabled) ? "[device disabled]" : "");

    for (i = 0; i < event->num_devices; i++)
    {
        char *use;
        switch(event->info[i].use)
        {
            case MasterPointer: use = "master pointer";
            case MasterKeyboard: use = "master keyboard"; break;
            case SlavePointer: use = "slave pointer";
            case SlaveKeyboard: use = "slave keyboard"; break;
            case FloatingSlave: use = "floating slave"; break;
                break;
        }

        printf("    device %d [%s (%d)] is %s\n",
                event->info[i].deviceid,
                use,
                event->info[i].attachment,
                (event->info[i].enabled) ? "enabled" : "disabled");
    }
}

static void print_rawevent(XIRawDeviceEvent *event)
{
    int i;
    double *val, *raw_val;

    printf("    device: %d\n", event->deviceid);
    printf("    detail: %d\n", event->detail);
    printf("    valuators:\n");

    val = event->valuators->values;
    raw_val = event->raw_values;
    for (i = 0; i < event->valuators->mask_len * 8; i++)
        if (BitIsOn(event->valuators->mask, i))
            printf("         %2d: %.2f (%.2f)\n", i, *val++, *raw_val++);
    printf("\n");
}

static void print_enterleave(XILeaveEvent* event)
{
    char *mode, *detail;
    int i;

    printf("    windows: root 0x%lx event 0x%lx child 0x%ld\n",
            event->root, event->event, event->child);
    switch(event->mode)
    {
        case NotifyNormal:       mode = "NotifyNormal"; break;
        case NotifyGrab:         mode = "NotifyGrab"; break;
        case NotifyUngrab:       mode = "NotifyUngrab"; break;
        case NotifyWhileGrabbed: mode = "NotifyWhileGrabbed"; break;
    }
    switch (event->detail)
    {
        case NotifyAncestor: detail = "NotifyAncestor"; break;
        case NotifyVirtual: detail = "NotifyVirtual"; break;
        case NotifyInferior: detail = "NotifyInferior"; break;
        case NotifyNonlinear: detail = "NotifyNonlinear"; break;
        case NotifyNonlinearVirtual: detail = "NotifyNonlinearVirtual"; break;
        case NotifyPointer: detail = "NotifyPointer"; break;
        case NotifyPointerRoot: detail = "NotifyPointerRoot"; break;
        case NotifyDetailNone: detail = "NotifyDetailNone"; break;
    }
    printf("    mode: %s (detail %s)\n", mode, detail);
    printf("    flags: %s %s\n", event->focus ? "[focus]" : "",
                                 event->same_screen ? "[same screen]" : "");
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

    printf("    root x/y:  %.2f / %.2f\n", event->root_x, event->root_y);
    printf("    event x/y: %.2f / %.2f\n", event->event_x, event->event_x);

}

int
test_xi2(Display	*display,
         int	argc,
         char	*argv[],
         char	*name,
         char	*desc)
{
    XIDeviceEventMask mask;
    Window win;

    list(display, argc, argv, name, desc);
    win = create_win(display);

    /* Select for motion events */
    mask.deviceid = AllDevices;
    mask.mask_len = 2;
    mask.mask = calloc(2, sizeof(char));
    SetBit(mask.mask, XI_ButtonPress);
    SetBit(mask.mask, XI_ButtonRelease);
    SetBit(mask.mask, XI_Motion);
    SetBit(mask.mask, XI_KeyPress);
    SetBit(mask.mask, XI_KeyPress);
    SetBit(mask.mask, XI_DeviceChanged);
    SetBit(mask.mask, XI_Enter);
    SetBit(mask.mask, XI_Leave);
    SetBit(mask.mask, XI_FocusIn);
    SetBit(mask.mask, XI_FocusOut);
    SetBit(mask.mask, XI_HierarchyChanged);
    XISelectEvent(display, win, &mask, 1);

    mask.deviceid = AllMasterDevices;
    memset(mask.mask, 0, 2);
    SetBit(mask.mask, XI_RawEvent);
    XISelectEvent(display, DefaultRootWindow(display), &mask, 1);

    free(mask.mask);

    while(1)
    {
        XIEvent ev;
        XNextEvent(display, (XEvent*)&ev);
        if (ev.type == GenericEvent)
        {
            XIDeviceEvent *event = (XIDeviceEvent*)&ev;

            printf("EVENT type %d\n", event->evtype);
            switch (event->evtype)
            {
                case XI_DeviceChanged:
                    print_devicechangedevent(display,
                                             (XIDeviceChangedEvent*)event);
                    break;
                case XI_HierarchyChanged:
                    print_hierarchychangedevent((XIDeviceHierarchyEvent*)event);
                    break;
                case XI_RawEvent:
                    print_rawevent((XIRawDeviceEvent*)event);
                    break;
                case XI_Enter:
                case XI_Leave:
                case XI_FocusIn:
                case XI_FocusOut:
                    print_enterleave((XILeaveEvent*)event);
                    break;
                default:
                    print_deviceevent(event);
                    break;
            }
        }

        XIFreeEventData(&ev);
    }

    return EXIT_SUCCESS;
}
