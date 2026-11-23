
#include "Playlist.h"
#include <algorithm>
#include <chrono>
#include <climits>
#include <cstdlib>
#include <vector>
#include <random>

// ======================
// Mini test framework
// ======================
static int absInt(int x){ return x < 0 ? -x : x; }

static string capturePlayRandomOnce(Playlist& p, int index) {
    ostringstream out;
    streambuf* old = cout.rdbuf(out.rdbuf());
    p.playRandom(index);
    cout.rdbuf(old);
    return out.str();
}

static long long brutePlayApproximate2(const vector<int>& dur, int step) {
    int n = (int)dur.size();
    if (n <= 1) return 0;
    if (step < 0) step = 0;
    const long long INF = (1LL<<60);
    vector<long long> dp(n, INF);
    dp[0] = 0;
    int jump = step + 1;
    for (int i = 1; i < n; i++) {
        for (int j = max(0, i - jump); j <= i - 1; j++) {
            dp[i] = min(dp[i], dp[j] + llabs((long long)dur[i] - dur[j]));
        }
    }
    return dp[n-1];
}

// forum: closest-by-position, testcase guarantees unique => no tie needed
static string refPlayRandomNoTie(const vector<int>& dur, int start) {
    int n = (int)dur.size();
    int cur = start;
    bool first = true;
    ostringstream out;
    while (true) {
        if (!first) out << ",";
        out << "S" << (cur+1) << "-A";
        first = false;

        int next = -1;
        int bestDist = INT_MAX;
        for (int i = 0; i < n; i++) {
            if (dur[i] > dur[cur]) {
                int dist = absInt(i-cur);
                if (dist < bestDist) {
                    bestDist = dist;
                    next = i;
                }
            }
        }
        if (next == -1) break;
        cur = next;
    }
    return out.str();
}
static int PASS = 0, FAIL = 0;

static void report(bool ok, const string& name,
                   const string& expected = "",
                   const string& got = "") {
    if (ok) {
        cout << "[PASS] " << name << "\n";
        PASS++;
    } else {
        cout << "[FAIL] " << name << "\n";
        if (!expected.empty() || !got.empty()) {
            cout << "  expected: " << expected << "\n";
            cout << "  got     : " << got << "\n";
        }
        FAIL++;
    }
}

static Song* mkSong(int id, int duration, int score,
                    const string& title, const string& artist = "A") {
    return new Song(id, title, artist, "alb", duration, score, "url");
}

static Playlist mkPlaylist(const string& name, const vector<Song*>& songs) {
    Playlist p(name);
    for (auto* s : songs) p.addSong(s);
    return p;
}

static string capturePlayRandom(Playlist& p, int index) {
    ostringstream out;
    streambuf* old = cout.rdbuf(out.rdbuf());
    p.playRandom(index);
    cout.rdbuf(old);
    return out.str();
}

// ======================
// Brute/reference models (ground truth)
// ======================

static long long bruteTotalScore(const vector<int>& scores) {
    // Spec: sum over all contiguous groups: min(group)*sum(group)
    int n = (int)scores.size();
    long long ans = 0;
    for (int l = 0; l < n; l++) {
        int mn = INT_MAX;
        long long sm = 0;
        for (int r = l; r < n; r++) {
            mn = min(mn, scores[r]);
            sm += scores[r];
            ans += 1LL * mn * sm;
        }
    }
    return ans;
}

static double bruteCompareMetric(const vector<int>& scores, int numSong) {
    // Spec: for each window of length numSong, take max; then average
    int n = (int)scores.size();
    if (numSong <= 0 || numSong > n) return NAN;
    long long sumMax = 0;
    int cnt = 0;
    for (int i = 0; i + numSong - 1 < n; i++) {
        int mx = scores[i];
        for (int j = i; j < i + numSong; j++) mx = max(mx, scores[j]);
        sumMax += mx;
        cnt++;
    }
    return (double)sumMax / cnt;
}

