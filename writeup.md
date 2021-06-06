### 0x01 Vulnerable Code

```c
//  hw/block/nvme.c
static uint16_t nvme_map_prp(QEMUSGList *qsg, QEMUIOVector *iov, uint64_t prp1,
                             uint64_t prp2, uint32_t len, NvmeCtrl *n)
{
    hwaddr trans_len = n->page_size - (prp1 % n->page_size);
    trans_len = MIN(len, trans_len);
    int num_prps = (len >> n->page_bits) + 1;

    if (unlikely(!prp1)) {
        trace_pci_nvme_err_invalid_prp();
        return NVME_INVALID_FIELD | NVME_DNR;
    } else if (n->bar.cmbsz && prp1 >= n->ctrl_mem.addr &&
               prp1 < n->ctrl_mem.addr + int128_get64(n->ctrl_mem.size)) {
        qsg->nsg = 0;
        qemu_iovec_init(iov, num_prps);
        qemu_iovec_add(iov, (void *)&n->cmbuf[prp1 - n->ctrl_mem.addr], trans_len);
    } else {
        pci_dma_sglist_init(qsg, &n->parent_obj, num_prps);
        qemu_sglist_add(qsg, prp1, trans_len);
    }
    ···
 unmap:
    qemu_sglist_destroy(qsg);
    return NVME_INVALID_FIELD | NVME_DNR;
}
```

The function **nvme_map_prp()** here means mapping a block of memory. And there are two ways for user to map memory, through **qemu_iovec_init()** or **pci_dma_sglist_init()**. The function jumps to **unmap** statement when handling errors, then the program will directly call **qemu_sglist_destroy()** without consideration how the memory was mapped, resulting in an uninitialized free.

### 0x02 Turn arbitrary free to UAF

​		There are two kinds of memory space in Qemu's process, Qemu's heap and physmap heap.

1. Heap spray to clear tcache freelist
2. Malloc a mapping table, filled with physmap address
3. Free the mapping table, putting it in head of the tcache freelist

4. Malloc a **NvmeRequest** structure, trigger the vulnerable bug, then the chunk in userspace will be added into Qemu's tcache freelist
5. Now the chunk in userspace seems like a state of free in host, but Qemu's guest still has R/W capability.

### 0x03 Find an information leak

1. Malloc a mapping table again, the alloced chunk will be shared between host and guest

 	2. Initialize the table, then we get the physmap address 
 	3. Heap fengshui again, create a new **sq** and place a **QEMUTimer** in userspace
 	4. Initialize the timer, then we get the Qemu address and Heap address

### 0x04 Hijack the control flow

1. Modify the cb to system address
2. Modify the opaque to our arguments address.
3. Run the timer, Control RIP!


### Tips

Note that executing the system function directly will cause QEMU to fork a new process, resulting in the removal of the mapping space of guest memory. Therefore, we can consider copying the commands of guest space to QEMU main process, and controlling rip to slirp_smb_cleanup function. Then the command is copied to the RDI register, followed by a command injection to complete the command execution.

```c
static void slirp_smb_cleanup(SlirpState *s)
{
    int ret;

    if (s->smb_dir) {
        gchar *cmd = g_strdup_printf("rm -rf %s", s->smb_dir);        // Control RIP to here.
        ret = system(cmd);
        if (ret == -1 || !WIFEXITED(ret)) {
            error_report("'%s' failed.", cmd);
        } else if (WEXITSTATUS(ret)) {
            error_report("'%s' failed. Error code: %d",
                         cmd, WEXITSTATUS(ret));
        }
        g_free(cmd);
        g_free(s->smb_dir);
        s->smb_dir = NULL;
    }
}
```
