# Zen C 語言伺服器協定 (LSP)

Zen C 隨附內建的語言伺服器 (LSP)，提供諸如自動補全、跳轉到定義和錯誤診斷等編輯器功能。

## 啟動伺服器

語言伺服器內建於 `zc` 編譯器中。您可以手動啟動它（儘管您的編輯器通常會自動處理）：

```bash
zc lsp
```

它透過標準輸入/輸出 (stdio) 進行通訊。

## 編輯器配置

### VS Code

對於 Visual Studio Code，請使用官方的 Zen C 擴充功能：

- **儲存庫**: [z-libs/zenc-vscode](https://github.com/z-libs/zenc-vscode)

直接從 **[Visual Studio Code Marketplace](https://marketplace.visualstudio.com/items?itemName=z-libs.zenc)** 安裝擴充功能。

或者，您可以從原始碼建置 `.vsix`。

### Vim / Neovim

透過 `Zen-C.vim` 外掛程式提供對 Vim 和 Neovim 的支援，其中包括語法高亮和 LSP 配置助手。

- **儲存庫**: [davidscholberg/Zen-C.vim](https://github.com/davidscholberg/Zen-C.vim)

#### Neovim (`lazy.nvim` 範例)

如果您使用的是 `nvim-lspconfig`，您可以註冊 `zc` 為自訂伺服器：

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

要在 Zed 中配置 Zen C，請將以下內容新增至您的 `settings.json` 或語言配置中：

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

### 通用編輯器 (Sublime, Emacs 等)

對於任何支援通用 LSP 用戶端的編輯器：

1.  **命令**: `zc`
2.  **參數**: `lsp`
3.  **傳輸**: `stdio`
4.  **檔案副檔名**: `.zc`

## 功能

- **診斷**: 即時語法和類型錯誤。
- **跳轉到定義**: 跳轉到結構體、函式和變數的定義。
- **自動補全**:針對欄位和方法的上下文感知建議。
- **懸停**: 懸停時顯示類型資訊和文件。
