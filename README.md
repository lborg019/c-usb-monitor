# c-usb-monitor
C program to monitor existence, additions, and removal of USB devices on Linux machines.

## Dependencies:
- [libudev](https://www.freedesktop.org/software/systemd/man/libudev.html)
- [pthreads](http://man7.org/linux/man-pages/man7/pthreads.7.html)

## libudev
`sudo apt-get update`
`sudo apt-get install libudev-dev` 

## Compilation/Run:
`gcc -Wall -g -o um usbmonitor.c -ludev -pthread`

`./um` 

### Links/Ref:
- [signal11](http://www.signal11.us/oss/udev/) (dead)
- [signal11](https://web.archive.org/web/20190210015919/www.signal11.us/oss/udev/) (snapshot)
- [gavv@github](https://github.com/gavv/snippets/blob/master/udev/udev_monitor_usb.c)
- [howtoinstall.co](https://www.howtoinstall.co/en/debian/stretch/libudev-dev)