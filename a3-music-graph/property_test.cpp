// Property test (added for this repo): random graphs checked against independent reference algorithms.
//  - Dijkstra cost      vs Bellman-Ford
//  - cycle detection    vs Kahn's topological sort
//  - cluster count      vs union-find on the undirected graph
//  - hub in-degree      vs direct edge count
#include "MusicGraph.h"
#include <random>
#include <numeric>
#include <functional>
#include <algorithm>

static string capture(const function<void()>& f) {
    ostringstream out; streambuf* old = cout.rdbuf(out.rdbuf()); f(); cout.rdbuf(old); return out.str();
}
static int countOf(const string& s, const string& pat) {
    int c = 0; for (size_t p = s.find(pat); p != string::npos; p = s.find(pat, p + 1)) c++; return c;
}

int main() {
    mt19937 rng(2026);
    int bad = 0, graphs = 300;
    for (int g = 0; g < graphs; g++) {
        int n = 2 + rng() % 9, m = rng() % (n * 2 + 1);
        MusicGraph mg;
        vector<string> id(n);
        for (int i = 0; i < n; i++) { id[i] = "V" + to_string(i); mg.addSong(id[i], "T" + to_string(i), "A", "G"); }
        struct E { int u, v; double w; };
        vector<E> edges;                       // directed edge list
        for (int k = 0; k < m; k++) {
            int u = rng() % n, v = rng() % n; if (u == v) continue;
            double w = 1 + rng() % 9; bool bi = rng() % 2;
            mg.addEdge(id[u], id[v], w, bi);
            edges.push_back({u, v, w}); if (bi) edges.push_back({v, u, w});
        }
        // 1) Dijkstra vs Bellman-Ford from V0 to V(n-1)
        const double INF = 1e18; vector<double> d(n, INF); d[0] = 0;
        for (int it = 0; it < n; it++) for (auto& e : edges) if (d[e.u] + e.w < d[e.v]) d[e.v] = d[e.u] + e.w;
        string out = capture([&]{ mg.findSmoothTransition(id[0], id[n - 1]); });
        if (d[n - 1] >= INF) { if (out.find("No transition path") == string::npos) bad++; }
        else {
            size_t p = out.find("Total Deviation (Cost): ");
            if (p == string::npos || fabs(stod(out.substr(p + 24)) - d[n - 1]) > 1e-9) bad++;
        }
        // 2) cycle detection vs Kahn
        vector<int> indeg(n, 0); vector<vector<int>> adj(n);
        for (auto& e : edges) { adj[e.u].push_back(e.v); indeg[e.v]++; }
        vector<int> q, deg = indeg; for (int i = 0; i < n; i++) if (!deg[i]) q.push_back(i);
        for (size_t h = 0; h < q.size(); h++) for (int v : adj[q[h]]) if (--deg[v] == 0) q.push_back(v);
        bool hasCycle = (int)q.size() < n;
        out = capture([&]{ mg.detectMusicLoop(); });
        if (hasCycle != (out.find("Music loop detected!") != string::npos)) bad++;
        // 3) cluster count vs union-find
        vector<int> par(n); iota(par.begin(), par.end(), 0);
        function<int(int)> f = [&](int x) { return par[x] == x ? x : par[x] = f(par[x]); };
        for (auto& e : edges) par[f(e.u)] = f(e.v);
        int comps = 0; for (int i = 0; i < n; i++) if (f(i) == i) comps++;
        out = capture([&]{ mg.generatePlaylistsByClusters(); });
        if (countOf(out, "=== Playlist ") != comps || countOf(out, "  * ") != n) bad++;
        // 4) hub in-degree
        int mx = *max_element(indeg.begin(), indeg.end());
        out = capture([&]{ mg.findMostPopularSong(); });
        if (out.find("(In-degree: " + to_string(mx) + ")") == string::npos) bad++;
    }
    printf("%d random graphs (Dijkstra, cycles, clusters, hub): %s (%d mismatches)\n", graphs, bad ? "FAIL" : "PASS", bad);
    return bad ? 1 : 0;
}
