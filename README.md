# Pepel Space - Post-Soviet VHS Stereo Processor

Плагин для пространственной обработки звука в стиле VHS-нуар.
Разработан для FL Studio (VST3).

## Требования к сборке
- Visual Studio 2022
- CMake 3.22+

## ⚠️ ВАЖНО: Ассеты (Assets)
Для сборки вам необходимо создать папку `Assets` в корне проекта и поместить туда:
1. `Doppelganger_RUS.ttf` (шрифт для заголовков)
2. `VHS.ttf` (шрифт для текста, например VCR OSD Mono)
3. `bg.png` (фоновая картинка ночного двора/панелек, желательно 1000x750)

Без этих файлов CMake выдаст ошибку при сборке BinaryData.

## Сборка
```bash
git clone <your-repo-url>
cd pepel-space
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release