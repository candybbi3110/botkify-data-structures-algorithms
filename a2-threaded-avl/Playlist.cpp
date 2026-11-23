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
    : id(id),
      title(title),
      artist(artist),
      album(album),
      duration(duration),
      score(score),
      url(url),
      play_count(0)
{
}

std::string Song::toString() const {
    std::ostringstream oss;
    oss << "Song[id=" << id 
        << ", title=\"" << title << "\""
        << ", artist=\"" << artist << "\""
        << ", album=\"" << album << "\""
        << ", duration=" << duration
        << ", score=" << score
        << ", url=\"" << url << "\""
        << ", play_count=" << play_count
        << "]";
    return oss.str();
}

// =======================
// Playlist implementation
// =======================

Playlist::Playlist(string name)
    : name(name),
      size(0),
      currentIndex(-1)
#ifdef USE_THREADED_AVL
    , currentIt(songs.endIt()),
      hasCurrent(false)
#endif
{
    // TODO
}

SongKey Playlist::makeKey(Song* s) const {
    if(s == nullptr) return SongKey("", 0);
    return SongKey(s -> getTitle(), s -> getId());
}

void Playlist::resetPlayback() {
    currentIndex = -1;
#ifdef USE_THREADED_AVL
    hasCurrent = false;
    currentIt = songs.endIt();
#endif
}

int Playlist::getSize() const {
    return size;
}

bool Playlist::empty() const {
    return size == 0;
}

void Playlist::clear() {
    list<SongKey> keys = songs.ascendingList();
    for (SongKey key : keys) {
        Song** ps = songs.find(key);
        if (ps != nullptr) {
            delete *ps;
        }
    }
    songs.clear();
    size = 0;
    resetPlayback();
}

void Playlist::addSong(Song* s) {
    if (s == nullptr) return;

    SongKey newKey = makeKey(s);

    bool hadCurrent = false;
    Song* currentSong = nullptr;
    SongKey currentKey("", 0);

#ifdef USE_THREADED_AVL
    hadCurrent = hasCurrent && !currentIt.isNull();
    if (hadCurrent) {
        currentSong = currentIt.value();
        currentKey = makeKey(currentSong);
    }
#else
    hadCurrent = (currentIndex != -1);
    if (hadCurrent) {
        currentSong = getSong(currentIndex);
        if (currentSong != nullptr) {
            currentKey = makeKey(currentSong);
        } else {
            hadCurrent = false;
        }
    }
#endif

    bool inserted = songs.insert(newKey, s);

    if (!inserted) {
        delete s;
        return;
    }

    size++;

    if (hadCurrent) {
        if (newKey < currentKey) {
            currentIndex++;
        }
#ifdef USE_THREADED_AVL
        hasCurrent = true;   
#endif
    }
}

void Playlist::removeSong(int index) {
    if (index < 0 || index >= size) return;

    Song* s = getSong(index);
    if (s == nullptr) return;

    SongKey key = makeKey(s);

    bool hadCurrent = (currentIndex != -1);
    bool removingCurrent = (currentIndex == index);

    bool removed = songs.erase(key);
    if (!removed) return;

    delete s;
    size--;

    if (size == 0) {
        resetPlayback();
        return;
    }

    if (!hadCurrent) {
        resetPlayback();
        return;
    }

    if (removingCurrent) {
        if (index >= size) currentIndex = 0;
        else currentIndex = index;
    }
    else if (currentIndex > index) {
        currentIndex--;
    }

#ifdef USE_THREADED_AVL
    Song* curSong = getSong(currentIndex);
    if (curSong == nullptr) {
        resetPlayback();
        return;
    }

    currentIt = songs.findIt(makeKey(curSong));
    if (currentIt.isNull()) {
        resetPlayback();
        return;
    }

    hasCurrent = true;
#endif
}

Song* Playlist::getSong(int index) const {
    if (index < 0 || index >= size) return nullptr;

#ifdef USE_THREADED_AVL
    list<SongKey> keys = const_cast<ThreadedAVL<SongKey, Song*>&>(songs).ascendingList();
#else
    list<SongKey> keys = const_cast<AVL<SongKey, Song*>&>(songs).ascendingList();
#endif

    int i = 0;
    for (SongKey key : keys) {
        if (i == index) {
#ifdef USE_THREADED_AVL
            Song** ps = const_cast<ThreadedAVL<SongKey, Song*>&>(songs).find(key);
#else
            Song** ps = const_cast<AVL<SongKey, Song*>&>(songs).find(key);
#endif
            if (ps != nullptr) return *ps;
            return nullptr;
        }
        i++;
    }
    return nullptr;
}

// =======================
// Playing control (TODO)
// =======================

