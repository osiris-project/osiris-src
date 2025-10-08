#!/bin/bash
set -e

nm -n usr/boot/osiris.elf | awk '$2=="T" { printf("{0x%s, \"%s\"},\n", $1, $3) }' > sys/include/osiris/kern/ksyms.h
