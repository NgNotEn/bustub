#include <iostream>
#include <cassert>
#include "orset.h" // 假设你的类定义在 orset.h 或直接粘贴在上面

void test_basic_add_remove() {
    std::cout << "[Test 1] Basic Add/Remove... ";
    orset<int> s;
    
    // 1. 初始状态为空
    assert(s.contain(1) == false);

    // 2. 添加 (1, 100)
    s.add(1, 100);
    assert(s.contain(1) == true);

    // 3. 删除 1
    s.remove(1);
    assert(s.contain(1) == false);

    // 4. 重复删除不应该报错
    s.remove(1);
    assert(s.contain(1) == false);
    
    std::cout << "Passed!" << std::endl;
}

void test_add_wins() {
    std::cout << "[Test 2] Add-Wins Logic... ";
    orset<int> s;

    // 1. 添加 (1, 100)
    s.add(1, 100);
    assert(s.contain(1) == true);

    // 2. 删除 1 (此时 (1,100) 进入墓碑)
    s.remove(1);
    assert(s.contain(1) == false);

    // 3. 再次添加 1，但使用新 ID (1, 101)
    // 根据 OR-Set 定义，新的 ID 没有被观测到删除，所以应该存活
    s.add(1, 101);
    assert(s.contain(1) == true);

    // 4. 再次删除，应该把 (1, 101) 也扔进墓碑
    s.remove(1);
    assert(s.contain(1) == false);

    std::cout << "Passed!" << std::endl;
}

void test_merge_propagation() {
    std::cout << "[Test 3] Merge & Tombstone Propagation... ";
    orset<int> A, B;

    // A 添加数据
    A.add(1, 10);
    A.add(2, 20);

    // B 同步 A 的数据
    B.merge(A);
    assert(B.contain(1) == true);
    assert(B.contain(2) == true);

    // B 删除 1
    B.remove(1);
    assert(B.contain(1) == false);
    assert(B.contain(2) == true);

    // A 还不知道 1 被删除了
    assert(A.contain(1) == true);

    // A 合并 B (A 应该吸收 B 的墓碑，从而知道 1 被删了)
    A.merge(B);
    assert(A.contain(1) == false); // 关键测试点
    assert(A.contain(2) == true);

    std::cout << "Passed!" << std::endl;
}

void test_concurrent_conflict() {
    std::cout << "[Test 4] Concurrent Add/Remove (Conflict)... ";
    orset<int> NodeA, NodeB;

    // 初始状态：两者都包含 (Apple, 10)
    NodeA.add(99, 10);
    NodeB.merge(NodeA);

    // 模拟网络分区，两者进行并发操作

    // NodeA: 执行删除
    // NodeA 看到了 (99, 10)，所以把它删了
    NodeA.remove(99); 
    assert(NodeA.contain(99) == false);

    // NodeB: 执行添加 (相同的元素值，但 ID 不同)
    // NodeB 不知道 NodeA 删除了，它加了一个新的 (99, 20)
    NodeB.add(99, 20);
    assert(NodeB.contain(99) == true);

    // 此时 NodeA 有墓碑 {(99, 10)}
    // NodeB 有活跃 {(99, 10), (99, 20)} (假设B没删10，或者B只关注20)
    // 注意：如果NodeB没有remove(99)，它保留着(99,10)和(99,20)

    // 网络恢复，合并！
    NodeA.merge(NodeB);

    // 结果分析：
    // (99, 10) -> 在 A 的墓碑里，死。
    // (99, 20) -> 是 B 新加的，A 的墓碑里没有 20。
    // 所以 99 应该存活！(Add-Wins)
    assert(NodeA.contain(99) == true); 

    std::cout << "Passed!" << std::endl;
}

int main() {
    test_basic_add_remove();
    test_add_wins();
    test_merge_propagation();
    test_concurrent_conflict();

    std::cout << "\nAll tests passed successfully!" << std::endl;
    return 0;
}