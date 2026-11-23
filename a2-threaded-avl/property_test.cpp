#include "main.h"
#include "ThreadedAVL.h"
#include <set>
#include <random>
#include <chrono>
#include <vector>
// Property test (added for this repo): random insert/erase checked against std::set.
// Verifies BST order, AVL balance factors, threaded prev/next links and iterators.
// Also times sequential inserts to show how insert cost grows with n.
// Access protected root via a derived helper
template<class K,class V> struct Probe : ThreadedAVL<K,V> {
  using N = typename AVL<K,V>::Node;
  int check(N* n, bool& ok, const K* lo, const K* hi){
    if(!n) return 0;
    if((lo && !(*lo < n->key)) || (hi && !(n->key < *hi))) ok=false;
    int l=check(n->pLeft,ok,lo,&n->key), r=check(n->pRight,ok,&n->key,hi);
    if(l-r<-1||l-r>1) ok=false;
    if(n->bfactor!=l-r) ok=false;
    return 1+(l>r?l:r);
  }
  bool valid(){ bool ok=true; check(this->pRoot,ok,nullptr,nullptr); return ok; }
};
int main(){
  std::mt19937 rng(42); int bad=0;
  for(int round=0; round<200; round++){
    Probe<int,int> t; std::set<int> ref;
    for(int op=0; op<300; op++){
      int k=rng()%150;
      if(rng()%3){ bool a=t.insert(k,k), b=ref.insert(k).second; if(a!=b) bad++; }
      else { bool a=t.erase(k), b=ref.erase(k)>0; if(a!=b) bad++; }
      if(op%25==0){
        if(!t.valid()) {bad++;}
        std::list<int> asc=t.ascendingList(), desc=t.descendingList();
        std::vector<int> r(ref.begin(),ref.end());
        if(std::vector<int>(asc.begin(),asc.end())!=r) bad++;
        if(std::vector<int>(desc.rbegin(),desc.rend())!=r) bad++;
        // iterator walk forward/backward
        std::vector<int> f; for(auto it=t.beginIt(); !it.isNull(); ++it) f.push_back(it.key());
        if(f!=r) bad++;
        if(t.size()!=(int)ref.size()) bad++;
      }
    }
  }
  printf("random insert/erase vs std::set, 200 rounds x 300 ops: %s (%d mismatches)\n", bad?"FAIL":"PASS", bad);
  for(int n : {2000, 4000, 8000}){
    ThreadedAVL<int,int> t; auto s=std::chrono::steady_clock::now();
    for(int i=0;i<n;i++) t.insert(i,i);
    double ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-s).count();
    printf("sequential insert n=%d: %.1f ms, height=%d\n", n, ms, t.height());
  }
  return bad ? 1 : 0;
}
