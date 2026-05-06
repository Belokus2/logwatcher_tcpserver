import socket
import json
import time
import threading

HOST = '127.0.0.1'
PORT = 8080

def listen_to_server(sock):
    buffer = ""
    while True:
        try:
            data = sock.recv(4096).decode('utf-8')
            if not data:
                print("\n[Сервер закрыл соединение]")
                break
            buffer += data
            while '\n' in buffer:
                line, buffer = buffer.split('\n', 1)
                if line.strip():
                    parsed = json.loads(line)
                    print(f"\n[Входящее сообщение:] {json.dumps(parsed, indent=2, ensure_ascii=False)}")
        except ConnectionAbortedError:
            break
        except Exception as e:
            print(f"\n[Ошибка чтения]: {e}")
            break

def send_command(sock, command_dict):
    print(f"\n[Отправка] {command_dict.get('command', 'UNKNOWN')}")
    msg = json.dumps(command_dict) + "\n"
    sock.sendall(msg.encode('utf-8'))

def main():
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        print(f"Попытка подключения к {HOST}:{PORT}...")
        sock.connect((HOST, PORT))
        print("Успешно подключено!")

        # Запускаем поток для прослушивания ответов от сервера
        listener = threading.Thread(target=listen_to_server, args=(sock,), daemon=True)
        listener.start()

        time.sleep(1)

        # Тест 1: Проверка связи (PING)
        send_command(sock, {"command": "PING"})
        time.sleep(1)

        # Тест 2: Подписка на алерты
        send_command(sock, {"command": "SUBSCRIBE"})
        time.sleep(1)

        # Тест 3: Запрос статистики (QUERY) для уровня Info
        send_command(sock, {
            "command": "QUERY",
            "level": "Info",
            "n": 5
        })
        time.sleep(2)

        # Тест 4: Запрос статистики (QUERY) для уровня Error
        send_command(sock, {
            "command": "QUERY",
            "level": "Error",
            "n": 3
        })
        time.sleep(2)

        # Тест 5: Отписка от алертов
        send_command(sock, {"command": "UNSUBSCRIBE"})
        time.sleep(2)

        print("\nВсе тесты отправлены. Ждем 5 секунд (вдруг прилетят алерты)...")
        time.sleep(5)

    except ConnectionRefusedError:
        print(f"\nНе удалось подключиться. Проверьте, запущен ли сервер на {HOST}:{PORT}.")
    except KeyboardInterrupt:
        print("\nПрервано пользователем.")
    except Exception as e:
        print(f"\nПроизошла непредвиденная ошибка: {e}")
    finally:
        print("\nЗакрытие соединения.")
        sock.close()

if __name__ == "__main__":
    main()
    input()