# libcco-design Work Plan

## TL;DR
- **我会得到什么**：一份可执行的纯 C (C11) 参考实现计划，用于 CCO 配置对象解析与序列化库 (libcco)。包含骨架、词法、语法、对象模型、符号表、实例化、序列化、API 包装等 19 个独立执行阶段。
- **为什么这样做**：作为 CNT 生态的配置语言，CCO 需要一个轻量、极速、零第三方依赖的 C 语言解析引擎。需要严格遵循 CPS 规范以保证工程质量。
- **不会做什么**：不实现网络 IO 获取配置、不引入多线程数据读写、不实现内核级沙箱、不引入第三方生成式 Parser (如 Bison/Flex)。
- **工作量**：19 个连续阶段，每个阶段约需 1-2 个工作日，预计总耗时 4 周左右。
- **风险**：严格的错误码冒泡与内存单点释放可能会使代码略显繁琐；宏控制字段导致 ABI 一致性需调用方与库强绑定。
- **我替你做的决定**：采用双轨内存模型（解析期 Arena，运行期非原子 Refcount），不实现并发安全的引用计数；限制最大诊断收集容量为 64，首错锁定。

## Scope
- **纳入范围**：
  - CCO 文本解析（构建 AST 及内存对象树）与错误诊断。
  - CCO 树的序列化（紧凑与美化格式）。
  - DOM 风格 C 语言 API（对象构造、查询、修改）。
  - $typedef, $enum, $temp, 模板继承、实例化及自定义构造器支持。
  - 静态方法与表达式求值（含 $format, $env 内建）。
  - C++ RAII 薄包装层。
- **排除范围**：
  - XCO 或其他通用配置格式的解析。
  - 网络加载配置或远程模板。
  - 多线程安全的节点访问。
  - 与 CCO 核心解析无关的业务层验证。
- **冲突裁定**：
  - 如发现规范与设计冲突，裁定优先级：用户明确裁定 > CPS 强制条款 (L1) > CPS 强烈推荐 (L2) > CCO 语言定义 > 设计方案。

## Verification strategy
- **单元测试**：Unity 框架 + CTest 集成。必须有 happy path 与 failure 路径测试（失败断言具体 `CCO_ERR_*`）。函数命名 `test_模块_场景`。覆盖率 >= 90% (核心), >= 80% (整体)。
- **Sanitizer 矩阵**：CI 中强制开启 AddressSanitizer 和 UndefinedBehaviorSanitizer，主分支额外开启 MemorySanitizer。
- **静态分析**：使用 clang-tidy 和 cppcheck，要求零告警。
- **集成测试**：提供一套完整的跨文件 CCO 样例解析与重新序列化，对比输出。
- **性能基准**：通过 CTest 引入基准测试程序，度量 Arena 分配效率与大文件解析速率。

## Execution strategy
- **Wave 划分**：共分为 19 个阶段 (Wave)。前一阶段的产物必须验证全绿才能进入下一阶段。
- **串行依赖**：绝大部分处于串行路径（底层 -> 词法 -> 语法 -> 语义 -> 上层）。
- **并行机会**：集成测试用例编写可与核心实现并行；C++ 包装可与测试加固并行。
- **共享推理不拆判断**：词法与语法的错误处理路径 (Arena 释放、单点退出) 存在强共享，交给 `deep` 或 `ultrabrain` 单一委派不作强行切分。

## Todos

- [ ] 1. 骨架与合规基线
  - **执行步骤**：
    1.1 初始化目录树 (`include/cnt/`, `src/internal/`, `test/`)。输入：无。输出：目录结构。限制：必须包含空文件占位。验证：`ls include/cnt`。失败模式：目录创建失败。
    1.2 配置 `.editorconfig` 与 `.clang-format`。输入：根目录。输出：两个配置文件。限制：不得带命令行参数覆盖。验证：`clang-format --dry-run`。失败模式：格式化报错。
    1.3 搭建 CMake 骨架。输入：C11 环境。输出：`CMakeLists.txt`。限制：必须配置 -Wall -Wextra -Werror 等。验证：`cmake -B build`。失败模式：CMake 配置失败。
  - **References**：CPS 第 5.6 节, 第 8 章。
  - **Acceptance**：构建系统能空跑，静态分析门禁开启。
  - **QA happy**：`cmake -B build && cmake --build build` 输出构建成功。
  - **QA failure**：写入违规语法文件测试，`make lint` 触发非零返回码，包含 clang-format 报错。
  - **Commit**：`chore: initialize project skeleton and compliance baselines`
  - **Recommended task executor category**：`quick` (样板代码搭建)

