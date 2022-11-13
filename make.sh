#! /bin/bash

wayland-scanner private-code /usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml src/xdg-shell-protocol.c
wayland-scanner client-header /usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml src/xdg-shell-client-protocol.h
gcc -o wsweep src/wsweep.c src/xdg-shell-protocol.c -lwayland-client -w
