# Echo Strikes Back

1) Реализуйте сокеты для файберов.
2) Напишите с их помощью синхронную версию эхо-сервера.

## Пререквизиты

1) Задача [Echo](/tasks/0-intro/echo)
2) Задача [SleepFor-Asio](/tasks/3-tinyfiber/sleep-asio)

## Асинхронность и поток управления

Самый простой способ писать сетевой код – [потоки](http://think-async.com/Asio/asio-1.12.2/src/examples/cpp11/echo/blocking_tcp_echo_server.cpp). Но если клиентов десятки тысяч, то по потоку на каждого из них не заведешь.

Альтернативный подход – использовать цикл событий и асинхронные операции. Правда при этом поток управления выворачивается наизнанку: теперь он подчиняется не вашему коду (как в случае с потоками), а циклу внутри `io_context`-а.

Ваш код разрывает на кусочки коллбэков, вы теряете возможность писать циклы. Читать, писать и отлаживать такой код очень сложно. Кроме того вы лишаетесь возможности использовать исключения, снимать стек-трейсы.

Такую цену вы платите за масштабируемость вашего сетевого кода.

Файберы позволяют разрешить дилемму между простотой и масштабируемостью и получить преимущества _обоих_ подходов. С помощью механизма переключения контекста можно склеить точки старта и завершения асинхронной операции и получить для пользователя файберов видимость синхронного вызова. В то же время под капотом будет крутиться тот же цикл событий.

## Кооперативность и I/O

Вспомним о кооперативной природе файберов – они могут уступать поток планировщика только добровольно. 

Операции сетевого ввода-вывода - естественные точки для кооперативного переключения: файбер стартует асинхронную операцию (например, чтение из сокета), планирует свое возобновление в коллбеке, после чего уступает поток планировщика другому файберу.

Во многих современных языках программирования такие точки требуется явно маркировать с помощью ключевого слова `await` в тех или иных вариациях: например, в С++ – это [`co_await`](https://en.cppreference.com/w/cpp/language/coroutines), в Rust – `.await`.

## Заметки по реализации

Вам потребуется интеграция планировщика файберов и цикла событий из Asio, эта часть уже должна быть готова после решения [SleepFor-Asio](/tasks/3-tinyfiber/sleep-asio). Если вы написали там хороший код, то в планировщик изменений вносить не потребуется.

Используйте [`asio::ip::tcp::socket`](http://think-async.com/Asio/asio-1.12.2/doc/asio/reference/ip__tcp/socket.html) и [`asio::ip::tcp::acceptor`](http://think-async.com/Asio/asio-1.12.2/doc/asio/reference/ip__tcp/acceptor.html) в реализациях `Socket` и `Acceptor`. Работать с низкоуровневыми системными интерфейсами не нужно.


Методы `ReadSome` и `Write` у сокета реализуйте через [`async_read_some`](http://think-async.com/Asio/asio-1.12.2/doc/asio/reference/basic_stream_socket/async_read_some.html) и [`async_write`](http://think-async.com/Asio/asio-1.12.2/doc/asio/reference/async_write/overload1.html) соответственно.

Метод `Read` реализуйте через `ReadSome`.

Метод `ListenAvailablePort` реализуется через `Listen(0)`.

### Установка соединения

Сначала вам потребуется транслировать `host` в IP-адреса, для этого используйте класс [`asio::ip::tcp::resolver`](http://think-async.com/Asio/asio-1.12.2/doc/asio/reference/ip__tcp/resolver.html).

Затем нужно проитерироваться по всем `endpoint`-ам и попробовать приконнектиться к каждому.

Рекомендуем реализовать в сокете вспомогательный метод `Connect`, который получает [`asio::ip::tcp::endpoint`](http://think-async.com/Asio/asio-1.12.2/doc/asio/reference/ip__tcp/endpoint.html).

Изучите пример [async_tcp_client](https://github.com/chriskohlhoff/asio/blob/master/asio/src/examples/cpp11/timeouts/async_tcp_client.cpp).

### Асинхронный резолвинг адреса (дополнительно)

По аналогии с `Acceptor` напишите класс `Resolver`, который использует `async_resolve`. Кто будет владеть `Resolver`-ом?

### Неблокирующие операции (дополнительно)

В реализации `ReadSome` попробуйте сначала оптимистично читать данные из сокета в неблокирующем режиме, и только в случае неудачи запускать `async_read_some`.

## Обработка ошибок

API сокетов построено на классах `Result<T>` и `Status` (синоним для `Result<void>`).

`Result` не навязывает конкретный способ обработки ошибок, вы можете использовать как обработку кодов, так и исключения.

Экземпляр `Result` гарантированно содержит _либо_ значение типа `T`, _либо_ код ошибки.

Если вы проигнорируете проверку `Result`-а, который получили из вызова метода или функции, то компилятор сгенерирует warning, см. [nodiscard](https://en.cppreference.com/w/cpp/language/attributes/nodiscard)

### Примеры использования

#### Исключения

```cpp
// Здесь срабатывает неявная конвертация из `Result<Socket> &&`
// с проверкой `ThrowIfError`
Socket client_socket = acceptor.Accept();

// Снова используем неявную конвертацию из `Result`
size_t bytes_read = client_socket.Read(asio::buffer(read_buf, kBufSize));

// Write возвращает `Status` - синоним `Result<void>`
// Проверка результата и выбрасывание исключения
// происходит в вызове ExpectOk()
client_socket->Write(asio::buffer(read_buf, bytes_read)).ExpectOk();

```

#### Коды ошибок

```cpp
// Здесь за auto прячется `Result<Socket>`
auto client_socket = acceptor.Accept();

// Для более явной проверки можно использовать `IsOk` или `HasError`
if (!socket) {
  // Handle socket.Error()
}

auto bytes_read = client_socket->Read(asio::buffer(read_buf, kBufSize));
if (bytes_read.HasError()) {
  // Handle bytes_read.Error()
}
 
// Метод `Write` возвращает `Status`, он же `Result<void>`
auto ok = client_socket->Write(asio::buffer(read_buf, *bytes_read));

```

#### Конструирование 

Можно строить `Result`-ы с помощью статических конструкторов `Ok` и `Fail`, но лучше воспользоваться свободными функциями из пространства имен `make_result`, они возьмут на себя вывод шаблонного типа: 

```cpp
Result<size_t> Socket::Read(MutableBuffer buffer) {
  // Выполняем чтение
    
  // Здесь error – std::error_code, полученный от asio
  if (error) {
    // Магия, не нужно указывать шаблонный тип целевого Result-а
    return make_result::Fail(error);
  }
  
  return make_result::Ok(bytes_read);
}

Status Socket::ShutdownWrite() {
  // ...

  // Здесь error - std::error_code, полученный от shutdown из asio
  return make_result::ToStatus(error);
}
```

### Подходы к обработке ошибок

- [Joe Duffy's Blog – The Error Model](http://joeduffyblog.com/2016/02/07/the-error-model/)
- Go: [Error handling and Go](https://blog.golang.org/error-handling-and-go), [Error Handling — Problem Overview](https://go.googlesource.com/proposal/+/master/design/go2draft-error-handling-overview.md), [Error Handling — Draft Design](https://go.googlesource.com/proposal/+/master/design/go2draft-error-handling.md)
- Rust: [Recoverable Errors with `Result`](https://doc.rust-lang.org/book/ch09-02-recoverable-errors-with-result.html)
- C++: [Boost.Outcome](https://www.boost.org/doc/libs/1_72_0/libs/outcome/doc/html/index.html), [`ErrorOr<T>`](https://github.com/llvm-mirror/llvm/blob/master/include/llvm/Support/ErrorOr.h) в LLVM, [`expected`](https://github.com/TartanLlama/expected)

## Эхо-сервер

## Файлы решения

Для поддержки сетевого ввода-вывода вы можете изменять файлы `scheduler.{hpp,cpp}`, `fiber.{hpp,cpp}` и `socket.{hpp,cpp}`.

Реализация эхо-сервера находится в `echo.{hpp,cpp}`.

