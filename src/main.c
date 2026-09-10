#include "mem/cartridge.h"
#include "gbc.h"



int main(int argc, char* argv[]) {
    gbc_t gbc;

    gbc_init(&gbc, argv[1]);
    cartridge_display_info(&gbc.cartridge);
    return 0;
}
