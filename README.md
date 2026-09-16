# cvcn — клиент-серверный чат

Учебный проект: TCP-сервер на epoll + Qt-клиент с собственным бинарным протоколом. Пользователи хранятся в SQLite.

## Зависимости

```bash
sudo apt install build-essential cmake ninja-build \
                 qtbase5-dev libfmt-dev libspdlog-dev libsqlite3-dev
```
Сборка
Сервер:

```bash
cd server
mkdir -p logs
cmake -S . -B build
cmake --build build -j
```
Бинарник — server/server.

Клиент:

```bash
cd client
mkdir -p logs
cmake -S . -B build
cmake --build build -j
```
Бинарник — client/client.

Запуск
Терминал 1 — сервер:

```bash
cd server
./server
```
Спросит порт. Enter → 55550. Остановка — Ctrl+C.

Терминал 2 — клиент:

```bash
cd client
./client
```
Сервер по умолчанию — 127.0.0.1:55550.

Проверка:

```bash
ss -tlnp | grep 55550
```
Использование
Ввести логин и пароль.

register — создать аккаунт.
authenticate — войти, откроется окно чата.
delete — пока не реализовано.

Логи: server/logs/server.log, client/logs/client.log.
БД: server/chat.db (SQLite).

```bash
sqlite3 server/chat.db 'SELECT id, username, created_at FROM users;'
```
Документация
Установить Doxygen и Graphviz:

```bash
sudo apt install doxygen graphviz
```
Сгенерировать HTML из корня проекта:

```bash
cd ~/cvcn-54
doxygen
```
Открыть:

```bash
xdg-open docs/html/index.html
Если Graphviz не нужен (нет диаграмм) — в Doxyfile поставить
HAVE_DOT = NO. Каталог docs/ — генерируемый, в .gitignore.
```
