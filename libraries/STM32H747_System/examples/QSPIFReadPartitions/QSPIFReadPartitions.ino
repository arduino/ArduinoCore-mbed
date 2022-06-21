#include <BlockDevice.h>

struct __attribute__((packed)) mbrEntry {
    uint8_t status;
    uint8_t chsStart[3];
    uint8_t type;
    uint8_t chsStop[3];
    uint32_t lbaOffset;
    uint32_t lbaSize;
};

struct __attribute__((packed)) mbrTable {
    mbrEntry entries[4];
    uint8_t signature[2];
};

using namespace mbed;

unsigned long allocatedSpace {};

void setup()
{
    Serial.begin(115200);
    for (const auto timeout = millis() + 2500; !Serial && millis() < timeout; delay(250))
        ;

    auto bd = BlockDevice::get_default_instance();
    auto ret = bd->init();
    if (ret) {
        Serial.println("ERROR! Unable to read the Block Device.");
        while (true)
            ;
    }

    // Allocate smallest buffer necessary to write MBR
    auto buffer_size = std::max<uint32_t>(bd->get_program_size(), sizeof(mbrTable));

    // Prevent alignment issues
    if (buffer_size % bd->get_program_size() != 0) {
        buffer_size += bd->get_program_size() - (buffer_size % bd->get_program_size());
    }

    auto buffer = new uint8_t[buffer_size];

    // Check for existing MBR
    ret = bd->read(buffer, 512 - buffer_size, buffer_size);
    if (ret) {
        Serial.println("ERROR! Unable to read the Master Boot Record");

        delete[] buffer;
        while (true)
            ;
    }

    auto table_start_offset = buffer_size - sizeof(mbrTable);
    auto table = reinterpret_cast<mbrTable*>(&buffer[table_start_offset]);

    Serial.println();
    Serial.print("Looking for Partitions on the Flash Memory... ");

    if (table->signature[0] != 0x55 || table->signature[1] != 0xAA) {
        Serial.println("MBR Not Found");
        Serial.println("Flash Memory doesn't have partitions.");
    } else {

        Serial.println("MBR Found");
        Serial.print("Boot Signature: 0x");
        Serial.print(table->signature[0], HEX);
        Serial.println(table->signature[1], HEX);

        Serial.println();
        Serial.println("Printing Partitions Table and Info...");

        auto part { 1u };
        for (auto const& entry : table->entries) {
            Serial.println("================================");
            Serial.print("Partition: ");
            Serial.println(part++);

            Serial.print("Bootable: ");
            Serial.println(entry.status == 0 ? "No" : "Yes");

            Serial.print("Type: 0x");
            if (entry.type < 0x10)
                Serial.print(0);
            Serial.println(entry.type, HEX);

            if (entry.type == 0x00)
                continue;

            Serial.print("Size [KBytes]: ");
            Serial.println((entry.lbaSize * 4096) >> 10);

            allocatedSpace += entry.lbaSize * 4096;

            Serial.print("Start [C/H/S]: ");
            Serial.print(entry.chsStart[0]);
            Serial.print("/");
            Serial.print(entry.chsStart[1]);
            Serial.print("/");
            Serial.println(entry.chsStart[2]);

            Serial.print("Stop [C/H/S]: ");
            Serial.print(entry.chsStop[0]);
            Serial.print("/");
            Serial.print(entry.chsStop[1]);
            Serial.print("/");
            Serial.println(entry.chsStop[2]);

            Serial.println();
        }

        Serial.println();
        Serial.println("No more partitions are present.");
    }

    Serial.println();
    Serial.print("Total Space [KBytes]:         ");
    Serial.println(bd->size() >> 10);
    Serial.print("Allocated Space [KBytes]:     ");
    Serial.println(allocatedSpace >> 10);
    Serial.print("Unallocated Space [KBytes]:   ");
    Serial.println((bd->size() - allocatedSpace) >> 10);
}

void loop()
{
    delay(10000);
}
