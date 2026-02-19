# Protocolo de Servidor de Linguagem Zen C (LSP)

O Zen C vem com um Servidor de Linguagem (LSP) integrado para fornecer recursos de editor como autocompletar, ir para definição e diagnóstico de erros.

## Iniciando o Servidor

O Servidor de Linguagem é integrado ao compilador `zc`. Você pode iniciá-lo manualmente (embora seu editor geralmente lide com isso) com:

```bash
zc lsp
```

Ele se comunica por entrada/saída padrão (stdio).

## Configuração do Editor

### VS Code

Para Visual Studio Code, use a extensão oficial do Zen C:

- **Repositório**: [z-libs/zenc-vscode](https://github.com/z-libs/zenc-vscode)

Instale a extensão diretamente do **[Visual Studio Code Marketplace](https://marketplace.visualstudio.com/items?itemName=z-libs.zenc)**.

Alternativamente, você pode compilar o `.vsix` a partir do código fonte.

### Vim / Neovim

O suporte para Vim e Neovim é fornecido através do plugin `Zen-C.vim`, que inclui destaque de sintaxe e auxiliares de configuração LSP.

- **Repositório**: [davidscholberg/Zen-C.vim](https://github.com/davidscholberg/Zen-C.vim)

#### Neovim (exemplo com `lazy.nvim`)

Se você usa `nvim-lspconfig`, pode registrar `zc` como um servidor personalizado:

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

Para configurar o Zen C no Zed, adicione o seguinte ao seu `settings.json` ou configuração de linguagem:

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

### Editores Genéricos (Sublime, Emacs, etc.)

Para qualquer editor que suporte clientes LSP genéricos:

1.  **Comando**: `zc`
2.  **Argumentos**: `lsp`
3.  **Transporte**: `stdio`
4.  **Extensões de arquivo**: `.zc`

## Recursos

- **Diagnósticos**: Erros de sintaxe e tipo em tempo real.
- **Ir para Definição**: Pular para definições de structs, funções e variáveis.
- **Autocompletar**: Sugestões sensíveis ao contexto para campos e métodos.
- **Hover**: Informações de tipo e documentação ao passar o mouse.
