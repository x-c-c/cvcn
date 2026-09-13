#!/usr/bin/env bash
#
# rename_server.sh
# Приводит имена файлов и классов сервера к системе
# {Subject}{Role}.
#
# Файлы переименовываются через git mv (история сохраняется).
# Классы и функции внутри файлов — через sed с границами слов.
#
# Использование:
#   cd ~/cvcn/server
#   bash rename_server.sh
#   make clean && make
#
# Если что-то не так — git reset --hard HEAD вернёт всё.

set -euo pipefail

# ---------------------------------------------------------------------------
# Настройки
# ---------------------------------------------------------------------------

# Переименовывать спорные модули.
#   ResponseSender -> PacketSender  (единообразие с Packet* в protocol/)
#   Epoller        -> EventPoller    (переносимость: не привязка к Linux epoll)
#
# Если не хотите — поставьте false.
RENAME_RESPONSE_SENDER=true
RENAME_EPOLLER=true

# ---------------------------------------------------------------------------
# Проверки
# ---------------------------------------------------------------------------

if ! git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
    echo "Ошибка: это не git-репозиторий." >&2
    exit 1
fi

if ! git diff --quiet || ! git diff --cached --quiet; then
    echo "Ошибка: в рабочем дереве есть незакоммиченные изменения." >&2
    echo "Сделайте git add -A && git commit или git stash." >&2
    exit 1
fi

# Убедимся, что все ожидаемые файлы на месте.
missing=0
for f in \
    app/CheckPort.h app/CheckPort.cpp \
    app/ServerStartStop.h app/ServerStartStop.cpp \
    app/SigintHandler.h app/SigintHandler.cpp \
    protocol/PacketDeserializer.h protocol/PacketDeserializer.cpp \
    net/ResponseSender.h net/ResponseSender.cpp \
    net/Epoller.h net/Epoller.cpp
do
    if [[ ! -f "$f" ]]; then
        echo "  НЕТ: $f"
        missing=$((missing + 1))
    fi
done

if [[ $missing -gt 0 ]]; then
    echo "Ошибка: $missing файлов не найдены. Проверьте расположение." >&2
    exit 1
fi

# ---------------------------------------------------------------------------
# Хелперы
# ---------------------------------------------------------------------------

# Глобальная замена идентификатора во всех .h/.cpp под server/,
# кроме build/. Использует границы слов, чтобы не задеть подстроки.
replace_identifier() {
    local old="$1"
    local new="$2"
    grep -rl --include='*.h' --include='*.cpp' -e "\b${old}\b" . \
        | grep -v '^./build/' \
        | xargs -r sed -i "s/\b${old}\b/${new}/g"
}

# Замена в конкретных файлах (передаются после old и new).
replace_in() {
    local old="$1"
    local new="$2"
    shift 2
    for f in "$@"; do
        sed -i "s/\b${old}\b/${new}/g" "$f"
    done
}

# ---------------------------------------------------------------------------
# 1. CheckPort -> PortSelector
# ---------------------------------------------------------------------------
echo "== CheckPort -> PortSelector =="

git mv app/CheckPort.h   app/PortSelector.h
git mv app/CheckPort.cpp app/PortSelector.cpp

# Header guard (uppercase — sed по границам слов даёт неверный результат,
# заменяем явно).
sed -i 's/\bCHECKPORT_H\b/PORTSELECTOR_H/g' app/PortSelector.h

# Классовых имён нет, только свободные функции.
replace_identifier  CheckPort            PortSelector
replace_identifier  tryCreateSocketOnPort isPortFree
replace_identifier  getValidPort          promptForPort
replace_identifier  CHECKPORT_H           PORTSELECTOR_H

# include-директива
sed -i 's/#include "CheckPort.h"/#include "PortSelector.h"/g' app/main.cpp

# ---------------------------------------------------------------------------
# 2. ServerStartStop -> ListeningSocket (и переезд в net/)
# ---------------------------------------------------------------------------
echo "== ServerStartStop -> ListeningSocket =="

git mv app/ServerStartStop.h   net/ListeningSocket.h
git mv app/ServerStartStop.cpp net/ListeningSocket.cpp

replace_identifier  ServerStartStop  ListeningSocket
replace_identifier  SERVERSTARTSTOP_H LISTENINGSOCKET_H

# Метод start -> listen (уникальный идентификатор, безопасно глобально)
replace_identifier  getServerSocketFD fileDescriptor

