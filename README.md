# Проект по физике
Это среда, позволяющая на современном маке билдить SDL_1.2-приложение 
для широкого диапазона версий macOS, начиная от Mac OS 8.1 m68k, 
заканчивая macOS 26 arm64.

*На данный момент пропускается только одна система, 10.5 Leopard для PowerPC, 
но это фиксится добавлением конфигурации для Retro68, при которой 
приложение билдится с Carbon API.*

## Кто и зачем?
Этот репозиторий сделан мной и [моим другом](https://github.com/gdz-ot-mayatina) в рамках нашего обучения в ИТМО, где нам на втором курсе нужно было сделать проект по физике.

## Требования

- Установленный Retro68
- Помещённые в `/Library/Developer/CommandLineTools/SDKs/` macOS SDK 10.4u, 10.15 и 26.0 (последнее требование — мягкое)
- Установленная модифицированная версия библиотеки SDL 1.2 https://github.com/gameblabla/SDL12_mac
- Компилятор Apple Clang 17 (но наверняка можно и другой)

## Сборка

Всего есть пять таргетов:
- `BUILD_FOR_11_0` — arm64 11.0+
- `BUILD_FOR_10_7` — x86_64 10.7+
- `BUILD_FOR_10_4` — i386 10.4–10.15
- `BUILD_FOR_PPC` — ppc 8.1–10.4
- `BUILD_FOR_M68K` — m68k 8.1–???

В `Makefile.mac` и `Makefile.macm68k` необходимо прописать СВОИ пути, по которым вы установили Retro68 и его компоненты.

Можно в первых строках `CMakeLists.txt` расставить флаги и скомпилировать только нужное.

По умолчанию, все флаги выставлены в ON, и по следующей команде можно собрать все таргеты в директорию `build` сразу:

```shell
cmake -S . -B build 
cmake --build build
```

## Игра

Сама игра, которая здесь компилируется в качестве примера — тетрис от Greg Kennedy (https://greg-kennedy.com/wordpress/2019/05/01/modest-bricks/), который мы немного модифицировали (убрали аудио).

## TODO
- [ ] Добавить билд с Carbon API
- [ ] Desloppify `CMakeLists.txt`
- [ ] Перевести README.md на английский
- [ ] Сделать сборку более self-contained, разместив Mac OS X SDK и соответствующие необходимые штуки в данном репозитории (если лицензия позволяет)
- [ ] Добавить возможность использовать SDL_mixer
- [ ] Сделать в сборке для классической Mac OS подобие bundle (или как там было принято в то время делать), а не просто экзешник + папка с картинками рядом

## Credits
Игра, используемая в проекте — Modest Bricks от Greg Kennedy:
- https://greg-kennedy.com/wordpress/2019/05/01/modest-bricks/

Файлы `Makefile.mac`, `Makefile.macm68k`, `Retro68APPL.r`, `SDL.r` взяты из репозитория от [gameblabla](https://github.com/gameblabla) (и первые два немного изменены):
- https://github.com/gameblabla/worship-vector/tree/macosclassic

Ипользуются сурсы библиотек SDL 1.2, SDL 2 и sdl12-compat:
- https://github.com/libsdl-org/SDL-1.2
- https://github.com/libsdl-org/SDL/tree/SDL2
- https://github.com/libsdl-org/sdl12-compat

И дополнительно используется модифицированная версия библиотеки SDL 1.2 от [gameblabla](https://github.com/gameblabla)
- https://github.com/gameblabla/SDL12_mac