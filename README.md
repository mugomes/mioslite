# MiOSLite

Estou reescrevendo um sistema operacional que desenvolvi quando era mais novo. Infelizmente, não tenho mais os códigos que criei na época, eu havia armazenado eles em um disquete, que infelizmente não funciona mais.

Por isso, farei um novo projeto totalmente do zero, em Assembly e C. Ele será disponibilizado neste repositório sob a licença GPL-2.0-only.

É um projeto pequeno, que terá uma interface gráfica simples, a ideia é apenas demonstrar os primeiros passos no desenvolvimento de um sistema operacional.

Como vou fazer isso em meu tempo livre, deve demorar um pouco até o lançamento, mas, conforme for desenvolvendo, vou atualizando este repositório mostrando a evolução do código.

## Compilação

Para compilar e testar você irá precisar:

- build-essential
- nasm
- gcc-x86-64-linux-gnu
- qemu-system-x86
- grub-common
- grub-pc-bin
- mtools
- xorriso

Use os comandos abaixo para compilar e testar o MiOSLite.

```
nasm -f elf32 src/boot.asm -o src/boot.o
gcc -m32 -ffreestanding -fno-stack-protector -c src/kernel.c -o src/kernel.o
ld -m elf_i386 -T linker.ld -o src/kernel.bin src/boot.o src/kernel.o

mkdir -p build/boot/grub
mkdir -p iso/

cp src/kernel.bin build/boot/
cp src/boot/grub.cfg build/boot/grub/

grub-mkrescue -o iso/mioslite.iso build

qemu-system-x86_64 -cdrom iso/mioslite.iso
```

## 💙 Apoie

- GitHub: https://github.com/sponsors/mugomes
- More: https://mugomes.github.io/apoie.html

## 👤 Autor

**Murilo Gomes Julio**

🔗 [https://mugomes.github.io](https://mugomes.github.io)

📺 [https://youtube.com/@mugomesoficial](https://youtube.com/@mugomesoficial)

---

## License

The MiOSLite is provided under:

[SPDX-License-Identifier: GPL-2.0-only](https://github.com/mugomes/mioslite/blob/main/LICENSE)

Beign under the terms of the GNU General Public License version 2 only.

All contributions to the MiOSLite are subject to this license.
