# Task3 Asymetric 


## Požadavky cvičící

binárka se musí jmenovat block a brát 3 poziční parametry v následující formě:

```sh
./block ACTION MODE FILENAME
ACTION   = {e,d}          má-li program provést šifrování (encrypt) nebo dešifrování (decrypt)
MODE     = {ecb,cbc,...}  operační mód blokové šifry
FILENAME = arg            soubor k {,de}šifrování

``` 
### Výstup
- návratová hodnota
  - 0 pokud je všechno v pořádku
  - 1 až 127 pokud něco se nepodařilo
- pokud FILENAME = BASENAME.tga, tak výstup je BASENAME_MODE_ACTION.tga
  - například po provedení ./block e cbc pic.tga výstupní soubor je pic_cbc_e.tga
  - když budu dešifrovat soubor pic_cbc_e.tga výstupní soubor bude pic_cbc_e_cbc_d.tga



- 💀 soubor, který jste dešifrovali musí být na velikost a po bajtech stejný, jako původní soubor
- 💀 klíč a inicializační vektor jsou hardcoded a jsou dostačující délky
 - nechceme překvapení stylu "použití neinicializované paměti", která v každém běhu obsahuje něco jiného než v tom předchozím
- 💀 soubor nenačítejte celý do paměti, ale po nějakých smysluplných blocích, třeba násobky 4096
- [-1b] kontrolujte výstupy funkcí, jsou vaši největší přátele pro debug
- [-1b] kontrolujte, jestli vstup dává smysl (soubor menší než minimální velikost hlavičky,…​)
- [-1b] do komentáře k merge requestu napište co vidíte, jaký je rozdíl mezi šifrováním pomocí ECB a CBC

Celkem 5/5 bodu
