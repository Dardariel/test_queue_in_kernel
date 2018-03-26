Решение тестового задания. Разработка модуля ядра

Задание:
Необходимо реализовать:
(а) загружаемый модуль ядра, который хранит в оперативной памяти очередь произвольных сообщений до 64К каждое. Размер очереди ограничен 1024 элементами.
(б) консольное приложение, которое взаимодействует с модулем ядра и помещает сообщения в конец очереди (а).
(в) user-mode демон, который вычитывает сообщения из очереди (а) и помещает в файловое хранилище. Предусмотреть корректные запуск и остановку.