- [ ] 2. 基础设施
  - **执行步骤**：
    2.1 实现 Arena 分配器 (`void* cco_arena_alloc(cco_arena_t* restrict a, size_t n)`)。输入：无。输出：`arena.h/c`。限制：单次与累计溢出守卫，销毁不单独释放，返回 NULL 抛错。验证：分配超大内存返回 NULL。失败模式：OOM。
    2.2 实现字符串缓冲 (`cco_strbuf_t`)。输入：Arena。输出：`strbuf.h/c`。限制：用 snprintf 及 vsnprintf 两段追加，守卫扩容不超过 `size_t/2`。验证：追加长字符串测试。失败模式：扩容溢出。
    2.3 实现哈希表与字符串驻留。输入：Arena, strbuf。输出：`hash.h/c`。限制：FNV-1a 64位，0.75 扩容。字符串驻留基于指针比较，拥有字符串副本。验证：同一字符串插入返回相同指针。失败模式：哈希冲突未正确探测。
    2.4 实现诊断存储。输入：无。输出：`diag.h/c`。限制：使用 `_Thread_local`，最多 64 条，首错保存在 `last_error`。验证：存入 65 条，确认丢弃最后一条。失败模式：多线程脱敏泄漏。
    2.5 定义运行时选项 (`cco_parse_options_t`)。输入：无。输出：`options.h`。限制：`CCO_ENABLE_*` 宏包裹字段，默认清零构造。验证：宏关闭时访问字段引发编译错。失败模式：ABI 不一致。
  - **References**：CPS 第 5.6 节错误处理与内存纪律。
  - **Acceptance**：基础组件通过单元测试且 Valgrind/ASan 无泄漏。
  - **QA happy**：`./build/test_arena` 正常退出 0。
  - **QA failure**：分配超过 Arena 最大值，返回 `CCO_ERR_OUT_OF_RANGE`。
  - **Commit**：`feat: implement foundational infrastructure (arena, hash, diag)`
  - **Recommended task executor category**：`deep` (涉及核心内存所有权和线程局部变量)

- [ ] 3. 词法分析器
  - **执行步骤**：
    3.1 定义 Token 结构 (`cco_token_type_t`) 与游标。输入：基础设施。输出：`lexer.h`。限制：行列号必须附带。验证：编译通过。失败模式：无。
    3.2 实现词法扫描主循环。输入：文本指针。输出：`lexer.c`。限制：支持跳过空白，根据 `CCO_ENABLE_COMMENT_PRESERVE` 侧通道保留或丢弃注释。验证：遇到非 ASCII 且非转义返回 `CCO_ERR_INVALID_ARG`。失败模式：死循环。
    3.3 解析数字、字符串及标识符。输入：扫描器状态。输出：补全 `lexer.c`。限制：守卫数字溢出，`\u` 按 UTF-8 编码，包含 NUL 处理。验证：解析 `1_000` 得到 1000。失败模式：越界读取。
  - **References**：cco-full-learn.md (Lexical rules)。
  - **Acceptance**：能把 CCO 文本转换为正确的 Token 流。
  - **QA happy**：`test_lexer_happy` 输出全绿。
  - **QA failure**：输入非法转义符 `\z`，触发 `CCO_ERR_INVALID_ARG`。
  - **Commit**：`feat: implement lexical analyzer`
  - **Recommended task executor category**：`unspecified-high` (已知模式的状态机实现)

