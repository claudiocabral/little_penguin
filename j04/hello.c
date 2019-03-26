// SPDX-License-Identifier: GPL-2.0+
#include <linux/init.h>
#include <linux/module.h>
#include <linux/usb.h>
#include <linux/hid.h>

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Claudio Cabral <cl@udio.co>");
MODULE_DESCRIPTION("This module talks to Morty with USB");

static int probe(struct usb_interface *intf,
		const struct usb_device_id *id)
{
	pr_info("Keyboar inserted\n");
	return 0;
}

static void disconnect(struct usb_interface *intf)
{
	pr_info("Keyboard disconnected\n");
}

static struct usb_device_id hello_id[] = {
	{ USB_INTERFACE_INFO(USB_INTERFACE_CLASS_HID,
			USB_INTERFACE_SUBCLASS_BOOT,
			USB_INTERFACE_PROTOCOL_KEYBOARD) },
	{ }
};

MODULE_DEVICE_TABLE(usb, hello_id);

static struct usb_driver hello_driver = {
	.name = "hello",
	.id_table = hello_id,
	.probe = probe,
	.disconnect = disconnect,
};

static __init int init_hello(void)
{
	pr_info("Hello Keyboard !\n");
	return usb_register(&hello_driver);
}

static __exit void exit_hello(void)
{
	pr_info("Cleaning up keyboard.\n");
	usb_deregister(&hello_driver);
}

module_init(init_hello);
module_exit(exit_hello);
