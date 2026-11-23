#ifndef PLAYLIST_H
#define PLAYLIST_H

#include "main.h"
#include "BotkifyLinkedList.h"

// =======================
// Class Song
// =======================
class Song
{
#ifdef TESTING
    friend class TestHelper;
#endif

private:
    int id;
    string title;
    string artist;
    string album;
    int duration;
    int score;
    string url;

public:
    Song(int id,
         string title,
         string artist,
         string album,
         int duration,
         int score,
         string url);

    int getID() const;
    int getDuration() const;
    int getScore() const;

    string toString() const;
};

// =======================
// Class Playlist
// =======================
class Playlist
{
#ifdef TESTING
    friend class TestHelper;
#endif

private:
    string name;
    BotkifyLinkedList<Song*> lstSong;
    int _size;
    int current;
    BotkifyLinkedList<Song*>::Node* currentN;
    bool songs;

public:
    Playlist(string name);
    ~Playlist();
    Playlist(const Playlist& other);
    Playlist& operator=(const Playlist& other);

    int getSize() const;
    int size() const { return getSize(); }
    bool empty() const;
    void clear();

    void addSong(Song* s);
    void removeSong(int index);
    Song* getSong(int index) const;

    Song* playNext();
    Song* playPrevious();

    int getTotalScore();
    bool compareTo(Playlist p, int numSong);

    void playRandom(int index);
    int playApproximate(int step);
};

#endif // PLAYLIST_H
