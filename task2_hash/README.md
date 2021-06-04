# Task2 - Hash

Napište program, který nalezne libovolnou zprávu, jejíž hash (SHA-384) začíná zleva na posloupnost nulových bitů:

Počet nulových bitů je zadán celým číslem jako parametrem na příkazové řádce.
Pořadí bitů je big-endian: Bajt 0 od MSB do LSB, Bajt 1 od MSB do LSB, …​, poslední bajt od MSB do LSB.
Nalezenou zprávu vypište v hexadecimální podobě

## Požadavky cvičící

- binárka se musí jmenovat hash a brát jeden a pouze jeden parametr, a to počet nulových bitů na začátku.
- přikládejte makefile, který umí zbuildit vaše řešení.
- format výstupu je
 - 💀 na prvním řádku vypište hexadecimálně bez mezer obsah zprávy, hash které produkuje požadovaný počet nul na začátku
 - 💀 na druhém řádku hash té zprávy.

příklad výstupu:

```sh
$ ./hash 4
3132330a
0ee756a11951fa5d3d9d1c8b85f3e33583569d5360cdd4599a3c62edb9d2cc943b2fd38ddd65af72d95e2d71d2b4b03b
```
jak ověřit?

```sh
$ echo -n "3132330a" | xxd -r -ps | openssl sha384
0ee756a11951fa5d3d9d1c8b85f3e33583569d5360cdd4599a3c62edb9d2cc943b2fd38ddd65af72d95e2d71d2b4b03b
# xxd -r -ps na stdout vypíše bajty které mají hexadecimální hodnoty dané na stdin
``` 


Zíkáno 3/3 bodu.
