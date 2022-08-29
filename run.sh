#!/bin/bash

qemu-system-arm -m 1g -M raspi2b -serial stdio -kernel myos.elf