static long long brutePlayApproximate(const vector<int>& dur, int step) {
    // Spec: path from 0 to n-1, can skip <= step songs => jump <= step+1
    int n = (int)dur.size();
    if (n <= 1) return 0;
    if (step < 0) step = 0;
    const long long INF = (1LL<<60);
    vector<long long> dp(n, INF);
    dp[0] = 0;
    int jump = step + 1;
    for (int i = 1; i < n; i++) {
        for (int j = max(0, i - jump); j <= i - 1; j++) {
            dp[i] = min(dp[i], dp[j] + llabs((long long)dur[i] - dur[j]));
        }
    }
    return dp[n-1];
}

static string refPlayRandom_byPosition_NoTie(const vector<int>& dur,
                                             const vector<string>& s,
                                             int index) {
    // Forum: next = closest-by-position song with duration > current.
    // Teacher: testcase ensures unique => no tie handling needed.
    int n = (int)dur.size();
    int cur = index;
    bool first = true;
    ostringstream out;

    while (true) {
        if (!first) out << ",";
        out << s[cur];
        first = false;

        int next = -1;
        int bestDist = INT_MAX;

        for (int i = 0; i < n; i++) {
            if (dur[i] > dur[cur]) {
                int dist = abs(i - cur);
                if (dist < bestDist) {
                    bestDist = dist;
                    next = i;
                }
            }
        }
        if (next == -1) break;
        cur = next;
    }
    return out.str();
}

// ======================
// Generators for "no-tie" playRandom cases
// ======================

static void buildNoTiePlayRandomCase(int n, int start,
                                     vector<int>& dur, vector<string>& ss) {
    // Goal: For each step, there is exactly 1 closest-by-position candidate > current.
    // Construction:
    // Put a "ramp" on the right of start strictly increasing, and all left side very large but far.
    // Also ensure only one nearest by position is > current at each step:
    // Make immediate right neighbor always the only closest greater by position.
    // Achieve by making immediate left neighbor duration < current (not candidate),
    // and any other candidate is farther.
    dur.assign(n, 0);
    ss.clear();
    ss.reserve(n);
    for (int i = 0; i < n; i++) ss.push_back("S" + to_string(i+1) + "-A");

    // base durations: left side low, right side increasing from a mid value
    // Ensure dur[start-1] < dur[start], and dur[start+1] > dur[start], etc.
    int base = 100;
    for (int i = 0; i < n; i++) dur[i] = 1; // default tiny

    // make a strictly increasing chain from start to right
    dur[start] = base;
    for (int i = start + 1; i < n; i++) dur[i] = dur[i-1] + 10;

    // left neighbors are smaller than dur[start] so they are not candidates
    for (int i = 0; i < start; i++) dur[i] = base - 50 - (start - i); // all < base

    // Add one far-left huge spike to create alternative candidates but farther by position
    // (won't beat immediate right neighbor distance=1)
    dur[0] = base + 1000; // candidate, but far if start>1
}

static vector<Song*> materializeSongs(const vector<int>& dur, const vector<int>& sc) {
    vector<Song*> songs;
    songs.reserve(dur.size());
    for (int i = 0; i < (int)dur.size(); i++) {
        int score = sc.empty() ? 1 : sc[i];
        songs.push_back(mkSong(1000 + i, dur[i], score, "S" + to_string(i+1)));
    }
    return songs;
}

static void cleanupSongs(vector<Song*>& songs) {
    for (auto* s : songs) delete s;
    songs.clear();
}

// ======================
// Tests
// ======================

