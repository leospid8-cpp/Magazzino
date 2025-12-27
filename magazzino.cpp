#include <iostream>
#include <string>
#include <vector>
#include <sqlite3.h>

struct Prodotto {
    int id;
    std::string nome;
    std::string descrizione;
    std::string categoria;
    int quantita;
    double prezzo;
    std::string posizione;
};

static void stampaMenu() {
    std::cout << "\n=== magazzino ===\n";
    std::cout << "1) inizializza database\n";
    std::cout << "2) aggiungi prodotto\n";
    std::cout << "3) lista prodotti\n";
    std::cout << "4) carico prodotto\n";
    std::cout << "5) scarico prodotto\n";
    std::cout << "6) cerca prodotto per nome\n";
    std::cout << "7) report sotto scorta\n";
    std::cout << "0) esci\n";
    std::cout << "scelta: ";
}

static bool eseguiSQL(sqlite3* db, const std::string& sql) {
    char* err = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err);
    if (rc != SQLITE_OK) {
        std::cerr << "errore sql: " << (err ? err : "sconosciuto") << "\n";
        sqlite3_free(err);
        return false;
    }
    return true;
}

static bool inizializzaDB(sqlite3* db) {
    std::string schema =
        "CREATE TABLE IF NOT EXISTS prodotti ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "nome TEXT NOT NULL,"
        "descrizione TEXT,"
        "categoria TEXT,"
        "quantita INTEGER NOT NULL DEFAULT 0,"
        "prezzo REAL NOT NULL DEFAULT 0,"
        "posizione TEXT"
        ");"
        "CREATE TABLE IF NOT EXISTS fornitori ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "nome TEXT NOT NULL,"
        "telefono TEXT,"
        "email TEXT"
        ");"
        "CREATE TABLE IF NOT EXISTS movimenti ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "prodotto_id INTEGER NOT NULL,"
        "tipo TEXT NOT NULL CHECK (tipo IN ('CARICO','SCARICO')),"
        "quantita INTEGER NOT NULL,"
        "data TEXT NOT NULL,"
        "note TEXT,"
        "FOREIGN KEY(prodotto_id) REFERENCES prodotti(id)"
        ");";
    return eseguiSQL(db, schema);
}

static void aggiungiProdotto(sqlite3* db) {
    std::string nome, descrizione, categoria, posizione;
    int quantita = 0;
    double prezzo = 0.0;

    // qui carico i dati del nuovo prodotto
    std::cin.ignore();
    std::cout << "nome: ";
    std::getline(std::cin, nome);
    std::cout << "descrizione: ";
    std::getline(std::cin, descrizione);
    std::cout << "categoria: ";
    std::getline(std::cin, categoria);
    std::cout << "posizione: ";
    std::getline(std::cin, posizione);
    std::cout << "quantita iniziale: ";
    std::cin >> quantita;
    std::cout << "prezzo: ";
    std::cin >> prezzo;

    sqlite3_stmt* stmt = nullptr;
    std::string sql = "INSERT INTO prodotti (nome, descrizione, categoria, quantita, prezzo, posizione) "
                      "VALUES (?, ?, ?, ?, ?, ?);";
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, nome.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, descrizione.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, categoria.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, quantita);
    sqlite3_bind_double(stmt, 5, prezzo);
    sqlite3_bind_text(stmt, 6, posizione.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) == SQLITE_DONE) {
        std::cout << "prodotto inserito.\n";
    } else {
        std::cout << "errore inserimento.\n";
    }
    sqlite3_finalize(stmt);
}

static void listaProdotti(sqlite3* db) {
    std::string sql = "SELECT id, nome, categoria, quantita, prezzo, posizione FROM prodotti ORDER BY nome;";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    // qui leggo i dati e li mostro
    std::cout << "\nid | nome | categoria | quantita | prezzo | posizione\n";
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::cout << sqlite3_column_int(stmt, 0) << " | "
                  << sqlite3_column_text(stmt, 1) << " | "
                  << sqlite3_column_text(stmt, 2) << " | "
                  << sqlite3_column_int(stmt, 3) << " | "
                  << sqlite3_column_double(stmt, 4) << " | "
                  << sqlite3_column_text(stmt, 5) << "\n";
    }
    sqlite3_finalize(stmt);
}

