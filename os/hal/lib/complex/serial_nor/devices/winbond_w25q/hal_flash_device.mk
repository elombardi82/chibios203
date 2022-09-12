# List of all the WINBOND W25Q device files.
SNORSRC := $(CHIBIOS)/os/hal/lib/complex/serial_nor/hal_serial_nor.c \
           $(CHIBIOS)/os/hal/lib/complex/serial_nor/devices/winbond_w25q/hal_flash_device.c

# Required include directories
SNORINC := $(CHIBIOS)/os/hal/lib/complex/serial_nor \
           $(CHIBIOS)/os/hal/lib/complex/serial_nor/devices/winbond_w25q

# Shared variables
ALLCSRC += $(SNORSRC)
ALLINC  += $(SNORINC)