Song* Playlist::playNext() {
    if(size == 0) return nullptr;
#ifdef USE_THREADED_AVL
    if (!hasCurrent) {
        currentIt = songs.beginIt();
        if (currentIt.isNull()) return nullptr;
        hasCurrent = true;
        currentIndex = 0;
        return currentIt.value();
    }

    ++currentIt;
    if (currentIt.isNull()) {
        currentIt = songs.beginIt();
        currentIndex = 0;
    } else {
        currentIndex = (currentIndex + 1) % size;
    }

    return currentIt.value();
#else
    if (currentIndex == -1) {
        currentIndex = 0;
    } else {
        currentIndex = (currentIndex + 1) % size;
    }

    return getSong(currentIndex);
#endif
}

Song* Playlist::playPrevious() {
    if (size == 0) return nullptr;

#ifdef USE_THREADED_AVL
    if (!hasCurrent) {
        currentIt = songs.rbeginIt();
        if (currentIt.isNull()) return nullptr;
        hasCurrent = true;
        currentIndex = size - 1;
        return currentIt.value();
    }

    if (currentIndex == 0) {
        currentIt = songs.rbeginIt();
        currentIndex = size - 1;
    } else {
        --currentIt;
        currentIndex--;
    }

    return currentIt.value();
#else
    if (currentIndex == -1) {
        currentIndex = size - 1;
    } else {
        currentIndex = (currentIndex - 1 + size) % size;
    }

    return getSong(currentIndex);
#endif
}

// =======================
// Score-related (TODO)
// =======================

int Playlist::getTotalScore() {
    int total = 0;
    list<SongKey> keys = songs.ascendingList();

    for (SongKey key : keys) {
        Song** ps = songs.find(key);
        if (ps != nullptr && *ps != nullptr) {
            total += (*ps)->getScore();
        }
    }

    return total;
}

bool Playlist::compareTo(const Playlist& p, int numSong) {
    if (numSong <= 0) return true;

    int sumThis = 0;
    int sumOther = 0;

#ifdef USE_THREADED_AVL
    list<SongKey> keysThis = const_cast<ThreadedAVL<SongKey, Song*>&>(songs).ascendingList();
    list<SongKey> keysOther = const_cast<ThreadedAVL<SongKey, Song*>&>(p.songs).ascendingList();
#else
    list<SongKey> keysThis = const_cast<AVL<SongKey, Song*>&>(songs).ascendingList();
    list<SongKey> keysOther = const_cast<AVL<SongKey, Song*>&>(p.songs).ascendingList();
#endif

    int count = 0;
    for (SongKey key : keysThis) {
        if (count >= numSong) break;

#ifdef USE_THREADED_AVL
        Song** ps = const_cast<ThreadedAVL<SongKey, Song*>&>(songs).find(key);
#else
        Song** ps = const_cast<AVL<SongKey, Song*>&>(songs).find(key);
#endif
        if (ps != nullptr && *ps != nullptr) {
            sumThis += (*ps)->getScore();
        }
        count++;
    }

    count = 0;
    for (SongKey key : keysOther) {
        if (count >= numSong) break;

#ifdef USE_THREADED_AVL
        Song** ps = const_cast<ThreadedAVL<SongKey, Song*>&>(p.songs).find(key);
#else
        Song** ps = const_cast<AVL<SongKey, Song*>&>(p.songs).find(key);
#endif
        if (ps != nullptr && *ps != nullptr) {
            sumOther += (*ps)->getScore();
        }
        count++;
    }

    return sumThis >= sumOther;
}


// =======================
// Advanced playing modes (TODO)
// =======================

void Playlist::playRandom(int index) {
    if (index < 0 || index >= size) return;

#ifdef USE_THREADED_AVL
    Song* s = getSong(index);
    if (s == nullptr) return;

    ThreadedAVL<SongKey, Song*>::Iterator it = songs.findIt(makeKey(s));
    if (it.isNull()) return;

    currentIt = it;
    hasCurrent = true;
    currentIndex = index;
#else
    currentIndex = index;
#endif
}

int Playlist::playApproximate(int step) {
    if (size == 0) return -1;

    int baseIndex;
    if (currentIndex == -1) baseIndex = 0;
    else baseIndex = currentIndex;

    int newIndex = (baseIndex + step) % size;
    if (newIndex < 0) newIndex += size;

#ifdef USE_THREADED_AVL
    Song* s = getSong(newIndex);
    if (s == nullptr) return -1;

    currentIt = songs.findIt(makeKey(s));
    if (currentIt.isNull()) return -1;

    hasCurrent = true;
#endif

    currentIndex = newIndex;
    return currentIndex;
}