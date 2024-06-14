# Язык программирования 

## Установка и запуск

```
git clone https://github.com/d3clane/ProgrammingLanguage.git
cd Src
make buildDirs && make
```

Запустить можно с помощью скрипта run в [examples](examples/).

## Описание

Основная цель - написать свой собственный язык программирования, который переводится в ассемблер для моей [эмулятора процессора](https://github.com/d3clane/Processor-Emulator), а затем исполняется на нем. 

Проект состоит из четырех частей:
1. [FrontEnd](#FrontEnd)
2. [MiddleEnd](#MiddleEnd)
3. [BackEnd](#BackEnd)
4. [BackFrontEnd](#BackFrontEnd)

Фактически, это четыре независимых программы:

1. Frontend переводит написанный на моем языке код в [AST](#AST) - abstract syntax tree.
2. Middle-end упрощает полученное дерево.
3. Backend переводит AST в ассемблер для моего эмулятора процессора.
4. Back-frontend - забавный бонус, который умеет переводить из AST в исходный код на моем языке.

Почему frontend, middle-end, backend - это три программы, а не одна монолитная? 

Представим ситуацию, что мне нужно реализовать frontend для $N$ различных языков программирования, а затем перевести их в $M$ различных ассемблеров. Если принять стандарт AST и переводить всегда в один и тот же вид, то получится, что мне надо реализовать всего $N$ frontend-ов и $M$ backend-ов, тем самым получив $N \cdot M$ различных вариантов компиляции. 

Если же писать монолитные программы, то потребуется $N '\cdot M$ таких программ. Очевидно, что первое предпочтительнее, даже несмотря на то, что появятся лишние издержки на запись и чтение дерева из файла(временного хранилища между программами).

## AST 

AST(abstract syntax tree) - это представление какого-то исходного кода в виде подвешенного дерева. Каждая из вершин, у которой есть дети, описывает какую-то операцию(например, while или add). Листья же дерева описывают операнды(числа, переменные). 

Порядок действий, в котором надо выполнять какие-то действия определяется структурой дерева. Например, даже если в исходном коде у арифметических выражений были скобочки, которые определяли порядок выполнения действий, в AST их уже нет, а порядок определяется расположением операций друг относительно друга. Рассмотрим на примере:

Выражение int value = $(2 + 3) \cdot (4 + 5)$ выглядит так:

![add_mul](https://github.com/d3clane/ProgrammingLanguage/blob/main/ReadmeAssets/imgs/add_mul.png)

А выражение int value = $2 + 3 \cdot (4 + 5)$ вот так:

![mul_add](https://github.com/d3clane/ProgrammingLanguage/blob/main/ReadmeAssets/imgs/mul_add.png)

При спуске по дереву сначала полностью рассчитывается выражение в левом поддереве, потом в правом, а затем применяется операция в вершине к полученным результатам. Фактически, на первом примере сначала посчитается сумма $2 + 3$, затем $4 + 5$, а потом их произведение. Во втором же случае посчитается $4 + 5$, затем $3 \cdot (4 + 5)$, а затем $2 + 3 \cdot (4 + 5)$. 

При написании данного проекта у меня и у [metaironia](https://github.com/metaironia) был принят единый стандарт синтаксического дерева([кое-кто](https://github.com/worthlane) должен был присоединиться, но пока его представление дерева отличается). Таким образом, совершенно разные языки могут иметь один и тот же вид AST, а значит для них подходит один и тот же backend и middle-end. 

## Frontend

Frontend переводит написанный на моем языке код в AST. Здесь важно понять, как анализировать исходный код. Фактически, прежде чем начать писать язык, надо придумать синтаксис для него. Для этого зададим грамматику, по которой затем будем делать разбор языка с помощью алгоритма рекурсивного спуска.

Frontend также разделен на две части - лексический анализ и синтаксический анализ. Лексический анализ нужен, чтобы разбить исходный код на токены - единицы грамматики, которые затем легче анализировать. На этом этапе можно избавиться от комментариев, пробелов, отступов и тому подобных вещей в коде - то, что не влияет на результат исполнения.

Синтаксический анализ - часть, в которой непосредственно массив из лексем(токенов) превращается в AST. В моем случае это происходит с помощью рекурсивного спуска.

### Синтаксис

Грамматика:

```
Grammar          ::= FUNC+ '\0'
FUNC             ::= FUNC_DEF
FUNC_DEF         ::= TYPE VAR FUNC_VARS_DEF '57' OP '{'
FUNC_VAR_DEF     ::= {TYPE VAR}*
OP               ::= { IF | WHILE | '57' OP+ '{' | {VAR_DEF | PRINT | ASSIGN | RET} '57' }
IF               ::= '57?' OR '57' OP
WHILE            ::= '57!' OR '57' OP
RET              ::= OR
VAR_DEF          ::= TYPE VAR '==' OR
PRINT            ::= '{' { ARG | CONST_STRING }
READ             ::= '{'
ASSIGN           ::= VAR '==' OR
OR               ::= AND {and AND}*
AND              ::= CMP {or CMP}*
CMP              ::= ADD_SUB {[<, <=, >, >=, =, !=] ADD_SUB}*
ADD_SUB          ::= MUL_DIV {[+, -] MUL_DIV}*
MUL_DIV          ::= POW {[*, /] POW}*
POW              ::= FUNC_CALL {['^'] FUNC_CALL}*
FUNC_CALL        ::= IN_BUILT_FUNCS | MADE_FUNC_CALL | EXPR
IN_BUILT_FUNCS   ::= [sin/cos/tan/cot/sqrt] '(' EXPR ')' | READ
MADE_FUNC_CALL   ::= VAR '{' FUNC_VARS_CALL '57' 
FUNC_VARS_CALL   ::= {OR}*
EXPR             ::= '(' OR ')' | ARG
ARG              ::= NUM | GET_VAR
NUM              ::= ['0'-'9']+
VAR              ::= ['a'-'z' 'A'-'Z' '_']+ ['a'-'z' 'A'-'Z' '_' '0'-'9']*
CONST_STRING     ::= '"' [ANY_ASCII_CHAR]+ '"'
TYPE             ::= 575757
```

Краткое справка по обозначениям:

- `[...]` - означает множество различный элементов в каком-то диапазоне. 
- `{ A | B | C | ...}` - означает, что на данном этапе должна появляться одна из грамматических конструкций в этом множестве. 
- `A+` - A повторяется один или более раз
- `A*` - A повторяется ноль или более раз
- `'A'` - значит, что должна встретиться непосредственно буква A.

Перечислю основные моменты, которые отличают мой язык программирования от C. Чтобы было понятнее, буду приводить примеры того, как бы это выглядело в C.

- В языке нет такого понятия, как запятая. Когда хочется перечислить аргументы или параметры функции, то достаточно просто поставить пробельный символ между ними. Вместо `func(val1, val2);` будет `func(val1 val2);`
- Какие-то скобки опущены. Например, при перечислении параметров функции в ее определении нет скобок, окружающих параметры. На примере C вместо `int func(int val1, int val2)` будет `int func int val1 int val2`. Также опущены открывающие скобки в конструкциях if / while.
- Если какая-то операция в языке возвращает результат, но при этом он никуда не присваивается, то считается, что это операция возврата из функции. Других способов вернуться из вызова функции нет. На примере C и функции рекурсивного вычисления факториала:

```
int Func(int n)
{
    if (n == 0)
        return 1;
    return Func(n - 1) * n;
}
```

Будет выглядеть так:

```
int Func(int n)
{
    if (n == 0)
        1;
    Func(n - 1) * n;
}
```

Конечно, из-за невнимательности может возникнуть проблема:

```
int Foo();
int Bar();

int Func()
{
    Foo();
    Bar();

    0;
}
```

Программист ожидает, что вызовутся функции `Foo()`, `Bar()` и вернется 0, а на деле вернется результат функции `Foo()`, а дальнейший код не будет исполняться.

Это легко избежать, просто присваивая полученные значения:

```
int Foo();
int Bar();

int Func()
{
    int tmp = Foo();
    tmp     = Bar();

    0;
}
```

- Все операции намеренно перепутаны: `+` это `-`, `*` это `/`, `<` это `>`, `=` это `==`.

Чтобы написать комментарий к коду, надо начать с символа `@`. 

Также, в языке присутствуют строковые литералы, которые предназначены только для использования вместе с оператором языка `print`. 

Примеры исходного кода на моем языке можно найти в папке [examples](examples/).

Перейдем к описанию обработки файлов исходного кода на моем языке.

### Лексический анализ 

Здесь ничего сложного. Просто проходясь по исходному коду, я превращаю последовательности символы в различные токены. Например, `575757` превратится в токен `TYPE_INT`, а `58` в число 58. В языке существуют неоднозначные конструкции(например, `57`), которые в разных контекстах могут означать разное. Например, иногда это может быть аналог `;`, а иногда аналог `}` в конструкциях if / while. В этом случае такие конструкции превращаются в токены вида `TOKEN_57`. 

На этом этапе также удаляются все комментарии, лишние пробелы и отступы, которые не влияют на результат исполнения.

### Рекурсивный спуск

После построения грамматики, в рекурсивном спуске нет никакой сложности. Все что надо - переписать действия, которые написаны в грамматике, в виде функций. Проходя по токенам, создавать вершины, если грамматическое правило верно, и подвязывать их, тем самым постепенно строя AST. Если ни одно из грамматических правил не подходит - выдать синтаксическую ошибку.

Пример AST, которое строится по исходному коду [программы для нахождения факториала числа](examples/factorial.txt):

![factorial](https://github.com/d3clane/ProgrammingLanguage/blob/main/ReadmeAssets/imgs/factorial.png)

Операции отмечены зеленым цветом, имена(переменных / функций / строковые литералы) голубым, числа темно-синим. 

## Middle-end

Middle-end на данный момент поддерживает сильно ограниченное количество оптимизаций, а конкретно всего две:

1. Свертка констант. Арифметические выражения, в которых не участвуют переменные, сворачиваются в одну константу. То есть, например, $(5 \cdot 6) + \frac{3}{1}$ свернется в одну вершину со значением $33$.
2. Удаление нейтральных вершин. Например, умножение любого выражения на ноль сворачивается в константу ноль. Или, умножение на единицу сворачивается просто в это же выражение, а единица и операция умножения удаляются.

Подобные оптимизации происходят до тех пор, пока они все еще могут происходить. Если после очередного цикла оптимизаций дерево не изменилось, middle-end завершает свою работу.

## Back-frontend

Это программа, которая по AST умеет строить исходный код. Зачем вообще это может быть нужно? Так как у меня и у [metaironia](https://github.com/metaironia) единый стандарт представления AST, можно переводить код из одного языка в другой - сначала из языка A в AST, а затем из AST в язык B. 

На примере, исходный код на языке metaironia:

```
ща на main блоке,
мы делаем банкноты
сюда x теперь читаю йоу 
сюда мой opp будет звонить factorial x гоу
флоу плохой сказал дурак opp йоу
ладно верни тогда 0;
йоу

factorial с гэнгом n эй
если n будто 1 эй
отдавай 1 без сомнений
йоу гоу
сюда x равно n минус 1 йо
верни пуллап в factorial
x множит n миллениал йоу йоу
```

Переводится сначала в AST, а затем моим back-frontend в исходный код на моем языке программирования:

```
575757 factorial 575757 n 
57
    57? (n != 1 ) 57
    57
        1 57
    {
    575757 x == ((n + 1 ) + (0 / 2 ) ) 57
    (factorial { x 57 / n ) 57
{

575757 main 
57
    575757 x == { 57
    575757 opp == factorial { x 57 57
    . opp 57
    0 57
{
```

К сожалению, обратного процесса не показать, у metaironia нет back-frontend.

## BackEnd

На данный момент происходит перевод в ассемблер для моей эмуляции процессора. Спускаясь по дереву для каждой вершины рекурсивно строим ее и детей. На выходе получается ассемблерный файл, который с помощью ассемблера моего эмулятора стекового процессора переводится в бинарный код для него, а затем исполняется на нем. 
