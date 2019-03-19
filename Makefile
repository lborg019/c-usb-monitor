um: usbmonitor.c
	gcc -Wall -g -o um usbmonitor.c -ludev -pthread
clean:
	rm um