# Только внутри класса и в main.cpp: server.start(...) -> listener.listen(...).
# Здесь можно было бы просто заменить start, но start — слишком общее слово.
# Поэтому меняем конкретно ListeningSocket::start и вызовы вида .start(.
sed -i 's/\bListeningSocket::start\b/ListeningSocket::listen/g' net/ListeningSocket.cpp
sed -i 's/\bvoid start(/void listen(/g' net/ListeningSocket.h

# В main.cpp переименуем переменную server -> listener и метод.
sed -i 's/\bserver\.start(/listener.listen(/g' app/main.cpp
sed -i 's/\bserver\.fileDescriptor()/listener.fileDescriptor()/g' app/main.cpp
sed -i 's/\bListeningSocket server\b/ListeningSocket listener/g' app/main.cpp

sed -i 's/#include "ServerStartStop.h"/#include "ListeningSocket.h"/g' app/main.cpp

# ---------------------------------------------------------------------------
# 3. SigintHandler -> ShutdownSignal
# ---------------------------------------------------------------------------
echo "== SigintHandler -> ShutdownSignal =="

git mv app/SigintHandler.h   app/ShutdownSignal.h
git mv app/SigintHandler.cpp app/ShutdownSignal.cpp

replace_identifier  SigintHandler       ShutdownSignal
replace_identifier  SIGINTHANDLER_H     SHUTDOWNSIGNAL_H
replace_identifier  isStopRequested     isRequested

sed -i 's/#include "SigintHandler.h"/#include "ShutdownSignal.h"/g' \
    app/main.cpp app/PortSelector.cpp net/EventPoller.cpp net/ListeningSocket.cpp 2>/dev/null || true

# ---------------------------------------------------------------------------
# 4. PacketDeserializer -> PacketParser
# ---------------------------------------------------------------------------
echo "== PacketDeserializer -> PacketParser =="

git mv protocol/PacketDeserializer.h   protocol/PacketParser.h
git mv protocol/PacketDeserializer.cpp protocol/PacketParser.cpp

replace_identifier  PacketDeserializer  PacketParser
replace_identifier  PACKETDESERIALIZER_H PACKETPARSER_H
replace_identifier  deserializeHeader   parseHeader
replace_identifier  deserializeData     parseData

sed -i 's/#include "PacketDeserializer.h"/#include "PacketParser.h"/g' \
    app/main.cpp net/Connection.cpp protocol/PacketDispatcher.cpp 2>/dev/null || true

# ---------------------------------------------------------------------------
# 5. ResponseSender -> PacketSender  (опционально)
# ---------------------------------------------------------------------------
if [[ "$RENAME_RESPONSE_SENDER" == "true" ]]; then
    echo "== ResponseSender -> PacketSender =="

    git mv net/ResponseSender.h   net/PacketSender.h
    git mv net/ResponseSender.cpp net/PacketSender.cpp

    replace_identifier  ResponseSender  PacketSender
    replace_identifier  RESPONSESENDER_H PACKETSENDER_H
    replace_identifier  sendResponse    send

    sed -i 's/#include "ResponseSender.h"/#include "PacketSender.h"/g' \
        net/ClientSession.h net/ClientSession.cpp 2>/dev/null || true
fi

# ---------------------------------------------------------------------------
# 6. Epoller -> EventPoller  (опционально)
# ---------------------------------------------------------------------------
if [[ "$RENAME_EPOLLER" == "true" ]]; then
    echo "== Epoller -> EventPoller =="

    git mv net/Epoller.h   net/EventPoller.h
    git mv net/Epoller.cpp net/EventPoller.cpp

    replace_identifier  Epoller      EventPoller
    replace_identifier  EPOLLER_H    EVENTPOLLER_H

    # Внутренний epoll-файловый дескриптор называется epollFD_ — это деталь
    # реализации, оставляем. Меняем только имя класса.
    sed -i 's/#include "Epoller.h"/#include "EventPoller.h"/g' \
        net/ClientSession.cpp net/SessionManager.cpp net/PacketSender.cpp app/main.cpp 2>/dev/null || true
fi

# ---------------------------------------------------------------------------
# Итог
# ---------------------------------------------------------------------------
echo
echo "== Готово =="
echo "Дальше:"
echo "  1. Обновите Makefile: список файлов изменился."
echo "  2. make clean && make"
echo "  3. Проверьте сборку и запуск."
echo "  4. git status — просмотрите изменения."
echo
echo "TODO (нужно сделать руками или отдельным коммитом):"
echo "  - ClientSession::getfileDescriptor() -> fileDescriptor()"
echo "  - ClientSession::getUserID()         -> userID()"
echo "  - ClientSession::getUsername()       -> username()"
echo "  - bool success в структурах вместо uint8_t"
echo "  - удалить Validator::validateSenderID"
echo "  - удалить MessageSendData::senderID"
