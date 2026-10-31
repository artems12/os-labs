# 1. Компиляция программы с оптимизацией -O2 и подключением библиотеки pthread
gcc -O2 -pthread main.c -o lab2

# 2. Запуск программы с указанием количества потоков (например, 4 потока)
./lab2 4

# 3. Демонстрация активных потоков ОС в реальном времени:
./lab2 16 & PID=$!; while kill -0 $PID 2>/dev/null; do OUT=$(ps -T -p $PID); if [ $(echo "$OUT" | wc -l) -gt 2 ]; then echo "$OUT"; break; fi; done; wait $PID
