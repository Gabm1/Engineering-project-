import asyncio
import json
import argparse
from datetime import datetime
import websockets


def now_ts() -> str:
    # czytelny timestamp do logu
    return datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f")[:-3]


async def logger(ws_url: str, out_path: str, raw_path: str | None, append: bool):
    file_mode = "a" if append else "w"

    while True:
        try:
            print(f"[{now_ts()}] Connecting to {ws_url} ...")
            async with websockets.connect(ws_url, ping_interval=20, ping_timeout=20) as ws:
                print(f"[{now_ts()}] Connected.")

                with open(out_path, file_mode, encoding="utf-8") as f_out:
                    f_raw = open(raw_path, file_mode, encoding="utf-8") if raw_path else None

                    try:
                        async for msg in ws:
                            # opcjonalnie zapis surowego payloadu
                            if f_raw:
                                f_raw.write(msg + "\n")
                                f_raw.flush()

                            # spodziewamy się JSON-a: {"type":"console","content":"..."}
                            try:
                                data = json.loads(msg)
                            except json.JSONDecodeError:
                                # jeśli przyszło coś nie-JSON, zapisz jako RAW
                                line = f"[{now_ts()}] RAW: {msg}"
                                f_out.write(line + "\n")
                                f_out.flush()
                                continue

                            msg_type = data.get("type", "")
                            if msg_type == "console":
                                content = data.get("content", "")
                                # content może zawierać '\r', więc czyścimy końcówki
                                content = str(content).rstrip("\r\n")
                                line = f"[{now_ts()}] {content}"
                                f_out.write(line + "\n")
                                f_out.flush()
                                print(line)
                            else:
                                # inne typy (info/error) też możesz logować:
                                line = f"[{now_ts()}] {msg_type.upper()}: {data}"
                                f_out.write(line + "\n")
                                f_out.flush()

                    finally:
                        if f_raw:
                            f_raw.close()

        except (OSError, websockets.InvalidURI, websockets.InvalidHandshake) as e:
            print(f"[{now_ts()}] Connection error: {e}")
        except websockets.ConnectionClosed as e:
            print(f"[{now_ts()}] Connection closed: {e}")
        except Exception as e:
            print(f"[{now_ts()}] Unexpected error: {e}")

        # reconnect delay
        await asyncio.sleep(1.0)


def main():
    p = argparse.ArgumentParser(description="ESP8266 WebSocket console logger")
    p.add_argument("--ip", required=True, help="IP ESP (np. 192.168.4.1 albo 192.168.0.34)")
    p.add_argument("--port", type=int, default=8000, help="Port WebSocket (domyślnie 8000)")
    p.add_argument("--out", default="esp_log.txt", help="Plik wyjściowy (domyślnie esp_log.txt)")
    p.add_argument("--raw", default=None, help="Opcjonalnie zapisuj surowe wiadomości do tego pliku")
    p.add_argument("--append", action="store_true", help="Dopisuj do pliku zamiast nadpisywać")
    args = p.parse_args()

    ws_url = f"ws://{args.ip}:{args.port}/"
    asyncio.run(logger(ws_url, args.out, args.raw, args.append))


if __name__ == "__main__":
    main()