static void registraMovimento(sqlite3* db, const std::string& tipo) {
    int idProdotto = 0;
    int quantita = 0;
    std::string data;
    std::string note;

    // qui registro un carico o scarico
    std::cout << "id prodotto: ";
    std::cin >> idProdotto;
    std::cout << "quantita: ";
    std::cin >> quantita;
    std::cin.ignore();
    std::cout << "data (yyyy-mm-dd): ";
    std::getline(std::cin, data);
    std::cout << "note: ";
    std::getline(std::cin, note);

    sqlite3_stmt* stmt = nullptr;
    std::string sqlMov = "INSERT INTO movimenti (prodotto_id, tipo, quantita, data, note) "
                         "VALUES (?, ?, ?, ?, ?);";
    sqlite3_prepare_v2(db, sqlMov.c_str(), -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, idProdotto);
    sqlite3_bind_text(stmt, 2, tipo.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, quantita);
    sqlite3_bind_text(stmt, 4, data.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, note.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) == SQLITE_DONE) {
        std::cout << "movimento registrato.\n";
    } else {
        std::cout << "errore movimento.\n";
    }
    sqlite3_finalize(stmt);

    std::string sqlAgg = "UPDATE prodotti SET quantita = quantita " +
                         std::string(tipo == "CARICO" ? "+" : "-") +
                         " ? WHERE id = ?;";
    sqlite3_prepare_v2(db, sqlAgg.c_str(), -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, quantita);
    sqlite3_bind_int(stmt, 2, idProdotto);
    if (sqlite3_step(stmt) == SQLITE_DONE) {
        std::cout << "quantita aggiornata.\n";
    } else {
        std::cout << "errore aggiornamento.\n";
    }
    sqlite3_finalize(stmt);
}

static void cercaProdotto(sqlite3* db) {
    std::string nome;
    std::cin.ignore();
    std::cout << "nome da cercare: ";
    std::getline(std::cin, nome);

    sqlite3_stmt* stmt = nullptr;
    std::string sql = "SELECT id, nome, descrizione, categoria, quantita, prezzo, posizione "
                      "FROM prodotti WHERE nome LIKE ? ORDER BY nome;";
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    std::string pattern = "%" + nome + "%";
    sqlite3_bind_text(stmt, 1, pattern.c_str(), -1, SQLITE_TRANSIENT);

    std::cout << "\nrisultati:\n";
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::cout << "id: " << sqlite3_column_int(stmt, 0) << "\n";
        std::cout << "nome: " << sqlite3_column_text(stmt, 1) << "\n";
        std::cout << "descrizione: " << sqlite3_column_text(stmt, 2) << "\n";
        std::cout << "categoria: " << sqlite3_column_text(stmt, 3) << "\n";
        std::cout << "quantita: " << sqlite3_column_int(stmt, 4) << "\n";
        std::cout << "prezzo: " << sqlite3_column_double(stmt, 5) << "\n";
        std::cout << "posizione: " << sqlite3_column_text(stmt, 6) << "\n";
        std::cout << "--------------------\n";
    }
    sqlite3_finalize(stmt);
}

static void reportSottoScorta(sqlite3* db) {
    int soglia = 0;
    std::cout << "soglia minima: ";
    std::cin >> soglia;
    sqlite3_stmt* stmt = nullptr;
    std::string sql = "SELECT id, nome, quantita FROM prodotti WHERE quantita <= ? ORDER BY quantita;";
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, soglia);

    std::cout << "\nsotto scorta:\n";
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::cout << sqlite3_column_int(stmt, 0) << " | "
                  << sqlite3_column_text(stmt, 1) << " | "
                  << sqlite3_column_int(stmt, 2) << "\n";
    }
    sqlite3_finalize(stmt);
}

int main() {
    sqlite3* db = nullptr;
    if (sqlite3_open("magazzino.db", &db) != SQLITE_OK) {
        std::cerr << "non apro il database.\n";
        return 1;
    }

    int scelta = -1;
    while (scelta != 0) {
        stampaMenu();
        std::cin >> scelta;
        switch (scelta) {
            case 1:
                if (inizializzaDB(db)) {
                    std::cout << "database pronto.\n";
                }
                break;
            case 2:
                aggiungiProdotto(db);
                break;
            case 3:
                listaProdotti(db);
                break;
            case 4:
                registraMovimento(db, "CARICO");
                break;
            case 5:
                registraMovimento(db, "SCARICO");
                break;
            case 6:
                cercaProdotto(db);
                break;
            case 7:
                reportSottoScorta(db);
                break;
            case 0:
                std::cout << "ciao.\n";
                break;
            default:
                std::cout << "scelta non valida.\n";
        }
    }

    sqlite3_close(db);
    return 0;
}
