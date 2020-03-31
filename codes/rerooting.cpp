/*
https://github.com/key-moon/Library/blob/master/src/Algorithm/rerooting.csx
keymoon による C# の実装を noshi91 が C++ に移植したものです
*/

#include <functional>
#include <stack>
#include <vector>

template <class T> class ReRooting {
public:
    int NodeCount;

private:
    std::vector<std::vector<int>> Adjacents;
    std::vector<std::vector<int>> IndexForAdjacent;

    std::vector<T> Res;
    std::vector<std::vector<T>> DP;

    T Identity;
    std::function<T(T, T)> Operate;
    std::function<T(T, int)> OperateNode;

public:
    ReRooting(int nodeCount, std::vector<std::vector<int>> edges, T identity,
                std::function<T(T, T)> operate,
                std::function<T(T, int)> operateNode) {
        NodeCount = nodeCount;

        Identity = identity;
        Operate = operate;
        OperateNode = operateNode;

        std::vector<std::vector<int>> adjacents(nodeCount);
        std::vector<std::vector<int>> indexForAdjacents(nodeCount);

        for (int i = 0; i < edges.size(); i++) {
            auto &edge = edges[i];
            indexForAdjacents[edge[0]].push_back(adjacents[edge[1]].size());
            indexForAdjacents[edge[1]].push_back(adjacents[edge[0]].size());
            adjacents[edge[0]].push_back(edge[1]);
            adjacents[edge[1]].push_back(edge[0]);
        }

        Adjacents = std::vector<std::vector<int>>(nodeCount);
        IndexForAdjacent = std::vector<std::vector<int>>(nodeCount);
        for (int i = 0; i < nodeCount; i++) {
            Adjacents[i] = adjacents[i];
            IndexForAdjacent[i] = indexForAdjacents[i];
        }

        DP = std::vector<std::vector<T>>(Adjacents.size());
        Res = std::vector<T>(Adjacents.size());

        for (int i = 0; i < Adjacents.size(); i++)
            DP[i] = std::vector<T>(Adjacents[i].size());
        if (NodeCount > 1)
            Initialize();
        else if (NodeCount == 1)
            Res[0] = OperateNode(Identity, 0);
    }

    T Query(int node) { return Res[node]; }

private:
    void Initialize() {
        std::vector<int> parents(NodeCount);
        std::vector<int> order(NodeCount);

#pragma region InitOrderedTree
        int index = 0;
        std::stack<int> stack;
        stack.push(0);
        parents[0] = -1;
        while (stack.size() > 0) {
            auto node = stack.top();
            stack.pop();
            order[index++] = node;
            for (int i = 0; i < Adjacents[node].size(); i++) {
                auto adjacent = Adjacents[node][i];
                if (adjacent == parents[node])
                    continue;
                stack.push(adjacent);
                parents[adjacent] = node;
            }
        }
#pragma endregion

#pragma region fromLeaf
        for (int i = order.size() - 1; i >= 1; i--) {
            auto node = order[i];
            auto parent = parents[node];

            T accum = Identity;
            int parentIndex = -1;
            for (int j = 0; j < Adjacents[node].size(); j++) {
                if (Adjacents[node][j] == parent) {
                    parentIndex = j;
                    continue;
                }
                accum = Operate(accum, DP[node][j]);
            }
            DP[parent][IndexForAdjacent[node][parentIndex]] =
                OperateNode(accum, node);
        }
#pragma endregion

#pragma region toLeaf
        for (int i = 0; i < order.size(); i++) {
            auto node = order[i];
            T accum = Identity;
            std::vector<T> accumsFromTail(Adjacents[node].size());
            accumsFromTail[accumsFromTail.size() - 1] = Identity;
            for (int j = accumsFromTail.size() - 1; j >= 1; j--)
                    accumsFromTail[j - 1] = Operate(DP[node][j], accumsFromTail[j]);
            for (int j = 0; j < accumsFromTail.size(); j++) {
                DP[Adjacents[node][j]][IndexForAdjacent[node][j]] =
                    OperateNode(Operate(accum, accumsFromTail[j]), node);
                accum = Operate(accum, DP[node][j]);
            }
            Res[node] = OperateNode(accum, node);
        }
#pragma endregion
    }
};