static void test_getTotalScore_hard() {
    {
        Playlist p("E");
        int got = p.getTotalScore();
        report(got == 0, "getTotalScore: empty", "0", to_string(got));
    }

    // nasty patterns
    vector<vector<int>> cases = {
        {7},
        {4,1,3,5},
        {2,2,2,2},
        {5,4,3,2,1},
        {1,2,3,4,5},
        {100000,1,100000,1,100000},      // huge range
        {3,1,3,1,3,1,3},                 // repeating minima
        {9,8,7,1,7,8,9},                 // deep valley
        {1,100,1,100,1},                 // alternating
    };

    for (int t = 0; t < (int)cases.size(); t++) {
        auto sc = cases[t];
        vector<int> dur(sc.size(), 10);
        auto songs = materializeSongs(dur, sc);
        Playlist p = mkPlaylist("P", songs);

        long long exp = bruteTotalScore(sc);
        int got = p.getTotalScore();
        report(got == (int)exp,
               "getTotalScore: hard_case_" + to_string(t+1),
               to_string(exp), to_string(got));

        cleanupSongs(songs);
    }

    // fuzz random (small n so brute is fine)
    {
        mt19937 rng(12345);
        for (int it = 0; it < 80; it++) {
            int n = 1 + (rng() % 12);
            vector<int> sc(n);
            for (int i = 0; i < n; i++) sc[i] = (int)(rng() % 11); // 0..10 allowed? if spec score positive, still catches bugs
            vector<int> dur(n, 10);

            auto songs = materializeSongs(dur, sc);
            Playlist p = mkPlaylist("FZ", songs);

            long long exp = bruteTotalScore(sc);
            int got = p.getTotalScore();
            if (got != (int)exp) {
                report(false, "getTotalScore: FUZZ_FAIL it=" + to_string(it),
                       to_string(exp), to_string(got));
                cleanupSongs(songs);
                return;
            }
            cleanupSongs(songs);
        }
        report(true, "getTotalScore: FUZZ(80)");
    }
}

static void test_compareTo_hard() {
    // deterministic cases
    {
        vector<int> A = {4,1,3,5};
        vector<int> B = {1,2,3};
        auto sa = materializeSongs(vector<int>(A.size(), 10), A);
        auto sb = materializeSongs(vector<int>(B.size(), 10), B);
        Playlist pA = mkPlaylist("A", sa);
        Playlist pB = mkPlaylist("B", sb);

        bool got = pA.compareTo(pB, 2);
        bool exp = bruteCompareMetric(A,2) >= bruteCompareMetric(B,2);
        report(got == exp, "compareTo: basic numSong=2",
               exp ? "true" : "false", got ? "true" : "false");

        cleanupSongs(sa);
        cleanupSongs(sb);
    }

    // tricky equalities / near equal
    {
        vector<int> A = {5,1,5,1,5};
        vector<int> B = {4,4,4,4,4};
        auto sa = materializeSongs(vector<int>(A.size(), 10), A);
        auto sb = materializeSongs(vector<int>(B.size(), 10), B);
        Playlist pA = mkPlaylist("A", sa);
        Playlist pB = mkPlaylist("B", sb);

        bool got = pA.compareTo(pB, 3);
        bool exp = bruteCompareMetric(A,3) >= bruteCompareMetric(B,3);
        report(got == exp, "compareTo: alternating spikes numSong=3",
               exp ? "true" : "false", got ? "true" : "false");

        cleanupSongs(sa);
        cleanupSongs(sb);
    }

    // edge numSong values
    {
        vector<int> A = {1,9,1};
        vector<int> B = {5,5,5};
        auto sa = materializeSongs(vector<int>(A.size(), 10), A);
        auto sb = materializeSongs(vector<int>(B.size(), 10), B);
        Playlist pA = mkPlaylist("A", sa);
        Playlist pB = mkPlaylist("B", sb);

        bool got = pA.compareTo(pB, 1);
        bool exp = bruteCompareMetric(A,1) >= bruteCompareMetric(B,1);
        report(got == exp, "compareTo: numSong=1 (avg score)",
               exp ? "true" : "false", got ? "true" : "false");

        cleanupSongs(sa);
        cleanupSongs(sb);
    }

    // fuzz random
    {
        mt19937 rng(777);
        for (int it = 0; it < 120; it++) {
            int nA = 1 + (rng() % 10);
            int nB = 1 + (rng() % 10);
            vector<int> A(nA), B(nB);
            for (int i=0;i<nA;i++) A[i] = (int)(rng()%21);
            for (int i=0;i<nB;i++) B[i] = (int)(rng()%21);
            int k = 1 + (rng() % max(1, min(nA,nB))); // choose valid-ish

            auto sa = materializeSongs(vector<int>(nA, 10), A);
            auto sb = materializeSongs(vector<int>(nB, 10), B);
            Playlist pA = mkPlaylist("A", sa);
            Playlist pB = mkPlaylist("B", sb);

            bool got = pA.compareTo(pB, k);
            bool exp = bruteCompareMetric(A,k) >= bruteCompareMetric(B,k);
            if (got != exp) {
                report(false, "compareTo: FUZZ_FAIL it=" + to_string(it) + " k=" + to_string(k),
                       exp ? "true" : "false", got ? "true" : "false");
                cleanupSongs(sa); cleanupSongs(sb);
                return;
            }
            cleanupSongs(sa); cleanupSongs(sb);
        }
        report(true, "compareTo: FUZZ(120)");
    }
}

