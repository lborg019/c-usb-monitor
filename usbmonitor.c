#include <libudev.h>
#include <stdlib.h>
#include <locale.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>

#define SUBSYSTEM "usb"

// declare threaded function
void* monitor_devices(void*);

struct udev_device* get_child(struct udev* udev, struct udev_device* parent, const char* subsystem)
{
    struct udev_device* child = NULL;
    struct udev_enumerate* enumerate = udev_enumerate_new(udev);

    udev_enumerate_add_match_parent(enumerate, parent);
    udev_enumerate_add_match_subsystem(enumerate, subsystem);
    udev_enumerate_scan_devices(enumerate);

    struct udev_list_entry* devices = udev_enumerate_get_list_entry(enumerate);
    struct udev_list_entry* entry;

    udev_list_entry_foreach(entry, devices){
        const char *path = udev_list_entry_get_name(entry);
        child = udev_device_new_from_syspath(udev, path);
        break;
    }

    udev_enumerate_unref(enumerate);
    return child;
}

static void enumerate_usb_mass_storage(struct udev* udev)
{
    struct udev_enumerate* enumerate = udev_enumerate_new(udev);

    udev_enumerate_add_match_subsystem(enumerate, "scsi");
    udev_enumerate_add_match_property(enumerate, "DEV_TYPE", "scsi_device");
    udev_enumerate_scan_devices(enumerate);

    struct udev_list_entry* devices = udev_enumerate_get_list_entry(enumerate);
    struct udev_list_entry* entry;

    udev_list_entry_foreach(entry, devices) {
        const char* path = udev_list_entry_get_name(entry);
        struct udev_device* scsi = udev_device_new_from_syspath(udev, path);

        struct udev_device* block = get_child(udev, scsi, "block");
        struct udev_device* scsi_disk = get_child(udev, scsi, "scsi_disk");

        struct udev_device* usb = udev_device_get_parent_with_subsystem_devtype(scsi,"usb","usb_device");

        if (block && scsi_disk && usb) {
            printf("block = %s, usb = %s:%s, scsi = %s\n",
                   udev_device_get_devnode(block),
                   udev_device_get_sysattr_value(usb, "idVendor"),
                   udev_device_get_sysattr_value(usb, "idProduct"),
                   udev_device_get_sysattr_value(scsi, "vendor"));
        }

        if(block) udev_device_unref(block);
        if(scsi_disk) udev_device_unref(scsi_disk);

        udev_device_unref(scsi);
    }
    udev_enumerate_unref(enumerate);
}

// monitor USB inputs
static void print_device(struct udev_device* dev)
{
    const char* action = udev_device_get_action(dev);
    if (! action) action = "exists";

    const char* vendor = udev_device_get_sysattr_value(dev, "idVendor");
    if (! vendor) vendor = "0000";

    const char* product = udev_device_get_sysattr_value(dev, "idProduct");
    if (! product) product = "0000";

    printf("%s %s %6s %s:%s: %s\n", udev_device_get_subsystem(dev), udev_device_get_devtype(dev), action, vendor, product, udev_device_get_devnode(dev));
}

static void process_device(struct udev_device* dev)
{
    if (dev) {
        if (udev_device_get_devnode(dev)) print_device(dev);
        udev_device_unref(dev);
    }
}

static void enumerate_devices(struct udev* udev) {
    struct udev_enumerate* enumerate = udev_enumerate_new(udev);

    udev_enumerate_add_match_subsystem(enumerate, SUBSYSTEM);
    udev_enumerate_scan_devices(enumerate);

    struct udev_list_entry* devices = udev_enumerate_get_list_entry(enumerate);
    struct udev_list_entry* entry;

    udev_list_entry_foreach(entry, devices) {
        const char* path = udev_list_entry_get_name(entry);
        struct udev_device* dev = udev_device_new_from_syspath(udev, path);
        process_device(dev);
    }

    udev_enumerate_unref(enumerate);
}

/*
static void monitor_devices(struct udev* udev)
{
    struct udev_monitor* mon = udev_monitor_new_from_netlink(udev, "udev");

    udev_monitor_filter_add_match_subsystem_devtype(mon, SUBSYSTEM, NULL);
    udev_monitor_enable_receiving(mon);

    int fd = udev_monitor_get_fd(mon);

    while(1) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fd, &fds);

        int ret = select(fd+1, &fds, NULL, NULL, NULL);
        if (ret <= 0) break;

        if(FD_ISSET(fd, &fds)) {
            struct udev_device* dev = udev_monitor_receive_device(mon);
            process_device(dev);
        }
    } 
}
*/

int main(void)
{
    // detect when USB is connected
    // fetch the file with the name we want    
    // copy that file to the mopidy directory
    // tell mopidy to reload
    // touchpanel click a button -> QSYS HTTP upload -> Pi, play track start stream -> QSYS catch steam and playback!
    struct udev* udev = udev_new();
    if (!udev) {
        fprintf(stderr, "udev_new() failed\n");
        return 1;
    }

    //enumerate_usb_mass_storage(udev);
    //udev_unref(udev);

    enumerate_devices(udev);

    //create thread for continuous device monitoring:
    pthread_t monitor_thread;
    if(pthread_create(&monitor_thread, NULL, monitor_devices, (void*)(struct udev*) udev) < 0 )
    {
        fprintf(stderr, "pthread_create() failed\n");
        return 1;
    }
    else
    {
        //printf("thread created successfully\n");
    }
    //monitor_devices(udev);

    //udev_unref(udev);
    pthread_join(monitor_thread, NULL);
    udev_unref(udev);
    return 0;
}

// define threaded function
void* monitor_devices(void* udev)
{
    //printf("I'm inside the monitor_devices function!");
    struct udev_monitor* mon = udev_monitor_new_from_netlink(udev, "udev");

    udev_monitor_filter_add_match_subsystem_devtype(mon, SUBSYSTEM, NULL);
    udev_monitor_enable_receiving(mon);

    int fd = udev_monitor_get_fd(mon);

    while(1) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fd, &fds);

        int ret = select(fd+1, &fds, NULL, NULL, NULL);
        if (ret <= 0) break;

        if(FD_ISSET(fd, &fds)) {
            struct udev_device* dev = udev_monitor_receive_device(mon);
            process_device(dev);
        }
    }
    return 0;
}