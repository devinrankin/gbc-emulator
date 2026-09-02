#include "mem/cartridge.h"
#include "gbc.h"


static gbc_t gbc;

int main(int argc, char* argv[]) {
    gbc_init(&gbc, argv[1]);
    cartridge_display_info(&gbc.cartridge);
    return 0;
}
