# Protocolo de Servidor de Lenguaje Zen C (LSP)

Zen C viene con un Servidor de Lenguaje (LSP) integrado para proporcionar características de editor como autocompletado, ir a definición y diagnóstico de errores.

## Iniciando el Servidor

El Servidor de Lenguaje está integrado en el compilador `zc`. Puedes iniciarlo manualmente (aunque tu editor generalmente maneja esto) con:

```bash
zc lsp
```

Se comunica a través de entrada/salida estándar (stdio).

## Configuración del Editor

### VS Code

Para Visual Studio Code, usa la extensión oficial de Zen C:

- **Repositorio**: [z-libs/zenc-vscode](https://github.com/z-libs/zenc-vscode)

Instala la extensión directamente desde el **[Visual Studio Code Marketplace](https://marketplace.visualstudio.com/items?itemName=z-libs.zenc)**.

Alternativamente, puedes construir el `.vsix` desde el código fuente.

### Vim / Neovim

El soporte para Vim y Neovim se proporciona a través del plugin `Zen-C.vim`, que incluye resaltado de sintaxis y helpers de configuración LSP.

- **Repositorio**: [davidscholberg/Zen-C.vim](https://github.com/davidscholberg/Zen-C.vim)

#### Neovim (ejemplo con `lazy.nvim`)

Si usas `nvim-lspconfig`, puedes registrar `zc` como un servidor personalizado:

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

Para configurar Zen C en Zed, añade lo siguiente a tu `settings.json` o configuración de lenguaje:

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

Para cualquier editor que soporte clientes LSP genéricos:

1.  **Comando**: `zc`
2.  **Argumentos**: `lsp`
3.  **Transporte**: `stdio`
4.  **Extensiones de archivo**: `.zc`

## Características

- **Diagnósticos**: Errores de sintaxis y tipos en tiempo real.
- **Ir a Definición**: Saltar a definiciones de structs, funciones y variables.
- **Autocompletado**: Sugerencias conscientes del contexto para campos y métodos.
- **Hover**: Información de tipos y documentación al pasar el cursor.
