## Questions

** mac 的几个 Clang compiler 有什么区别?**

```bash
$which cc # => /usr/bin/cc
$which gcc # => /usr/bin/gcc
$which clang # => /usr/bin/clang
```

**以 `const` 声明的指针为什么可以赋值？**

```c
int main() {
    char aa = '1';
    char bb = '2';
    const char *a = &aa;
    const char *b = &bb;
    const char *temp;

    temp = a;
    a = b;
    b = temp;

    printf("%c\n", *a); // => 2
    printf("%c\n", *b); // => 1
}
```

```c
int *a               // a 可变 *a 可变
const int *a         // a 可变 *a 不可变
int const *a         // a 可变 *a 不可变
int *(const a)       // a 不可变 *a 可变
const int *(const a) // a 不可变 *a 不可变
```

**C89 C99?**

1. K&R C
2. C89(ANSI C)
3. C90(IOS C)
4. C95,C96
5. C99
