## SpinLock

В этой задаче мы узнаем из чего сделаны [атомики](https://en.cppreference.com/w/cpp/atomic/atomic).

Вам дана [реализация простейшего Test-and-Set спинлока](spinlock.hpp).

Но в ней не хватает атомарных операций – `AtomicStore` и `AtomicExchange`. 

Семантика этих операций точно такая же, как у `store` и `exchange` в атомиках из стандартной библиотеки.

Реализуйте их!

А заодно заполните реализацию метода `TryLock` у спинлока.

---

Реализовать атомики в языке невозможно, придется писать на ассемблере. И реализация получится платформо-зависимой, в нашем случае это x86-64.

Заготовки для функций вы найдете в файле [atomics.S](atomics.S). 

Вспомните _calling conventions_ – [System V AMD64 ABI](https://en.wikipedia.org/wiki/X86_calling_conventions#System_V_AMD64_ABI):
- Функции получают первые два аргумента через регистры `%rdi` и `%rsi`.
- Возвращают результат (если он есть) через регистр `%rax`.

Для справки по ассемблеру см. [Introduction to X86-64 Assembly for Compiler Writers](https://web.archive.org/web/20160714182232/https://www3.nd.edu/~dthain/courses/cse40243/fall2015/intel-intro.html)