#!/bin/bash
sudo rmmod fiber
make clean
make
sudo dmesg -C
sudo insmod fiber.ko
