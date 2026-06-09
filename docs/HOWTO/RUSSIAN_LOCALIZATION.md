# Russian Language Patch / Русский языковой патч

**Applies to / Применимо к:** GeneralsX Zero Hour (English version only / только английская версия)

**Download / Скачать:** [00RussianZH.zip](https://mega.nz/file/tJpDFIwC#3ITms8nZH59pRKDwmNLVx_-7oyCy4WnH2L9_FAxsdgY)

---

## English

### Overview

This patch adds Russian text support to GeneralsX Zero Hour. It works by patching the English `Language.ini` and `generals.csf` files inside the English BIG archive, replacing selected strings with Russian translations.

>  **Important:** This patch only works on the **English version** of the game. It directly modifies the English language `.big` file — it does not add a new language option.

### Installation

1. **Download** `00RussianZH.zip` from the link above.

2. **Locate your game installation directory:**
   | Platform | Path |
   |---|---|
   | macOS | `~/Library/Application Support/GeneralsX/GeneralsZH/` |
   | Linux | `~/.local/share/GeneralsX/GeneralsZH/` |

3. **Extract** the zip file directly into the game directory. The zip contains a single folder `00RussianZH/`. After extraction the structure should look like:
   ```
   <game-dir>/
   ├── 00RussianZH/
   │   ├── Data/
   │   │   ├── English/
   │   │   │   ├── Language.ini
   │   │   │   └── generals.csf
   │   │   └── ini/
   │   │       ├── object/
   │   │       └── ... (other INI overrides)
   ```

4. **Launch** the game. Russian text should appear in menus, tooltips, and 3D captions.

### Requirements

- GeneralsX Zero Hour (English version)
- Cyrillic-capable font installed on your system (e.g., Arial Unicode MS) — typically ships with macOS and most Linux distros

### Notes

- This is a community mod patch, not an official translation.
- The patch overrides English strings — it does not add a separate language selector.
- Some mods that change the same files may conflict.

### Troubleshooting

| Problem | Solution |
|---|---|
| Text shows as squares `□□□□` | Install a font with Cyrillic glyphs (e.g., Arial Unicode MS, Noto Sans) or run the game with `LANG=ru_RU.UTF-8` |
| Labels are blank/missing | The `generals.csf` override is incomplete — reinstall the patch |
| Game crashes on startup | Remove the `00RussianZH/` folder and verify game files |

---

## Русский

### Обзор

Этот патч добавляет поддержку русского текста в GeneralsX Zero Hour. Он работает путём редактирования английских файлов `Language.ini` и `generals.csf` внутри английского BIG-архива, заменяя выбранные строки на русский перевод.

>  **Важно:** Патч работает только на **английской версии** игры. Он напрямую изменяет английский языковой `.big` файл — новая опция языка не добавляется.

### Установка

1. **Скачайте** `00RussianZH.zip` по ссылке выше.

2. **Найдите папку с игрой:**
   | Платформа | Путь |
   |---|---|
   | macOS | `~/Library/Application Support/GeneralsX/GeneralsZH/` |
   | Linux | `~/.local/share/GeneralsX/GeneralsZH/` |

3. **Распакуйте** zip-архив прямо в папку с игрой. Внутри находится папка `00RussianZH/`. После распаковки структура должна выглядеть так:
   ```
   <папка-игры>/
   ├── 00RussianZH/
   │   ├── Data/
   │   │   ├── English/
   │   │   │   ├── Language.ini
   │   │   │   └── generals.csf
   │   │   └── ini/
   │   │       ├── object/
   │   │       └── ... (другие INI-переопределения)
   ```

4. **Запустите** игру. Русский текст должен отображаться в меню, подсказках и 3D-титрах.

### Требования

- GeneralsX Zero Hour (английская версия)
- Шрифт с кириллицей (например, Arial Unicode MS) — обычно есть на macOS и большинстве дистрибутивов Linux

### Примечания

- Это мод от сообщества, а не официальный перевод.
- Патч заменяет английские строки — отдельного переключателя языка не добавляется.
- Другие моды, изменяющие те же файлы, могут конфликтовать.

### Решение проблем

| Проблема | Решение |
|---|---|
| Текст отображается как `□□□□` | Установите шрифт с кириллицей (Arial Unicode MS, Noto Sans) или запустите игру с `LANG=ru_RU.UTF-8` |
| Надписи пустые/отсутствуют | CSF-переопределение неполное — переустановите патч |
| Игра вылетает при запуске | Удалите папку `00RussianZH/` и проверьте файлы игры |