- [ ] 4. 对象模型与引用计数
  - **执行步骤**：
    4.1 定义 8 种类型 (`cco_type_t`) 与 `struct cco_object_s`。输入：无。输出：`object.h`。限制：对象大小断言，字符串双保险(NUL+长度)，非原子 refcount。验证：编译期 `_Static_assert` 对象体积。失败模式：平台对齐导致体积暴涨。
    4.2 实现构造、深拷贝、深比较。输入：对象模型。输出：`object.c`。限制：映射需包装之前的哈希表。验证：深度比较两个构造的映射。失败模式：循环引用死锁 (CCO 设计上树结构不可成环)。
  - **References**：CPS 第 5.6 节内存处理。
  - **Acceptance**：API 可创建和销毁各种节点，无泄漏。
  - **QA happy**：`test_object_lifecycle` 正常。
  - **QA failure**：分配对象触及沙箱总分配量上限，返回 `CCO_ERR_OUT_OF_RANGE`。
  - **Commit**：`feat: implement object model and reference counting`
  - **Recommended task executor category**：`unspecified-high`

- [ ] 5. 语法分析器
  - **执行步骤**：
    5.1 定义解析上下文。输入：Lexer, Object, Options。输出：`parser.h`。限制：需包含深度计数、分配预算、单点退出机制。验证：初始化正确。失败模式：参数不合规。
    5.2 实现一 token 前瞻与值分派。输入：上下文。输出：`parser.c`。限制：三条歧义消解规则（映射与数组判断），控制嵌套深度不合并预算。验证：解析简写顶层。失败模式：递归爆栈。
    5.3 错误恢复与限制。输入：上下文。输出：`parser.c`补全。限制：单诊断及 `CCO_ENABLE_ERROR_RECOVERY` 多诊断同步点处理，无限循环守卫。验证：连续两个非法 token 触发强制退出。失败模式：错误诊断丢失。
  - **References**：cco-full-learn.md (Syntax rules, limits)。
  - **Acceptance**：能把正确文本转为树，畸形文本准确报错。
  - **QA happy**：`test_parser_valid` 成功返回对象根。
  - **QA failure**：输入嵌套深度超过配置值，断言返回 `CCO_ERR_OUT_OF_RANGE`。
  - **Commit**：`feat: implement LL(1) syntax parser`
  - **Recommended task executor category**：`deep` (递归下降与复杂错误恢复)

- [ ] 6. 符号表与模板定义
  - **执行步骤**：
    6.1 实现符号表结构（三种表：别名、枚举、模板）。输入：Object。输出：`symtab.h/c`。限制：需环检测（如 A->B, B->A 别名）。验证：注入环形别名抛错。失败模式：内存泄漏。
    6.2 解析 `$typedef` 与 `$enum`。输入：Parser。输出：`sym_parser.c`。限制：必须在顶层。验证：非顶层声明报错。失败模式：作用域渗透。
    6.3 解析 `$temp` 模板与继承。输入：Parser。输出：继续 `sym_parser.c`。限制：继承展平需带防环检查，点分名冲突报错。验证：深继承无环通过，有环报 `CCO_ERR_INVALID_ARG`。失败模式：解析死锁。
  - **References**：cco-full-learn.md (Declarations)。
  - **Acceptance**：能构建出符号引用，解决继承依赖。
  - **QA happy**：解析包含枚举和别名的 CCO。
  - **QA failure**：循环别名定义触发 `CCO_ERR_INVALID_ARG`。
  - **Commit**：`feat: implement symbol tables and template definitions`
  - **Recommended task executor category**：`unspecified-high`

- [ ] 7. 模板实例化与构造器
  - **执行步骤**：
    7.1 解析位置与命名实例化 (`#Temp(x)`)。输入：Parser, Symtab。输出：`inst.c`。限制：默认值补齐，计数器守卫。验证：漏字段时启用默认值。失败模式：参数缺失且无默认值未报错。
    7.2 实现 `$function.@` 构造器调用。输入：Parser。输出：`inst.c`。限制：受 `CCO_ENABLE_CONSTRUCTORS` 控制。禁用时返回 `CCO_ERR_FORBIDDEN`。验证：宏关闭时抛错。失败模式：沙箱绕过。
  - **References**：cco-full-learn.md (Instantiation)。
  - **Acceptance**：实例化的产物是标准对象映射。
  - **QA happy**：成功实例化带有继承的子模板。
  - **QA failure**：在宏关闭下使用 `$function.@` 断言 `CCO_ERR_FORBIDDEN`。
  - **Commit**：`feat: template instantiation and custom constructors`
  - **Recommended task executor category**：`deep`

