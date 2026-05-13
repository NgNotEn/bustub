bustub
=====

简要说明与使用指南（中文）

- **项目**: 一个教学/轻量级的数据库引擎原型（C++17，CMake）。
- **目标**: 演示火山执行模型、表存储（per-table files）、共享 BufferPool、简单 SQL 执行器链（TableScan → Filter → Insert/Delete/Update/Select）。

快速开始

- 构建（在项目根目录下）:

  ```bash
  mkdir -p build && cd build
  cmake ..
  cmake --build . -j4
  ```

- 安装到指定前缀（可选）:

  ```bash
  cmake --install . --prefix /opt/bustub
  ```

- 生成发行包（tar.gz）:

  ```bash
  cpack -G TGZ
  # 产物示例: build/bustub-0.1.0-Linux.tar.gz
  ```

运行

- 直接运行二进制（build 输出目录）:

  ```bash
  ./build/bin/bustub <dbname>
  # 或安装后
  /opt/bustub/bin/bustub <dbname>
  ```

- 示例交互（每条 SQL 单独一行，以分号结尾）:

  ```sql
  CREATE TABLE t (id INT, name VARCHAR(32));
  INSERT INTO t VALUES (1, 'alice');
  SELECT * FROM t;
  UPDATE t SET name = 'bob' WHERE id = 1;
  DELETE FROM t WHERE id = 1;
  DROP TABLE t;
  ```

注意与当前限制（重要）

- 输入方式:
  - CLI 以**单行**读入并立即解析执行；多行 SQL（换行后再结束）目前不做自动合并。请把每条 SQL 放在一行中并以分号或回车结束。
  - `exec "SQL"` 支持把整个 SQL 用引号包起来；同时程序会把双引号字符串字面量自动规范为单引号以兼容解析器。

- SQL 特性限制（受限/简化实现）:
  - 仅支持基本类型: **INT** 与 **VARCHAR**（VARCHAR 有长度限制）。
  - `CREATE TABLE`, `DROP TABLE`, `DESC`, `tables`, `help`, `exit`：支持。
  - `INSERT`：只支持 `VALUES` 形式，且当前仅接受字面量常数（整型/字符串/浮点/NULL）。
  - `SELECT`：目前仅支持 `SELECT * FROM <table>`（不支持列投影表达式、聚合、JOIN）。
  - `DELETE`：支持无 WHERE（删除所有行）与基于简单 WHERE 的过滤（FilterExecutor）；复杂表达式受限。
  - `UPDATE`：已实现按列名映射的 SET 语义，但表达式只接受字面量常数（暂不支持复杂表达式和子查询）。

- 表与存储:
  - 每个表对应一个文件（data/table_<id>_<name>.tbl），目录 `data/<dbname>` 下。
  - Catalog 元数据保存在 `catalog.meta`，程序启动会尝试打开已知表的文件。

- 并发/事务/恢复:
  - 当前实现为教学原型：**不保证**事务隔离、并发控制与 crash-recovery。

已知问题与实现注记

- 解析器使用 third_party/sql-parser（hsql），对 SQL 语法的兼容性受限；为避免常见解析错误，请尽量使用标准、简洁的语句并按说明将字符串用单引号或双引号（双引号会被自动规范）包裹。
- 程序现在针对教学可观测性做了一些日志/异常处理，发布版已去掉异常打印噪音。

贡献与开发

- 代码结构（简要）: `src/` 下按照模块划分（`buffer/`, `catalog/`, `storage/`, `execution/`, `main/` 等）。
- 如果要扩展:
  - 优先改进表达式求值器以支持列运算/比较/AND/OR。
  - 完善 `SELECT` 投影、聚合与 JOIN。
  - 添加事务（锁/日志）与崩溃恢复支持。