# Bitset

В этом задании необходимо реализовать структуру данных, представляющую собой последовательность бит, с поддержкой итерирования по ним, а также побитовых операций.

## Основные особенности

- Размер известен в момент конструирования и далее не меняется (кроме как через присваивающие операторы и сдвиги).
- Компактность: количество памяти, используемое под `bitset`, не должно превышать `size + C` бит, где `size` &mdash; количество хранимых битов, а `C` &mdash; какая-то константа, не зависящая от `size`.
- Об исключениях и гарантиях, связанных с ними, в этом задании можно не задумываться.
- Поддержка семантики перемещения не требуется.

## Вспомогательные классы

Помимо непосредственно класса `bitset`, вам понадобится реализовать:

### Итераторы

В качестве категории стоит выбрать random-access.

Для итераторов не нужно реализовывать `operator->` (всё равно он не имеет смысла для `bool`). При этом type member `pointer` у итератора можно оставить равным `void`.

Из-за этих упрощений получившийся итератор, вероятно, не будет строго удовлетворять требованиям [LegacyForwardIterator](https://en.cppreference.com/w/cpp/named_req/ForwardIterator), однако будет удовлетворять требованиям итераторов нового образца ([forward_iterator](https://en.cppreference.com/w/cpp/iterator/forward_iterator)) из C++20.


### Представления (views)

View &mdash; это легковесный объект, который ссылается на какой-то диапазон значений (не владея им), и по которому можно итерироваться. Часто он может быть представлен парой итераторов.

View для битсета интересен тем, что над ним тоже может иметь смысл проводить некоторые операции, в том числе побитовые. Для создания `view` используется `bitset::subview(offset, count)` или конструкторы `view`.

### Прокси-объекты для эмуляции ссылок

Поскольку биты хранятся упакованно, возникают проблемы с тем, что возвращать из `bitset::operator[]`. Несложно понять, что с учётом этого требования это не сможет быть `bool &`.

Для решения этой проблемы предлагается в качестве `bitset::reference` использовать вспомогательный класс, который поддерживает следующие операции:
- `bs[i]` &mdash; неявно приводится к `bool`;
- `bs[i] = false` &mdash; оператор присваивания от `bool` для изменения бита на заданное значение;
- `bs[i].flip()` &mdash; инвертировать значение бита на обратное (для неконстантной ссылки).

Обратите внимание, что поддержка `&bs[i]` не требуется, что даёт некоторую свободу для выбора типов для `reference` и `const_reference`.

## Методы `bitset`

#### Конструкторы

- `bitset()` &mdash; пустая последовательность битов;
- `bitset(std::size_t size, bool value)` &mdash; `size` битов, каждый из которых равен `value`;
- `bitset(const bitset& other)` &mdash; конструктор копирования;
- `bitset(std::string_view str)` &mdash; на основе строки, состоящей из символов `'0'` и `'1'`;
- `bitset(const const_view& other)` &mdash; копия переданного `view`;
- `bitset(const_iterator start, const_iterator end)` &mdash; копия последовательности битов заданной двумя итераторами.

#### Операторы присваивания

- `operator=(const bitset& other)` &mdash; оператор копирующего присваивания;
- `operator=(std::string_view other)` &mdash; см. аналогичный конструктор;
- `operator=(const const_view& other)` &mdash; см. аналогичный конструктор.

#### Изменяющие операции

- `operator&=(const const_view& other)` &mdash; применить к каждому биту побитовое "и", где в качестве второго операнда служит соответствующий бит из `other`;
- `operator|=(const const_view& other)` &mdash; аналогично `operator&=`, но с побитовым "или";
- `operator^=(const const_view& other)` &mdash; аналогично `operator&=`, но с побитовым "xor";
- `operator<<=(std::size_t count)` &mdash; битовый сдвиг влево на `count`;
- `operator>>=(std::size_t count)` &mdash; битовый сдвиг вправо на `count`;
- `flip()` &mdash; инвертировать все биты;
- `set()` &mdash; установить все биты в `1`;
- `reset()` &mdash; установить все биты в `0`.

#### Побитовые операции

- `bitset operator&(const bitset& lhs, const bitset& rhs)` &mdash; побитовое "и";
- `bitset operator|(const bitset& lhs, const bitset& rhs)` &mdash; побитовое "или";
- `bitset operator^(const bitset& lhs, const bitset& rhs)` &mdash; побитовый "xor";
- `bitset operator~(const bitset& bs)` &mdash; побитовая инверсия;
- `bitset operator<<(const bitset& bs, std::size_t count)` &mdash; битовый сдвиг влево `bs` на `count`;
- `bitset operator>>(const bitset& bs, std::size_t count)` &mdash; битовый сдвиг вправо `bs` на `count`.

Здесь и в предыдущем пункте для побитовых операций над двумя `bitset`-ами поведение определено лишь при равенстве их размеров.

#### Операции для доступа к элементам

- `reference operator[](std::size_t index)` &mdash; возвращает прокси-объект на бит с индексом `index` (отсчитывая от старшего);
- `begin()`, `end()` &mdash; итераторы на первый бит и на бит после последнего соответственно;
- `bool all()` &mdash; правда ли, что все биты равны `1`;
- `bool any()` &mdash; правда ли, что хотя бы один бит равен `1`;
- `std::size_t count()` &mdash; количество битов, равных `1`;
- `operator==`, `operator!=` &mdash; сравнение на равенство.

#### Прочие методы

- `void swap(bitset& other)` &mdash; поменять местами состояния текущего `bitset` и `other`;
- `std::size_t size()` &mdash; текущее количество хранимых битов;
- `bool empty()` &mdash; пустой ли это `bitset`;
- `subview(std::size_t offset = 0, std::size_t count = npos)` &mdash; получить view:
  - пустой, если `offset > size()`;
  - на `[offset, offset + count)`, если `offset + count <= size()`;
  - на `[offset, size())`, если `offset + count > size()`.

#### Свободные функции

- `void swap(bitset& lhs, bitset& rhs)` &mdash; поменять местами состояния `lhs` и `rhs`;
- `std::string to_string(const bitset& bs)` &mdash; перевод в строку из `'0'` и `'1'`;
- `std::ostream& operator<<(std::ostream& out, const bitset& bs)` &mdash; вывод в поток вывода `out`, возвращает исходный поток.

## Методы `bitset::view` и `bitset::const_view`

#### Конструирование и операторы присваивания

- Копирующий конструктор;
- Конструктор от двух итераторов;
- Оператор копирующего присваивания;
- Неявное конструирование из ссылки на `bitset`.

#### Остальные методы

Все те же методы, что и у `bitset`, если они имеют смысл.

## Type members

Присутствуют как в `bitset`, так и в views:
- `value_type`
- `reference`
- `const_reference`
- `iterator`
- `const_iterator`
- `view`
- `const_view`
- `word_type` &mdash; тип, использующийся для разрядов (не менее 32 бит)
