#include "cartridge.h"
#include "gb.h"


static emulator_t emulator;

int main(int argc, char* argv[]) { 
    cartridge_t cartridge;
    cartridge_load_file(&cartridge, argv[1]);
    cartridge_parse_header(&cartridge);
    cartridge_display_info(&cartridge);

    return 0;
}
