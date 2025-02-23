# Vector со small-object и copy-on-write оптимизациями

В этом задании необходимо реализовать класс, аналогичный `std::vector`, но имеющий *small-object* и *copy-on-write* оптимизации.

*Small Object Optimization* предполагает, что вектор умеет хранить небольшое число элементов без динамической аллокации памяти.

*Copy-on-write* предполагает, что копирование/присваивание больших векторов не копирует все элементы само, а откладывает копирование элементов до момента, когда к объекту применят модифицирующую операцию.

## Основные требования

Реализуемый класс должен называться `socow_vector` и лежать в заголовочном файле `socow-vector.h`. Он должен иметь два шаблонных параметра: тип хранимых объектов и размер маленького буффера.

```cpp
template <typename T, std::size_t SMALL_SIZE>
class socow_vector;
```

Из-за наличия *small-object* и *copy-on-write* оптимизаций, некоторые операции имеют другую вычислительную сложность и/или предоставляют другую гарантию безопасности исключений:

- Конструктор копирования и оператор присваивания должны работать за `O(SMALL_SIZE)`, а не за `O(size)`.
- Если в `b` хранится small object, `a = b` должен предоставлять сильную гарантию безопасности исключений, иначе nothrow.
- Неконстантные операции `operator[]`, `data()`, `front()`, `back()`, `begin()`, `end()` должны работать за `O(size)` и удовлетворять сильной гарантии безопасности исключений, если требуется копирование для *copy-on-write*, и за `O(1)` и nothrow иначе.
- Как и со стандартным вектором, `reserve` должен гарантировать, что после выполнения `reserve(n)` вставки в вектор не будут приводить к переаллокациям, пока размер не достигнет `n`.

Вы можете полагаться, что конструктор перемещения и оператор перемещающего присваивания для `T` не бросают исключения.

## Методы `socow_vector`

- Конструктор по умолчанию;
- Конструктор копирования;
- Конструктор перемещения;
- Оператор копирующего присваивания;
- Оператор перемещающего присваивания;
- `swap(socow_vector& other)` &mdash; поменять состояния текущего вектора и `other` местами;
- `size()` &mdash; размер вектора;
- `capacity()` &mdash; вместимость вектора;
- `empty()` &mdash; является ли вектор пустым;
- `operator[](std::size_t index)`; &mdash; обращение к элементу вектора;
- `front()`, `back()` &mdash; обращение к первому/последнему элементу вектора;
- `data()` &mdash; указатель на начало вектора;
- `begin()`, `end()` &mdash; итераторы;
- `push_back(...)` &mdash; вставить элемент в конец вектора (аргументом может быть lvalue или rvalue);
- `insert(const_iterator pos, ...)` &mdash; вставить элемент перед `pos`;
- `pop_back()` &mdash; удалить элемент из конца вектора;
- `erase(const_iterator pos)` &mdash; удалить элемент по итератору;
- `erase(const_iterator first, const_iterator last)` &mdash; удалить все элементы в диапазоне `[first, last)`;
- `clear()` &mdash; очистить вектор от всех элементов;
- `reserve(size_t new_capacity)` &mdash; установить вместимость вектора, если текущая меньше;
- `shrink_to_fit()` &mdash; сжать вместимость вектора до текущего размера.