static void test_playApproximate_hard() {
    // hand-crafted nasty: local greedy fails
    {
        vector<int> dur = {10, 100, 11, 99, 12, 98, 13, 97, 14, 96};
        int step = 1;
        auto songs = materializeSongs(dur, vector<int>(dur.size(), 1));
        Playlist p = mkPlaylist("PA", songs);

        long long exp = brutePlayApproximate(dur, step);
        int got = p.playApproximate(step);
        report(got == (int)exp, "playApproximate: zigzag greedy-trap step=1",
               to_string(exp), to_string(got));
        cleanupSongs(songs);
    }

    // step=0 no skip
    {
        vector<int> dur = {50, 5, 80, 10, 70, 15};
        int step = 0;
        auto songs = materializeSongs(dur, vector<int>(dur.size(), 1));
        Playlist p = mkPlaylist("PA0", songs);

        long long exp = brutePlayApproximate(dur, step);
        int got = p.playApproximate(step);
        report(got == (int)exp, "playApproximate: step=0 (must sequential)",
               to_string(exp), to_string(got));
        cleanupSongs(songs);
    }

    // step huge
    {
        vector<int> dur = {1, 1000, 2, 999, 3, 998, 4, 997};
        int step = 100;
        auto songs = materializeSongs(dur, vector<int>(dur.size(), 1));
        Playlist p = mkPlaylist("PAH", songs);

        long long exp = brutePlayApproximate(dur, step);
        int got = p.playApproximate(step);
        report(got == (int)exp, "playApproximate: huge step",
               to_string(exp), to_string(got));
        cleanupSongs(songs);
    }

    // fuzz random
    {
        mt19937 rng(2026);
        for (int it = 0; it < 140; it++) {
            int n = 2 + (rng() % 14);
            vector<int> dur(n);
            for (int i = 0; i < n; i++) dur[i] = (int)(rng() % 300);
            int step = (int)(rng() % 6); // small step

            auto songs = materializeSongs(dur, vector<int>(n, 1));
            Playlist p = mkPlaylist("FZ", songs);

            long long exp = brutePlayApproximate(dur, step);
            int got = p.playApproximate(step);
            if (got != (int)exp) {
                report(false, "playApproximate: FUZZ_FAIL it=" + to_string(it) + " step=" + to_string(step),
                       to_string(exp), to_string(got));
                cleanupSongs(songs);
                return;
            }
            cleanupSongs(songs);
        }
        report(true, "playApproximate: FUZZ(140)");
    }
}

