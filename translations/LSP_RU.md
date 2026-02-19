# Протокол языкового сервера Zen C (LSP)

Zen C поставляется со встроенным языковым сервером (LSP), который предоставляет функции редактора, такие как автодополнение, переход к определению и диагностика ошибок.

## Запуск сервера

Языковой сервер встроен в компилятор `zc`. Вы можете запустить его вручную (хотя ваш редактор обычно делает это сам) с помощью:

```bash
zc lsp
```

Он общается через стандартный ввод/вывод (stdio).

## Настройка редактора

### VS Code

Для Visual Studio Code используйте официальное расширение Zen C:

- **Репозиторий**: [z-libs/zenc-vscode](https://github.com/z-libs/zenc-vscode)

Установите расширение прямо из **[Visual Studio Code Marketplace](https://marketplace.visualstudio.com/items?itemName=z-libs.zenc)**.

Кроме того, вы можете собрать `.vsix` из исходного кода.

### Vim / Neovim

Поддержка Vim и Neovim обеспечивается через плагин `Zen-C.vim`, который включает подсветку синтаксиса и помощники по настройке LSP.

- **Репозиторий**: [davidscholberg/Zen-C.vim](https://github.com/davidscholberg/Zen-C.vim)

#### Neovim (пример с `lazy.nvim`)

Если вы используете `nvim-lspconfig`, вы можете зарегистрировать `zc` как пользовательский сервер:

```lua
local lspconfig = require('lspconfig')
local configs = require('lspconfig.configs')

if not configs.zenc then
  configs.zenc = {
    default_config = {
      cmd = { 'zc', 'lsp' },
      filetypes = { 'zenc', 'zc' },
      root_dir = lspconfig.util.root_pattern('.git', 'build.bat', 'Makefile'),
      settings = {},
    },
  }
end

lspconfig.zenc.setup {}
```

### Zed

Чтобы настроить Zen C в Zed, добавьте следующее в ваш `settings.json` или конфигурацию языка:

```json
{
  "lsp": {
    "zenc": {
      "binary": {
        "path": "zc",
        "arguments": ["lsp"]
      }
    }
  },
  "languages": {
    "Zen C": {
      "language_servers": ["zenc"]
    }
  }
}
```

### Универсальные редакторы (Sublime, Emacs и т.д.)

Для любого редактора, поддерживающего универсальные клиенты LSP:

1.  **Команда**: `zc`
2.  **Аргументы**: `lsp`
3.  **Транспорт**: `stdio`
4.  **Расширения файлов**: `.zc`

## Возможности

- **Диагностика**: Ошибки синтаксиса и типов в реальном времени.
- **Переход к определению**: Переход к определениям структур, функций и переменных.
- **Автодополнение**: Контекстно-зависимые предложения для полей и методов.
- **Наведение**: Информация о типе и документация при наведении курсора.