- [ ] 8. 序列化
  - **执行步骤**：
    8.1 实现紧凑与美化格式化。输入：Object。输出：`serializer.c`。限制：处理内存溢出，转义规则需匹配 Lexer。验证：序列化后能反序列化一致。失败模式：深树序列化 OOM。
    8.2 模板实例与枚举值输出。输入：Object 标记。输出：`serializer.c`补全。限制：必须正确转义。验证：包含内部引号的字符串。失败模式：格式损坏。
  - **References**：cco-full-learn.md (Serialization)。
  - **Acceptance**：对象可存回合法的 CCO 文本。
  - **QA happy**：`cco_serialize_file` 输出符合预期的字符串。
  - **QA failure**：给定非法对象类型调用，断言 `CCO_ERR_TYPE_MISMATCH`。
  - **Commit**：`feat: implement serialization to compact and pretty formats`
  - **Recommended task executor category**：`unspecified-high`

- [ ] 9. 表达式与格式内建
  - **执行步骤**：
    9.1 AST 节点构建与求值器开关。输入：Lexer, Parser。输出：`eval.c`。限制：构建无条件，求值由 `CCO_ENABLE_EVAL` 控制。关闭直接返回禁止。验证：开关测试。失败模式：泄漏。
    9.2 优先级处理与逻辑求值。输入：Eval。输出：`eval.c`补全。限制：短路求值。验证：运算优先级与短路。失败模式：右值类型检查崩溃。
    9.3 实现 `$format` 和 `$env`。输入：Eval。输出：`builtins.c`。限制：双重递归控制。`$env` 宏控制。未找到返回 None。验证：解析内部含插值的字符串。失败模式：循环 format。
  - **References**：cco-full-learn.md (Expressions, Builtins)。
  - **Acceptance**：表达式正确计算。
  - **QA happy**：`1 + 2 * 3` 求值为 7。
  - **QA failure**：在宏关闭下调用 `$env`，断言 `CCO_ERR_FORBIDDEN`。
  - **Commit**：`feat: implement expression evaluation and builtins`
  - **Recommended task executor category**：`deep`

- [ ] 10. 静态调用
  - **执行步骤**：
    10.1 点调用与冒号实例化。输入：Symtab, Eval。输出：`call.c`。限制：由 `CCO_ENABLE_STATIC_CALLS` 控制，校验签名。验证：执行数学计算辅助函数。失败模式：无视签名强算。
  - **References**：cco-full-learn.md (Static methods)。
  - **Acceptance**：可挂载并调用类方法。
  - **QA happy**：`#Math:square(5)` 得到 25。
  - **QA failure**：参数类型错误断言 `CCO_ERR_TYPE_MISMATCH`。
  - **Commit**：`feat: implement static calls`
  - **Recommended task executor category**：`unspecified-high`

- [ ] 11. 沙箱与选项生效点
  - **执行步骤**：
    11.1 实现文件与路径沙箱。输入：Options。输出：`sandbox.c`。限制：使用 `realpath` 并在 `base_dir` 下校验前缀。验证：`../../etc/passwd` 读取抛错。失败模式：路径穿越。
    11.2 生效选项与脱敏。输入：Options, Diag。输出：散布。限制：环境值或密文在诊断中被脱敏。验证：触发诊断检查无敏感内容。失败模式：信息泄露。
  - **References**：CPS 第 9 章安全。
  - **Acceptance**：安全限制严格生效。
  - **QA happy**：读取沙箱内文件成功。
  - **QA failure**：尝试读沙箱外文件，断言 `CCO_ERR_FORBIDDEN`。
  - **Commit**：`feat: implement sandbox and options enforcement`
  - **Recommended task executor category**：`deep` (安全边界审查)

