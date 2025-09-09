# Mini-POS
Build a small C++17 console app for Linux that acts as a POS “gateway” talking to a payment terminal over TCP. It should: 1. perform a SALE over a simplified OPI-like text protocol, persist results to SQLite, 2. provide a CLI to query recent transactions, No frameworks. Use only the C++ standard library + system sockets and sqlite3 (C API).
