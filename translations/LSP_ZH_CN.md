# Zen C 语言服务器协议 (LSP)

Zen C 自带内置的语言服务器 (LSP)，提供诸如自动补全、跳转到定义和错误诊断等编辑器功能。

## 启动服务器

语言服务器内置于 `zc` 编译器中。您可以手动启动它（尽管您的编辑器通常会自动处理）：

```bash
zc lsp
```

它通过标准输入/输出 (stdio) 进行通信。

## 编辑器配置

### VS Code

对于 Visual Studio Code，请使用官方的 Zen C 扩展：

- **仓库**: [z-libs/zenc-vscode](https://github.com/z-libs/zenc-vscode)

直接从 **[Visual Studio Code Marketplace](https://marketplace.visualstudio.com/items?itemName=z-libs.zenc)** 安装扩展。

或者，您可以从源代码构建 `.vsix`。

### Vim / Neovim

通过 `Zen-C.vim` 插件提供对 Vim 和 Neovim 的支持，其中包括语法高亮和 LSP 配置助手。

- **仓库**: [davidscholberg/Zen-C.vim](https://github.com/davidscholberg/Zen-C.vim)

#### Neovim (`lazy.nvim` 示例)

如果您使用的是 `nvim-lspconfig`，您可以注册 `zc` 为自定义服务器：

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

要在 Zed 中配置 Zen C，请将以下内容添加到您的 `settings.json` 或语言配置中：

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

### 通用编辑器 (Sublime, Emacs 等)

对于任何支持通用 LSP 客户端的编辑器：

1.  **命令**: `zc`
2.  **参数**: `lsp`
3.  **传输**: `stdio`
4.  **文件扩展名**: `.zc`

## 功能

- **诊断**: 实时语法和类型错误。
- **跳转到定义**: 跳转到结构体、函数和变量的定义。
- **自动补全**:针对字段和方法的上下文感知建议。
- **悬停**: 悬停时显示类型信息和文档。
