# cpp-terminal

## Общее описание
Эмулятора shell.
Эмулятор запускаеться из реальной командной строки в режиме GUI, а файл с
виртуальной файловой системой представлен образом в виде файла формата
tar.

## Описание всех функций и настроек
Поддержка команд ls, cd и exit, а также:
1. cp
2. find
3. tree
## Cборка проекта

Необходимые зависимости для разработки:
- clang++
- clangd
- [Conan](https://conan.io/downloads)
- [CMake](https://cmake.org/download/)
- [Bear](https://github.com/rizsotto/Bear)

Убедитесь, что профиль conan использует clang(libstdc++11 и указать buildenv СXX и LD) или же msvc(для сборки под win)
Пример для clang(под linux):
```yml
[settings]
arch=x86_64
build_type=Release
compiler=clang
compiler.cppstd=gnu17
compiler.libcxx=libstdc++11
compiler.version=19
os=Linux
[buildenv]
CC=/usr/bin/clang
CXX=/usr/bin/clang++
LD=/usr/bin/lld
```

Далее в терминале для сборки:
```bash
conan profile detect --force
mkdir build
conan install . --output-folder=build --build=missing
cd build
cmake .. --preset conan-release 
# или cmake .. --preset conan-default 
cmake --build . --config Release
```
Собранное приложение будет находится в директории build/Release
## Примеры использования
1. Открыть терминал
2. Ввести tree . - получим дерево от текущей директории
3. сp file folder/file - скопируем файл в папку folder
## Результаты тестов