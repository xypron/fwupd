# MTD Recovery

This plugin allows a system without an exported MTD device to be updated by matching the DMI
details of the platform.

This plugin supports the following protocol ID:

* org.infradead.mtd

## GUID Generation

These devices are generated from the baseboard vendor ID:

* `MTD\VEN_Lenovo&DEV_ThinkPad`
* `MTD\VEN_Lenovo`

## Quirk Use

This plugin uses the following plugin-specific quirks:

### MtdRecoveryGpioNumber

Optional GPIO device number to use as a MUX, e.g. `123`.

Since: 1.7.2

### MtdRecoveryKernelDriver

Kernel driver name, e.g. `npcm-fiu`.

Since: 1.7.2

### MtdRecoveryBindId

Optional kernel ID for binding the device, e.g. `10000000.spi`.

Since: 1.7.2

## Update Behavior

On detach the platform driver for the MTD device is loaded, optionally also using a GPIO MUX.
The discovered MTD device is used as a proxy and the image it written, then on detach the MUX is
optionally disabled and the MTD device unloaded.

## Vendor ID Security

The vendor ID is set from the baseboard vendor, for example `DMI:LENOVO`

## External Interface Access

This plugin requires read/write access to `/dev/mtd`.
