#include "kernel.h"
#include "vga.h"

#define ATA_PRIMARY_DATA      0x1F0
#define ATA_PRIMARY_ERROR     0x1F1
#define ATA_PRIMARY_SECTORS   0x1F2
#define ATA_PRIMARY_LBA_LO    0x1F3
#define ATA_PRIMARY_LBA_MID   0x1F4
#define ATA_PRIMARY_LBA_HI    0x1F5
#define ATA_PRIMARY_DRIVE     0x1F6
#define ATA_PRIMARY_COMMAND   0x1F7
#define ATA_PRIMARY_STATUS    0x1F7

#define ATA_CMD_READ          0x20
#define ATA_CMD_WRITE         0x30
#define ATA_CMD_IDENTIFY      0xEC

static int ata_drive_present = 0;

int ata_init(void) {
    outb(ATA_PRIMARY_DRIVE, 0xA0);
    outb(ATA_PRIMARY_SECTORS, 0);
    outb(ATA_PRIMARY_LBA_LO, 0);
    outb(ATA_PRIMARY_LBA_MID, 0);
    outb(ATA_PRIMARY_LBA_HI, 0);
    outb(ATA_PRIMARY_COMMAND, ATA_CMD_IDENTIFY);

    uint8_t status = inb(ATA_PRIMARY_STATUS);
    if (status == 0) {
        ata_drive_present = 0;
        return -1;
    }

    while ((status & 0x80) != 0) {
        if (status & 0x01) {
            ata_drive_present = 0;
            return -1;
        }
        status = inb(ATA_PRIMARY_STATUS);
    }

    while ((status & 0x08) == 0) {
        status = inb(ATA_PRIMARY_STATUS);
    }

    ata_drive_present = 1;
    return 0;
}

int ata_read_sector(uint32_t lba, uint8_t *buffer) {
    if (!ata_drive_present) return -1;

    while (inb(ATA_PRIMARY_STATUS) & 0x80) {}

    outb(ATA_PRIMARY_DRIVE, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_PRIMARY_SECTORS, 1);
    outb(ATA_PRIMARY_LBA_LO, lba & 0xFF);
    outb(ATA_PRIMARY_LBA_MID, (lba >> 8) & 0xFF);
    outb(ATA_PRIMARY_LBA_HI, (lba >> 16) & 0xFF);
    outb(ATA_PRIMARY_COMMAND, ATA_CMD_READ);

    uint8_t status = inb(ATA_PRIMARY_STATUS);
    while ((status & 0x80) != 0) {
        if (status & 0x01) return -1;
        status = inb(ATA_PRIMARY_STATUS);
    }

    for (int i = 0; i < 256; i++) {
        uint16_t data = inw(ATA_PRIMARY_DATA);
        buffer[i * 2] = data & 0xFF;
        buffer[i * 2 + 1] = (data >> 8) & 0xFF;
    }

    return 0;
}

int ata_write_sector(uint32_t lba, uint8_t *buffer) {
    if (!ata_drive_present) return -1;

    while (inb(ATA_PRIMARY_STATUS) & 0x80) {}

    outb(ATA_PRIMARY_DRIVE, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_PRIMARY_SECTORS, 1);
    outb(ATA_PRIMARY_LBA_LO, lba & 0xFF);
    outb(ATA_PRIMARY_LBA_MID, (lba >> 8) & 0xFF);
    outb(ATA_PRIMARY_LBA_HI, (lba >> 16) & 0xFF);
    outb(ATA_PRIMARY_COMMAND, ATA_CMD_WRITE);

    for (int i = 0; i < 256; i++) {
        uint16_t data = buffer[i * 2] | (buffer[i * 2 + 1] << 8);
        outw(ATA_PRIMARY_DATA, data);
    }

    uint8_t status = inb(ATA_PRIMARY_STATUS);
    while ((status & 0x80) != 0) {
        if (status & 0x01) return -1;
        status = inb(ATA_PRIMARY_STATUS);
    }

    return 0;
}

int ata_is_present(void) {
    return ata_drive_present;
}