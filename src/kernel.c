// Copyright (C) 2026 Murilo Gomes Julio
// SPDX-License-Identifier: GPL-2.0-only

// Site: https://github.com/mugomes

#include <stdint.h>

#define vWidth 80
#define vHeight 25

void clearScreen()
{
    volatile char *video = (char *)0xB8000;
    for (int i = 0; i < vWidth * vHeight; i++)
    {
        video[i * 2] = ' ';
        video[i * 2 + 1] = 0x07;
    }
}

void kernelMain(uint32_t bottom)
{
    clearScreen();

    volatile char *video = (char *)0xB8000;
    const char *msg = "MiOSLite em breve!";

    if (bottom != 0)
    {
        for (int i = 0; msg[i] != '\0'; i++)
        {
            video[i * 2] = msg[i];
            video[i * 2 + 1] = 0x0B;
        }
    }

    while (1)
    {
        __asm__ volatile("hlt");
    }
}