#ifndef AVL_H
#define AVL_H

#include "IBST.h"
template <typename K, typename V>
class AVL : public IBST<K, V> {
public:
    struct Node {
        K key;
        V value;
        int bfactor;

        Node* pLeft;
        Node* pRight;

        Node(const K& k, const V& v)
            : key(k), value(v), bfactor(0),
              pLeft(nullptr), pRight(nullptr) {}

        virtual ~Node() = default;

        int balance() const { return bfactor; }

        static std::string defaultEntry2Str(const K&, const V&) {
            return "<entry>";
        }

        std::string toString(std::string (*entry2str)(const K&, const V&) = nullptr) const {
            std::string entry = entry2str ? entry2str(key, value) : defaultEntry2Str(key, value);
            std::string bf = std::to_string(balance());

            if (!pLeft && !pRight) {
                return "[" + entry + ":" + bf + "]";
            }
            else if (!pLeft && pRight) {
                return " (" + entry + ":" + bf + "[.]" + pRight->toString(entry2str) + ")";
            }
            else if (pLeft && !pRight) {
                return " (" + entry + ":" + bf + pLeft->toString(entry2str) + "[.]" + ")";
            }
            else {
                return " (" + entry + ":" + bf
                     + pLeft->toString(entry2str)
                     + pRight->toString(entry2str)
                     + ") ";
            }
        }
    };

protected:
    Node* pRoot;

    virtual void onInserted(Node* /*newNode*/, Node* /*pred*/, Node* /*succ*/) {}
    virtual void onErasing(Node* /*delNode*/, Node* /*pred*/, Node* /*succ*/) {}
    virtual void onReplaceBySuccessor(Node* /*target*/, Node* /*successor*/, Node* /*successorNext*/) {}

    virtual Node* createNode(const K& key, const V& value) {
        return new Node(key, value);
    }

    int nodeHeight(Node* node) const {
        if (node == nullptr) return 0;

        int leftH = nodeHeight(node->pLeft);
        int rightH = nodeHeight(node->pRight);

        return 1 + (leftH > rightH ? leftH : rightH);
    }

    int calculateBalance(Node* node) const {
        if (node == nullptr) return 0;
        return nodeHeight(node->pLeft) - nodeHeight(node->pRight);
    }

    void updateBalance(Node* node) {
        if (node != nullptr) {
            node->bfactor = calculateBalance(node);
        }
    }

    Node* rotateL(Node* x) {
        Node* y = x->pRight;
        Node* t = y->pLeft;

        y->pLeft = x;
        x->pRight = t;

        updateBalance(x);
        updateBalance(y);

        return y;
    }

    Node* rotateR(Node* y) {
        Node* x = y->pLeft;
        Node* t = x->pRight;

        x->pRight = y;
        y->pLeft = t;

        updateBalance(y);
        updateBalance(x);

        return x;
    }

    Node* reBalance(Node* node) {
        if (node == nullptr) return nullptr;

        updateBalance(node);

        // Left heavy
        if (node->bfactor > 1) {
            if (calculateBalance(node->pLeft) < 0) {
                node->pLeft = rotateL(node->pLeft);
            }
            return rotateR(node);
        }

        // Right heavy
        if (node->bfactor < -1) {
            if (calculateBalance(node->pRight) > 0) {
                node->pRight = rotateR(node->pRight);
            }
            return rotateL(node);
        }

        return node;
    }

    Node* minofNode(Node* node) const {
        if (node == nullptr) return nullptr;

        while (node->pLeft != nullptr) {
            node = node->pLeft;
        }
        return node;
    }

    Node* maxofNode(Node* node) const {
        if (node == nullptr) return nullptr;

        while (node->pRight != nullptr) {
            node = node->pRight;
        }
        return node;
    }

    Node* minNodeWithSucc(Node* node, Node*& succNext, Node* fallbackGreater) const {
        Node* parentGreater = fallbackGreater;
        Node* cur = node;

        while (cur != nullptr && cur->pLeft != nullptr) {
            parentGreater = cur;
            cur = cur->pLeft;
        }

        if (cur == nullptr) {
            succNext = fallbackGreater;
            return nullptr;
        }

        if (cur->pRight != nullptr) {
            succNext = minofNode(cur->pRight);
        }
        else {
            succNext = parentGreater;
        }

        return cur;
    }

    void clearRec(Node* node) {
        if (node == nullptr) return;

        clearRec(node->pLeft);
        clearRec(node->pRight);
        delete node;
    }

    int sizeRec(Node* node) const {
        if (node == nullptr) return 0;
        return 1 + sizeRec(node->pLeft) + sizeRec(node->pRight);
    }

