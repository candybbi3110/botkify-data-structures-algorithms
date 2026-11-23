#include "Playlist.h"

// =======================
// Song implementation
// =======================

Song::Song(int id,
           string title,
           string artist,
           string album,
           int duration,
           int score,
           string url)
{
    this -> id = id;
    this -> title = title;
    this -> artist = artist;
    this -> album = album;
    this -> duration = duration;
    this -> score = score;
    this -> url = url;
}

int Song::getID() const
{
    return id;
}

int Song::getDuration() const
{
    return duration;
}

int Song::getScore() const
{
    return score;
}

string Song::toString() const
{
    return title + "-" + artist;
}

// =======================
// Playlist implementation
// =======================

Playlist::Playlist(string name)
{
    this -> name = name;
    this -> _size = 0;
    this->current = -1;
    this->currentN = nullptr;
    this->songs = true;
}

int Playlist::getSize() const
{
    return _size;
}

bool Playlist::empty() const
{
    if (_size == 0) return true;
    return false;
}

void Playlist::clear()
{
    lstSong.clear();
    _size = 0;
    current = -1;
    currentN = nullptr;
}

void Playlist::addSong(Song* s)
{
    lstSong.add(s);
    _size++;
    if (_size == 1) {
        current = 0;
        currentN = lstSong.getHead();
    }
}

void Playlist::removeSong(int index)
{
    if (index < 0 || index >= _size) throw out_of_range("Index is invalid!");

    bool removingCur = false;
    if (currentN != nullptr) {
        auto node = lstSong.getHead();
        for (int i = 0; i < index; i++) node = node->next;
        removingCur = (node == currentN);
    }

    lstSong.removeAt(index);
    _size--;

    if (_size == 0) {
        current = -1;
        currentN = nullptr;
        return;
    }

    if (index < current) current--;
    if (current < 0 || current >= _size) current = 0;

    if (removingCur) {
        int newIndex = index;
        if (newIndex >= _size) newIndex = 0;

        currentN = lstSong.getHead();
        for (int i = 0; i < newIndex; i++) currentN = currentN->next;
        current = newIndex;
    }
}

Song* Playlist::getSong(int index) const
{
    if (index < 0 || index >= _size) throw out_of_range("Index is invalid!");
    return lstSong.get(index);
}

// =======================
// Playing control
// =======================

Song* Playlist::playNext()
{
    if (_size == 0) throw out_of_range("Index is invalid!");
    if (currentN == nullptr || current < 0 || current >= _size) {
        currentN = lstSong.getHead();
        current = 0;
    }

    if(currentN->next != nullptr) currentN = currentN->next;
    else currentN = lstSong.getHead();
    current = (current + 1) % _size;
    return currentN -> data;
}

Song* Playlist::playPrevious()
{
    if (_size == 0) throw out_of_range("Index is invalid!");
    if (currentN == nullptr || current < 0 || current >= _size) {
        currentN = lstSong.getHead();
        current = 0;
    }
    if (currentN->extra != nullptr) currentN = currentN->extra;
    else currentN = lstSong.getTail();
    current = (current - 1 + _size) % _size;
    return currentN->data;
}

// =======================
// Score-related
// =======================
static long long sumofRange(long long* cal2, int l, int r)
{
    if (l > r) return 0;
    if (l == 0) return cal2[r];
    return cal2[r] - cal2[l - 1];
}

int Playlist::getTotalScore()
{
    int sizeofSong = _size;
    if (sizeofSong == 0) return 0;
    long long* score = new long long [sizeofSong];
    auto node = lstSong.getHead();
    for (int i = 0; i < sizeofSong; i++) {
        score[i] = node -> data -> getScore();
        node = node -> next;
    }
    long long* cal = new long long[sizeofSong + 1];
    cal[0] = 0;
    for (int i = 0; i < sizeofSong; i++) cal[i + 1] = cal[i] + score[i];

    long long* cal2 = new long long[sizeofSong + 1];
    cal2[0] = cal[0];
    for (int i = 1; i <= sizeofSong; i++) cal2[i] = cal2[i - 1] + cal[i];

    int* L = new int[sizeofSong];
    int* R = new int[sizeofSong];
    int* a = new int[sizeofSong];
    int index = -1;

    for (int i = 0; i < sizeofSong; i++) {
        while (index >= 0 && score[i] <= score[a[index]]) index--;
        L[i] = (index >= 0) ? a[index] : -1;
        index++;
        a[index] = i;
    }

    index = -1;
    for (int i = sizeofSong - 1; i >= 0; i--) {
        while (index >= 0 && score[i] < score[a[index]]) index--;
        R[i] = (index >= 0) ? a[index] : sizeofSong;
        index++;
        a[index] = i;
    }

    long long answer = 0;
    for (int i = 0; i < sizeofSong; i++) {
        int countL = i - L[i];
        int countR = R[i] - i;

        long long sumRight = sumofRange(cal2, i + 1, R[i]);
        long long sumLeft  = sumofRange(cal2, L[i] + 1, i);

        long long totalSum = 1LL * countL * sumRight - 1LL * countR * sumLeft;
        answer += score[i] * totalSum;
    }
    delete[] score;
    delete[] cal;
    delete[] cal2;
    delete[] L;
    delete[] R;
    delete[] a;

    return (int)answer;
}

