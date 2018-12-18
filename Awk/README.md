
Table of Contents
=================

* [Truth Values and Conditions](truth-values-and-conditions)
* [Array](#array)
* [Built-in Functions](#built-in-functions)
* [ User-Defined Functions](#user-defined-functions)

Truth Values and Conditions
===========================

```
```

See also [awk manual 6.3](https://www.gnu.org/software/gawk/manual/gawk.html#Truth-Values-and-Conditions)

[Back to TOC](#table-of-contents)


Array
=====

```awk
a[1] = 1
a["foo"] = "bar"

if ("foo" in a) {
    print a["foo"]  # bar
}

if (a["foo"]) {
    print a["foo"]  # bar
}

for (k in a) {
    print k " = " a[k]
}
# 1 = 1
# foo = bar

delete a["foo"]

for (k in a) {
    print k " = " a[k]
}
# 1 = 1
```

* `delete array[indx]`: delete an individual element
* `delete array`: delete all of the elements in an array

See also [awk manual 8](https://www.gnu.org/software/gawk/manual/gawk.html#Arrays)

[Back to TOC](#table-of-contents)

Built-in Functions
==================

* [sqrt](#sqrt)
* [int](#int)
* [match](#match)

See also [awk manual 9.1](https://www.gnu.org/software/gawk/manual/gawk.html#Built_002din)

[Back to TOC](#table-of-contents)

sqrt
----

```awk
print sqrt(4)  # 2
print sqrt(5)  # 2.23607
```

[Back to TOC](#built-in-functions)

int
---

```awk
print int(3.14)  # 3
print int(-3.14)  # -3
```

[Back to TOC](#built-in-functions)

match
-----

```awk
if (match("hello, 1234", "([0-9])([0-9]+)", mu)) {
    print mu[0]  # 1234
    print mu[1]  # 1
    print mu[2]  # 234
}
```

Regexp:

| Class | Meaning                                      |
| ---   | ---                                          |
| ^     | matches the beginning of a string            |
| $     | matches only at the end of a string          |
| .     | matches any single character                 |
| \s    | matches any whitespace character             |
| \S    | matches any character that is not whitespace |
| [0-9] | matches any numeric character                |


[Back to TOC](#built-in-functions)

User-Defined Functions
======================

```awk
function sum(a, b) {
    return a + b
}

BEGIN {
    print sum(1, 2)  # 3
}
```

See also [awk manual 9.2](https://www.gnu.org/software/gawk/manual/gawk.html#User_002ddefined).

[Back to TOC](#table-of-contents)
