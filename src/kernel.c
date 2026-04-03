// Copyright (C) 2026 Murilo Gomes Julio
// SPDX-License-Identifier: GPL-2.0-only

// Site: https://github.com/mugomes

#include <stdint.h>

#define vWidth 80
#define vHeight 25
#define MAX_DIRS 32
#define MAX_NAME 32
#define CMD_BUFFER 64
#define DISK_LBA 10

volatile uint16_t *video = (uint16_t *)0xB8000;
int cursor = 0;

// Limpar Tela
void clearScreen()
{
    for (int i = 0; i < vWidth * vHeight; i++)
        video[i] = (uint16_t)' ' | 0x0700;
    cursor = 0;
}

void putc(char c)
{
    // Tratamento de quebra de linha
    if (c == '\n')
    {
        cursor += vWidth - (cursor % vWidth);
    }
    // Tratamento de caracteres imprimíveis (incluindo espaço ' ')
    else
    {
        video[cursor] = (uint16_t)c | 0x0700;
        cursor++;
    }

    // Scroll simples: se chegar ao fim da tela, limpa
    if (cursor >= vWidth * vHeight)
    {
        clearScreen();
    }
}

void puts(const char *str)
{
    for (int i = 0; str[i]; i++)
        putc(str[i]);
}

// ATA PIO
static inline void outb(uint16_t port, uint8_t val)
{
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void ata_wait()
{
    while (inb(0x1F7) & 0x80)
        ;
}

void ata_read_sector(uint32_t lba, uint8_t *buffer)
{
    ata_wait();
    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(0x1F2, 1);
    outb(0x1F3, (uint8_t)lba);
    outb(0x1F4, (uint8_t)(lba >> 8));
    outb(0x1F5, (uint8_t)(lba >> 16));
    outb(0x1F7, 0x20);
    ata_wait();
    for (int i = 0; i < 256; i++)
    {
        uint16_t data;
        __asm__ volatile("inw %1, %0" : "=a"(data) : "Nd"(0x1F0));
        buffer[i * 2] = data & 0xFF;
        buffer[i * 2 + 1] = data >> 8;
    }
}

void ata_write_sector(uint32_t lba, uint8_t *buffer)
{
    ata_wait();
    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(0x1F2, 1);
    outb(0x1F3, (uint8_t)lba);
    outb(0x1F4, (uint8_t)(lba >> 8));
    outb(0x1F5, (uint8_t)(lba >> 16));
    outb(0x1F7, 0x30);
    ata_wait();
    for (int i = 0; i < 256; i++)
    {
        uint16_t data = buffer[i * 2] | (buffer[i * 2 + 1] << 8);
        __asm__ volatile("outw %0, %1" : : "a"(data), "Nd"(0x1F0));
    }
}

// FS Simples
char dirs[MAX_DIRS][MAX_NAME];
int dir_count = 0;
uint8_t disk_buffer[512];

void load_dirs()
{
    ata_read_sector(DISK_LBA, disk_buffer);
    dir_count = disk_buffer[0];
    if (dir_count > MAX_DIRS)
        dir_count = 0;
    for (int i = 0; i < dir_count; i++)
        for (int j = 0; j < MAX_NAME; j++)
            dirs[i][j] = disk_buffer[1 + i * MAX_NAME + j];
}

void save_dirs()
{
    for (int i = 0; i < 512; i++)
        disk_buffer[i] = 0;
    disk_buffer[0] = dir_count;
    for (int i = 0; i < dir_count; i++)
        for (int j = 0; j < MAX_NAME; j++)
            disk_buffer[1 + i * MAX_NAME + j] = dirs[i][j];
    ata_write_sector(DISK_LBA, disk_buffer);
}

void strcpy(char *dest, const char *src)
{
    int i = 0;
    while (src[i] && i < MAX_NAME - 1)
    {
        dest[i] = src[i];
        i++;
    }
    dest[i] = 0;
}

// Power Off
static inline void outw(uint16_t port, uint16_t val)
{
    __asm__ volatile("outw %0, %1" : : "a"(val), "Nd"(port));
}

void poweroff()
{
    puts("Desligando...\n");

    // Método correto (16 bits)
    outw(0x604, 0x2000);

    // Fallback
    outw(0xB004, 0x2000);

    while (1)
    {
        __asm__ volatile("hlt");
    }
}

// Comandos
void mkdir_cmd(const char *name)
{
    if (dir_count >= MAX_DIRS)
    {
        puts("Erro: Limite de pastas atingido\n");
        return;
    }
    strcpy(dirs[dir_count++], name);
    save_dirs();
    puts("Pasta criada com sucesso\n");
}

void dir_cmd()
{
    if (dir_count == 0)
    {
        puts("(vazio)\n");
        return;
    }
    for (int i = 0; i < dir_count; i++)
    {
        puts(dirs[i]);
        putc('\n');
    }
}

// Remover Diretório
int strcmp(const char *a, const char *b)
{
    int i = 0;
    while (a[i] && b[i])
    {
        if (a[i] != b[i])
            return 0;
        i++;
    }
    return a[i] == b[i];
}

void rmdir_cmd(const char *name)
{
    if (dir_count == 0)
    {
        puts("Erro: Nenhuma pasta\n");
        return;
    }

    int found = -1;

    // Procurar diretório
    for (int i = 0; i < dir_count; i++)
    {
        if (strcmp(dirs[i], name))
        {
            found = i;
            break;
        }
    }

    if (found == -1)
    {
        puts("Erro: Pasta nao encontrada\n");
        return;
    }

    // Deslocar (remover)
    for (int i = found; i < dir_count - 1; i++)
    {
        for (int j = 0; j < MAX_NAME; j++)
        {
            dirs[i][j] = dirs[i + 1][j];
        }
    }

    dir_count--;
    save_dirs();

    puts("Pasta removida\n");
}

// TECLADO
// Mapeamento ASCII para Scancodes Set 1
char keymap[128] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '};

char getch()
{
    uint8_t scancode;
    while (1)
    {
        scancode = inb(0x60);
        if (!(scancode & 0x80))
            break; // Espera pressionar
    }

    char c = 0;
    // O scancode 0x39 é o espaço no Set 1
    if (scancode == 0x39)
        c = ' ';
    else if (scancode < 128)
        c = keymap[scancode];

    // Espera soltar a tecla (evita repetição infinita no buffer)
    while (!(inb(0x60) & 0x80))
        ;

    return c;
}

// PARSER
int starts_with(const char *str, const char *prefix)
{
    int i = 0;
    while (prefix[i])
    {
        if (str[i] != prefix[i])
            return 0;
        i++;
    }
    return 1;
}

void execute(char *cmd)
{
    // Pula espaços iniciais
    while (*cmd == ' ')
        cmd++;

    if (starts_with(cmd, "mkdir"))
    {
        char *name = cmd + 5;
        // Pula espaços entre o comando e o argumento
        while (*name == ' ')
            name++;

        if (*name == 0)
        {
            puts("Uso: mkdir [nome]\n");
            return;
        }
        mkdir_cmd(name);
    }
    else if (starts_with(cmd, "rmdir"))
    {
        char *name = cmd + 5;
        while (*name == ' ')
            name++;

        if (*name == 0)
        {
            puts("Uso: rmdir [nome]\n");
            return;
        }

        rmdir_cmd(name);
    }
    else if (starts_with(cmd, "dir"))
    {
        dir_cmd();
    }
    else if (starts_with(cmd, "poweroff"))
    {
        poweroff();
    }
    else if (*cmd == 0)
    {
        return; // Enter vazio
    }
    else
    {
        puts("Comando desconhecido\n");
    }
}

// Principal
void kernelMain()
{
    clearScreen();
    load_dirs();

    puts("MiOSLite v0.0.2\n");
    puts("> ");

    char buffer[CMD_BUFFER];
    int len = 0;

    while (1)
    {
        char c = getch();

        if (c == '\n')
        {
            putc('\n');
            buffer[len] = 0;
            execute(buffer);
            len = 0;
            puts("> ");
        }
        else if (c == '\b')
        {
            if (len > 0)
            {
                len--;
                cursor--;
                video[cursor] = ' ' | 0x0700;
            }
        }
        else if (c >= ' ') // Captura apenas caracteres imprimíveis (incluindo espaço)
        {
            if (len < CMD_BUFFER - 1)
            {
                buffer[len++] = c;
                putc(c);
            }
        }
    }
}