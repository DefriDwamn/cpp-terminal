# cpp-terminal

## Общее описание

## Описание всех функций и настроек

## Cборка проекта

Необходимые зависимости:
- [Conan](https://conan.io/downloads)
- [CMake](https://cmake.org/download/)

Далее в терминале для сборки:
```bash
conan profile detect --force
mkdir build
conan install . --output-folder=build --build=missing
cd build
cmake .. --preset conan-default
cmake --build . --config Release
```
Собранное приложение будет находится в директории build/Release
## Примеры использования

## Результаты тестов