# libcco 开发者快速入门指南 (Quick Start)

欢迎使用 `libcco`！这是一个基于纯 C11 编写的高性能、防崩溃、动态 AST 配置解析引擎。CCO（Config & Command Object）语法摒弃了传统 JSON/HOCON 中的 `{}` 和 `[]`，**万物皆用 `()` 进行包裹**，并通过键值对拓扑智能推导类型。

本文档将带你快速掌握 `libcco` 的核心集成与开发姿势。

---

## 1. 快速集成与编译

`libcco` 没有任何第三方依赖。你可以直接将 `src/` 和 `include/` 加入你的构建系统，或使用自带的 Makefile 构建静态库：

```bash
# 生成 build/debug/liblibcco.a
make build.debug

# 或生成带有 -O3 优化的 release 版
make build.release
```

在你的项目中，只需引入头文件并链接该静态库：
```c
#include <cnt/cco.h>
// 编译：cc your_code.c -I/path/to/libcco/include -L/path/to/libcco/build/debug -llibcco
```

---

## 2. 核心 C API 使用范例

`libcco` 的 C API 极其简洁且高度内存安全。所有的对象树操作由 `cco_object_t` 句柄承载。

### 2.1 基础解析与取值
CCO 支持**顶级隐式字典 (Shorthand Map)**，即最外层无需包围 `()`。

```c
#include <cnt/cco.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    // 纯正 CCO 语法，仅使用 ()
    const char* cco_text = 
        "server: (\n"
        "    host: \"127.0.0.1\",\n"
        "    port: 8080,\n"
        "    debug: true\n"
        ")\n";

    // 1. 解析字符串 (opts 传 NULL 将使用默认的安全沙盒配置)
    cco_object_t* config = cco_parse_string(cco_text, strlen(cco_text), NULL);
    if (!config) {
        printf("解析失败！错误码：%d\n", cco_get_last_error());
        return 1;
    }

    // 2. 验证类型并提取字段
    if (cco_object_get_type(config) == CCO_TYPE_MAP) {
        // 从字典中获取 'server' 节点
        cco_object_t* server_obj = cco_map_get_by_key(config, "server");
        
        if (server_obj && cco_object_get_type(server_obj) == CCO_TYPE_MAP) {
            // 获取 host (String)
            cco_object_t* host_obj = cco_map_get_by_key(server_obj, "host");
            const char* host_str; size_t len;
            if (cco_object_get_string(host_obj, &host_str, &len)) {
                printf("Host: %s\n", host_str);
            }

            // 获取 port (Integer)
            cco_object_t* port_obj = cco_map_get_by_key(server_obj, "port");
            int64_t port;
            if (cco_object_get_integer(port_obj, &port)) {
                printf("Port: %lld\n", (long long)port);
            }
        }
    }

    // 3. 必须释放根节点 (连同子树会一并回收)
    cco_object_release(config);
    return 0;
}
```

---

## 3. 防爆沙盒与高级选项

针对不可信的外部输入文件，`libcco` 提供了强悍的安全防护沙盒。通过传入 `cco_parse_options_t` 结构体，你可以限制解析器的深度和内存申请。

```c
cco_parse_options_t opts;
opts.max_depth = 10;                     // 限制最大嵌套层级（防爆栈）
opts.max_document_size = 1024 * 1024;    // 限制最大文件大小 1MB
opts.max_total_allocation = 5 * 1024 * 1024; // 限制最大内存占用 5MB
opts.lenient_brackets = true;            // 开启宽容模式 (自动纠错)

cco_object_t* config = cco_parse_string(text, len, &opts);
```

### 🎯 宽容纠错模式 (Lenient Brackets)
开启 `lenient_brackets = true`（默认即为开启）时，如果用户习惯性地敲下了 JSON 风格的 `{}` 或 `[]`，引擎**不会崩溃或拒收**，而是会智能将其降级纠正为合法的 `()` 拓扑，仅在诊断日志中记录警告，极大提升应用对新手用户的包容度。

---

## 4. 数组与字典的遍历 (Getters)

对于动态大小的集合，使用遍历接口：

**遍历 Array：**
```c
size_t count = cco_array_get_count(arr_obj);
for (size_t i = 0; i < count; i++) {
    cco_object_t* item = cco_array_get_item(arr_obj, i);
    // 处理 item...
}
```

**遍历 Map：**
```c
size_t count = cco_map_get_count(map_obj);
for (size_t i = 0; i < count; i++) {
    const char* key = cco_map_get_key(map_obj, i);
    cco_object_t* val = cco_map_get_value(map_obj, i);
    // 处理 key 和 val...
}
```

---

## 5. 现代 C++ 封装 (C++ RAII Wrapper)

如果你使用 C++ 开发，可以直接引入 `<cnt/cco.hpp>`，彻底免除 `cco_object_release` 的心智负担。

```cpp
#include <cnt/cco.hpp>
#include <iostream>
#include <string>

int main() {
    std::string config = R"(
        app: (
            version: 1.0,
            modules: ( "auth", "db" )
        )
    )";

    try {
        // cnt::cco::Object 支持 RAII，出了作用域自动清理内存
        cnt::cco::Object obj = cnt::cco::parse(config);
        
        std::cout << "Parse Type ID: " << obj.type() << "\n";
        
        // 序列化回字符串
        std::cout << "Serialized:\n" << obj.serialize(true) << "\n";
    } 
    catch (const cnt::cco::ParseError& e) {
        std::cerr << "语法错误 (Code: " << e.code() << "): " << e.what() << "\n";
    }

    return 0;
}
```

---

## 6. 深入探索
更多组合范例，请进入项目的 `examples/` 目录：
- `make examples name=tree_print`：体验优美的终端 AST 树打印。
- `make examples name=auto_correction`：体验引擎如何通过宽容模式拯救非标准的 `{}` 书写。
