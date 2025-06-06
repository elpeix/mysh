# mysh

Una shell minimalista en C amb funcionalitats avançades.

## Característiques

- Historial de comandes persistent (`~/.mysh_history`)
- Navegació per l'historial amb fletxes
- Pipes (`|`)
- Redirecció d'entrada i sortida (`<`, `>`)
- Execució en background (`&`)
- Comandes internes: `cd`, `exit`, `help`, `mysh --version`
- Prompt personalitzat amb colors i directori actual
- Suport per comentaris (`#`)

## Compilació

```fish
make
```

## Ús

```fish
./mysh
```

## Exemples

- Executar una comanda:
  ```
  ls -l
  ```
- Pipe:
  ```
  ls | grep .c
  ```
- Redirecció:
  ```
  ls > sortida.txt
  cat < sortida.txt
  ```
- Execució en background:
  ```
  sleep 10 &
  ```
- Canviar de directori:
  ```
  cd /tmp
  cd
  cd -
  ```
- Veure la versió:
  ```
  mysh --version
  ```
- Comentaris:
  ```
  # Això és un comentari
  ```

## Estructura del projecte

- `src/` — Codi font principal
- `test/` — Tests
- `makefile` — Compilació

## Autor
Francesc d'Assís Requesens i Roca

## Llicència
Projecte educatiu sense llicència específica.
