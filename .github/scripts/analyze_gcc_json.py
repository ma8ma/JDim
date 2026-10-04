#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""
gcc のコンパイラーオプション -fdiagnostics-format=json-file を使ってビルドすると生成される
jsonファイルを解析しエラーを分類、カウントしてMarkdown形式で標準出力に書き出すスクリプト
"""

import glob
import json
import os
import sys
from collections import Counter


def parse_gcc_json_files(json_dir):
    warnings = Counter()
    errors = Counter()
    analyzed_files = 0

    # builddir 配下の全 json ファイルを探す
    json_files = glob.glob(os.path.join(json_dir, "**", "*.gcc.json"), recursive=True)

    for file_path in json_files:
        try:
            with open(file_path, "r", encoding="utf-8") as f:
                data = json.load(f)
                # GCCのJSONはトップレベルがリスト構造
                for diag in data:
                    kind = diag.get("kind")
                    raw_option = diag.get("option")
                    message = diag.get("message", "")

                    if kind == "warning":
                        # [-Wdeprecated-declarations] などの警告フラグごとにカウント
                        # 警告フラグ + メッセージの簡易分類
                        if raw_option:
                            key = raw_option
                        elif message:
                            key = message.split("\n")[0][:80]
                        else:
                            key = "other-error/warning"
                        warnings[key] += 1
                    elif kind == "error":
                        # エラーはメッセージの先頭部分等で分類
                        short_msg = (
                            message.split("\n")[0][:60] if message else "Unknown Error"
                        )
                        errors[short_msg] += 1
            analyzed_files += 1
        except (ValueError, OSError) as e:
            # 万が一読み込みに失敗してもスキップ
            print(f"Failed to parse {file_path}: {e}", file=sys.stderr)
            continue

    return warnings, errors, analyzed_files


if __name__ == "__main__":
    if len(sys.argv) < 2:
        sys.exit("usage: analyze_gcc_json.py json_dir")

    json_dir = sys.argv[1]
    warnings, errors, analyzed_files = parse_gcc_json_files(json_dir)

    # Markdown 形式で出力
    print("## 📊 GTKMM4 Migration Diagnostics\n")
    print(f"* JSON files analyzed: {analyzed_files}")
    print(f"* Diagnostics analyzed: {sum(warnings.values()) + sum(errors.values())}\n")

    print("### ❌ Errors\n")
    if errors:
        print("| Count | Error Summary |\n|---|---|")
        for err, count in errors.most_common():
            print(f"| {count} | `{err}` |")
    else:
        print("No errors found! 🎉\n\n")

    print("\n### ⚠️ Warnings\n")
    if warnings:
        print("| Count | Warning Flag |\n|---|---|")
        for warn, count in warnings.most_common():
            print(f"| {count} | `{warn}` |")
    else:
        print("No warnings found!")