- [ ] 12. 公共 API 头文件整合
  - **执行步骤**：
    12.1 整合 `include/cnt/cco.h`。输入：全内部接口。输出：`cco.h`。限制：外部声明守卫 `extern "C"`，所有权注释详尽，单一入口包含。验证：纯 C 和 C++ 各自 Include 一次。失败模式：符号未导出。
  - **References**：CPS 第 5.6 节公共 API 规范。
  - **Acceptance**：下游引用一键生效。
  - **QA happy**：编写小样例程序通过链接。
  - **QA failure**：包含头文件未使用 `CCO_ENABLE_*` 一致宏配置导致链接报错。
  - **Commit**：`feat: consolidate public API headers`
  - **Recommended task executor category**：`quick`

- [ ] 13. C++ 包装
  - **执行步骤**：
    13.1 编写 `cco.hpp` RAII 包装层。输入：C API。输出：`include/cnt/cco.hpp`。限制：薄包装，异常映射(例外豁免文档记录)。验证：通过智能指针管理节点生存期。失败模式：C/C++ 内存接力泄漏。
  - **References**：CPS 第 5.5 节 C++ 包装豁免。
  - **Acceptance**：提供现代 C++ 的易用接口。
  - **QA happy**：`test_cpp_wrapper` C++ 用例运行无泄漏。
  - **QA failure**：捕获 CCO 解析错误并作为 `std::runtime_error` 抛出验证。
  - **Commit**：`feat: add C++ RAII wrapper with exception mapping`
  - **Recommended task executor category**：`unspecified-low`

- [ ] 14 - 18. 测试集成、文档与加固
  - **执行步骤**：
    14-18.1 分阶段编写全量集成测试、跑 Benchmark (CTest)、补全 Doxygen 注释、加固 CI 工作流。限制：警告必须视为失败。覆盖率门禁生效。
  - **QA happy**：`make test` 100% 通过。
  - **QA failure**：修改代码导致 Valgrind 报警，CI 拦截。
  - **Commit**：`chore: finalize testing, benchmarks, and docs`
  - **Recommended task executor category**：`unspecified-low`

- [ ] 19. 发布打包
  - **执行步骤**：
    19.1 配置 CMake 打包与导出，设定版本 `v0.1.0`。
  - **Commit**：`build: prepare v0.1.0 release`
  - **Recommended task executor category**：`quick`

## Final verification wave
- [ ] F1. 执行 `make clean && cmake -B build && cmake --build build && cd build && ctest`，确保测试全绿。
- [ ] F2. 检查 ASan / UBSan 日志，确保无任何泄漏或未定义行为告警。
- [ ] F3. 随机抽取破坏性输入，验证程序不产生段错误且能够由 `cco_get_last_error` 获取安全诊断。

## Commit strategy
- **分支命名**：`feat/lexer`, `fix/arena-oom`，短生命周期，存活 < 2 日。
- **消息类型**：遵循 Conventional Commits (feat, fix, refactor, test, docs, chore, build, ci)。
- **合并规则**：Squash Merge，合后删除，主干开发。
- **签名与发布**：强制带 `-s` 生成 DCO (Signed-off-by)。CI 自动在推 tag 时创建 GPG 签名 Release `v0.1.0`，人工绝不手打 tag。
- **Plan 回执**：如果 `libcco-design.md` 存在，最终提交需在 footer 追加 `Plan: .omo/plans/libcco-design.md`。

## Success criteria
- 所有的 CCO 语法特性可在 C 语言中完美解析与验证。
- ASan、UBSan 检测全程通过。
- 零第三方依赖 (除了测试用的 Unity)。
- API 头文件符合 CPS L1 约束 (单点退出、错误码冒泡、显式内存归属、无禁止函数)。
- 提供 `libcco-design.md` 作为全盘可执行蓝图，可由下游 Agent 无追问执行。
