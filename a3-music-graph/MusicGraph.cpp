#include "MusicGraph.h"

// =============================================================================
// PRIVATE HELPER METHODS
// =============================================================================

int MusicGraph::getSongIndex(const string& id) const {
    for(int i = 0; i < (int)songsList.size(); i++){
      if(songsList[i].id == id){
        return i;
      }
    }
    return -1;
}

bool MusicGraph::isVisited(const string& id, const vector<string>& visitedList) const {
    for(int i = 0; i < (int)visitedList.size(); i++){
      if(visitedList[i] == id) return true;
    }
    return false;
}

// =============================================================================
// PUBLIC METHODS
// =============================================================================

void MusicGraph::addSong(const string& id, const string& title, const string& artist, const string& genre) {
    if(getSongIndex(id) != -1) return;
    Song newSong;
    newSong.id = id;
    newSong.title = title;
    newSong.artist = artist;
    newSong.genre = genre;

    SongEntry newSongEntry;
    newSongEntry.id = id;
    newSongEntry.data = newSong;

    songsList.push_back(newSongEntry);
    this -> addVertex(id);
}

void MusicGraph::printSongInfo(const string& id) const {
    int index = getSongIndex(id);
    if(index == -1){
      cout << "[Unknown Song]";
      return;
    }
    cout <<"[" << songsList[index].data.id << "] "
               << songsList[index].data.title << " - "
               << songsList[index].data.artist;
}

// =============================================================================
// REQUIREMENT 1: Recommend related songs (BFS)
// =============================================================================
void MusicGraph::recommendRelatedSongs(const string& startId) const {
    if (!this->hasVertex(startId)) return;

    cout << "\n[1] SUGGEST RELATED SONGS FOR: ";
    printSongInfo(startId);
    cout << "\n-------------------------------------------------\n";

    vector<string> visited;
    vector<string> waitingList;
    int position = 0;

    visited.push_back(startId);
    waitingList.push_back(startId);
    while(position < (int)waitingList.size()){
      string current = waitingList[position];
      position++;

      vector<Edge> neighbors = this -> getNeighbors(current);

      for(int i = 0; i < (int) neighbors.size(); i++){
        string nextId = neighbors[i].target;
        if(!isVisited(nextId, visited)){
          visited.push_back(nextId);
          waitingList.push_back(nextId);
          cout << "-> ";
          printSongInfo(nextId);
          cout << "\n";
        }
      }
    }
}

// =============================================================================
// REQUIREMENT 2: Create playlist by clusters (Connected Components using BFS)
// =============================================================================
void MusicGraph::generatePlaylistsByClusters() const {
    cout << "\n[2] CREATE PLAYLIST BY CLUSTERS (CONNECTED COMPONENTS)\n";
    cout << "-------------------------------------------------\n";

    vector<string> globalVisited;
    int cnt = 0;

    for (int i = 0; i < (int)this->adjList.size(); i++) {
        string start = this->adjList[i].vertex;

        if (!isVisited(start, globalVisited)) {
            cnt++;
            cout << "=== Playlist " << cnt << " ===\n";

            vector<string> waitingList;
            int position = 0;

            globalVisited.push_back(start);
            waitingList.push_back(start);

            while (position < (int)waitingList.size()) {
                string current = waitingList[position];
                position++;

                cout << "  * ";
                printSongInfo(current);
                cout << "\n";

                vector<Edge> neighbors = this->getNeighbors(current);

                for (int k = 0; k < (int)neighbors.size(); k++) {
                    string nextId = neighbors[k].target;

                    if (!isVisited(nextId, globalVisited)) {
                        globalVisited.push_back(nextId);
                        waitingList.push_back(nextId);
                    }
                }

                for (int a = 0; a < (int)this->adjList.size(); a++) {
                    string sourceId = this->adjList[a].vertex;
                    vector<Edge> sourceNeighbors = this->getNeighbors(sourceId);

                    for (int b = 0; b < (int)sourceNeighbors.size(); b++) {
                        if (sourceNeighbors[b].target == current) {
                            if (!isVisited(sourceId, globalVisited)) {
                                globalVisited.push_back(sourceId);
                                waitingList.push_back(sourceId);
                            }
                        }
                    }
                }
            }
        }
    }
}

