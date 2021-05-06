# scavenger

This is an exploit for an uninitialized free in nvme:nvme_map_prp(). For more information, see the [writeup](https://github.com/hustdebug/scavenger/blob/main/writeup.md) the [slides](https://github.com/hustdebug/scavenger/blob/main/as-21-Pan-Scavenger-Misuse-Error-Handling-Leading-To-QEMU-KVM-Escape.pdf) for the talk in Blackhat Asis 2021.

# Environment
```
$ ./qemu-system-x86_64 --version
QEMU emulator version 4.2.1 (Debian 1:4.2-3ubuntu6.7)
Copyright (c) 2003-2019 Fabrice Bellard and the QEMU Project developers
```


Command used to start QEMU
```
./qemu-system-x86_64_exp -enable-kvm -boot c -m 4G -drive format=qcow2,file=./ubuntu.img \
        -nic user,hostfwd=tcp:0.0.0.0:5555-:22 \
        -drive file=./nvme.img,if=none,id=D11 -device nvme,drive=D11,serial=1234,cmb_size_mb=64  \
        -device virtio-gpu -display none
```