static void test_playRandom_hard() {
    // “no tie” generated cases (start away from 0 so far-left spike exists but not nearest)
    for (int t = 0; t < 10; t++) {
        int n = 10 + t;
        int start = 3 + (t % 4);
        vector<int> dur;
        vector<string> ss;
        buildNoTiePlayRandomCase(n, start, dur, ss);

        auto songs = materializeSongs(dur, vector<int>(n, 1));
        Playlist p = mkPlaylist("PR", songs);

        string got = capturePlayRandom(p, start);
        string exp = refPlayRandom_byPosition_NoTie(dur, ss, start);
        report(got == exp,
               "playRandom: GEN_no_tie n=" + to_string(n) + " start=" + to_string(start),
               exp, got);

        cleanupSongs(songs);
    }

    // “no output when no greater duration exists” (should print only start)
    {
        vector<int> dur = {100, 90, 80, 70, 60};
        vector<string> ss = {"S1-A","S2-A","S3-A","S4-A","S5-A"};
        auto songs = materializeSongs(dur, vector<int>(dur.size(), 1));
        Playlist p = mkPlaylist("PR2", songs);

        string got = capturePlayRandom(p, 0);
        string exp = refPlayRandom_byPosition_NoTie(dur, ss, 0);
        report(got == exp, "playRandom: strictly decreasing (only first)",
               exp, got);

        cleanupSongs(songs);
    }

    // fuzz: random but enforce no-tie by construction (monotone segment)
    {
        mt19937 rng(999);
        for (int it = 0; it < 60; it++) {
            int n = 8 + (rng() % 10);
            int start = 2 + (rng() % (n-3));
            vector<int> dur(n, 1);
            vector<string> ss;
            ss.reserve(n);
            for (int i=0;i<n;i++) ss.push_back("S"+to_string(i+1)+"-A");

            // build: left side all tiny, start mid, right strictly increasing => nearest is start+1 always
            int base = 100 + (rng()%50);
            for (int i=0;i<start;i++) dur[i] = base - 200 - (start-i);
            dur[start] = base;
            for (int i=start+1;i<n;i++) dur[i] = dur[i-1] + 10 + (rng()%3);

            auto songs = materializeSongs(dur, vector<int>(n,1));
            Playlist p = mkPlaylist("FZ", songs);

            string got = capturePlayRandom(p, start);
            string exp = refPlayRandom_byPosition_NoTie(dur, ss, start);
            if (got != exp) {
                report(false, "playRandom: FUZZ_FAIL it=" + to_string(it),
                       exp, got);
                cleanupSongs(songs);
                return;
            }
            cleanupSongs(songs);
        }
        report(true, "playRandom: FUZZ_no_tie(60)");
    }
}

static void test_playlist_stability_remove_next_prev() {
    vector<Song*> songs;
    for (int i=0;i<6;i++) songs.push_back(mkSong(100+i, 10+i, 1, "S"+to_string(i+1)));
    Playlist p = mkPlaylist("STB", songs);

    bool ok = true;
    try {
        p.playNext(); p.playNext(); p.playPrevious();
        p.removeSong(0);
        p.playNext();
        p.removeSong(2);
        p.playPrevious();
        p.removeSong(p.getSize()-1);
        p.playNext();
    } catch(...) { ok = false; }
    report(ok, "stability: removeSong + playNext/playPrevious (no crash)");

    cleanupSongs(songs);
}