// =============================================================================
// REQUIREMENT 3: Smooth song transition (Dijkstra's Algorithm)
// =============================================================================
void MusicGraph::findSmoothTransition(const string& startId, const string& endId) const {
    cout << "\n[3] SMOOTHEST TRANSITION (DIJKSTRA)\n";
    cout << "From: "; printSongInfo(startId); cout << "\n";
    cout << "To: "; printSongInfo(endId); cout << "\n";
    cout << "-------------------------------------------------\n";

    int n = this->adjList.size();
    int startIdx = this->getVertexIndex(startId);
    int endIdx = this->getVertexIndex(endId);

    if (startIdx == -1 || endIdx == -1) {
        cout << "Error: Song not found!\n";
        return;
    }

    double INF = 999999999.0;
    vector<double> distance(n, INF);
    vector<int> previous(n, -1);
    vector<bool> visited(n, false);

    distance[startIdx] = 0;

    for (int count = 0; count < n; count++) {
        int u = -1;
        double minDista = INF;

        for (int i = 0; i < n; i++) {
            if (!visited[i] && distance[i] < minDista) {
                minDista = distance[i];
                u = i;
            }
        }

        if (u == -1) {
            break;
        }

        visited[u] = true;

        if (u == endIdx) {
            break;
        }

        vector<Edge> neighbors = this->getNeighbors(this->adjList[u].vertex);

        for (int i = 0; i < (int)neighbors.size(); i++) {
            int v = this->getVertexIndex(neighbors[i].target);
            double weight = neighbors[i].weight;

            if (v != -1 && !visited[v] && distance[u] + weight < distance[v]) {
                distance[v] = distance[u] + weight;
                previous[v] = u;
            }
        }
    }

    if (distance[endIdx] == INF) {
        cout << "No transition path between these two songs.\n";
        return;
    }

    vector<string> path;
    int current = endIdx;

    while (current != -1) {
        path.push_back(this->adjList[current].vertex);
        current = previous[current];
    }

    cout << "-> Total Deviation (Cost): " << distance[endIdx] << "\n";
    cout << "-> Playback Order:\n";

    for (int i = (int)path.size() - 1; i >= 0; i--) {
        cout << "   " << (int)path.size() - i << ". ";
        printSongInfo(path[i]);
        cout << "\n";
    }
}

// =============================================================================
// REQUIREMENT 4: Find the network hub song (In-degree Calculation)
// =============================================================================
void MusicGraph::findMostPopularSong() const {
    cout << "\n[4] FIND NETWORK HUB SONG (IN-DEGREE)\n";
    cout << "-------------------------------------------------\n";

    int n = this->adjList.size();

    if (n == 0) {
        cout << "No songs in the network.\n";
        return;
    }

    vector<int> inDegree(n, 0);

    for (int i = 0; i < n; i++) {
        vector<Edge> neighbors = this->getNeighbors(this->adjList[i].vertex);

        for (int j = 0; j < (int)neighbors.size(); j++) {
            int targetIndex = this->getVertexIndex(neighbors[j].target);

            if (targetIndex != -1) {
                inDegree[targetIndex]++;
            }
        }
    }

    int maxIndex = 0;

    for (int i = 1; i < n; i++) {
        if (inDegree[i] > inDegree[maxIndex]) {
            maxIndex = i;
        }
    }

    cout << "-> Network Hub Song:\n";
    cout << "   ";
    printSongInfo(this->adjList[maxIndex].vertex);
    cout << "\n";
    cout << "   (In-degree: " << inDegree[maxIndex] << ")\n";
}

// =============================================================================
// REQUIREMENT 5: Detect music loop (DFS Cycle Detection)
// =============================================================================

// DFS recursive helper function
bool MusicGraph::dfsCycleHelper(int idx, vector<bool>& visited, vector<bool>& recursionStack,
                                 vector<int>& parent, bool& found) const {
    if (found) {
        return true;
    }

    visited[idx] = true;
    recursionStack[idx] = true;

    vector<Edge> neighbors = this->getNeighbors(this->adjList[idx].vertex);

    for (int i = 0; i < (int)neighbors.size(); i++) {
        int nextIdx = this->getVertexIndex(neighbors[i].target);

        if (nextIdx == -1) {
            continue;
        }

        if (!visited[nextIdx]) {
            parent[nextIdx] = idx;

            if (dfsCycleHelper(nextIdx, visited, recursionStack, parent, found)) {
                return true;
            }
        }
        else if (recursionStack[nextIdx]) {
            found = true;

            cout << "-> Music loop detected!\n";
            cout << "-> Loop:\n";

            vector<int> reversePath;
            int current = idx;

            while (current != nextIdx && current != -1) {
                reversePath.push_back(current);
                current = parent[current];
            }

            cout << "   ";
            printSongInfo(this->adjList[nextIdx].vertex);
            cout << "\n";

            for (int j = (int)reversePath.size() - 1; j >= 0; j--) {
                cout << "   ";
                printSongInfo(this->adjList[reversePath[j]].vertex);
                cout << "\n";
            }

            cout << "   ";
            printSongInfo(this->adjList[nextIdx].vertex);
            cout << "\n";

            return true;
        }
    }

    recursionStack[idx] = false;
    return false;
}

void MusicGraph::detectMusicLoop() const {
    cout << "\n[5] DETECT MUSIC LOOP (DFS CYCLE DETECTION)\n";
    cout << "-------------------------------------------------\n";

    int n = this->adjList.size();

    vector<bool> visited(n, false);
    vector<bool> recursionStack(n, false);
    vector<int> parent(n, -1);

    bool found = false;

    for (int i = 0; i < n; i++) {
        if (!visited[i]) {
            if (dfsCycleHelper(i, visited, recursionStack, parent, found)) {
                return;
            }
        }
    }

    cout << "No music loop detected.\n";
}