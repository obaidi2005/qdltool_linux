#include "qdl_usb.h"
#include "generic.h"
#include <sys/ioctl.h>

#define EP_OUT 0x01
#define EP_IN  0x81
#define VENDOR_ID 0x05c6
#define PRODUCT_ID 0x9008

struct libusb_device_handle *handle;

int qdl_usb_init()
{
    int r;
    r = libusb_init(NULL);
    if (r != 0){
        xerror("libusb init error");
    } 
    handle = libusb_open_device_with_vid_pid(NULL, VENDOR_ID, PRODUCT_ID);
    if (handle == NULL){
        xerror("usb error");
    }
    libusb_detach_kernel_driver(handle, 0); //interface 0
    r = libusb_claim_interface(handle, 0);  //interface 0
    if (r != 0){
        xerror("usb claim interface error");
    }
}

int write_tx(void *buf, int len, int *act)
{
    int ret;
    int nil;
    int *count;
    if (!act)
        count = &nil;
    else
        count = act;

    do{
        ret  = libusb_bulk_transfer(handle, EP_OUT, buf, len, count, 1000*5);
        if (ret == LIBUSB_ERROR_PIPE && *count== 0) {
            libusb_clear_halt(handle, EP_OUT);
            continue;
        }else{
            return ret;
        }
    }while(1);
}

int read_rx_timeout(void *buf, int length, int *act, int timeout)
{
    int ret;
    int nil;
    int *count;
    if (!act)
        count = &nil;
    else
        count = act;

    do{
        ret = libusb_bulk_transfer(handle, EP_IN, buf, length, count, timeout);
        if (ret == LIBUSB_ERROR_PIPE && *count == 0) {
            libusb_clear_halt(handle, EP_IN);
            continue;
        }else{
            return ret;
        }
    }while(1);
}

int read_rx(void *buf, int length, int *act)
{
    return read_rx_timeout(buf, length, act, 0);
}

void qdl_usb_close()
{
    libusb_release_interface(handle, 0);
    libusb_close(handle);
    libusb_exit(NULL);
}
