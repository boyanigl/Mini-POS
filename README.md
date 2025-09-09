# Mini-POS
Build a small C++17 console app for Linux that acts as a POS “gateway” talking to a payment terminal over TCP.


Main executable commands:<br />
● posgw sale --amount 12.34 --host 127.0.0.1 --port 9000<br />
● posgw last --n 5 (prints last N transactions from DB)<br />
● posgw recon (prints daily totals: count + sum of approved/declined)<br />