static void test_getTotalScore_extreme() {
    // n=60, score lớn -> dễ overflow nếu dùng int ở giữa
    int n = 60;
    vector<int> sc(n);
    for (int i = 0; i < n; i++) sc[i] = (i % 2 == 0) ? 100000 : 99999;

    vector<Song*> songs;
    songs.reserve(n);
    for (int i = 0; i < n; i++) songs.push_back(mkSong(2000+i, 10+i, sc[i], "S"+to_string(i+1)));

    Playlist p = mkPlaylist("EXT", songs);

    long long exp = bruteTotalScore(sc);
    int got = p.getTotalScore();

    report(got == (int)exp, "getTotalScore: EXTREME overflow-check (n=60, big scores)",
           to_string(exp), to_string(got));

    cleanupSongs(songs);
}
static void test_playApproximate_greedy_traps() {
    {
        // trap 1: chọn bước gần nhất local bị sai
        vector<int> dur = {0, 100, 1, 99, 2, 98, 3, 97, 1000};
        int step = 2;
        auto songs = materializeSongs(dur, vector<int>(dur.size(), 1));
        Playlist p = mkPlaylist("TRAP1", songs);

        long long exp = brutePlayApproximate2(dur, step);
        int got = p.playApproximate(step);
        report(got == (int)exp, "playApproximate: GREEDY_TRAP_1 step=2",
               to_string(exp), to_string(got));
        cleanupSongs(songs);
    }

    {
        // trap 2: step lớn nhưng vẫn phải nối 0->n-1 hợp lý
        vector<int> dur = {5, 1000, 6, 999, 7, 998, 8, 997, 9};
        int step = 3;
        auto songs = materializeSongs(dur, vector<int>(dur.size(), 1));
        Playlist p = mkPlaylist("TRAP2", songs);

        long long exp = brutePlayApproximate2(dur, step);
        int got = p.playApproximate(step);
        report(got == (int)exp, "playApproximate: GREEDY_TRAP_2 step=3",
               to_string(exp), to_string(got));
        cleanupSongs(songs);
    }
}
static void test_playRandom_more_evil() {
    {
        // no-tie: chỉ có 1 nearest-by-position candidate mỗi bước
        // start=4 => next should always be start+1, then start+2...
        int n = 15, start = 4;
        vector<int> dur(n, 1);
        dur[start] = 100;
        for (int i = start+1; i < n; i++) dur[i] = dur[i-1] + 10;
        for (int i = 0; i < start; i++) dur[i] = 50 - (start-i); // all < 100 => not candidate
        dur[0] = 2000; // candidate but far (dist=start) => never chosen vs right neighbor dist=1

        vector<Song*> songs;
        songs.reserve(n);
        for (int i = 0; i < n; i++) songs.push_back(mkSong(3000+i, dur[i], 1, "S"+to_string(i+1)));

        Playlist p = mkPlaylist("PR_EVIL", songs);
        string got = capturePlayRandomOnce(p, start);
        string exp = refPlayRandomNoTie(dur, start);

        report(got == exp, "playRandom: EVIL no-tie long chain",
               exp, got);

        cleanupSongs(songs);
    }

    {
        // stop condition: start is already the max => must print only 1 song
        vector<int> dur = {999, 1, 2, 3, 4, 5, 6};
        vector<Song*> songs;
        for (int i=0;i<(int)dur.size();i++) songs.push_back(mkSong(4000+i, dur[i], 1, "S"+to_string(i+1)));
        Playlist p = mkPlaylist("PR_STOP", songs);

        string got = capturePlayRandomOnce(p, 0);
        string exp = "S1-A"; // only itself
        report(got == exp, "playRandom: STOP when no greater",
               exp, got);

        cleanupSongs(songs);
    }
}
static void test_stress_remove_and_play() {
    mt19937 rng(20260226);
    int n = 40;
    vector<Song*> songs;
    songs.reserve(n);
    for (int i=0;i<n;i++) {
        int dur = 10 + (rng()%200);
        int sc  = (rng()%50);
        songs.push_back(mkSong(5000+i, dur, sc, "S"+to_string(i+1)));
    }
    Playlist p = mkPlaylist("STRESS", songs);
    vector<Song*> extra;   // songs created during the run (Playlist does not own Song*)

    bool ok = true;
    try {
        // random operations
        for (int it=0; it<300; it++) {
            if (p.getSize() == 0) break;
            int op = (int)(rng()%4);
            if (op == 0) p.playNext();
            else if (op == 1) p.playPrevious();
            else if (op == 2) {
                int idx = (int)(rng()%p.getSize());
                p.removeSong(idx);
            } else {
                // occasional clear+rebuild small to hit edge states
                if (rng()%25 == 0) {
                    p.clear();
                    int m = 1 + (rng()%10);
                    for (int k=0;k<m;k++) {
                        int dur = 10 + (rng()%200);
                        int sc  = (rng()%50);
                        extra.push_back(mkSong(900000+it*100+k, dur, sc, "X"+to_string(it)+"_"+to_string(k)));
                        p.addSong(extra.back());
                    }
                }
            }
        }
    } catch(...) { ok = false; }

    report(ok, "STRESS: removeSong + playNext/playPrevious + clear (no crash)");

    cleanupSongs(songs);
    cleanupSongs(extra);
}
int main() {
    cout << "==== HARD TESTSUITE (main.cpp) ====\n";

    test_getTotalScore_hard();
    test_compareTo_hard();
    test_playApproximate_hard();
    test_playRandom_hard();
    test_playlist_stability_remove_next_prev();
    test_getTotalScore_extreme();
    test_playApproximate_greedy_traps();
    test_playRandom_more_evil();
    test_stress_remove_and_play();

    cout << "\n========== SUMMARY ==========\n";
    cout << "PASS: " << PASS << "\n";
    cout << "FAIL: " << FAIL << "\n";
    cout << "=============================\n";

    return (FAIL == 0) ? 0 : 1;
}