Playlist::~Playlist() {
    clear();
}

Playlist::Playlist(const Playlist& other) {
    name = other.name;
    lstSong = other.lstSong;
    _size = other._size;
    current = other.current;

    songs = false;
    currentN = nullptr;

    if (current >= 0 && current < _size) {
        currentN = lstSong.getHead();
        for (int i = 0; i < current; i++) currentN = currentN->next;
    }
}

Playlist& Playlist::operator = (const Playlist& other) {
    if (this == &other) return *this;

    clear();

    name = other.name;
    lstSong = other.lstSong;
    _size = other._size;
    current = other.current;

    songs = false;
    currentN = nullptr;

    if (current >= 0 && current < _size) {
        currentN = lstSong.getHead();
        for (int i = 0; i < current; i++) currentN = currentN->next;
    }
    return *this;
}

bool Playlist::compareTo(Playlist p, int numSong)
{
    auto calc = [&](Playlist& pList, long long& sumMax, long long& count) {
        sumMax = 0; count = 0;
        int n = pList._size;
        int f = numSong;
        if (f <= 0 || n <= 0 || f > n) return;

        int* a = new int[n];
        auto node = pList.lstSong.getHead();
        for (int i = 0; i < n; i++) { a[i] = node->data->getScore(); node = node->next; }

        int* pt = new int[n];
        int l = 0, r = -1;

        for (int i = 0; i < n; i++) {
            while (l <= r && a[pt[r]] <= a[i]) r--;
            pt[++r] = i;

            while (l <= r && pt[l] <= i - f) l++;

            if (i >= f - 1) {
                sumMax += a[pt[l]];
                count++;
            }
        }

        delete[] pt;
        delete[] a;
    };

    long long sumofA, countA, sumofB, countB;
    calc(*this, sumofA, countA);
    calc(p,     sumofB, countB);

    if (countA == 0 && countB == 0) return true;
    if (countA == 0) return false;
    if (countB == 0) return true;

    return sumofA * countB >= sumofB * countA;
}
// =======================
// Advanced playing modes
// =======================

void Playlist::playRandom(int index)
{
    int n = _size;
    if (n <= 0) return;
    if (index < 0 || index >= n) return;

    int* durat = new int[n];
    string* s = new string[n];

    auto node = lstSong.getHead();
    for (int i = 0; i < n; i++) {
        durat[i] = node->data->getDuration();
        s[i] = node->data->toString();
        node = node->next;
    }

    int* L = new int[n];
    int* pt = new int[n];
    int idx = -1;
    for (int i = 0; i < n; i++) {
        while (idx >= 0 && durat[pt[idx]] <= durat[i]) idx--;
        L[i] = (idx >= 0) ? pt[idx] : -1;
        idx++;
        pt[idx] = i;
    }

    int* R = new int[n];
    idx = -1;
    for (int i = n - 1; i >= 0; i--) {
        while (idx >= 0 && durat[pt[idx]] <= durat[i]) idx--;
        R[i] = (idx >= 0) ? pt[idx] : -1;
        idx++;
        pt[idx] = i;
    }
    int current = index;
    bool first = true;
    while (true) {
        if (!first) cout << ",";
        cout << s[current];
        first = false;

        int next = -1;
        int bestDist = 2147483647;

        for (int i = 0; i < n; i++) {
            if (durat[i] > durat[current]) {
                int dist = (i > current) ? (i - current) : (current - i);
                if (dist < bestDist) {
                    bestDist = dist;
                    next = i;
                }
            }
        }

        if (next == -1) break;
        current = next;
    }
    delete[] durat;
    delete[] s;
    delete[] L;
    delete[] R;
    delete[] pt;
}

int Playlist::playApproximate(int step)
{
    int sizeofSong = _size;
    if (sizeofSong <= 1) return 0;
    if (step < 0) step = 0;

    int* durat = new int[sizeofSong];
    auto node = lstSong.getHead();
    for (int i = 0; i < sizeofSong; i++) {
        durat[i] = node->data->getDuration();
        node = node->next;
    }

    const long long INF = (1LL << 60);
    long long* pt = new long long[sizeofSong];
    for (int i = 0; i < sizeofSong; i++) pt[i] = INF;
    pt[0] = 0;

    int nSkip = step + 1;

    for (int i = 1; i < sizeofSong; i++) {
        int x = i - nSkip;
        if (x < 0) x = 0;

        long long best = INF;
        for (int j = x; j <= i - 1; j++) {
            long long diff = (long long)durat[i] - durat[j];
            if (diff < 0) diff = -diff;
            long long sumofTime = pt[j] + diff;
            if (sumofTime < best) best = sumofTime;
        }
        pt[i] = best;
    }

    long long answer = pt[sizeofSong - 1];

    delete[] durat;
    delete[] pt;

    return (int)answer;
}