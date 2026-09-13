#!/usr/bin/env python3
"""
fix_client_remaining.py

Доделывает правки, которые не сработали в fix_client_naming.py
из-за ошибки экранирования regex (двойные обратные слэши в raw-строках).

  1. Header guards: MODEL_H -> PROTOCOLCLIENT_H, CONTROLLER_H -> MAINCONTROLLER_H.
  2. resp.success == 1 -> resp.success.
  3. Удалить #include <QDebug>.
  4. Удалить определение Validator::validateSenderID из .cpp.

Запускать из ~/cvcn/client/ в чистом git-репозитории.
Откат: git reset --hard HEAD && git clean -fd
"""

import re
import subprocess
import sys
from pathlib import Path

ROOT = Path.cwd()
SKIP_DIRS = {"build", ".git", "CMakeFiles", "logs", ".cache", "ClientOnQt"}


def read(p): return p.read_text(encoding="utf-8")
def write(p, t): p.write_text(t, encoding="utf-8")


def iter_h_cpp():
    for p in ROOT.rglob("*"):
        if not p.is_file() or p.suffix not in (".h", ".cpp"):
            continue
        parts = p.relative_to(ROOT).parts
        if any(x in parts for x in SKIP_DIRS):
            continue
        yield p


def check_git_clean():
    for args in (["git", "diff", "--quiet"], ["git", "diff", "--cached", "--quiet"]):
        try:
            r = subprocess.run(args, cwd=ROOT, capture_output=True, check=False)
            if r.returncode != 0:
                print("ОШИБКА: незакоммиченные изменения. git reset --hard HEAD.",
                      file=sys.stderr)
                sys.exit(1)
        except FileNotFoundError:
            break


def sub_file(path, pattern, replacement):
    p = ROOT / path
    if not p.exists():
        print(f"  SKIP: {path} не найден")
        return 0
    text = read(p)
    text, n = re.subn(pattern, replacement, text)
    if n:
        write(p, text)
    return n


def sub_all(pattern, replacement):
    compiled = re.compile(pattern)
    total = 0
    for p in iter_h_cpp():
        text = read(p)
        new_text, n = compiled.subn(replacement, text)
        if n:
            write(p, new_text)
            total += n
    return total


def main():
    check_git_clean()

    print("=== 1: header guards ===")
    n1 = sub_file("protocol/ProtocolClient.h", r"\bMODEL_H\b", "PROTOCOLCLIENT_H")
    n2 = sub_file("logic/MainController.h",    r"\bCONTROLLER_H\b", "MAINCONTROLLER_H")
    print(f"  ProtocolClient.h: {n1}")
    print(f"  MainController.h: {n2}")

    print("=== 2: resp.success == 1 -> resp.success ===")
    n = sub_all(r"\bresp\.success\s*==\s*1\b", "resp.success")
    print(f"  всего: {n}")

    print("=== 3: убрать #include <QDebug> ===")
    n = sub_all(r"#include\s*<QDebug>\s*\n", "")
    print(f"  всего: {n}")

    print("=== 4: Validator::validateSenderID из .cpp ===")
    n = sub_file(
        "utils/Validator.cpp",
        r"\nbool Validator::validateSenderID\(uint32_t senderID\)\s*\{\s*return senderID > 0;\s*\}\n",
        "\n")
    print(f"  Validator.cpp: {n}")

    print()
    print("=== Готово ===")
    print("Проверка:")
    print("  grep -rn 'MODEL_H\\|CONTROLLER_H' protocol logic --include='*.h'")
    print("  grep -rn 'success == 1' protocol logic --include='*.cpp'")
    print("  grep -rn 'QDebug' . --include='*.cpp' | grep -v build")
    print("  grep -rn 'validateSenderID' utils")


if __name__ == "__main__":
    main()
