# Защитник

Реализуйте обертку `Guarded<T>`, которая автомагически превращает произвольный класс в *потокобезопасный* (*thread-safe*).

Пример:

```cpp
Guarded<std::vector<int>> items; // vector<int> + mutex
...
// mutex.lock() -> push_back(42) -> mutex.unlock()
items->push_back(42);
```

Набор защищаемых методов `Guarded`-у заранее неизвестен, он должен уметь оборачивать произвольный класс.

Изучите [Synchronized](https://github.com/facebook/folly/blob/master/folly/docs/Synchronized.md) из библиотеки folly.