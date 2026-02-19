# Protocollo Server di Linguaggio Zen C (LSP)

Zen C include un Server di Linguaggio (LSP) integrato per fornire funzionalità dell'editor come autocompletamento, vai alla definizione e diagnostica degli errori.

## Avvio del Server

Il Server di Linguaggio è integrato nel compilatore `zc`. Puoi avviarlo manualmente (sebbene il tuo editor di solito lo gestisca) con:

```bash
zc lsp
```

Comunica tramite input/output standard (stdio).

## Configurazione dell'Editor

### VS Code

Per Visual Studio Code, usa l'estensione ufficiale di Zen C:

- **Repository**: [z-libs/zenc-vscode](https://github.com/z-libs/zenc-vscode)

Installa l'estensione direttamente dal **[Visual Studio Code Marketplace](https://marketplace.visualstudio.com/items?itemName=z-libs.zenc)**.

In alternativa, puoi compilare il `.vsix` dal codice sorgente.

### Vim / Neovim

Il supporto per Vim e Neovim è fornito tramite il plugin `Zen-C.vim`, che include evidenziazione della sintassi e helper di configurazione LSP.

- **Repository**: [davidscholberg/Zen-C.vim](https://github.com/davidscholberg/Zen-C.vim)

#### Neovim (esempio con `lazy.nvim`)

Se usi `nvim-lspconfig`, puoi registrare `zc` come server personalizzato:

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

Per configurare Zen C in Zed, aggiungi quanto segue al tuo `settings.json` o configurazione del linguaggio:

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

### Editor Generici (Sublime, Emacs, ecc.)

Per qualsiasi editor che supporti client LSP generici:

1.  **Comando**: `zc`
2.  **Argomenti**: `lsp`
3.  **Trasporto**: `stdio`
4.  **Estensioni file**: `.zc`

## Funzionalità

- **Diagnostica**: Errori di sintassi e tipo in tempo reale.
- **Vai alla Definizione**: Salta alle definizioni di struct, funzioni e variabili.
- **Autocompletamento**: Suggerimenti contestuali per campi e metodi.
- **Hover**: Informazioni sul tipo e documentazione al passaggio del mouse.