    void inorderRec(Node* node, std::list<K>& result) const {
        if (node == nullptr) return;

        inorderRec(node->pLeft, result);
        result.push_back(node->key);
        inorderRec(node->pRight, result);
    }

    void reverseInorderRec(Node* node, std::list<K>& result) const {
        if (node == nullptr) return;

        reverseInorderRec(node->pRight, result);
        result.push_back(node->key);
        reverseInorderRec(node->pLeft, result);
    }

    Node* insertRec(Node* node,
                    const K& key,
                    const V& value,
                    bool& inserted,
                    Node*& newNode,
                    Node*& pred,
                    Node*& succ) {
        if (node == nullptr) {
            inserted = true;
            newNode = createNode(key, value);
            return newNode;
        }

        if (key < node->key) {
            succ = node;
            node->pLeft = insertRec(node->pLeft, key, value, inserted, newNode, pred, succ);
        }
        else if (node->key < key) {
            pred = node;
            node->pRight = insertRec(node->pRight, key, value, inserted, newNode, pred, succ);
        }
        else {
            return node;
        }

        return reBalance(node);
    }

    Node* eraseRec(Node* node,
                   const K& key,
                   bool& erased,
                   Node* pred,
                   Node* succ,
                   bool callEraseHook = true) {
        if (node == nullptr) return nullptr;

        if (key < node->key) {
            node->pLeft = eraseRec(node->pLeft, key, erased, pred, node, callEraseHook);
        }
        else if (node->key < key) {
            node->pRight = eraseRec(node->pRight, key, erased, node, succ, callEraseHook);
        }
        else {
            erased = true;

            if (node->pLeft != nullptr && node->pRight != nullptr) {
                Node* nextSucc = nullptr;
                Node* curSucc = minNodeWithSucc(node->pRight, nextSucc, succ);

                onReplaceBySuccessor(node, curSucc, nextSucc);

                node->key = curSucc->key;
                node->value = curSucc->value;

                bool deleted = false;
                node->pRight = eraseRec(node->pRight, curSucc->key, deleted, node, nextSucc, false);
            }
            else {
                Node* actualPred = (node->pLeft != nullptr) ? maxofNode(node->pLeft) : pred;
                Node* actualSucc = (node->pRight != nullptr) ? minofNode(node->pRight) : succ;

                if (callEraseHook) {
                    onErasing(node, actualPred, actualSucc);
                }

                Node* child = (node->pLeft != nullptr) ? node->pLeft : node->pRight;
                delete node;
                return child;
            }
        }

        return reBalance(node);
    }

public:
    AVL() : pRoot(nullptr) {}
    virtual ~AVL() { clear(); }

    bool insert(const K& key, const V& value) override {
        bool inserted = false;
        Node* newNode = nullptr;
        Node* pred = nullptr;
        Node* succ = nullptr;

        pRoot = insertRec(pRoot, key, value, inserted, newNode, pred, succ);

        if (inserted) {
            onInserted(newNode, pred, succ);
        }

        return inserted;
    }

    bool erase(const K& key) override {
        bool erased = false;
        pRoot = eraseRec(pRoot, key, erased, nullptr, nullptr, true);
        return erased;
    }

    V* find(const K& key) override {
        Node* cur = pRoot;

        while (cur != nullptr) {
            if (key < cur->key) {
                cur = cur->pLeft;
            }
            else if (cur->key < key) {
                cur = cur->pRight;
            }
            else {
                return &(cur->value);
            }
        }

        return nullptr;
    }

    bool contains(const K& key) const override {
        Node* cur = pRoot;

        while (cur != nullptr) {
            if (key < cur->key) {
                cur = cur->pLeft;
            }
            else if (cur->key < key) {
                cur = cur->pRight;
            }
            else {
                return true;
            }
        }

        return false;
    }

    int size() const override {
        return sizeRec(pRoot);
    }

    bool empty() const override {
        return pRoot == nullptr;
    }

    void clear() override {
        clearRec(pRoot);
        pRoot = nullptr;
    }

    int height() const override {
        return nodeHeight(pRoot);
    }

    std::list<K> ascendingList() const override {
        std::list<K> result;
        inorderRec(pRoot, result);
        return result;
    }

    std::list<K> descendingList() const override {
        std::list<K> result;
        reverseInorderRec(pRoot, result);
        return result;
    }

    std::string toString(std::string (*entry2str)(const K&, const V&) = nullptr) const override {
        if (!pRoot) return "(NULL)";
        return pRoot->toString(entry2str);
    }
};

#endif /* AVL_H */