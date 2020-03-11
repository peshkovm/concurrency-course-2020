# SleepFor (Asio-версия)

Эта задача полностью повторяет соседниюю задачу [SleepFor](/tasks/3-tinyfiber/sleep) – вы по-прежнему должны научить файберы спать.

Но в этом варианте задачи вам запрещается:
- использовать функции [std::this_thread::sleep_for](https://en.cppreference.com/w/cpp/thread/sleep_for) и [sleep](http://man7.org/linux/man-pages/man3/sleep.3.html), т.е. явно ставить поток на паузу,
- писать свои собственные очереди спящих потоков.

Вместо этого вы должны использовать таймеры и event loop (`io_context`) из библиотеки [Asio](https://github.com/chriskohlhoff/asio).

## Пререквизиты

Перед тем, как приступать к этой задаче, рекомендуем:

1) Решить задачу [Echo](/tasks/0-intro/echo) и познакомиться с библиотекой _Asio_.
2) Реализовать `SleepFor` голыми руками в задаче [SleepFor](/tasks/3-tinyfiber/sleep).

Структурно решения двух вариантов задачи будут очень похожи, и возможно, после решения этой версии задачи вам захочется порефакторить ваше решение соседней задачи.

## Asio

Прочтите tutorial по таймерам:  [Using a timer asynchronously](http://think-async.com/Asio/asio-1.12.2/doc/asio/tutorial/tuttimer2.html).

Используйте специализацию `WaitableTimer`, определенную в [timer.hpp](/tasks/3-tinyfiber/sleep-asio/timer.hpp).

Изучите варианты методов `run` и `poll` из документации [io_context](http://think-async.com/Asio/asio-1.12.2/doc/asio/reference/io_context.html).

### Остановка цикла

Если в `io_context` заканчиваются события, то он _останавливается_ (см. метод [`stopped`](http://think-async.com/Asio/asio-1.12.2/doc/asio/reference/io_context/stopped.html)): все последующие вызовы `run` и `poll` будут моментально возвращать управление, не вызывая никаких хэндлеров. Как вы увидите, такое поведение будет доставлять неудобства.

Воспользуйтесь рецептом из [Stopping the io_context from running out of work](http://think-async.com/Asio/asio-1.12.2/doc/asio/reference/io_context.html#asio.reference.io_context.stopping_the_io_context_from_running_out_of_work).

Постоянно перезапускать `io_context` с помощью метода `restart` запрещается.

## Файлы решения

Вы можете менять содержимое файлов `fiber.{hpp,cpp}` и `scheduler.{hpp,cpp}`.

## Мораль

Эта задача – конечно же не про реализацию `SleepFor`, в конце концов это не самая полезная функциональность для файберов. 

Главное содержание задачи – с помощью переключения контекста научиться клеить разорванный асинхронными вызовами поток управления. В качестве повода для этого выбран `SleepFor`.

Ну а после этого можно будет научить ваши однопоточные файберы вообще _всему_, что умеет библиотека _Asio_!

Таким образом, с помощью файберов возможно вправить вывихнутый поток управления _Asio_ на коллбэках и писать код, который будет работать почти так же эффективно, как событийный, но при этом выглядеть так же просто, как многопоточный синхронный.