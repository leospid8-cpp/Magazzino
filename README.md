# Progetto Magazzino in C++ 📦

Questo è il mio progetto per la gestione di un magazzino, scritto in C++ collegato a un database SQLite.
Serve per aggiungere prodotti, gestire carico/scarico merce e controllare le scorte.

## Come farlo partire

Per farlo funzionare non basta compilare il file `.cpp`, serve anche la libreria di SQLite.

1. Scarica i file `sqlite3.c` e `sqlite3.h` (dal sito di SQLite, pacchetto "amalgamation").
2. Mettili nella stessa cartella del mio file `main.cpp`.
3. Apri il terminale nella cartella e scrivi:

```bash
g++ main.cpp sqlite3.c -o magazzino
