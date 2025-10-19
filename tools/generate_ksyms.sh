#!/bin/bash
set -e

nm -n root/boot/osiris.elf | awk '$2=="T" { printf("{0x%s, \"%s\"},\n", $1, $3) }' > include/sys/ksyms.h 
