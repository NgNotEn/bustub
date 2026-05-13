再看最底层公共定义
先过 config.h、rid.h、exception.h、page.h、channel.h。这一层决定了页、RID、异常、线程通信这些“地基”概念。

接着看类型系统
读 type_id.h、value.h、value.cpp、type.h、type.cpp、integer_type.cpp、varlen_type.cpp。这一层是后面解析 SQL、比较谓词、做 Tuple 序列化的基础。

然后看 catalog
读 column.h、schema.h、column.cpp、schema.cpp。你后面做建表、插入、查询、Join，都会依赖 schema 来解释列布局。

再看磁盘和缓冲池
按这个顺序读：disk_manager.h -> disk_manager.cpp -> disk_scheduler.h -> disk_scheduler.cpp -> lru_k_replacer.h -> lru_k_replacer.cpp -> buffer_pool_manager.h -> buffer_pool_manager.cpp。
这一层要重点理解三件事：页怎么从磁盘读进来、frame 怎么复用、脏页什么时候刷回去。

再看表页和表堆
读 tuple.h -> tuple.cpp -> table_page.h -> table_page.cpp -> table_heap.h -> table_heap.cpp。
这是你数据库最核心的数据路径：一条记录怎么序列化、怎么塞进页、怎么通过 RID 找回来、怎么遍历整张表。