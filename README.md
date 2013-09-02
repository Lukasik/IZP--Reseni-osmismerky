# Řešení osmisměrky

Projekty jsou zde v takovém stavu, ve kterém jsem je odevzdával k hodnocení.

## Hodnocení

7/10

### Chyby na obhajobě

 * -0.50 Seg. fault při záporném rozměru
 * +0.25 Opraveno
 * -0.50 Alokace bez testů
 * +0.25 Opraveno

### Chyby po odevzdání

 * -0.50 Nezavírá soubor při špatných parametrech/špatném vstupu
 * -0.25 proj3.c:434 - zbytečné omezení
 * -1.00 search() - memory leak
 * -0.50 solve() - resource leak
 * -0.50 Špatné uvolnění paměti po částečné alokaci matice
