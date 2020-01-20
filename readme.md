# 在 CMakeLists.txt 中添加编译动态库选项

在使用 CMake 构建项目时，一个常见的应用就是使用 CMake 编译一个库文件了。而编译成一个动态库或者静态库又是编译库文件时经常使用的一个选项。本文介绍了如何在 CMake 中添加一个选项来控制是否将库编译为动态库，且该选项可以和 CMake 一样跨平台使用。

本文并不从动态库和静态库的起源开始讲起，因此有些预备知识需要你提前了解。这些知识不需要你深刻的了解并实践过，只需看过相关文章有所了解即可。这些预备知识包括：

* C++ 基本编程知识。

* 什么是动态库。
* 动态库和静态库的区别。
* CMake 是干什么的，CMake 的基本知识。

上面的预备知识可以在网上很容易查阅得到，[Static vs Dynamic Libraries](https://www.bogotobogo.com/cplusplus/libraries.php) 这篇英文文章介绍的也比较详细，你可以参考一下。

## 主要难点

关于在 CMake 中添加动态库的选项，主要的难点就在于 windows 平台上的改动较大。在 linux 平台下，动态库和静态库的源代码是完全一样的，只需要修改编译参数即可。而 windows 平台下，如果想把静态库变成动态库的话，所有的源码都需要改变！因此如果你之前写了大量代码而没有考虑到 windows 平台上编译动态库的话，修改起来可能会是一个很大的工程。本文将以一个简单的实例来介绍如何编写一个可跨平台的带动静态编译参数的 CMake 项目。示例代码可以在 [这里](https://github.com/liu-yabin/shared_lib_test) 查看。

## Windows 平台下的动态库导入导出

前面我们提到过，在 Windows 平台中生成动态库其源码和静态库是不同的。在 Windows 平台中，我们导出动态库时，除了会生成 `.dll` 动态库之外还会生成一个 `.lib` 文件。这个 `.lib` 文件和静态库的 `.lib` 文件不同，它里面并不保存代码生成的二进制文件，而是所有需要导出符号的符号表。因此这个 `.lib` 文件和编译静态库生成的 `.lib` 文件相比会小很多。而这个导出的符号表是需要我们在源码中进行指定的。如果我们希望将将一个符号（symbol）导出（这里的符号可以指类、函数等各种类型），需要在其前面加上 `__declspec(dllexport)` 标志。这样这个符号的相关信息就会导出的 `.lib` 中的符号表中了。如果我们的源码中没有任何 `__declspec(dllexport)` 的话，我们依然可以成功的编译出动态库，但是并不会生成保存符号表的`.lib` 文件。这也是在 Windows 平台下编译动态库经常出现的问题，如果我们的源码是在 Linux 平台下编写的话，更是很容易忘记修改源码。以下是一个导出 `MyClass` 的例子：

```c++
class __declspec(dllexport) MyClass {
public:
    static void MyPrint();
};
```

除了导出符号标识符 `__declspec(dllexport)` 以外，我们作为用户使用动态库的时候，对应头文件中的符号还需要有 `__declspec(dllimport)` 标识符来表示这个符号是从动态库导入的。对应上面的 `MyClass` 这个例子，我们包含的头文件应该有以下内容：

```c++
class __declspec(dllimport) MyClass {
public:
    static void MyPrint();
};
```

一般对于一个库文件我们并不想对导入和导出分别写两个几乎同样的头文件，因此往往使用宏来替代直接使用 `__declspec(dllexport)` 和 `__declspec(dllimport)` 关键字。即：

```c++
#pragma once

#ifdef MY_LIB_EXPORTS
#define MY_LIB_API __declspec(dllexport)
#else
#define MY_LIB_API __declspec(dllimport)
#endif

class MY_LIB_API MyClass {
public:
    static void MyPrint();
};
```

这样我们只需要在编译（导出）这个库的时候，给编译器添加 `MY_LIB_EXPORTS` 宏。而在使用该库的时候什么都不定义即可。

## 编写 export 头文件

了解了 windows 平台下头文件的导出规则之后，现在我们来编写一个名为 *my_lib_export.h* 的头文件来专门的控制 `MY_LIB_API` 宏的定义。注意到上面的讨论只是在 Windows 平台以及编写动态库时才会发生，对应 Linux 平台以及编写静态库时，我们按照教科书上的定义按部就班的写我们的 C++ 代码即可。为了使 `MY_LIB_API` 的定义同时考虑到 Linux 平台以及静态库的的情况。这里丰富 `MY_LIB_API` 的定义如下（*my_lib_export.h* 中的内容）：

```c++
#pragma once

#ifdef MY_LIB_SHARED_BUILD
#ifdef _WIN32
#ifdef MY_LIB_EXPORTS
#define MY_LIB_API __declspec(dllexport)
#else
#define MY_LIB_API __declspec(dllimport)
#endif  // MY_LIB_EXPORTS
#else
#define MY_LIB_API
#endif  // _WIN32
#else
#define MY_LIB_API
#endif  // MY_LIB_SHARED_BUILD
```

这里除了使用 `MY_LIB_EXPORTS` 宏来判断是否为导出动态库以外，还使用到了编译器自带的 `_WIN32` 宏来判断是否是在 windows 平台上以及使用了需要我们自己定义的另外一个宏 `MY_LIB_SHARED_BUILD` 来判断是否正在编译动态库。除了上一节讨论的情况以外，我们均将 `MY_LIB_API` 的值设置为了空，即什么都没有定义。此时和普通的类定义完全相同。有了这个头文件之后，我们只需要在导出符号表的头文件中包含该头文件，就可以使用 `MY_LIB_API` 宏了。

关于 `MY_LIB_SHARED_BUILD` 和 `MY_LIB_EXPORTS` 宏的定义，我将在下面 *CMakeLists.txt* 的编写一节进行介绍。最后在多介绍一点，事实上 *my_lib_export.h* 这个头文件是可以通过 CMake 提供的 [GenerateExportHeader](https://cmake.org/cmake/help/latest/module/GenerateExportHeader.html) 命令自动生成的。考虑到自动生成的额外配置以及生成后的文件多出的无关内容不易于理解，这里不对该命令的使用进行介绍了，如果有兴趣的话，可以自行了解。以下内容摘抄自一个自动生成的 export 头文件，可以看出其表达的内容和我们上面自行定义的基本相同：

```c++
#ifndef LOGGING_EXPORT_H
#define LOGGING_EXPORT_H

#ifdef LOGGING_STATIC_DEFINE
#  define LOGGING_EXPORT
#  define LOGGING_NO_EXPORT
#else
#  ifndef LOGGING_EXPORT
#    ifdef logging_EXPORTS
        /* We are building this library */
#      define LOGGING_EXPORT __declspec(dllexport)
#    else
        /* We are using this library */
#      define LOGGING_EXPORT __declspec(dllimport)
#    endif
#  endif

#  ifndef LOGGING_NO_EXPORT
#    define LOGGING_NO_EXPORT
#  endif
#endif

#ifndef LOGGING_DEPRECATED
#  define LOGGING_DEPRECATED __declspec(deprecated)
#endif

#ifndef LOGGING_DEPRECATED_EXPORT
#  define LOGGING_DEPRECATED_EXPORT LOGGING_EXPORT LOGGING_DEPRECATED
#endif

#ifndef LOGGING_DEPRECATED_NO_EXPORT
#  define LOGGING_DEPRECATED_NO_EXPORT LOGGING_NO_EXPORT LOGGING_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef LOGGING_NO_DEPRECATED
#    define LOGGING_NO_DEPRECATED
#  endif
#endif

#endif /* LOGGING_EXPORT_H */
```

## 编写 CMakeLists 文件

上面的内容介绍了如何修改源码：

* 编写 *my_lib_export.h* 头文件在其中定义 `MY_LIB_API` 宏。
* 在需要导出的符号前，加上 `MY_LIB_API` 宏。

接下来将介绍如何编写 *CMakeLists.txt* 文件，其实只需要在 *CMakeLists.txt* 文件中做两件事：

* 添加是否编译动态库选项。
* 定义相关的宏帮助源码“理解”应该如何定义 `MY_LIB_API` 宏。

首先顶层 *CMakeLists.txt* 文件中需要添加如下语句来添加编译选项：

```cmake
option(BUILD_SHARED_LIBS "Specifies the type of libraries (SHARED or STATIC) to build" OFF)
```

其次在编译库的 *CMakeLists.txt* 文件中需要根据指定的编译选项，来定义不同的编译形式以及宏定义：

```cmake
if (BUILD_SHARED_LIBS)
    add_library(my_lib SHARED ${SrcFiles})
    target_compile_definitions(my_lib PUBLIC -DMY_LIB_SHARED_BUILD)
    target_compile_definitions(my_lib PRIVATE -DMY_LIB_EXPORTS)
else()
    add_library(my_lib STATIC ${SrcFiles})
endif()
```

这里在编译动态库的时候会添加两个宏定义 `MY_LIB_SHARED_BUILD` 以及 `MY_LIB_EXPORTS`。注意到这里 `MY_LIB_EXPORTS` 宏定义的访问符为 `PRIVATE` 即这个宏定义只是在编译时有效。而 `MY_LIB_SHARED_BUILD` 宏定义的访问符为 `PUBLIC` ，即无论是编译还是安装后作为库文件引用时均有效。这样我们就可以保证源码中 `MY_LIB_API` 宏被正确定义了。

完整的 *CMakeLists.txt* 文件和源码可以在 [github 仓库](https://github.com/liu-yabin/shared_lib_test) 中查看。

参考资料：

[GenerateExportHeader](https://cmake.org/cmake/help/latest/module/GenerateExportHeader.html)  
[Writing a Cross-Platform Dynamic Library](https://atomheartother.github.io/c++/2018/07/12/CPPDynLib.html)
