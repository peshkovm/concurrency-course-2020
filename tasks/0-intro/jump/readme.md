# Прыжки

В этой задаче вы должны реализовать функции для манипуляции потоком управления ([_control flow_](https://en.wikipedia.org/wiki/Control_flow), не путать с _threads_):

- `Capture(&ctx)` – захватить текущий контекст исполнения, сохранить его в переменную `ctx`
- `JumpTo(&ctx)` – прыгнуть в вызов `Capture(&ctx)`, который сохранил переданный контекст исполнения

Разобраться в поведении этих функций проще всего с помощью [юнит-тестов](https://gitlab.com/Lipovsky/tpcc-course-2020/blob/master/tasks/0-intro/jump/unit_test.cpp).

Как вы можете увидеть, вызовы функций `Capture` и `JumpTo` ведут себя необычно:
- Управление никогда не возвращается из вызова `JumpTo(&ctx)`
- Зато из одного вызова `Capture(&ctx)` можно возвращаться многократно: один раз – как из обычного вызова, а последующие – после прыжков

Можно считать, что мы хотим реализовать аналог оператора [goto](https://xkcd.com/292/): вызов `Capture` соответствует объявлению метки, а `JumpTo` – безусловному переходу.

Для решения задачи вам нужно понимать [механику стека вызовов](https://manybutfinite.com/post/journey-to-the-stack/).

---

В С и C++ доступны аналогичные функции – [`setjmp`](https://en.cppreference.com/w/cpp/utility/program/setjmp) / [`longjmp`](https://en.cppreference.com/w/cpp/utility/program/longjmp).

Отличие наших функций от `setjmp` / `longjmp` – мы допускаем прыжки только в пределах одного вызова функции, пересекать границы стековых фреймов во время прыжка запрещается. Подумайте, почему локальные прыжки выполнять гораздо проще.

Попробуйте самостоятельно придумать практичные применения нелокальным прыжкам.

---

Для решения задачи заполните реализации функций `Capture` и `JumpTo` в файле `jump.S`, а также определение `JumpContext` в `jump.hpp`.

Проверять, что прыжки выполняются только в пределах одного стекового фрейма, не нужно. Если пользователь нарушит это требование, то получит неопределенное поведение.

## Полезные ссылки:
- [Introduction to X86-64 Assembly for Compiler Writers](https://www3.nd.edu/~dthain/courses/cse40243/fall2015/intel-intro.html)
- [Journey to the Stack](https://manybutfinite.com/post/journey-to-the-stack/)
- [Stack frame layout on x86-64](https://eli.thegreenplace.net/2011/09/06/stack-frame-layout-on-x86-64/)
