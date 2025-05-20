#!/usr/bin/env python3
import sys
import json

def main():
    # バイトストリームを読み込んで、utf-8-sig でデコード（BOM を除去）
    raw = sys.stdin.buffer.read().decode('utf-8-sig')
    data = json.loads(raw)

    for item in data.get("ui_snapshot", []):
        print(item.get("answer_from_LLM", ""))

if __name__ == "__main__":
    main()
