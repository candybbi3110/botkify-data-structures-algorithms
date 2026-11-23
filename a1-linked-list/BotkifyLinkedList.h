#ifndef BOTKIFY_LINKED_LIST_H
#define BOTKIFY_LINKED_LIST_H

#include "main.h"

template <class T>
class BotkifyLinkedList
{
public:
    class Node
    {
    public:
        T data;
        Node* next;
        Node* extra;

        Node() : next(nullptr), extra(nullptr) {}
        Node(const T& data, Node* next = nullptr, Node* extra = nullptr) : data(data), next(next), extra(extra) {}
    };
private:
    Node* head;
    Node* tail;
    int count;

    void check_index_add(int index) const{
      if(index < 0 || index > count) throw out_of_range("Index is invalid!");
    }

    void check_index_removeAt(int index) const{
      if(index < 0 || index >= count) throw out_of_range("Index is invalid!");
    }

    Node* getN(int index) const{
      Node*current = head;
      for(int i = 0; i < index; i++) current = current -> next;
      return current;
    }

  void copy(const BotkifyLinkedList& other) {
      head = tail = nullptr;
      count = 0;
      Node* cur = other.head;
      while (cur != nullptr) {
        add(cur->data);
        cur = cur->next;
      }
    }


public:
    BotkifyLinkedList() : head(nullptr), tail(nullptr), count(0) {}
    ~BotkifyLinkedList() {clear();}

  Node* getHead() const { return head; }
  Node* getTail() const { return tail; }

    void add(T e){
      Node* newN = new Node(e,nullptr, tail);
      if(count == 0) head = tail = newN;
      else{
        tail -> next = newN;
        tail = newN;
      }
      count++;
    }

    void add(int index, T e) {
      check_index_add(index);
      if(index == count){
        add(e);
        return;
      }
      if (index == 0) {
        Node* newHead = new Node(e, head, nullptr);
        if (head) head->extra = newHead;
        head = newHead;
        if (count == 0) tail = newHead;
        count++;
        return;
      }

      Node* p = getN(index - 1);
      Node* nxt = p->next;
      Node* newMid = new Node(e, nxt, p);
      p->next = newMid;
      if (nxt) nxt->extra = newMid;
      else tail = newMid;
      count++;
    }

    T removeAt(int index){
      check_index_removeAt(index);
      Node* rev = nullptr;
      if(index == 0){
        rev = head;
        head = head -> next;
        if (head != nullptr) head -> extra = nullptr;
        if(count == 1) tail = nullptr;
      }
      else{
        Node* p = getN(index - 1);
        rev = p -> next;
        Node* nxt = rev->next;
        p->next = nxt;
        if(nxt) nxt->extra = p;
        else tail = p;
      }

      T ret = rev->data;
      delete rev;
      count--;
      return ret;
    }

  bool removeItem(T item){
      if(count == 0) return false;

      Node* current = head;
      while(current){
        if(current->data == item){
          Node* prev = current->extra;
          Node* nxt  = current->next;

          if(prev) prev->next = nxt;
          else head = nxt;

          if(nxt) nxt->extra = prev;
          else tail = prev;

          delete current;
          count--;
          return true;
        }
        current = current->next;
      }
      return false;
    }

    bool empty() const {return count == 0;}

    int size() const {return count;}

    void clear(){
      Node* current = head;
      while (current != nullptr){
        Node* next = current -> next;
        delete current;
        current = next;
      }
      head = tail = nullptr;
      count = 0;
    }

    T& get(int index) const{
      check_index_removeAt(index);
      return getN(index)->data;
    }

    string toString() const{
      ostringstream oss;
      Node* current = head;
      while(current != nullptr){
        oss << current -> data;
        if(current -> next != nullptr) oss << ",";
        current = current -> next;
      }
      return oss.str();
    }

  BotkifyLinkedList(const BotkifyLinkedList& other) {
      copy(other);
    }

  BotkifyLinkedList& operator = (const BotkifyLinkedList& other) {
      if (this == &other) return *this;
      clear();
      copy(other);
      return *this;
    }
};

#endif // BOTKIFY_LINKED_LIST_H
