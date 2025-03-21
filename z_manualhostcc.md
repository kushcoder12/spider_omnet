/home/kush/Desktop/spiderproj/spider_omnet/hostInitialize.cc

#ifndef INITIALIZE_H
#define INITIALIZE_H
#include "hostInitialize.h"

bool probesRecent(unordered_map<int, PathInfo> probes){
    for (auto iter : probes){
        int key = iter.first;
        if ((iter.second).lastUpdated == -1  || ((simTime() - (iter.second).lastUpdated) > _maxTravelTime) )
            return false;
    }
    return true;
}


/* generate_channels_balances_map - reads from file and constructs adjacency list of graph topology (channels), and hash map
 *      of directed edges to initial balances, modifies global maps in place
 *      each line of file is of form
 *      [node1] [node2] [1->2 delay] [2->1 delay] [balance at node1 end] [balance at node2 end]
 */
void generateChannelsBalancesMap(string topologyFile) {
    string line;
    ifstream myfile (topologyFile);
    int lineNum = 0;
    int numEdges = 0;
    double sumDelays = 0.0;
    if (myfile.is_open())
    {
        while ( getline (myfile,line) )
        {
            lineNum++;
            vector<string> data = split(line, ' ');
            // parse all the landmarks from the first line
            if (lineNum == 1) {
                for (auto node : data) {
                    char nodeType = node.back();
                    int nodeNum = stoi((node).substr(0,node.size()-1)); 
                    if (nodeType == 'r') {
                        nodeNum = nodeNum + _numHostNodes;
                    }
                    _landmarks.push_back(nodeNum);
                    _landmarksWithConnectivityList.push_back(make_tuple(_channels[nodeNum].size(), nodeNum));
                }
                // don't do anything else
                continue;
            }
            //generate _channels - adjacency map
            char node1type = data[0].back();
            char node2type = data[1].back();

            if (_loggingEnabled) {
                cout <<"node2type: " << node2type << endl;
                cout <<"node1type: " << node1type << endl;
                cout << "data size" << data.size() << endl;
            }

            int node1 = stoi((data[0]).substr(0,data[0].size()-1)); //
            if (node1type == 'r')
                node1 = node1 + _numHostNodes;

            int node2 = stoi(data[1].substr(0,data[1].size()-1)); //
            if (node2type == 'r')
                node2 = node2 + _numHostNodes;

            int delay1to2 = stoi(data[2]);
            int delay2to1 = stoi(data[3]);
            if (_channels.count(node1)==0){ //node 1 is not in map
                vector<pair<int,int>> tempVector = {};
                tempVector.push_back(make_pair(node2,delay1to2));
                _channels[node1] = tempVector;
            }
            else //(node1 is in map)
                _channels[node1].push_back(make_pair(node2,delay1to2));

            if (_channels.count(node2)==0){ //node 1 is not in map
                vector<pair<int,int>> tempVector = {make_pair(node1,delay2to1)};
                _channels[node2] = tempVector;
            }
            else //(node2 is in map)
                _channels[node2].push_back(make_pair(node1, delay2to1));

            sumDelays += delay1to2 + delay2to1;
            numEdges += 2;
            //generate _balances map
            double balance1 = stod( data[4]);
            double balance2 = stod( data[5]);
            _balances[make_tuple(node1,node2)] = balance1;
            _balances[make_tuple(node2,node1)] = balance2;

            tuple<int, int> senderReceiverPair = (node1 < node2) ? make_tuple(node1, node2) :
                make_tuple(node2, node1);
            _capacities[senderReceiverPair] = balance1 + balance2;
            data = split(line, ' ');
        }

        myfile.close();
        _avgDelay = sumDelays/numEdges;
    }
    else 
        cout << "Unable to open file " << topologyFile << endl;

    cout << "finished generateChannelsBalancesMap whose size: " << _capacities.size() << endl;
    return;
}


/* set_num_nodes -
 */
void setNumNodes(string topologyFile){
    int maxHostNode = -1;
    int maxRouterNode = -1;
    string line;
    int lineNum = 0;
    ifstream myfile (topologyFile);
    if (myfile.is_open())
    {
        while ( getline (myfile,line) )
        {
            lineNum++;
            // skip landmark line
            if (lineNum == 1) {
                continue;
            }
            vector<string> data = split(line, ' ');
            //generate channels - adjacency map
            char node1type = data[0].back();
            //  cout <<"node1type: " << node1type << endl;
            char node2type = data[1].back();
            //   cout <<"node2type: " << node2type << endl;

            int node1 = stoi((data[0]).substr(0,data[0].size()-1)); //
            if (node1type == 'r' && node1 > maxRouterNode){
                maxRouterNode = node1;
                //node1 = node1+ _numHostNodes;
            }
            else if (node1type == 'e' && node1 > maxHostNode){
                maxHostNode = node1;
            }

            int node2 = stoi(data[1].substr(0,data[1].size()-1)); //
            if (node2type == 'r' && node2 > maxRouterNode){
                maxRouterNode = node2;
                //node2 = node2 + _numHostNodes;
            }
            else if (node2type == 'e' && node2 > maxHostNode){
                maxHostNode = node2;
            }
        }
        myfile.close();
    }
    else 
        cout << "Unable to open file" << topologyFile << endl;
    _numHostNodes = maxHostNode + 1;
    _numRouterNodes = maxRouterNode + 1;
    _numNodes = _numHostNodes + _numRouterNodes;
    return;
}

/*
 *  generate_trans_unit_list - reads from file and generates global transaction unit job list.
 *      each line of file is of form:
 *      [amount] [timeSent] [sender] [receiver] [priorityClass]
 */
//Radhika: do we need to pass global variables as arguments?
void generateTransUnitList(string workloadFile){
    string line;
    ifstream myfile (workloadFile);
    double lastTime = -1; 
    int lineNum = 0;
    if (myfile.is_open())
    {
        while ( getline (myfile,line) )
        {
            vector<string> data = split(line, ' ');
            lineNum++;

            //data[0] = amount, data[1] = timeSent, data[2] = sender, data[3] = receiver, data[4] = priority class; (data[5] = time out)
            double amount = stod(data[0]);
            double timeSent = stod(data[1]);
            int sender = stoi(data[2]);
            int receiver = stoi(data[3]);
            int priorityClass = stoi(data[4]);
            double timeOut=-1;
            double largerTxnID = lineNum;
            double hasTimeOut = _timeoutEnabled;
            
            if (timeSent >= _transStatStart && timeSent <= _transStatEnd) {
                if (_transactionArrivalBySize.count(amount) > 0) {
                    _transactionArrivalBySize[amount] += 1;
                }
                else {
                    _transactionCompletionBySize[amount] = 0;
                    _transactionArrivalBySize[amount] = 1;
                    _txnAvgCompTimeBySize[amount] = 0;
                }
            }

            if (data.size()>5 && _timeoutEnabled){
                timeOut = stoi(data[5]);
                //cout << "timeOut: " << timeOut << endl;
            }
            else if (_timeoutEnabled) {
                timeOut = 5.0;
            }

            if (_waterfillingEnabled) { 
                if (timeSent < _waterfillingStartTime || timeSent > _shortestPathEndTime) {
                    continue;
                 }
            }
            else if (_landmarkRoutingEnabled || _lndBaselineEnabled) { 
                if (timeSent < _landmarkRoutingStartTime || timeSent > _shortestPathEndTime) 
                    continue;
            }
            else if (!_priceSchemeEnabled && !_dctcpEnabled) {// shortest path
                if (timeSent < _shortestPathStartTime || timeSent > _shortestPathEndTime) 
                    continue;
            }
            
            if (timeSent > lastTime)
                 lastTime = timeSent;
            // instantiate all the transUnits that need to be sent
            int numSplits = 0;
            double totalAmount = amount;
            while (amount >= _splitSize && (_waterfillingEnabled || _priceSchemeEnabled || _lndBaselineEnabled || _dctcpEnabled || _celerEnabled)
                   && _splittingEnabled) {
                TransUnit tempTU = TransUnit(_splitSize, timeSent, 
                        sender, receiver, priorityClass, hasTimeOut, timeOut, largerTxnID);
                amount -= _splitSize;
                _transUnitList[sender].push(tempTU);
                numSplits++;
            }
            if (amount > 0) {
                TransUnit tempTU = TransUnit(amount, timeSent, sender, receiver, priorityClass, 
                        hasTimeOut, timeOut, largerTxnID);
                _transUnitList[sender].push(tempTU);
                numSplits++;
            }

            // push the transUnit into a priority queue indexed by the sender, 
            _destList[sender].insert(receiver);
            
            SplitState temp = {};
            temp.numTotal = numSplits;
            temp.numReceived = 0;
            temp.numArrived = 0;
            temp.numAttempted = 0;
            temp.totalAmount = totalAmount; 
            temp.firstAttemptTime = -1;
            _numSplits[sender][largerTxnID] = temp;

        }
        //cout << "finished generateTransUnitList" << endl;
        myfile.close();
        /*if (lastTime + 5 < _simulationLength) {
            cout << "Insufficient txns" << endl;
            assert(false);
        }*/
    }
    else 
        cout << "Unable to open file" << workloadFile << endl;
    return;
}


/* updateMaxTravelTime - calculate max travel time, called on each new route, and updates _maxTravelTime value
 */
void updateMaxTravelTime(vector<int> route){
    int nextNode;
    vector<pair<int,int>>* channel; 
    double maxTime = 0;

    for (int i=0; i< ( route.size()-1) ; i++){
        //find the propogation delay per edge of route
        //Radhika TODO: might be better to store channel map indexed using both nodes. check if it breaks anything.
        channel = &(_channels[route[i]]);
        nextNode = route[i+1];

        auto it = find_if( (*channel).begin(), (*channel).end(),
                [&nextNode](const pair<int, int>& element){ return element.first == nextNode;} );
        if (it != (*channel).end()){
            double deltaTime = it->second;
            if (deltaTime > _maxOneHopDelay)
                _maxOneHopDelay = deltaTime/1000;
            maxTime = maxTime + deltaTime;
        }
        else{
            cout << "ERROR IN updateMaxTravelTime, could not find edge" << endl;
            cout << "node to node " << route[i] << "," << route[i+1] << endl;
        }
    }
    maxTime = (maxTime)/1000 *2; //double for round trip, and convert from milliseconds to seconds
    if (maxTime > _maxTravelTime){
        _maxTravelTime = maxTime;
    }
    _delta = _maxTravelTime;
    return;
}


/*get_route- take in sender and receiver graph indicies, and returns
 *  BFS shortest path from sender to receiver in form of node indicies,
 *  includes sender and reciever as first and last entry
 */
vector<int> getRoute(int sender, int receiver){
    vector<int> route = dijkstraInputGraph(sender, receiver, _channels);
    updateMaxTravelTime(route);
    return route;
}

double bottleneckOnPath(vector<int> route) {
   double min = 10000000;
    // ignore end hosts
    for (int i = 1; i < route.size() - 2; i++) {
        double cap = _balances[make_tuple(i, i + 1)] + _balances[make_tuple(i+1, i)];
        if (cap < min)
            min = cap;
    }
    return min;
}

/* find the bottleneck "capacity" on the path
 * so that windows are not allowed to grow larger than them 
 */
double bottleneckCapacityOnPath(vector<int> route) {
   double min = 10000000;
    // ignore end hosts
    for (int i = 1; i < route.size() - 2; i++) {
        int thisNode = route[i];
        int nextNode = route[i + 1];
        tuple<int, int> senderReceiverTuple = (thisNode < nextNode) ? make_tuple(thisNode, nextNode) :
                make_tuple(nextNode, thisNode);
        double cap = _capacities[senderReceiverTuple];
        if (cap < min)
            min = cap;
    }
    return min;
}

void updateCannonicalRTT(vector<int> route) {
        // update cannonical RTT
        double sumRTT = (route.size() - 1) * 2 * _avgDelay / 1000.0;
        sumRTT += _cannonicalRTT * _totalPaths;
        _totalPaths += 1;
        _cannonicalRTT = sumRTT / _totalPaths;
}

vector<vector<int>> getKShortestRoutes(int sender, int receiver, int k){
    //do searching without regard for channel capacities, DFS right now
    if (_loggingEnabled) {
        printf("sender: %i; receiver: %i \n ", sender, receiver);
        cout<<endl;
    }
    vector<vector<int>> shortestRoutes = {};
    vector<int> route;
    auto tempChannels = _channels;
    for ( int it = 0; it < k; it++ ){
        route = dijkstraInputGraph(sender, receiver, tempChannels);
        
        if (route.size() <= 1){
            return shortestRoutes;
        }
        else{
            updateMaxTravelTime(route);
            updateCannonicalRTT(route);
            shortestRoutes.push_back(route);
        }
        if (_loggingEnabled) {
            cout << "getKShortestRoutes 1" <<endl;
            cout << "route size: " << route.size() << endl;
            cout << "getKShortestRoutes 2" <<endl;
        }
        tempChannels = removeRoute(tempChannels,route);
        cout << "Route Num " << it + 1 << " " << " ";
        printVector(route);

    }
    if (_loggingEnabled)
        cout << "Number of Routes between " << sender << " and " << receiver << " is " << shortestRoutes.size() << endl;
    return shortestRoutes;
}

void initializePathMaps(string filename) {
    string line;
    int lineNum = 0;
    ifstream myfile (filename);
    if (myfile.is_open())
    {
        int sender = -1;
        int receiver = -1;
        vector<vector<int>> pathList;
        while ( getline (myfile,line) ) 
        {
            vector<string> data = split(line, ' ');
            lineNum++;
            if (data[0].compare("pair") == 0) {
                if (lineNum > 1) {
                    _pathsMap[sender][receiver] = pathList;
                    // cout << data[0] <<  " " << data[1] << endl;
                }
                sender = stoi(data[1]);
                receiver = stoi(data[2]);
                pathList.clear();
                //cout << " sender " << sender << " receiver " << receiver << endl;
            }
            else {
                vector<int> newPath;
                for (string d : data) {
                    newPath.push_back(stoi(d));
                }
                pathList.push_back(newPath);
                if (_loggingEnabled) 
                    printVector(newPath);
            }
        }
        
        // push the last path in
        _pathsMap[sender][receiver] = pathList;
    }
    else {
        cout << "unable to open paths file " << filename << endl;
    }
}


vector<vector<int>> getKPaths(int sender, int receiver, int k) {
    if (!_widestPathsEnabled && !_kspYenEnabled && !_obliviousRoutingEnabled && !_heuristicPathsEnabled) 
        return getKShortestRoutes(sender, receiver, k);

    if (_pathsMap.empty()) {
        cout << "path Map uninitialized" << endl;
        throw std::exception();
    }

    if (_pathsMap.count(sender) == 0) {
        cout << " sender " << sender << " has no paths at all " << endl;
        throw std::exception();
    }

    if (_pathsMap[sender].count(receiver) == 0) {
        cout << " sender " << sender << " has no paths to receiver " << receiver << endl;
        throw std::exception();
    }
    
    vector<vector<int>> finalPaths;
    int numPaths = 0;
    double sumRTT = 0;
    for (vector<int> path : _pathsMap[sender][receiver]) {
        if (numPaths >= k)
            break;
        numPaths++;
        finalPaths.push_back(path);
        updateMaxTravelTime(path);
        updateCannonicalRTT(path);
    }

    return finalPaths;
}

// get the next path after the kth one when changing paths
tuple<int, vector<int>> getNextPath(int sender, int receiver, int k) {
    if (!_widestPathsEnabled && !_kspYenEnabled && !_obliviousRoutingEnabled && !_heuristicPathsEnabled) {
        cout << "no path Map" << endl;
        throw std::exception();
    }

    if (_pathsMap.empty()) {
        cout << "path Map uninitialized" << endl;
        throw std::exception();
    }

    if (_pathsMap.count(sender) == 0) {
        cout << " sender " << sender << " has no paths at all " << endl;
        throw std::exception();
    }

    if (_pathsMap[sender].count(receiver) == 0) {
        cout << " sender " << sender << " has no paths to receiver " << receiver << endl;
        throw std::exception();
    }
        
    if (_pathsMap[sender][receiver].size() >= k + 2) 
        return make_tuple(k + 1, _pathsMap[sender][receiver][k + 1]);
    else 
        return make_tuple(0, _pathsMap[sender][receiver][0]);
}

bool vectorContains(vector<int> smallVector, vector<vector<int>> bigVector) {
    for (auto v : bigVector) {
        if (v == smallVector)
            return true;
    }
    return false;
}

vector<vector<int>> getKShortestRoutesLandmarkRouting(int sender, int receiver, int k){
    int landmark;
    vector<int> pathSenderToLandmark;
    vector<int> pathLandmarkToReceiver;
    vector<vector<int>> kRoutes = {};
    int numPaths = minInt(_landmarksWithConnectivityList.size(), k);
    for (int i=0; i< numPaths; i++){
        landmark = get<1>(_landmarksWithConnectivityList[i]);
        pathSenderToLandmark = breadthFirstSearch(sender, landmark); //use breadth first search
        pathLandmarkToReceiver = breadthFirstSearch(landmark, receiver); //use breadth first search
			
	pathSenderToLandmark.insert(pathSenderToLandmark.end(), 
                pathLandmarkToReceiver.begin() + 1, pathLandmarkToReceiver.end());
        if ((pathSenderToLandmark.size() < 2 ||  pathLandmarkToReceiver.size() < 2 || 
                    vectorContains(pathSenderToLandmark, kRoutes))) { 
            if (numPaths < _landmarksWithConnectivityList.size()) {
                numPaths++;
            }
        } else {
            kRoutes.push_back(pathSenderToLandmark);
        } 
    }
    return kRoutes;
}



vector<int> breadthFirstSearchByGraph(int sender, int receiver, unordered_map<int, set<int>> graph){
    //TODO: fix, and add to header
    deque<vector<int>> nodesToVisit = {};
    bool visitedNodes[_numNodes];
    for (int i=0; i<_numNodes; i++){
        visitedNodes[i] =false;
    }
    visitedNodes[sender] = true;

    vector<int> temp;
    temp.push_back(sender);
    nodesToVisit.push_back(temp);

    while ((int) nodesToVisit.size() > 0){
        vector<int> current = nodesToVisit[0];
        nodesToVisit.pop_front();
        int lastNode = current.back();
        for (auto iter = graph[lastNode].begin(); iter != graph[lastNode].end(); iter++){
            int thisNode = *iter;
            if (!visitedNodes[thisNode]){
                temp = current; // assignment copies in case of vector
                temp.push_back(thisNode);
                nodesToVisit.push_back(temp);
                visitedNodes[thisNode] = true;
                if (thisNode == receiver)
                    return temp;
            } 
        }
    }
    vector<int> empty = {};
    return empty;
}


vector<int> breadthFirstSearch(int sender, int receiver){
    deque<vector<int>> nodesToVisit;
    bool visitedNodes[_numNodes];
    for (int i=0; i<_numNodes; i++){
        visitedNodes[i] =false;
    }
    visitedNodes[sender] = true;

    vector<int> temp;
    temp.push_back(sender);
    nodesToVisit.push_back(temp);

    while ((int)nodesToVisit.size()>0){
        vector<int> current = nodesToVisit[0];
        nodesToVisit.pop_front();
        int lastNode = current.back();
        for (int i=0; i<(int)_channels[lastNode].size();i++){
            if (!visitedNodes[_channels[lastNode][i].first]){
                temp = current; // assignment copies in case of vector
                temp.push_back(_channels[lastNode][i].first);
                nodesToVisit.push_back(temp);
                visitedNodes[_channels[lastNode][i].first] = true;
                if (_channels[lastNode][i].first==receiver)
                    return temp;
            } //end if (!visitedNodes[_channels[lastNode][i]])
        }//end for (i)
    }//end while
    vector<int> empty = {};
    return empty;
}

template <class T,class S> struct pair_equal_to : binary_function <T,pair<T,S>,bool> {
    bool operator() (const T& y, const pair<T,S>& x) const
    {
        return x.first==y;
    }
};

/* removeRoute - function used to remove paths found to get k shortest disjoint paths
 */
unordered_map<int, vector<pair<int,int>>> removeRoute( unordered_map<int, vector<pair<int,int>>> channels, vector<int> route){
    int start, end;
    for (int i=0; i< (route.size() -1); i++){
        start = route[i];
        end = route[i+1];
        //only erase if edge is between two router nodes
        if (start >= _numHostNodes && end >= _numHostNodes) {
            vector< pair <int, int> >::iterator it = find_if(channels[start].begin(),channels[start].end(),
                    bind1st(pair_equal_to<int,int>(),end));
            channels[start].erase(it);
        }
    }
    return channels;
}
int minInt(int x, int y){
    if (x< y) return x;
    return y;
}
/* split - same as split function in python, convert string into vector of strings using delimiter
 */
vector<string> split(string str, char delimiter){
    vector<string> internal;
    stringstream ss(str); // Turn the string into a stream.
    string tok;
    while(getline(ss, tok, delimiter)) {
        internal.push_back(tok);
    }
    return internal;
}


/*  A utility function to find the vertex with minimum distance
 * value, from the set of vertices not yet included in shortest path tree
 */
int minDistance(int dist[],
        bool sptSet[])
{

    // Initialize min value
    int min = INT_MAX;
    int min_index = -1;

    for (int v = 0; v < _numNodes; v++)
        if (sptSet[v] == false &&
                dist[v] <= min)
            min = dist[v], min_index = v;

    if (min == INT_MAX){
        return -1;
    }
    else{
        return min_index;
    }
}

// Function to print shortest
// path from source to j
// using parent array
void printPath(int parent[], int j)
{

    // Base Case : If j is source
    if (parent[j] == - 1)
        return;

    printPath(parent, parent[j]);

    printf("%d ", j);
}


vector<int> getPath(int parent[], int j)
{
    vector<int> result = {};
    // Base Case : If j is source
    if (parent[j] == - 1){
        result.push_back(j);
        return result;
    }
    else if (j == -2){
        vector<int> empty = {};
        return empty;

    }

    result = getPath(parent, parent[j]);
    result.push_back(j);
    return result;
}

// A utility function to print
// the constructed distance
// array
void printSolution(int dist[], int source,
        int parent[])
{
    int src = source;
    printf("Vertex\t Distance\tPath");
    for (int i = 0; i < _numNodes; i++)
    {
        printf("\n%d -> %d \t\t %d\t\t%d ",
                src, i, dist[i], src);
        printPath(parent, i);

        printf("\n getResultSolution: \n");
        vector<int> resultVect = getPath(parent, i);
        for (int i =0; i<resultVect.size(); i++){
            printf("%i, ", resultVect[i]);
        }
    }
    cout << "end print solution " << endl;
}

vector<int> dijkstraInputGraph(int src,  int dest, unordered_map<int, vector<pair<int,int>>> channels){
    // The output array. dist[i] will hold the shortest distance from src to i
    int dist[_numNodes];

    // sptSet[i] will true if vertex i is included / in shortest path tree or shortest distance from src to i is finalized
    bool sptSet[_numNodes];

    // Parent array to store shortest path tree
    int parent[_numNodes];

    // Initialize all distances as INFINITE and stpSet[] as false
    for (int i = 0; i < _numNodes; i++)
    {
        dist[i] = INT_MAX;
        parent[i] = -2;
        sptSet[i] = false;
    }

    // Parent of source is -1 (used for identifying source later) 
    parent[src] = -1;
    // Distance of source vertex from itself is always 0
    dist[src] = 0;

    // Find shortest path for all vertices
    for (int count = 0; count < _numNodes - 1; count++)
    {
        // Pick the minimum distance vertex from the set of vertices not yet processed.
        // u is always equal to src in first iteration.
        int u = minDistance(dist, sptSet);
        if (u == -1){
            vector<int> empty = {};
            return empty;
        }

        // Mark the picked vertex as processed
        sptSet[u] = true;

        vector<pair<int,int>>::iterator vectIter;
        // Update dist value of the adjacent vertices of the picked vertex.
        for (vectIter = channels[u].begin(); vectIter != channels[u].end(); vectIter++){

            // Update dist[v] only if is not in sptSet, there is an edge from u to v, and
            // total weight of path from src to v through u is smaller than current value of dist[v]
            if (!sptSet[vectIter->first]){
                if(dist[u] + (vectIter->second) < dist[vectIter->first]){
                    parent[vectIter->first] = u;
                    dist[vectIter->first] = dist[u] + vectIter->second;

                }
            }
        }
    }
    return getPath(parent, dest);
}

void dijkstraInputGraphTemp(int src,  int dest, unordered_map<int, vector<pair<int,int>>> channels){
    // The output array. dist[i] will hold the shortest distance from src to i
    int dist[_numNodes];

    // sptSet[i] will true if vertex i is included / in shortest path tree or shortest distance from src to i is finalized
    bool sptSet[_numNodes];

    // Parent array to store shortest path tree
    int parent[_numNodes];

    // Initialize all distances as INFINITE and stpSet[] as false
    for (int i = 0; i < _numNodes; i++)
    {
        parent[src] = -1;
        parent[i] = -2;
        dist[i] = INT_MAX;
        sptSet[i] = false;
    }

    // Distance of source vertex from itself is always 0
    dist[src] = 0;

    // Find shortest path for all vertices
    for (int count = 0; count < _numNodes - 1; count++)
    {
        // Pick the minimum distance vertex from the set of vertices not yet processed.
        // u is always equal to src in first iteration.
        int u = minDistance(dist, sptSet);
        if (u==-1){
            vector<int> empty = {};
            return ;

        }

        // Mark the picked vertex as processed
        sptSet[u] = true;

        vector<pair<int,int>>::iterator vectIter;
        // Update dist value of the adjacent vertices of the picked vertex.
        for (vectIter = channels[u].begin(); vectIter != channels[u].end(); vectIter++){

            /*for (int ka=0; ka<_numNodes; ka++){
                printf("[%i]: %i,  ", ka, parent[ka] );

            }*/
            // Update dist[v] only if is not in sptSet, there is an edge from u to v, and
            // total weight of path from src to v through u is smaller than current value of dist[v]

            // find first element with first == 42
            if (!sptSet[vectIter->first]){
                if(dist[u] + (vectIter->second) < dist[vectIter->first]){
                    parent[vectIter->first] = u;
                    dist[vectIter->first] = dist[u] + vectIter->second;

                }
            }
        }
    }

    // print the constructed
    // distance array
    /*for (int ka=0; ka<_numNodes; ka++)
        printf("[%i]: %i,  ", ka, parent[ka] );*/
    return;// getPath(parent, dest);
}
// Function that implements Dijkstra's  single source shortest path algorithm for a graph represented
// using adjacency matrix representation
vector<int> dijkstra(int src,  int dest)
{

    // The output array. dist[i] will hold the shortest distance from src to i
    int dist[_numNodes];

    // sptSet[i] will true if vertex i is included / in shortest path tree or shortest distance from src to i is finalized
    bool sptSet[_numNodes];

    // Parent array to store shortest path tree
    int parent[_numNodes];

    // Initialize all distances as INFINITE and stpSet[] as false
    for (int i = 0; i < _numNodes; i++)
    {
        parent[src] = -1;
        dist[i] = INT_MAX;
        sptSet[i] = false;
    }

    // Distance of source vertex from itself is always 0
    dist[src] = 0;

    // Find shortest path for all vertices
    for (int count = 0; count < _numNodes - 1; count++)
    {
        // Pick the minimum distance vertex from the set of vertices not yet processed.
        // u is always equal to src in first iteration.
        int u = minDistance(dist, sptSet);

        // Mark the picked vertex as processed
        sptSet[u] = true;

        vector<pair<int,int>>::iterator vectIter;
        // Update dist value of the adjacent vertices of the picked vertex.
        for (vectIter = _channels[u].begin(); vectIter != _channels[u].end(); vectIter++){

            // Update dist[v] only if is not in sptSet, there is an edge from u to v, and
            // total weight of path from src to v through u is smaller than current value of dist[v]

            // find first element with first == 42
            //= find_if(channels[u].begin(),channels[u].end(), CompareFirst(v));
            if (!sptSet[vectIter->first]){
                //if (vectIter != channels[u].end() ){
                if(dist[u] + (vectIter->second) < dist[vectIter->first]){
                    parent[vectIter->first] = u;
                    dist[vectIter->first] = dist[u] + vectIter->second;
                    //  }

            }
            }
        }
    }
    return getPath(parent, dest);
}

bool sortHighToLowConnectivity(tuple<int,int> x, tuple<int,int> y){
    if (get<0>(x) > get<0>(y)) 
        return true;
    else if (get<0>(x) < get<0>(y)) 
        return false;
    else
        return get<1>(x) < get<1>(y);
}

/*
 * sortFunction - helper function used to sort queued transUnit list by ascending priorityClass, then by
 *      ascending amount
 *      note: largest element gets accessed first
 */
bool sortPriorityThenAmtFunction(const tuple<int,double, routerMsg*, Id> &a,
        const tuple<int,double, routerMsg*, Id> &b)
{
    if (get<0>(a) < get<0>(b)){
        return false;
    }
    else if (get<0>(a) == get<0>(b)){
        return (get<1>(a) > get<1>(b));
    }
    return true;
}


double minVectorElemDouble(vector<double> v){
    double min = v[0];
    for (double d: v){
        if (d < min)
            min=d;
    }
    return min;
}



double maxDouble(double x, double y){
    if (x>y) return x;
    return y;
}


void printChannels(){
    printf("print of channels\n" );
    for (auto i : _channels){
        printf("key: %d [",i.first);
        for (auto k: i.second){
            printf("(%d, %d) ",get<0>(k), get<1>(k));
        }
        printf("] \n");
    }
    cout<<endl;
}


void printVector(vector<int> v){
    for (auto temp : v) {
        cout << temp << ", ";
    }
    cout << endl;
}

void printVectorReverse(vector<int> v){
    for (auto it = v.rbegin(); it != v.rend(); ++it) {
        cout << *it << ", ";
    }
    cout << endl;
}



/*
 * sortFunction - helper function used to sort queued transUnit list by ascending priorityClass, then by
 *      ascending amount
 *      note: largest element gets accessed first
 */
bool sortPriorityThenAmtFunction(const tuple<int,double, routerMsg*, Id, simtime_t> &a,
      const tuple<int,double, routerMsg*, Id, simtime_t> &b)
{
   if (get<0>(a) < get<0>(b)){
      return false;
   }
   else if (get<0>(a) == get<0>(b)){
      return (get<1>(a) > get<1>(b));
   }
   return true;
}



/*
 * sortFunction - to do FIFO sorting 
 */
bool sortFIFO(const tuple<int,double, routerMsg*, Id, simtime_t> &a,
      const tuple<int,double, routerMsg*, Id, simtime_t> &b)
{
    return (get<4>(a).dbl() < get<4>(b).dbl());
}

/*
 * sortFunction - to do LIFO sorting 
 */
bool sortLIFO(const tuple<int,double, routerMsg*, Id, simtime_t> &a,
      const tuple<int,double, routerMsg*, Id, simtime_t> &b)
{
    return (get<4>(a).dbl() > get<4>(b).dbl());
}

/*
 * sortFunction - to do shortest payment first sorting 
 */
bool sortSPF(const tuple<int,double, routerMsg*, Id, simtime_t> &a,
      const tuple<int,double, routerMsg*, Id, simtime_t> &b)
{
    transactionMsg *transA = check_and_cast<transactionMsg *>((get<2>(a))->getEncapsulatedPacket());
    transactionMsg *transB = check_and_cast<transactionMsg *>((get<2>(b))->getEncapsulatedPacket());
    
    SplitState splitInfoA = _numSplits[transA->getSender()][transA->getLargerTxnId()];
    SplitState splitInfoB = _numSplits[transB->getSender()][transB->getLargerTxnId()];

    if (splitInfoA.totalAmount != splitInfoB.totalAmount)
        return splitInfoA.totalAmount < splitInfoB.totalAmount;
    else
        return (get<4>(a).dbl() > get<4>(b).dbl());
}

/*
 * sortFunction - to do earliest deadline first sorting 
 */
bool sortEDF(const tuple<int,double, routerMsg*, Id, simtime_t> &a,
      const tuple<int,double, routerMsg*, Id, simtime_t> &b)
{
    transactionMsg *transA = check_and_cast<transactionMsg *>((get<2>(a))->getEncapsulatedPacket());
    transactionMsg *transB = check_and_cast<transactionMsg *>((get<2>(b))->getEncapsulatedPacket());
    
    return (transA->getTimeSent() < transB->getTimeSent());
}

#endif

#############################################################################################
/home/kush/Desktop/spiderproj/spider_omnet/hostNodeBase.cc

#include "hostNodeBase.h"
#include <queue>

#define MSGSIZE 100
#define MAX_SENDER_PER_DEST_QUEUE 15000

//global parameters
unordered_map<int, priority_queue<TransUnit, vector<TransUnit>, LaterTransUnit>> _transUnitList;
unordered_map<double, int> _transactionCompletionBySize;
unordered_map<double, int> _transactionArrivalBySize;
unordered_map<double, double> _txnAvgCompTimeBySize;
unordered_map<double, vector<double>> _txnTailCompTimesBySize;
ofstream _succRetriesFile, _failRetriesFile;
ofstream _tailCompBySizeFile;
unordered_map<int, set<int>> _destList;
unordered_map<int, unordered_map<double, SplitState>> _numSplits;
unordered_map<int, unordered_map<int, vector<vector<int>>>> _pathsMap;
int _numNodes;
int _numRouterNodes;
int _numHostNodes;
double _maxTravelTime;
double _maxOneHopDelay;
double _statRate;
double _clearRate;
int _kValue;
double _simulationLength;
int _serviceArrivalWindow; 

double _transStatStart;
double  _transStatEnd;
double _waterfillingStartTime;
double _landmarkRoutingStartTime;
double _shortestPathStartTime;
double _shortestPathEndTime;
double _splitSize;
double _bank;
double _percentile;

 //adjacency list format of graph edges of network
unordered_map<int, vector<pair<int,int>>> _channels;

//unordered_map of balances for each edge; key = <int,int> is <source, destination>
unordered_map<tuple<int,int>,double, hashId> _balances;

unordered_map<tuple<int,int>,double, hashId> _capacities;

// controls algorithm and what is outputted
bool _waterfillingEnabled;
bool _dctcpEnabled;
bool _timeoutEnabled;
bool _loggingEnabled;
bool _signalsEnabled;
bool _priceSchemeEnabled;
bool _landmarkRoutingEnabled;
bool _windowEnabled;
bool _lndBaselineEnabled;
bool _splittingEnabled;
bool _celerEnabled;


bool _widestPathsEnabled;
bool _heuristicPathsEnabled;
bool _obliviousRoutingEnabled;
bool _kspYenEnabled;

// scheduling algorithms
bool _LIFOEnabled;
bool _FIFOEnabled;
bool _RREnabled;
bool _SPFEnabled;
bool _EDFEnabled;
bool (*_schedulingAlgorithm) (const tuple<int,double, routerMsg*, Id, simtime_t> &a,
      const tuple<int,double, routerMsg*, Id, simtime_t> &b);

// for all precision errors
double _epsilon; 


//global parameters for fixed size queues
bool _hasQueueCapacity;
int _queueCapacity;

// rebalancing related
bool _rebalancingEnabled;
double _rebalancingUpFactor;
double _queueDelayThreshold;
double _gamma;
double _maxGammaImbalanceQueueSize;
double _delayForAddingFunds;
double _rebalanceRate;
double _computeBalanceRate;

Define_Module(hostNodeBase);

void hostNodeBase::setIndex(int index) {
    this->index = index;
}

int hostNodeBase::myIndex() {
    return index;
}

/* print channel information */
bool hostNodeBase:: printNodeToPaymentChannel(){
    bool invalidKey = false;
    printf("print of channels\n" );
    for (auto i : nodeToPaymentChannel){
       printf("(key: %d )",i.first);
       if (i.first<0) invalidKey = true;
    }
    cout<<endl;
    return !invalidKey;
}

/* print any vector */
void printVectorDouble(vector<double> v){
    for (auto temp : v){
        cout << temp << ", ";
    }
    cout << endl;
}

/* get total amount on queue to node x */
double hostNodeBase::getTotalAmount(int x) {
    if (_hasQueueCapacity && _queueCapacity == 0)
        return 0;
    return nodeToPaymentChannel[x].totalAmtInQueue;
} 

/* get total amount inflight incoming node x */
double hostNodeBase::getTotalAmountIncomingInflight(int x) {
    return nodeToPaymentChannel[x].totalAmtIncomingInflight;
} 

/* get total amount inflight outgoing node x */
double hostNodeBase::getTotalAmountOutgoingInflight(int x) {
    return nodeToPaymentChannel[x].totalAmtOutgoingInflight;
} 


/* samples a random number (index) of the passed in vector
 *  based on the actual probabilities passed in
 */
int hostNodeBase::sampleFromDistribution(vector<double> probabilities) {
    vector<double> cumProbabilities { 0 };

    double sumProbabilities = accumulate(probabilities.begin(), probabilities.end(), 0.0); 
    assert(sumProbabilities <= 1.0);
    
    // compute cumulative probabilities
    for (int i = 0; i < probabilities.size(); i++) {
        cumProbabilities.push_back(probabilities[i] + cumProbabilities[i]);
    }

    // generate the next index to send on based on these probabilities
    double value  = (rand() / double(RAND_MAX));
    int index;
    for (int i = 1; i < cumProbabilities.size(); i++) {
        if (value < cumProbabilities[i])
            return i - 1;
    }
    return 0;
}

/* helper function to push a transaction into the appropriate sender queue 
 * in order 
 */
void hostNodeBase::pushIntoSenderQueue(DestInfo* destInfo, routerMsg* ttmsg) {
    multiset<routerMsg*, transCompare> *senderQ = &(destInfo->transWaitingToBeSent);
    senderQ->insert(ttmsg);
    auto position = senderQ->end();
    if (senderQ->size() > MAX_SENDER_PER_DEST_QUEUE) {
        position--;
        routerMsg* lastMsg = *position;
        senderQ->erase(position);
        deleteTransaction(lastMsg);
    }
}

/* helper function to delete router message and encapsulated transaction message
 */
void hostNodeBase::deleteTransaction(routerMsg* ttmsg) {
    transactionMsg *transMsg = check_and_cast<transactionMsg *>(ttmsg->getEncapsulatedPacket());
    int destination = transMsg->getReceiver();
    statNumTimedOut[destination] += 1;
    ttmsg->decapsulate();
    delete transMsg;
    delete ttmsg;
}

/* generate next Transaction to be processed at this node 
 * this is an optimization to prevent all txns from being loaded initially
 */
void hostNodeBase::generateNextTransaction() {
      if (_transUnitList[myIndex()].empty())
          return;
      TransUnit j = _transUnitList[myIndex()].top();
      double timeSent = j.timeSent;
      
      routerMsg *msg = generateTransactionMessage(j);

      // override default shortest path 
      if (_waterfillingEnabled || _priceSchemeEnabled || _landmarkRoutingEnabled || _lndBaselineEnabled 
              || _dctcpEnabled || _celerEnabled){
         vector<int> blankPath = {};
         msg->setRoute(blankPath);
      }

      scheduleAt(timeSent, msg);

      if (_timeoutEnabled && j.hasTimeOut){
         routerMsg *toutMsg = generateTimeOutMessage(msg);
         scheduleAt(timeSent + j.timeOut, toutMsg );
      }
      _transUnitList[myIndex()].pop();
}




/* register a signal per destination for this path of the particular type passed in
 * and return the signal created
 */
simsignal_t hostNodeBase::registerSignalPerDestPath(string signalStart, int pathIdx, int destNode) {
    string signalPrefix = signalStart + "PerDestPerPath";
    char signalName[64];
    string templateString = signalPrefix + "Template";
    
    if (destNode < _numHostNodes){
        sprintf(signalName, "%s_%d(host %d)", signalPrefix.c_str(), pathIdx, destNode);
    } else{
        sprintf(signalName, "%s_%d(router %d [%d] )", signalPrefix.c_str(),
             pathIdx, destNode - _numHostNodes, destNode);
    }
    simsignal_t signal = registerSignal(signalName);
    cProperty *statisticTemplate = getProperties()->get("statisticTemplate", 
            templateString.c_str());
    getEnvir()->addResultRecorders(this, signal, signalName,  statisticTemplate);
    return signal;
}



/* register a signal per destination for this path of the particular type passed in
 * and return the signal created
 */
simsignal_t hostNodeBase::registerSignalPerChannelPerDest(string signalStart, int chnlEndNode, int destNode) {
    string signalPrefix = signalStart + "PerChannelPerDest";
    char signalName[64];
    string templateString = signalPrefix + "Template";

    if (chnlEndNode < _numHostNodes){
        sprintf(signalName, "%s_%d(host %d)", signalPrefix.c_str(), destNode, chnlEndNode);
    } else{
        sprintf(signalName, "%s_%d(router %d [%d] )", signalPrefix.c_str(),
             destNode, chnlEndNode - _numHostNodes, chnlEndNode);
    }
    simsignal_t signal = registerSignal(signalName);
    cProperty *statisticTemplate = getProperties()->get("statisticTemplate",
            templateString.c_str());
    getEnvir()->addResultRecorders(this, signal, signalName,  statisticTemplate);
    return signal;
}


/* register a signal per channel of the particular type passed in
 * and return the signal created
 */
simsignal_t hostNodeBase::registerSignalPerChannel(string signalStart, int id) {
    string signalPrefix = signalStart + "PerChannel";
    char signalName[64];
    string templateString = signalPrefix + "Template";

    if (id < _numHostNodes){
        sprintf(signalName, "%s(host %d)", signalPrefix.c_str(), id);
    } else{
        sprintf(signalName, "%s(router %d [%d] )", signalPrefix.c_str(),
                id - _numHostNodes, id);
    }
    simsignal_t signal = registerSignal(signalName);
    cProperty *statisticTemplate = getProperties()->get("statisticTemplate", 
            templateString.c_str());
    getEnvir()->addResultRecorders(this, signal, signalName,  statisticTemplate);
    return signal;
}


/* register a signal per dest of the particular type passed in
 * and return the signal created
 */
simsignal_t hostNodeBase::registerSignalPerDest(string signalStart, int destNode, string suffix) {
    string signalPrefix = signalStart + "PerDest" + suffix;
    char signalName[64];
    string templateString = signalStart + "PerDestTemplate"; 

    sprintf(signalName, "%s(host node %d)", signalPrefix.c_str(), destNode);  
    simsignal_t signal = registerSignal(signalName);
    cProperty *statisticTemplate = getProperties()->get("statisticTemplate", 
            templateString.c_str());
    getEnvir()->addResultRecorders(this, signal, signalName,  statisticTemplate);

    return signal;
}

/* helper function to record the tail completion times for every algorithm
 * called by each algorithm wheever it computes its completion time
 */
void hostNodeBase::recordTailCompletionTime(simtime_t timeSent, double amount, double completionTime){
    if (timeSent >= _transStatStart && timeSent <= _transStatEnd) {
        _txnTailCompTimesBySize[amount].push_back(completionTime);
        if (_txnTailCompTimesBySize[amount].size() == 1000) {
            _tailCompBySizeFile << amount << ": ";
            for (auto const& time : _txnTailCompTimesBySize[amount]) 
                _tailCompBySizeFile << time << " ";
            _tailCompBySizeFile << endl;
            _tailCompBySizeFile.flush();
            _txnTailCompTimesBySize[amount].clear();
        }
    }
}

/****** MESSAGE GENERATORS **********/

/* responsible for generating one HTLC for a particular path 
 * for any algorithm  after the path has been decided by 
 * some function that does splitTransaction
 */
routerMsg* hostNodeBase::generateTransactionMessageForPath(double amt, 
        vector<int> path, int pathIndex, transactionMsg* transMsg) {
    char msgname[MSGSIZE];
    sprintf(msgname, "tic-%d-to-%d split-transMsg", myIndex(), transMsg->getReceiver());
    
    transactionMsg *msg = new transactionMsg(msgname);
    msg->setAmount(amt);
    msg->setTimeSent(transMsg->getTimeSent());
    msg->setSender(transMsg->getSender());
    msg->setReceiver(transMsg->getReceiver());
    msg->setPriorityClass(transMsg->getPriorityClass());
    msg->setHasTimeOut(transMsg->getHasTimeOut());
    msg->setPathIndex(pathIndex);
    msg->setTimeOut(transMsg->getTimeOut());
    msg->setTransactionId(transMsg->getTransactionId());
    msg->setLargerTxnId(transMsg->getLargerTxnId());
    msg->setIsMarked(false);

    // find htlc for txn
    int transactionId = transMsg->getTransactionId();    
    int htlcIndex = 0;
    if (transactionIdToNumHtlc.count(transactionId) == 0) {
        transactionIdToNumHtlc[transactionId] = 1;
    }
    else {
        htlcIndex =  transactionIdToNumHtlc[transactionId];
        transactionIdToNumHtlc[transactionId] = transactionIdToNumHtlc[transactionId] + 1;
    }
    msg->setHtlcIndex(htlcIndex);

    // routerMsg on the outside
    sprintf(msgname, "tic-%d-to-%d split-routerTransMsg", myIndex(), transMsg->getReceiver());
    routerMsg *rMsg = new routerMsg(msgname);
    rMsg->setRoute(path);
    rMsg->setHopCount(0);
    rMsg->setMessageType(TRANSACTION_MSG);
    rMsg->encapsulate(msg);
    return rMsg;

} 

/* Main function responsible for using TransUnit object and 
 * returning corresponding routerMsg message with encapsulated transactionMsg inside.
 *      note: calls get_route function to get route from sender to receiver
 */
routerMsg *hostNodeBase::generateTransactionMessage(TransUnit unit) {
    char msgname[MSGSIZE];
    sprintf(msgname, "tic-%d-to-%d transactionMsg", unit.sender, unit.receiver);
    
    transactionMsg *msg = new transactionMsg(msgname);
    msg->setAmount(unit.amount);
    msg->setTimeSent(unit.timeSent);
    msg->setSender(unit.sender);
    msg->setReceiver(unit.receiver);
    msg->setPriorityClass(unit.priorityClass);
    msg->setTransactionId(msg->getId());
    msg->setHtlcIndex(0);
    msg->setHasTimeOut(unit.hasTimeOut);
    msg->setTimeOut(unit.timeOut);
    msg->setLargerTxnId(unit.largerTxnId);
    msg->setIsMarked(false);
    
    sprintf(msgname, "tic-%d-to-%d router-transaction-Msg %f", unit.sender, unit.receiver, unit.timeSent);
    
    routerMsg *rMsg = new routerMsg(msgname);
    // compute route only once
    if (destNodeToPath.count(unit.receiver) == 0){
        vector<int> route;
        if (_celerEnabled)
            route = {};
        else
            route = getRoute(unit.sender,unit.receiver);
       destNodeToPath[unit.receiver] = route;
       rMsg->setRoute(route);
    }
    else{
       rMsg->setRoute(destNodeToPath[unit.receiver]);
    }
    rMsg->setHopCount(0);
    rMsg->setMessageType(TRANSACTION_MSG);
    rMsg->encapsulate(msg);
    return rMsg;
}

/* Generates a duplicate transaction message using an ack for the same transaction message
 * useful if you have to duplicate a transaction from the ackMessage 
*/
routerMsg *hostNodeBase::generateDuplicateTransactionMessage(ackMsg* aMsg) {
   char msgname[MSGSIZE];
   int sender = myIndex();
   int receiver = aMsg->getReceiver();
   sprintf(msgname, "tic-%d-to-%d transactionMsg", sender, receiver);
   
   transactionMsg *msg = new transactionMsg(msgname);
   msg->setAmount(aMsg->getAmount());
   msg->setTimeSent(aMsg->getTimeSent());
   msg->setSender(sender);
   msg->setReceiver(receiver);
   msg->setPriorityClass(aMsg->getPriorityClass());
   msg->setTransactionId(aMsg->getTransactionId());
   msg->setHtlcIndex(aMsg->getHtlcIndex());
   msg->setHasTimeOut(aMsg->getHasTimeOut());
   msg->setTimeOut(aMsg->getTimeOut());
   msg->setLargerTxnId(aMsg->getLargerTxnId());
   
   sprintf(msgname, "tic-%d-to-%d router-transaction-Msg %f", sender, receiver, aMsg->getTimeSent());
   
   routerMsg *rMsg = new routerMsg(msgname);
   rMsg->setHopCount(0);
   rMsg->setMessageType(TRANSACTION_MSG);
   rMsg->encapsulate(msg);
   return rMsg;
}

/* called only when a transactionMsg reaches end of its path to mark
 * the acknowledgement and receipt of the transaction at the receiver,
 * we assume no delay in procuring the key and that the receiver 
 * immediately starts sending an ack in the opposite direction that
 * unlocks funds along the reverse path
 * isSuccess denotes whether the ack is in response to a transaction
 * that succeeded or failed.
 */
routerMsg *hostNodeBase::generateAckMessage(routerMsg* ttmsg, bool isSuccess) {
    int sender = (ttmsg->getRoute())[0];
    int receiver = (ttmsg->getRoute())[(ttmsg->getRoute()).size() -1];
    vector<int> route = ttmsg->getRoute();

    transactionMsg *transMsg = check_and_cast<transactionMsg *>(ttmsg->getEncapsulatedPacket());
    int transactionId = transMsg->getTransactionId();
    double timeSent = transMsg->getTimeSent();
    double amount = transMsg->getAmount();
    bool hasTimeOut = transMsg->getHasTimeOut();
    
    char msgname[MSGSIZE];
    sprintf(msgname, "receiver-%d-to-sender-%d ackMsg", receiver, sender);
    routerMsg *msg = new routerMsg(msgname);
    ackMsg *aMsg = new ackMsg(msgname);
    aMsg->setTransactionId(transactionId);
    aMsg->setIsSuccess(isSuccess);
    aMsg->setTimeSent(timeSent);
    aMsg->setAmount(amount);
    aMsg->setReceiver(transMsg->getReceiver());
    aMsg->setHasTimeOut(hasTimeOut);
    aMsg->setHtlcIndex(transMsg->getHtlcIndex());
    aMsg->setPathIndex(transMsg->getPathIndex());
    aMsg->setLargerTxnId(transMsg->getLargerTxnId());
    aMsg->setPriorityClass(transMsg->getPriorityClass());
    aMsg->setTimeOut(transMsg->getTimeOut());
    aMsg->setTimeAttempted(transMsg->getTimeAttempted());
    aMsg->setIsMarked(transMsg->getIsMarked());
    if (!isSuccess){
        aMsg->setFailedHopNum((route.size()-1) - ttmsg->getHopCount());
    }

    //no need to set secret - not modelled
    reverse(route.begin(), route.end());
    msg->setRoute(route);

    //need to reverse path from current hop number in case of partial failure
    msg->setHopCount((route.size()-1) - ttmsg->getHopCount());

    msg->setMessageType(ACK_MSG); 
    ttmsg->decapsulate();
    delete transMsg;
    delete ttmsg;
    msg->encapsulate(aMsg);
    return msg;
}



/* generates messages responsible for recognizing that a txn is complete
 * and funds have been securely transferred from a previous node to a 
 * neighboring node after the ack/secret has been received
 * Always goes only one hop, no more
 */
routerMsg *hostNodeBase::generateUpdateMessage(int transId, 
        int receiver, double amount, int htlcIndex){
    char msgname[MSGSIZE];
    sprintf(msgname, "tic-%d-to-%d updateMsg", myIndex(), receiver);
    
    routerMsg *rMsg = new routerMsg(msgname);
    vector<int> route={myIndex(),receiver};
    rMsg->setRoute(route);
    rMsg->setHopCount(0);
    rMsg->setMessageType(UPDATE_MSG);

    updateMsg *uMsg = new updateMsg(msgname);
    uMsg->setAmount(amount);
    uMsg->setTransactionId(transId);
    uMsg->setHtlcIndex(htlcIndex);
    rMsg->encapsulate(uMsg);
    return rMsg;
}

/* generate statistic trigger message every x seconds
 * to output all statistics which can then be plotted
 */
routerMsg *hostNodeBase::generateTriggerRebalancingMessage(){
    char msgname[MSGSIZE];
    sprintf(msgname, "node-%d rebalancingMsg", myIndex());
    routerMsg *rMsg = new routerMsg(msgname);
    rMsg->setMessageType(TRIGGER_REBALANCING_MSG);
    return rMsg;
}


/* generate statistic trigger message every x seconds
 * to output all statistics which can then be plotted
 */
routerMsg *hostNodeBase::generateStatMessage(){
    char msgname[MSGSIZE];
    sprintf(msgname, "node-%d statMsg", myIndex());
    routerMsg *rMsg = new routerMsg(msgname);
    rMsg->setMessageType(STAT_MSG);
    return rMsg;
}

/* generate message trigger t generate balances for all the payment channels
 */
routerMsg *hostNodeBase::generateComputeMinBalanceMessage(){
    char msgname[MSGSIZE];
    sprintf(msgname, "node-%d computeMinBalanceMsg", myIndex());
    routerMsg *rMsg = new routerMsg(msgname);
    rMsg->setMessageType(COMPUTE_BALANCE_MSG);
    return rMsg;
}

/* generate a periodic message to remove
 * any state pertaining to transactions that have 
 * timed out
 */
routerMsg *hostNodeBase::generateClearStateMessage(){
    char msgname[MSGSIZE];
    sprintf(msgname, "node-%d clearStateMsg", myIndex());
    routerMsg *rMsg = new routerMsg(msgname);
    rMsg->setMessageType(CLEAR_STATE_MSG);
    return rMsg;
}

/* special type of time out message for waterfilling, etd. designed for a specific path so that
 * such messages will be sent on all paths considered for waterfilling
 */
routerMsg* hostNodeBase::generateTimeOutMessageForPath(vector<int> path, 
        int transactionId, int receiver){
    char msgname[MSGSIZE];
    sprintf(msgname, "tic-%d-to-%d path-timeOutMsg", myIndex(), receiver);
    timeOutMsg *msg = new timeOutMsg(msgname);

    msg->setReceiver(receiver);
    msg->setTransactionId(transactionId);

    sprintf(msgname, "tic-%d-to-%d path-router-timeOutMsg", myIndex(), receiver);
    routerMsg *rMsg = new routerMsg(msgname);
    rMsg->setRoute(path);

    rMsg->setHopCount(0);
    rMsg->setMessageType(TIME_OUT_MSG);
    rMsg->encapsulate(msg);
    return rMsg;
}

/* responsible for generating the generic time out message 
 * generated whenever transaction is sent out into the network
 */
routerMsg *hostNodeBase::generateTimeOutMessage(routerMsg* msg) {
    transactionMsg *transMsg = check_and_cast<transactionMsg *>(msg->getEncapsulatedPacket());
    char msgname[MSGSIZE];
    sprintf(msgname, "tic-%d-to-%d timeOutMsg", transMsg->getSender(), transMsg->getReceiver());
   
    timeOutMsg *toutMsg = new timeOutMsg(msgname);
    toutMsg->setTransactionId(transMsg->getTransactionId());
    toutMsg->setReceiver(transMsg->getReceiver());

    sprintf(msgname, "tic-%d-to-%d routerTimeOutMsg(%f)", 
            transMsg->getSender(), transMsg->getReceiver(), transMsg->getTimeSent());
   
    routerMsg *rMsg = new routerMsg(msgname);
    rMsg->setRoute(msg->getRoute());
    rMsg->setHopCount(0);
    rMsg->setMessageType(TIME_OUT_MSG);
    rMsg->encapsulate(toutMsg);
    return rMsg;
}

/* generate a message that designates which payment channels at this router need funds
 * and how much funds they need, will be processed a few seconds/minutes later to 
 * actually add the funds to those payment channels */
routerMsg *hostNodeBase::generateAddFundsMessage(map<int, double> fundsToBeAdded) {
    map<int,double> pcsNeedingFunds = fundsToBeAdded;
    
    char msgname[MSGSIZE];
    sprintf(msgname, "addfundmessage-at-%d", myIndex());
    routerMsg *msg = new routerMsg(msgname);
    addFundsMsg *afMsg = new addFundsMsg(msgname);
    afMsg->setPcsNeedingFunds(pcsNeedingFunds);

    msg->setMessageType(ADD_FUNDS_MSG); 
    msg->encapsulate(afMsg);
    return msg;
}

/***** MESSAGE HANDLERS *****/
/* overall controller for handling messages that dispatches the right function
 * based on message type
 */
void hostNodeBase::handleMessage(cMessage *msg){
    routerMsg *ttmsg = check_and_cast<routerMsg *>(msg);
 
    if (simTime() > _simulationLength){
        auto encapMsg = (ttmsg->getEncapsulatedPacket());
        ttmsg->decapsulate();
        delete ttmsg;
        delete encapMsg;
        return;
    }

    // handle all messges by type
    switch (ttmsg->getMessageType()) {
        case ACK_MSG:
            if (_loggingEnabled) 
                cout << "[HOST "<< myIndex() <<": RECEIVED ACK MSG] " << msg->getName() << endl;
            if (_timeoutEnabled)
                handleAckMessageTimeOut(ttmsg);
            handleAckMessageSpecialized(ttmsg);
            if (_loggingEnabled) cout << "[AFTER HANDLING:]" <<endl;
            break;

        case TRANSACTION_MSG:
            { 
                if (_loggingEnabled) 
                    cout<< "[HOST "<< myIndex() <<": RECEIVED TRANSACTION MSG]  "
                     << msg->getName() <<endl;
             
                transactionMsg *transMsg = 
                    check_and_cast<transactionMsg *>(ttmsg->getEncapsulatedPacket());
                if (transMsg->isSelfMessage() && simTime() == transMsg->getTimeSent()) {
                    generateNextTransaction();
                }
             
                if (_timeoutEnabled && handleTransactionMessageTimeOut(ttmsg)){
                    return;
                }
                handleTransactionMessageSpecialized(ttmsg);
                if (_loggingEnabled) cout << "[AFTER HANDLING:]" << endl;
                break;
            }

        case UPDATE_MSG:
            if (_loggingEnabled) cout<< "[HOST "<< myIndex() 
                <<": RECEIVED UPDATE MSG] "<< msg->getName() << endl;
                handleUpdateMessage(ttmsg);
            if (_loggingEnabled) cout<< "[AFTER HANDLING:]  "<< endl;
            break;

        case STAT_MSG:
            if (_loggingEnabled) cout<< "[HOST "<< myIndex() 
                <<": RECEIVED STAT MSG] "<< msg->getName() << endl;
                handleStatMessage(ttmsg);
            if (_loggingEnabled) cout<< "[AFTER HANDLING:]  "<< endl;
            break;

        case TIME_OUT_MSG:
            if (_loggingEnabled) cout<< "[HOST "<< myIndex() 
                <<": RECEIVED TIME_OUT_MSG] "<< msg->getName() << endl;
       
            if (!_timeoutEnabled){
                cout << "timeout message generated when it shouldn't have" << endl;
                return;
            }
            handleTimeOutMessage(ttmsg);
            if (_loggingEnabled) cout<< "[AFTER HANDLING:]  "<< endl;
            break;
        
        case CLEAR_STATE_MSG:
            if (_loggingEnabled) cout<< "[HOST "<< myIndex() 
                <<": RECEIVED CLEAR_STATE_MSG] "<< msg->getName() << endl;
                handleClearStateMessage(ttmsg);
            if (_loggingEnabled) cout<< "[AFTER HANDLING:]  "<< endl;
            break;
        
        case TRIGGER_REBALANCING_MSG:
            if (_loggingEnabled) cout<< "[HOST "<< myIndex() 
                <<": RECEIVED TRIGGER REBALANCE MSG] "<< msg->getName() << endl;
                handleTriggerRebalancingMessage(ttmsg);
            if (_loggingEnabled) cout<< "[AFTER HANDLING:]  "<< endl;
            break;

        case COMPUTE_BALANCE_MSG:
            if (_loggingEnabled) cout<< "[HOST "<< myIndex() 
                <<": RECEIVED COMPUTE BALANCE MSG] "<< msg->getName() << endl;
                handleComputeMinAvailableBalanceMessage(ttmsg);
            if (_loggingEnabled) cout<< "[AFTER HANDLING:]  "<< endl;
            break;

        case ADD_FUNDS_MSG:
            if (_loggingEnabled) cout<< "[HOST "<< myIndex() 
                <<": RECEIVED ADD FUNDS MSG] "<< msg->getName() << endl;
                handleAddFundsMessage(ttmsg);
            if (_loggingEnabled) cout<< "[AFTER HANDLING:]  "<< endl;
            break;
        
        default:
            handleMessage(ttmsg);

    }

}

/* Specialized function to handle transactions as per the algorithm
 * Since this is shortest path, no other fancy handler required 
 */
void hostNodeBase::handleTransactionMessageSpecialized(routerMsg *ttmsg) {
    handleTransactionMessage(ttmsg);
}

/* Main handler for normal processing of a transaction
 * checks if message has reached sender
 *      1. has reached  - turn transactionMsg into ackMsg, forward ackMsg
 *      2. has not reached yet - add to appropriate job queue q, process q as
 *          much as we have funds for
 */
void hostNodeBase::handleTransactionMessage(routerMsg* ttmsg, bool revisit){
    transactionMsg *transMsg = check_and_cast<transactionMsg *>(ttmsg->getEncapsulatedPacket());
    int hopcount = ttmsg->getHopCount();
    vector<tuple<int, double , routerMsg *, Id, simtime_t>> *q;
    int destination = transMsg->getReceiver();
    int sender = transMsg->getSender();
    int transactionId = transMsg->getTransactionId();
    
    if (!revisit && transMsg->getTimeSent() >= _transStatStart && 
            transMsg->getTimeSent() <= _transStatEnd && transMsg->isSelfMessage()) {
        statRateArrived[destination] += 1;
        statAmtArrived[destination] += transMsg->getAmount();
        statRateAttempted[destination] += 1;
        statAmtAttempted[destination] += transMsg->getAmount();
    }
    else if (!revisit && transMsg->isSelfMessage()) 
        statNumArrived[destination] += 1;


    
    // if it is at the destination
    if (ttmsg->getHopCount() ==  ttmsg->getRoute().size() - 1) {
        // add to incoming trans units 
        int prevNode = ttmsg->getRoute()[ttmsg->getHopCount() - 1];
        unordered_map<Id, double, hashId> *incomingTransUnits = 
            &(nodeToPaymentChannel[prevNode].incomingTransUnits);
        (*incomingTransUnits)[make_tuple(transMsg->getTransactionId(), transMsg->getHtlcIndex())] = 
            transMsg->getAmount();
        nodeToPaymentChannel[prevNode].totalAmtIncomingInflight += transMsg->getAmount();
        
        if (_timeoutEnabled){
            auto iter = canceledTransactions.find(make_tuple(transactionId, 0, 0, 0, 0));
            if (iter != canceledTransactions.end()) {
                canceledTransactions.erase(iter);
            }
        }

        // send ack even if it has timed out because txns wait till _maxTravelTime before being 
        // cleared by clearState
        routerMsg* newMsg =  generateAckMessage(ttmsg);
        forwardMessage(newMsg);
        return;
    }

    else{
        //at the sender
        int destNode = transMsg->getReceiver();
        int nextNode = ttmsg->getRoute()[hopcount+1];
        PaymentChannel *neighbor = &(nodeToPaymentChannel[nextNode]);
        q = &(nodeToPaymentChannel[nextNode].queuedTransUnits);
        tuple<int,int > key = make_tuple(transMsg->getTransactionId(), transMsg->getHtlcIndex());
        
        if (!revisit) 
            transMsg->setTimeAttempted(simTime().dbl());

        // mark the arrival
        neighbor->arrivalTimeStamps.push_back(make_tuple(transMsg->getAmount(), simTime()));
        neighbor->sumArrivalWindowTxns += transMsg->getAmount();
        if (neighbor->arrivalTimeStamps.size() > _serviceArrivalWindow) {
            double frontAmt = get<0>(neighbor->serviceArrivalTimeStamps.front());
            neighbor->arrivalTimeStamps.pop_front(); 
            neighbor->sumArrivalWindowTxns -= frontAmt;
        }

        // if there is insufficient balance at the first node, return failure
        if (_hasQueueCapacity && _queueCapacity == 0) {
            if (forwardTransactionMessage(ttmsg, destNode, simTime()) == 0) {
                routerMsg * failedAckMsg = generateAckMessage(ttmsg, false);
                handleAckMessage(failedAckMsg);
            }
        }
        else if (_hasQueueCapacity && getTotalAmount(nextNode) >= _queueCapacity) {
            // there are other transactions ahead in the queue so don't attempt to forward 
            routerMsg * failedAckMsg = generateAckMessage(ttmsg, false);
            handleAckMessage(failedAckMsg);
        }
        else{
            // add to queue and process in order of queue
            (*q).push_back(make_tuple(transMsg->getPriorityClass(), transMsg->getAmount(),
                  ttmsg, key, simTime()));
            neighbor->totalAmtInQueue += transMsg->getAmount();
            push_heap((*q).begin(), (*q).end(), _schedulingAlgorithm);
            processTransUnits(nextNode, *q);
        }
    }
}


/* handler responsible for prematurely terminating the processing
 * of a transaction if it has timed out and deleteing it. Returns
 * true if the transaction is timed out so that no special handlers
 * are called after
 */
bool hostNodeBase::handleTransactionMessageTimeOut(routerMsg* ttmsg) {
    transactionMsg *transMsg = check_and_cast<transactionMsg *>(ttmsg->getEncapsulatedPacket());
    int transactionId = transMsg->getTransactionId();

    // look for transaction in cancelled txn set and delete if present
    auto iter = canceledTransactions.find(make_tuple(transactionId, 0, 0, 0, 0));
    if ( iter!=canceledTransactions.end() ){
        //delete yourself
        ttmsg->decapsulate();
        delete transMsg;
        delete ttmsg;
        return true;
    }
    else{
        return false;
    }
}

/*  Default action for time out message that is responsible for either recognizing
 *  that txn is complete and timeout is a noop or inserting the transaction into 
 *  a cancelled transaction list
 *  The actual cancellation/clearing of the state happens on the clear state 
 *  message
 */
void hostNodeBase::handleTimeOutMessage(routerMsg* ttmsg){
    timeOutMsg *toutMsg = check_and_cast<timeOutMsg *>(ttmsg->getEncapsulatedPacket());
    int destination = toutMsg->getReceiver();
    int transactionId = toutMsg->getTransactionId();
    
    if (ttmsg->isSelfMessage()) {
            if (successfulDoNotSendTimeOut.count(transactionId) > 0) {
                successfulDoNotSendTimeOut.erase(toutMsg->getTransactionId());
                ttmsg->decapsulate();
                delete toutMsg;
                delete ttmsg;
            }
            else {
                int nextNode = (ttmsg->getRoute())[ttmsg->getHopCount()+1];
                CanceledTrans ct = make_tuple(transactionId, 
                        simTime(),-1, nextNode, destination);
                canceledTransactions.insert(ct);
                forwardMessage(ttmsg);
            }
    }
    else { 
        //is at the destination
        CanceledTrans ct = make_tuple(transactionId, simTime(), 
                (ttmsg->getRoute())[ttmsg->getHopCount()-1],-1, destination);
        canceledTransactions.insert(ct);
        ttmsg->decapsulate();
        delete toutMsg;
        delete ttmsg;
    }
}

/* handler that periodically computes the minimum balance on a payment channel 
 * this is then used accordingly to trigger rebalancing events */
void hostNodeBase::handleComputeMinAvailableBalanceMessage(routerMsg* ttmsg) {
    // reschedule this message to be sent again
    if (simTime() > _simulationLength || !_rebalancingEnabled) {
        delete ttmsg;
    }
    else {
        scheduleAt(simTime()+_computeBalanceRate, ttmsg);
    }
    
    for (auto it = nodeToPaymentChannel.begin(); it!= nodeToPaymentChannel.end(); it++){
        PaymentChannel *p = &(it->second);
        if (p->balance < p->minAvailBalance)
            p->minAvailBalance = p->balance;
    }
}


/* handler for the periodic rebalancing message that gets triggered 
 * that is responsible for implicit rebalancing at the end-hosts
 * basically refunds from what has been sent out and remove what's been
 * received, all to be adjusted on the single payment channel
 * connected to it
 */ 
void hostNodeBase::handleTriggerRebalancingMessage(routerMsg* ttmsg) {
    delete ttmsg;
    return;
    // reschedule the message again to be periodic
    if (simTime() > _simulationLength || !_rebalancingEnabled){
        delete ttmsg;
    }
    else {
        scheduleAt(simTime()+_rebalanceRate, ttmsg);
    }

    map<int, double> pcsNeedingFunds;
    for (auto it = nodeToPaymentChannel.begin(); it!= nodeToPaymentChannel.end(); it++){
        // technically there's only payment channel here
        int id = it->first;
        PaymentChannel *p = &(it->second);
        double totalAmtReceived = 0;
        double totalAmtSent = 0;
        double currentlyInflight = getTotalAmountOutgoingInflight(it->first);

        // remove funds for everything that has been received and needs to be refunded
        for (auto nodeIt = senderToAmtRefundable.begin(); nodeIt != senderToAmtRefundable.end(); nodeIt++) {
            totalAmtReceived += nodeIt->second;
            nodeIt->second = 0;
        }
        // schedule message to add funds for everything that has been sent and therefore is getting refunded 
        for (auto receiverIt = receiverToAmtRefunded.begin(); receiverIt != receiverToAmtRefunded.end(); 
                receiverIt++) {
            totalAmtSent += receiverIt->second;
            receiverIt->second = 0;
        }

        p->owedFunds += max(totalAmtReceived - totalAmtSent, 0.0);
        double removableFunds = min(p->owedFunds, p->balance);
        
        if (removableFunds > 0) {
            _bank += removableFunds;
            setPaymentChannelBalanceByNode(it->first, p->balance - removableFunds);
            p->owedFunds -= removableFunds;
            if (p->balance < 0)
                cout << "abhishtu at " << myIndex() << " removable  " 
                    << removableFunds << " balance " << p->balance << "min available balance "
                    << p->minAvailBalance << " bank balance " << _bank << endl;
            
            p->amtImplicitlyRebalanced -= removableFunds;
            p->numRebalanceEvents += 1;
            
            tuple<int, int> senderReceiverTuple = (id < myIndex()) ? make_tuple(id, myIndex()) :
                make_tuple(myIndex(), id);
            _capacities[senderReceiverTuple] -= removableFunds; 
        } 
        
        p->entitledFunds += max(totalAmtSent - totalAmtReceived, 0.0);
        double addableFunds = min(p->entitledFunds, _bank); 
        if (addableFunds > 0) {
            // add this to the list of payment channels to be addressed 
            // along with a particular addFundsEvent
            pcsNeedingFunds[id] = addableFunds;
            _bank -= addableFunds;
            p->entitledFunds -= addableFunds;
        } 
    }

    // generate and schedule add funds message to add these funds after some fixed time period
    if (pcsNeedingFunds.size() > 0) {
        routerMsg* addFundsMsg = generateAddFundsMessage(pcsNeedingFunds);
        scheduleAt(simTime() + _delayForAddingFunds, addFundsMsg);
    }
}

/* handler to add the desired amount of funds to the given payment channels when an addFundsMessage
 * is received 
 */
void hostNodeBase::handleAddFundsMessage(routerMsg* ttmsg) {
    addFundsMsg *afMsg = check_and_cast<addFundsMsg *>(ttmsg->getEncapsulatedPacket());
    map<int, double> pcsNeedingFunds = afMsg->getPcsNeedingFunds();
    for (auto it = pcsNeedingFunds.begin(); it!= pcsNeedingFunds.end(); it++) {
        int pcIdentifier = it->first;
        double fundsToAdd = it->second;
        PaymentChannel *p = &(nodeToPaymentChannel[pcIdentifier]);

        // add funds at this end
        setPaymentChannelBalanceByNode(pcIdentifier, p->balance + fundsToAdd);
        tuple<int, int> senderReceiverTuple = (pcIdentifier < myIndex()) ? 
            make_tuple(pcIdentifier, myIndex()) :
            make_tuple(myIndex(), pcIdentifier);
        _capacities[senderReceiverTuple] +=  fundsToAdd;
        
        p->numRebalanceEvents += 1;
        p->amtAdded += fundsToAdd;
        p->amtImplicitlyRebalanced += fundsToAdd;

        // process as many new transUnits as you can for this payment channel
        processTransUnits(pcIdentifier, p->queuedTransUnits);
    }
    
    ttmsg->decapsulate();
    delete afMsg;
    delete ttmsg;
}

/* specialized ack handler that does the routine if this is a shortest paths 
 * algorithm. In particular, collects stats assuming that this is the only
 * one path on which a txn might complete
 * NOTE: acks are on the reverse path relative to the original sender
 */
void hostNodeBase::handleAckMessageSpecialized(routerMsg* ttmsg) { 

    int destNode = ttmsg->getRoute()[0];
    ackMsg *aMsg = check_and_cast<ackMsg *>(ttmsg->getEncapsulatedPacket());

    if (aMsg->getIsSuccess() == false && aMsg->getTimeSent() >= _transStatStart && 
            aMsg->getTimeSent() <= _transStatEnd) {
        statRateFailed[destNode] = statRateFailed[destNode] + 1;
        statAmtFailed[destNode] += aMsg->getAmount();
    }
    else if (aMsg->getTimeSent() >= _transStatStart && 
            aMsg->getTimeSent() <= _transStatEnd) {
        statRateCompleted[destNode] = statRateCompleted[destNode] + 1;
        _transactionCompletionBySize[aMsg->getAmount()] += 1;
        statAmtCompleted[destNode] += aMsg->getAmount();

        // stats
        double timeTaken = simTime().dbl() - aMsg->getTimeSent();
        statCompletionTimes[destNode] += timeTaken * 1000;
        _txnAvgCompTimeBySize[aMsg->getAmount()] += timeTaken * 1000;
        recordTailCompletionTime(aMsg->getTimeSent(), aMsg->getAmount(), timeTaken * 1000);
    }
    else 
        statNumCompleted[destNode] += 1;

    hostNodeBase::handleAckMessage(ttmsg);
}


/* default routine for handling an ack that is responsible for 
 * updating outgoing transunits and incoming trans units 
 * and triggering an update message to the next node on the path
 * before forwarding the ack back to the previous node
 */
void hostNodeBase::handleAckMessage(routerMsg* ttmsg){
    assert(myIndex() == ttmsg->getRoute()[ttmsg->getHopCount()]);
    ackMsg *aMsg = check_and_cast<ackMsg *>(ttmsg->getEncapsulatedPacket());
    
    // this is previous node on the ack path, so next node on the forward path
    // remove txn from outgone txn list
    Id thisTrans = make_tuple(aMsg->getTransactionId(), aMsg->getHtlcIndex());
    int prevNode = ttmsg->getRoute()[ttmsg->getHopCount()-1];
    PaymentChannel *prevChannel = &(nodeToPaymentChannel[prevNode]);
    double timeInflight = (simTime() - prevChannel->txnSentTimes[thisTrans]).dbl();
    (prevChannel->outgoingTransUnits).erase(thisTrans);
    (prevChannel->txnSentTimes).erase(thisTrans);
   
    if (aMsg->getIsSuccess() == false) {
        // increment funds on this channel unless this is the node that caused the fauilure
        // in which case funds were never decremented in the first place
        if (aMsg->getFailedHopNum() < ttmsg->getHopCount()) {
            double updatedBalance = prevChannel->balance + aMsg->getAmount();
            prevChannel->balanceEWMA = 
                (1 -_ewmaFactor) * prevChannel->balanceEWMA + (_ewmaFactor) * updatedBalance; 
            setPaymentChannelBalanceByNode(prevNode, updatedBalance);
            prevChannel->totalAmtOutgoingInflight -= aMsg->getAmount();
        }
        
        // no relevant incoming_trans_units because no node on fwd path before this
        if (ttmsg->getHopCount() < ttmsg->getRoute().size() - 1) {
            int nextNode = ttmsg->getRoute()[ttmsg->getHopCount()+1];
            unordered_map<Id, double, hashId> *incomingTransUnits = 
                &(nodeToPaymentChannel[nextNode].incomingTransUnits);
            (*incomingTransUnits).erase(make_tuple(aMsg->getTransactionId(), 
                        aMsg->getHtlcIndex()));
            nodeToPaymentChannel[nextNode].totalAmtIncomingInflight -= aMsg->getAmount();
        }
    }
    else { 
        // mark the time it spent inflight
        prevChannel->sumTimeInFlight += timeInflight;
        prevChannel->timeInFlightSamples += 1;
        prevChannel->totalAmtOutgoingInflight -= aMsg->getAmount();

        routerMsg* uMsg =  generateUpdateMessage(aMsg->getTransactionId(), 
                prevNode, aMsg->getAmount(), aMsg->getHtlcIndex() );
        prevChannel->numUpdateMessages += 1;
        forwardMessage(uMsg);

        // keep track of how much you have sent to others if rebalancing is enabled
        // and how much of that needs to be replenished and
        // replenish my end host - router link immediately to make up (simulates receiving money back)
        int dest = aMsg->getReceiver();
        if (_rebalancingEnabled) {
            tuple<int, int> senderReceiverTuple = make_tuple(myIndex(), prevNode);
            _capacities[senderReceiverTuple] += aMsg->getAmount();
            double newBalance = prevChannel->balance + aMsg->getAmount();
            prevChannel->balanceEWMA = (1 -_ewmaFactor) * prevChannel->balanceEWMA + 
            (_ewmaFactor) * newBalance;
            setPaymentChannelBalanceByNode(prevNode, newBalance);
        }
    }
    
    //delete ack message
    ttmsg->decapsulate();
    delete aMsg;
    delete ttmsg;
}



/* handles the logic for ack messages in the presence of timeouts
 * in particular, removes the transaction from the cancelled txns
 * to mark that it has been received 
 * it uses the successfulDoNotSendTimeout to detect if txns have
 * been completed when handling the timeout - so insert into it here
 */
void hostNodeBase::handleAckMessageTimeOut(routerMsg* ttmsg){
    ackMsg *aMsg = check_and_cast<ackMsg *>(ttmsg->getEncapsulatedPacket());
    int transactionId = aMsg->getTransactionId();
    
    // only if it isn't waterfilling
    if (aMsg->getIsSuccess()) {
        auto iter = canceledTransactions.find(make_tuple(transactionId, 0, 0, 0, 0));
        if (iter != canceledTransactions.end()) {
            canceledTransactions.erase(iter);
        }
        successfulDoNotSendTimeOut.insert(aMsg->getTransactionId());
    }
}



/*
 * handleUpdateMessage - called when receive update message, increment back funds, see if we can
 *      process more jobs with new funds, delete update message
 */
void hostNodeBase::handleUpdateMessage(routerMsg* msg) {
    vector<tuple<int, double , routerMsg *, Id, simtime_t>> *q;
    int prevNode = msg->getRoute()[msg->getHopCount()-1];
    updateMsg *uMsg = check_and_cast<updateMsg *>(msg->getEncapsulatedPacket());
    PaymentChannel *prevChannel = &(nodeToPaymentChannel[prevNode]);
   
    //increment the in flight funds back
    double newBalance = prevChannel->balance + uMsg->getAmount();
    prevChannel->balanceEWMA = (1 -_ewmaFactor) * prevChannel->balanceEWMA 
        + (_ewmaFactor) * newBalance;
    setPaymentChannelBalanceByNode(prevNode, newBalance);

    // immediately remove these funds - simulates giving end host back these funds
    if (_rebalancingEnabled) {
        tuple<int, int> senderReceiverTuple = make_tuple(myIndex(), prevNode);
        _capacities[senderReceiverTuple] -= uMsg->getAmount();
        double newBalance = prevChannel->balance - uMsg->getAmount();
        prevChannel->balanceEWMA = (1 -_ewmaFactor) * prevChannel->balanceEWMA + 
        (_ewmaFactor) * newBalance;
        setPaymentChannelBalanceByNode(prevNode, newBalance);
    }

    //remove transaction from incoming_trans_units
    unordered_map<Id, double, hashId> *incomingTransUnits = &(prevChannel->incomingTransUnits);
    (*incomingTransUnits).erase(make_tuple(uMsg->getTransactionId(), uMsg->getHtlcIndex()));
    prevChannel->totalAmtIncomingInflight -= uMsg->getAmount();

    msg->decapsulate();
    delete uMsg;
    delete msg; //delete update message

    //see if we can send more jobs out
    q = &(prevChannel->queuedTransUnits);
    processTransUnits(prevNode, *q);
} 


/* emits all the default statistics across all the schemes
 * until the end of the simulation
 */
void hostNodeBase::handleStatMessage(routerMsg* ttmsg){
    // reschedule this message to be sent again
    if (simTime() > _simulationLength){
        delete ttmsg;
    }
    else {
        scheduleAt(simTime() + _statRate, ttmsg);
    }
    
    if (_signalsEnabled) {
        // per channel Stats
        for ( auto it = nodeToPaymentChannel.begin(); it!= nodeToPaymentChannel.end(); it++){
            PaymentChannel *p = &(it->second);
            int id = it->first;
            if (myIndex() == 0) {
                emit(p->bankSignal, _bank);
                double sumCapacities = accumulate(begin(_capacities), end(_capacities), 0,
                    [] (double value, const std::map<tuple<int,int>, double>::value_type& p)
                   { return value + p.second; }
                    ); 
            }
            
            emit(p->amtInQueuePerChannelSignal, getTotalAmount(it->first));
            emit(p->balancePerChannelSignal, p->balance);
            emit(p->explicitRebalancingAmtPerChannelSignal, p->amtExplicitlyRebalanced/_statRate);
            emit(p->implicitRebalancingAmtPerChannelSignal, p->amtImplicitlyRebalanced/_statRate);
            emit(p->timeInFlightPerChannelSignal, p->sumTimeInFlight/p->timeInFlightSamples);
            emit(p->numInflightPerChannelSignal, getTotalAmountIncomingInflight(it->first) +
                    getTotalAmountOutgoingInflight(it->first));
            p->sumTimeInFlight = 0;
            p->timeInFlightSamples = 0;
            p->amtExplicitlyRebalanced = 0;
            p->amtImplicitlyRebalanced = 0;
            
            tuple<int, int> senderReceiverTuple = (id < myIndex()) ? make_tuple(id, myIndex()) :
                make_tuple(myIndex(), id);
            emit(p->capacityPerChannelSignal, _capacities[senderReceiverTuple]);
        }
    }

    recordScalar("time", simTime());
    for (auto it = 0; it < _numHostNodes; it++){
        if (_destList[myIndex()].count(it) > 0) {
            char buffer[30];
            sprintf(buffer, "rateCompleted %d -> %d", myIndex(), it);
            recordScalar(buffer, statRateCompleted[it]);
            sprintf(buffer, "amtCompleted %d -> %d", myIndex(), it);
            recordScalar(buffer, statAmtCompleted[it]);

            sprintf(buffer, "rateAttempted %d -> %d", myIndex(), it);
            recordScalar(buffer, statRateAttempted[it]);
            sprintf(buffer, "amtAttempted  %d -> %d", myIndex(), it);
            recordScalar(buffer, statAmtAttempted[it]);

            sprintf(buffer, "rateArrived %d -> %d", myIndex(), it);
            recordScalar(buffer, statRateArrived[it]);
            sprintf(buffer, "amtArrived  %d -> %d", myIndex(), it);
            recordScalar(buffer, statAmtArrived[it]);

            sprintf(buffer, "completionTime %d -> %d ", myIndex(), it);
            recordScalar(buffer, statCompletionTimes[it]);

            statRateAttempted[it] = 0;
            statAmtAttempted[it] = 0;
            statRateArrived[it] = 0;
            statAmtArrived[it] = 0;
            statAmtCompleted[it] = 0;
            statRateCompleted[it] = 0;
        }

        // per destination stats
        if (it != getIndex() && _destList[myIndex()].count(it) > 0) {
            if (nodeToShortestPathsMap.count(it) > 0) {
                for (auto p: nodeToShortestPathsMap[it]) {
                    PathInfo *pathInfo = &(nodeToShortestPathsMap[it][p.first]);
                    
                    //emit rateCompleted per path
                    pathInfo->statRateAttempted = 0;
                    pathInfo->statRateCompleted = 0;
                    
                    //emit rateAttempted per path
                    if (_signalsEnabled) {
                        emit(pathInfo->rateAttemptedPerDestPerPathSignal, 
                            pathInfo->statRateAttempted);
                        emit(pathInfo->rateCompletedPerDestPerPathSignal, 
                            pathInfo->statRateCompleted);
                    }
                }
            }

            if (_signalsEnabled) {
                if (_hasQueueCapacity){
                    emit(rateFailedPerDestSignals[it], statRateFailed[it]);
                }
                emit(rateCompletedPerDestSignals[it], statRateCompleted[it]);
                emit(rateAttemptedPerDestSignals[it], statRateAttempted[it]);
                emit(rateArrivedPerDestSignals[it], statRateArrived[it]);
                
                emit(numTimedOutPerDestSignals[it], statNumTimedOut[it]);
                emit(numPendingPerDestSignals[it], destNodeToNumTransPending[it]);
                emit(numCompletedPerDestSignals[it], statNumCompleted[it]);
                emit(numArrivedPerDestSignals[it], statNumArrived[it]);
                double frac = ((float(statNumCompleted[it]))/(max(statNumArrived[it],1)));
                statNumCompleted[it] = 0;
                statNumArrived[it] = 0;
                emit(fracSuccessfulPerDestSignals[it],frac);
            }
        }
    }
}

/* handler that is responsible for removing all the state associated
 * with a cancelled transaction once its grace period has passed
 * this included removal from outgoing/incoming units and any
 * queues
 */
void hostNodeBase::handleClearStateMessage(routerMsg* ttmsg){
    //reschedule for the next interval
    if (simTime() > _simulationLength){
        delete ttmsg;
    }
    else{
        scheduleAt(simTime()+_clearRate, ttmsg);
    }

    for ( auto it = canceledTransactions.begin(); it!= canceledTransactions.end(); ) {       
        int transactionId = get<0>(*it);
        simtime_t msgArrivalTime = get<1>(*it);
        int prevNode = get<2>(*it);
        int nextNode = get<3>(*it);
        int destNode = get<4>(*it);
        
        // if grace period has passed
        if (simTime() > (msgArrivalTime + _maxTravelTime)){
            // remove from queue to next node
            if (nextNode != -1){   
                vector<tuple<int, double, routerMsg*, Id, simtime_t>>* queuedTransUnits = 
                    &(nodeToPaymentChannel[nextNode].queuedTransUnits);

                auto iterQueue = find_if((*queuedTransUnits).begin(),
                  (*queuedTransUnits).end(),
                  [&transactionId](const tuple<int, double, routerMsg*, Id, simtime_t>& p)
                  { return (get<0>(get<3>(p)) == transactionId); });
                
                // delete first occurences of this transaction in the queue
                // tids are unique, so this can't be a problem (we do splitting before hand)
                // especially if there are splits
                if (iterQueue != (*queuedTransUnits).end()){
                    routerMsg * rMsg = get<2>(*iterQueue);
                    auto tMsg = rMsg->getEncapsulatedPacket();
                    rMsg->decapsulate();
                    nodeToPaymentChannel[nextNode].totalAmtInQueue -= get<1>(*iterQueue);
                    iterQueue = (*queuedTransUnits).erase(iterQueue);
                    delete tMsg;
                    delete rMsg;
                }
                
                // resort the queue based on priority
                make_heap((*queuedTransUnits).begin(), (*queuedTransUnits).end(), 
                        _schedulingAlgorithm);
            }

            // remove from incoming TransUnits from the previous node
            if (prevNode != -1){
                unordered_map<Id, double, hashId> *incomingTransUnits = 
                    &(nodeToPaymentChannel[prevNode].incomingTransUnits);
                auto iterIncoming = find_if((*incomingTransUnits).begin(),
                  (*incomingTransUnits).end(),
                  [&transactionId](const pair<tuple<int, int >, double> & p)
                  { return get<0>(p.first) == transactionId; });
                
                if (iterIncoming != (*incomingTransUnits).end()){
                    nodeToPaymentChannel[prevNode].totalAmtIncomingInflight -= iterIncoming->second;
                    iterIncoming = (*incomingTransUnits).erase(iterIncoming);
                }
            }
        }

        // remove from outgoing transUnits to nextNode and restore balance on own end
        if (simTime() > (msgArrivalTime + _maxTravelTime + _maxOneHopDelay)){
            if (nextNode != -1){
                unordered_map<tuple<int,int>, double, hashId> *outgoingTransUnits = 
                    &(nodeToPaymentChannel[nextNode].outgoingTransUnits);
                
                auto iterOutgoing = find_if((*outgoingTransUnits).begin(),
                  (*outgoingTransUnits).end(),
                  [&transactionId](const pair<tuple<int, int >, double> & p)
                  { return get<0>(p.first) == transactionId; });
                
                if (iterOutgoing != (*outgoingTransUnits).end()){
                    double amount = iterOutgoing -> second;
                    iterOutgoing = (*outgoingTransUnits).erase(iterOutgoing);
              
                    PaymentChannel *nextChannel = &(nodeToPaymentChannel[nextNode]);
                    double updatedBalance = nextChannel->balance + amount;
                    setPaymentChannelBalanceByNode(nextNode, updatedBalance);
                    nextChannel->balanceEWMA = (1 -_ewmaFactor) * nextChannel->balanceEWMA + 
                        (_ewmaFactor) * updatedBalance;
                    nextChannel->totalAmtOutgoingInflight -= amount;
                }
            }
            
            // all done, can remove txn and update stats
            it = canceledTransactions.erase(it);

            //end host didn't receive ack, so txn timed out 
            statNumTimedOut[destNode] = statNumTimedOut[destNode]  + 1;
        }
        else{
            it++;
        }
    }
}



/*
 *  Given a message representing a TransUnit, increments hopCount, finds next destination,
 *  adjusts (decrements) channel balance, sends message to next node on route
 *  as long as it isn't cancelled
 */
int hostNodeBase::forwardTransactionMessage(routerMsg *msg, int dest, simtime_t arrivalTime) {
    transactionMsg *transMsg = check_and_cast<transactionMsg *>(msg->getEncapsulatedPacket());
    int nextDest = msg->getRoute()[msg->getHopCount()+1];
    int transactionId = transMsg->getTransactionId();
    PaymentChannel *neighbor = &(nodeToPaymentChannel[nextDest]);
    int amt = transMsg->getAmount();

    if (neighbor->balance <= 0 || transMsg->getAmount() > neighbor->balance){
        return false;
    }
    else {
        // return true directly if txn has been cancelled
        // so that you waste not resources on this and move on to a new txn
        // if you return false processTransUnits won't look for more txns
        // return true directly if txn has been cancelled
        // so that you waste not resources on this and move on to a new txn
        // if you return false processTransUnits won't look for more txns
        auto iter = canceledTransactions.find(make_tuple(transactionId, 0, 0, 0, 0));
        if (iter != canceledTransactions.end()) {
            msg->decapsulate();
            delete transMsg;
            delete msg;
            neighbor->totalAmtInQueue -= amt;
            return true;
        }

        // update state to send transaction out
        msg->setHopCount(msg->getHopCount()+1);

        // update service arrival times
        neighbor->serviceArrivalTimeStamps.push_back(
                make_tuple(transMsg->getAmount(), simTime(), arrivalTime));
        neighbor->sumServiceWindowTxns += transMsg->getAmount();
        if (neighbor->serviceArrivalTimeStamps.size() > _serviceArrivalWindow) {
            double frontAmt = get<0>(neighbor->serviceArrivalTimeStamps.front());
            neighbor->serviceArrivalTimeStamps.pop_front(); 
            neighbor->sumServiceWindowTxns -= frontAmt;
        }

        // add amount to outgoing map, mark time sent
        Id thisTrans = make_tuple(transMsg->getTransactionId(), transMsg->getHtlcIndex());
        (neighbor->outgoingTransUnits)[thisTrans] = transMsg->getAmount();
        neighbor->txnSentTimes[thisTrans] = simTime();
        neighbor->totalAmtOutgoingInflight += transMsg->getAmount();
      
        // update balance
        int amt = transMsg->getAmount();
        double newBalance = neighbor->balance - amt;
        setPaymentChannelBalanceByNode(nextDest, newBalance);
        neighbor-> balanceEWMA = (1 -_ewmaFactor) * neighbor->balanceEWMA + 
            (_ewmaFactor) * newBalance;
        neighbor->totalAmtInQueue -= amt;

        if (_loggingEnabled) cout << "forwardTransactionMsg send: " << simTime() << endl;
        send(msg, nodeToPaymentChannel[nextDest].gate);
        return true;
    } 
}



/* responsible for forwarding all messages but transactions which need special care
 * in particular, looks up the next node's interface and sends out the message
 */
void hostNodeBase::forwardMessage(routerMsg* msg){
   // Increment hop count.
   msg->setHopCount(msg->getHopCount()+1);
   //use hopCount to find next destination
   int nextDest = msg->getRoute()[msg->getHopCount()];
   if (_loggingEnabled) cout << "forwarding " << msg->getMessageType() << " at " 
       << simTime() << endl;
   send(msg, nodeToPaymentChannel[nextDest].gate);
}

/* initialize() all of the global parameters and basic
 * per channel information as well as default signals for all
 * payment channels and destinations
 */
void hostNodeBase::initialize() {
    successfulDoNotSendTimeOut = {};
    string topologyFile_ = par("topologyFile");
    string workloadFile_ = par("workloadFile");

    // initialize global parameters once
    if (getIndex() == 0){  
        _simulationLength = par("simulationLength");
        _statRate = par("statRate");
        _clearRate = par("timeoutClearRate");
        _waterfillingEnabled = par("waterfillingEnabled");
        _timeoutEnabled = par("timeoutEnabled");
        _signalsEnabled = par("signalsEnabled");
        _loggingEnabled = par("loggingEnabled");
        _priceSchemeEnabled = par("priceSchemeEnabled");
        _dctcpEnabled = par("dctcpEnabled");
        _splittingEnabled = par("splittingEnabled");
        cout << "splitting" << _splittingEnabled << endl;
        _serviceArrivalWindow = par("serviceArrivalWindow");
        string resultPrefix = par("resultPrefix");

        _hasQueueCapacity = true;
        _queueCapacity = 12000;

        _transStatStart = par("transStatStart");
        _transStatEnd = par("transStatEnd");
        _waterfillingStartTime = 0;
        _landmarkRoutingStartTime = 0;
        _shortestPathStartTime = 0;
        _shortestPathEndTime = 5000;

        _widestPathsEnabled = par("widestPathsEnabled");
        _heuristicPathsEnabled = par("heuristicPathsEnabled");
        _kspYenEnabled = par("kspYenEnabled");
        _obliviousRoutingEnabled = par("obliviousRoutingEnabled");

        _splitSize = par("splitSize");
        _percentile = 0.01;

        _celerEnabled = par("celerEnabled");
        _lndBaselineEnabled = par("lndBaselineEnabled");
        _landmarkRoutingEnabled = par("landmarkRoutingEnabled");
                                  
        if (_landmarkRoutingEnabled || _lndBaselineEnabled || _celerEnabled){
            _hasQueueCapacity = true;
            _queueCapacity = 0;
            if (!_celerEnabled)
                _timeoutEnabled = false;
        }

        // rebalancing related flags/parameters
        _rebalancingEnabled = par("rebalancingEnabled");
        _rebalancingUpFactor = 3.0;
        _queueDelayThreshold = par("queueDelayThreshold");
        _gamma = par("gamma");
        _maxGammaImbalanceQueueSize = par("gammaImbalanceQueueSize");
        _delayForAddingFunds = par("rebalancingDelayForAddingFunds");
        _rebalanceRate = par("rebalancingRate");
        _computeBalanceRate = par("minBalanceComputeRate");
        _bank = 0;

        // files for recording tail completion and retries
        _tailCompBySizeFile.open(resultPrefix + "_tailCompBySize.txt");
        if (_lndBaselineEnabled) {
            _succRetriesFile.open(resultPrefix + "_succRetries.txt");
            _failRetriesFile.open(resultPrefix + "_failRetries.txt");
        }

        // path choices
        string pathFileName;
        if (_widestPathsEnabled)
            pathFileName = topologyFile_ + "_widestPaths";
        else if (_obliviousRoutingEnabled)
            pathFileName = topologyFile_ + "_obliviousPaths";
        else if (_heuristicPathsEnabled)
            pathFileName = topologyFile_ + "_heuristicPaths";
        else if (_kspYenEnabled)
            pathFileName = topologyFile_ + "_kspYenPaths";

        // scheduling algorithms
        _LIFOEnabled = par("LIFOEnabled");
        _FIFOEnabled = par("FIFOEnabled");
        _SPFEnabled = par("SPFEnabled");
        _RREnabled = par("RREnabled");
        _EDFEnabled = par("EDFEnabled");

        if (_LIFOEnabled) 
            _schedulingAlgorithm = &sortLIFO;
        else if (_FIFOEnabled)
            _schedulingAlgorithm = &sortFIFO;
        else if (_SPFEnabled)
            _schedulingAlgorithm = &sortSPF;
        else if (_EDFEnabled)
            _schedulingAlgorithm = &sortEDF;
        else // default
            _schedulingAlgorithm = &sortLIFO;

        if (_widestPathsEnabled || _kspYenEnabled || _obliviousRoutingEnabled || _heuristicPathsEnabled)
            initializePathMaps(pathFileName);

        _epsilon = pow(10, -6);
        cout << "epsilon" << _epsilon << endl;
        _maxTravelTime = 0.0;
        _delta = 0.01; // to avoid divide by zero 
        
        if (_waterfillingEnabled || _priceSchemeEnabled || _landmarkRoutingEnabled || _dctcpEnabled){
           _kValue = par("numPathChoices");
        }
        
        setNumNodes(topologyFile_);
        generateTransUnitList(workloadFile_);
        generateChannelsBalancesMap(topologyFile_);
    }
    
    // set index and compute the top percentile size to choose elements accordingly
    setIndex(getIndex());
    maxPercentileHeapSize =  round(_numSplits[myIndex()].size() * _percentile);
    statNumTries.push(0);
    
    // Assign gates to all the payment channels
    const char * gateName = "out";
    cGate *destGate = NULL;

    int i = 0;
    int gateSize = gate(gateName, 0)->size();
    
    do {
        destGate = gate(gateName, i++);
        cGate *nextGate = destGate->getNextGate();
        if (nextGate) {
            PaymentChannel temp =  {};
            temp.gate = destGate;

            bool isHost = nextGate->getOwnerModule()->par("isHost");
            int key = nextGate->getOwnerModule()->getIndex();
            if (!isHost){
                key = key + _numHostNodes;
            }
            nodeToPaymentChannel[key] = temp;
        }
    } while (i < gateSize);


    //initialize everything for adjacent nodes/nodes with payment channel to me
    for(auto iter = nodeToPaymentChannel.begin(); iter != nodeToPaymentChannel.end(); ++iter)
    {
        int key =iter->first; //node

        //fill in balance field of nodeToPaymentChannel
        nodeToPaymentChannel[key].balance = _balances[make_tuple(myIndex(),key)];
        nodeToPaymentChannel[key].balanceEWMA = nodeToPaymentChannel[key].balance;

        // intialize capacity
        double balanceOpp =  _balances[make_tuple(key, myIndex())];
        nodeToPaymentChannel[key].origTotalCapacity = nodeToPaymentChannel[key].balance + balanceOpp;

        //initialize queuedTransUnits
        vector<tuple<int, double , routerMsg *, Id, simtime_t>> temp;
        make_heap(temp.begin(), temp.end(), _schedulingAlgorithm);
        nodeToPaymentChannel[key].queuedTransUnits = temp;

        //register PerChannel signals
        if (_signalsEnabled) {
            simsignal_t signal;
            signal = registerSignalPerChannel("numInQueue", key);
            nodeToPaymentChannel[key].amtInQueuePerChannelSignal = signal;

            signal = registerSignalPerChannel("balance", key);
            nodeToPaymentChannel[key].balancePerChannelSignal = signal;
            
            signal = registerSignalPerChannel("capacity", key);
            nodeToPaymentChannel[key].capacityPerChannelSignal = signal;
            
            signal = registerSignalPerChannel("timeInFlight", key);
            nodeToPaymentChannel[key].timeInFlightPerChannelSignal = signal;

            signal = registerSignalPerChannel("numInflight", key);
            nodeToPaymentChannel[key].numInflightPerChannelSignal = signal;
            
            signal = registerSignalPerChannel("bank", key);
            nodeToPaymentChannel[key].bankSignal = signal;
            
            signal = registerSignalPerChannel("implicitRebalancingAmt", key);
            nodeToPaymentChannel[key].implicitRebalancingAmtPerChannelSignal = signal;
            
            signal = registerSignalPerChannel("explicitRebalancingAmt", key);
            nodeToPaymentChannel[key].explicitRebalancingAmtPerChannelSignal = signal;
        }
    }

    //initialize signals with all other nodes in graph
    for (int i = 0; i < _numHostNodes; ++i) {
        if (_destList[myIndex()].count(i) > 0) {
            simsignal_t signal;
            if (_signalsEnabled) {
                signal = registerSignalPerDest("rateCompleted", i, "_Total");
                rateCompletedPerDestSignals[i] = signal;

                signal = registerSignalPerDest("rateAttempted", i, "_Total");
                rateAttemptedPerDestSignals[i] = signal;

                signal = registerSignalPerDest("rateArrived", i, "_Total");
                rateArrivedPerDestSignals[i] = signal;

                signal = registerSignalPerDest("numTimedOut", i, "_Total");
                numTimedOutPerDestSignals[i] = signal;

                signal = registerSignalPerDest("numPending", i, "_Total");
                numPendingPerDestSignals[i] = signal;

                signal = registerSignalPerDest("numArrived", i, "_Total");
                numArrivedPerDestSignals[i] = signal;

                signal = registerSignalPerDest("numCompleted", i, "_Total");
                numCompletedPerDestSignals[i] = signal;
                
                signal = registerSignalPerDest("fracSuccessful", i, "_Total");
                fracSuccessfulPerDestSignals[i] = signal;

                signal = registerSignalPerDest("rateFailed", i, "");
                rateFailedPerDestSignals[i] = signal;
            }

            statRateCompleted[i] = 0;
            statAmtCompleted[i] = 0;
            statNumCompleted[i] = 0;
            statRateAttempted[i] = 0;
            statAmtAttempted[i] = 0;
            statRateArrived[i] = 0;
            statNumArrived[i] = 0;
            statAmtArrived[i] = 0;
            statNumTimedOut[i] = 0;;
            statRateFailed[i] = 0;
            statAmtFailed[i] = 0;
            statCompletionTimes[i] = 0;
        }
    }
    
    // generate first transaction
    generateNextTransaction();

    //generate stat message
    routerMsg *statMsg = generateStatMessage();
    scheduleAt(simTime() + 0, statMsg);

    if (_timeoutEnabled){
       routerMsg *clearStateMsg = generateClearStateMessage();
       scheduleAt(simTime()+ _clearRate, clearStateMsg);
    }

    // generate rebalancing trigger messages
    if (_rebalancingEnabled) {
        routerMsg *triggerRebalancingMsg = generateTriggerRebalancingMessage();
        scheduleAt(simTime() + _rebalanceRate, triggerRebalancingMsg);
        
        routerMsg *computeMinBalanceMsg = generateComputeMinBalanceMessage();
        scheduleAt(simTime() + _computeBalanceRate, computeMinBalanceMsg);
    }
}

/* function that is called at the end of the simulation that
 * deletes any remaining messages and records scalars
 */
void hostNodeBase::finish() {
    deleteMessagesInQueues();

    for (int it = 0; it < _numHostNodes; ++it) {
        if (_destList[myIndex()].count(it) > 0) {
            char buffer[30];
            sprintf(buffer, "rateCompleted %d -> %d", myIndex(), it);
            recordScalar(buffer, statRateCompleted[it]);
            sprintf(buffer, "amtCompleted %d -> %d", myIndex(), it);
            recordScalar(buffer, statAmtCompleted[it]);

            sprintf(buffer, "rateAttempted %d -> %d", myIndex(), it);
            recordScalar(buffer, statRateAttempted[it]);
            sprintf(buffer, "amtAttempted  %d -> %d", myIndex(), it);
            recordScalar(buffer, statAmtAttempted[it]);

            sprintf(buffer, "rateArrived %d -> %d", myIndex(), it);
            recordScalar(buffer, statRateArrived[it]);
            sprintf(buffer, "amtArrived  %d -> %d", myIndex(), it);
            recordScalar(buffer, statAmtArrived[it]);

            sprintf(buffer, "completionTime %d -> %d ", myIndex(), it);
            recordScalar(buffer, statCompletionTimes[it]);
        }
    }

    // print all the PQ items for number of tries 
    while (!statNumTries.empty()) {
        char buffer[350];
        sprintf(buffer, "retries top percentile %d ", myIndex());
        recordScalar(buffer, statNumTries.top());
        statNumTries.pop();
    }


    if (myIndex() == 0) {
        // can be done on a per node basis also if need be
        // all in seconds
        recordScalar("max travel time", _maxTravelTime);
        recordScalar("delta", _delta);
        recordScalar("average delay", _avgDelay/1000.0);
        recordScalar("epsilon", _epsilon);
        recordScalar("cannonical RTT", _cannonicalRTT);

        for (auto const& x : _transactionCompletionBySize) {
            double amount = x.first;
            int completed = x.second;
            int arrived = _transactionArrivalBySize[amount];
            
            char buffer[60];
            sprintf(buffer, "size %d: arrived (%d) completed", int(amount), arrived);
            recordScalar(buffer, completed);
            
            if (completed > 0) {
                double avg = _txnAvgCompTimeBySize[amount] / completed;
                sprintf(buffer, "size %d: avg_comp_time ", int(amount));
                recordScalar(buffer, avg);

                _tailCompBySizeFile << amount << ": ";
                for (auto const& time : _txnTailCompTimesBySize[amount]) 
                    _tailCompBySizeFile << time << " ";
                _tailCompBySizeFile << endl;
                _txnTailCompTimesBySize[amount].clear();
            }
        }
        _tailCompBySizeFile.close(); 
    }
}


/*
 *  given an adjacent node, and TransUnit queue of things to send to that node, sends
 *  TransUnits until channel funds are too low
 *  calls forwardTransactionMessage on every individual TransUnit
 *  returns true when it still can continue processing more transactions
 */
bool hostNodeBase:: processTransUnits(int dest, vector<tuple<int, double , routerMsg *, Id, simtime_t>>& q) {
    int successful = true;
    while ((int)q.size() > 0 && successful == 1) {
        pop_heap(q.begin(), q.end(), _schedulingAlgorithm);
        successful = forwardTransactionMessage(get<2>(q.back()), dest, get<4>(q.back()));
        if (successful == 1){
            q.pop_back();
        }
    }
    return (successful != 0); // anything other than balance exhausted implies you can go on
}


/* removes all of the cancelled messages from the queues to any
 * of the adjacent payment channels
 */
void hostNodeBase::deleteMessagesInQueues(){
    for (auto iter = nodeToPaymentChannel.begin(); iter!=nodeToPaymentChannel.end(); iter++){
        int key = iter->first;
        for (auto temp = (nodeToPaymentChannel[key].queuedTransUnits).begin();
                temp!= (nodeToPaymentChannel[key].queuedTransUnits).end(); ){
            routerMsg * rMsg = get<2>(*temp);
            auto tMsg = rMsg->getEncapsulatedPacket();
            rMsg->decapsulate();
            delete tMsg;
            delete rMsg;
            temp = (nodeToPaymentChannel[key].queuedTransUnits).erase(temp);
        }
    }

    // remove any waiting transactions too
    for (auto iter = nodeToDestInfo.begin(); iter!=nodeToDestInfo.end(); iter++){
        int dest = iter->first;
        for (auto &rMsg : nodeToDestInfo[dest].transWaitingToBeSent) {
            auto tMsg = rMsg->getEncapsulatedPacket();
            rMsg->decapsulate();
            delete tMsg;
            delete rMsg;
        }
        nodeToDestInfo[dest].transWaitingToBeSent.clear();
    }
}

/* helper method to set a particular payment channel's balance to the passed in amount 
 */ 
void hostNodeBase::setPaymentChannelBalanceByNode(int node, double amt){
       nodeToPaymentChannel[node].balance = amt;
}

#########################################################################################
/home/kush/Desktop/spiderproj/spider_omnet/hostNodeCeler.cc

#include "routerNodeCeler.h"

Define_Module(routerNodeCeler);

/* initialization function to initialize debt queues to 0 */
void routerNodeCeler::initialize(){
    routerNodeBase::initialize();

    for (int i = 0; i < _numHostNodes; ++i) { 
        _nodeToDebtQueue[myIndex()][i] = 0;
        nodeToDestNodeStruct[i].queueTimedOutSignal = registerSignalPerDest("queueTimedOut", i, "");
        nodeToDestNodeStruct[i].destQueueSignal = registerSignalPerDest("destQueue", i, "");
    }

    for ( auto it = nodeToPaymentChannel.begin(); it!= nodeToPaymentChannel.end(); it++){
        PaymentChannel *p = &(it->second);
        int id = it->first;

        if (id < _numHostNodes){
            adjacentHostNodes.insert(id);
        }
        p->kStarSignal = registerSignalPerChannel("kStar", id); 
        for (int destNode = 0; destNode < _numHostNodes; destNode++){
            simsignal_t signal;
            signal = registerSignalPerChannelPerDest("cpi", id, destNode);
            p->destToCPISignal[destNode] = signal;
            p->destToCPIValue[destNode] = -1;
        }
    }
}

/* end routine to get rid of messages in per router queues to every destination */
void routerNodeCeler::finish() {
    for (int i = 0; i < _numHostNodes; ++i) {
        if (nodeToDestNodeStruct.count(i) > 0) {
            vector<tuple<int, double, routerMsg*,  Id, simtime_t >> *q = 
                &(nodeToDestNodeStruct[i].queuedTransUnits);
            for (auto temp = q->begin(); temp != q->end(); ){
                routerMsg * rMsg = get<2>(*temp);
                auto tMsg = rMsg->getEncapsulatedPacket();
                rMsg->decapsulate();
                delete tMsg;
                delete rMsg;
                temp = q->erase(temp);
            }
        }
    }
    routerNodeBase::finish();
}

/* handler for the statistic message triggered every x seconds
 * emits kstar for every payment channel and queue sizes to every destination
 */
void routerNodeCeler::handleStatMessage(routerMsg* ttmsg){
    if (_signalsEnabled) {
        for ( auto it = nodeToPaymentChannel.begin(); it!= nodeToPaymentChannel.end(); it++){
            int node = it->first; 
            PaymentChannel* p = &(nodeToPaymentChannel[node]);
            unordered_set<int> exclude;
            emit(p->kStarSignal, findKStar(node, exclude));
            for (auto destNode = 0; destNode < _numHostNodes; destNode++){
                emit(p->destToCPISignal[destNode], p->destToCPIValue[destNode]);
            }
        }
        for (auto destNode = 0; destNode < _numHostNodes; destNode++) {
            DestNodeStruct *destNodeInfo = &(nodeToDestNodeStruct[destNode]);
            emit(destNodeInfo->queueTimedOutSignal, destNodeInfo->totalNumTimedOut);
            emit(destNodeInfo->destQueueSignal, destNodeInfo->totalAmtInQueue);
        }
    }

    // call the base method to output rest of the stats
    routerNodeBase::handleStatMessage(ttmsg);
}



/* handler for timeout messages that is responsible for removing messages from 
 * per-dest queues if they haven't been sent yet and sends explicit time out messages
 * for messages that have been sent on a path already
 * uses a structure to find the next hop and sends the time out there
 */
void routerNodeCeler::handleTimeOutMessage(routerMsg* ttmsg) {
    timeOutMsg *toutMsg = check_and_cast<timeOutMsg *>(ttmsg->getEncapsulatedPacket());
    int destination = toutMsg->getReceiver();
    int transactionId = toutMsg->getTransactionId();
    int prevNode = (ttmsg->getRoute())[ttmsg->getHopCount()-1];

    // check where to send timeout message next if a next hop exists 
    int nextNode = -1;
    if (transToNextHop.count(transactionId) > 0) {
        nextNode = transToNextHop[transactionId].front();
        transToNextHop[transactionId].pop_front();
        if (transToNextHop[transactionId].size() == 0)
            transToNextHop.erase(transactionId);
        appendNextHopToTimeOutMessage(ttmsg, nextNode);
        forwardMessage(ttmsg);
    }
    if (nextNode == -1)     
        updateMaxTravelTime(ttmsg->getRoute());

    // if there's an old entry, update with current time
    // otherwise insert transactions into canceled trans list
    auto iter = canceledTransactions.find(make_tuple(transactionId, 0, 0, 0, 0));
    if (iter != canceledTransactions.end()){
        canceledTransactions.erase(iter);
    } 
    CanceledTrans ct = make_tuple(toutMsg->getTransactionId(), 
    simTime(), prevNode, nextNode, destination);
    canceledTransactions.insert(ct);
    
    if (nextNode == -1) {
        ttmsg->decapsulate();
        delete toutMsg;
        delete ttmsg;
    }
}


/* handler for clearing state associated with timed out transaction
 * from per dest queues 
 */
void routerNodeCeler::handleClearStateMessage(routerMsg* ttmsg) {
    //reschedule for the next interval
    if (simTime() > _simulationLength){
        delete ttmsg;
    }
    else{
        scheduleAt(simTime()+_clearRate, ttmsg);
    }

    for ( auto it = nodeToPaymentChannel.begin(); it!= nodeToPaymentChannel.end(); it++){
        PaymentChannel *p = &(it->second);
        int id = it->first;
        p->channelImbalance.push_back(p->deltaAmtReceived - p->deltaAmtSent);
        emit(p->numSentPerChannelSignal, p->deltaAmtSent);
	p->deltaAmtReceived = 0;
        p->deltaAmtSent = 0;
        if (p->channelImbalance.size() > _maxImbalanceWindow){ 
            p->channelImbalance.erase(p->channelImbalance.begin());
        }
    }
    
    for ( auto it = canceledTransactions.begin(); it!= canceledTransactions.end(); ) {      
        int transactionId = get<0>(*it);
        simtime_t msgArrivalTime = get<1>(*it);
        int prevNode = get<2>(*it);
        int nextNode = get<3>(*it);
        int destNode = get<4>(*it);
        vector<tuple<int, double, routerMsg*,  Id, simtime_t >> *transList = 
        &(nodeToDestNodeStruct[destNode].queuedTransUnits);
        
        // if grace period has passed
        if (simTime() > msgArrivalTime + _maxTravelTime){  
            // check if txn is still in just sender queue, just delete and return then
            auto iter = find_if(transList->begin(),
                transList->end(),
                [&transactionId](tuple<int, double, routerMsg*,  Id, simtime_t> p)
                { return get<0>(get<3>(p)) == transactionId; });

            if (iter != transList->end()) {
                deleteTransaction(get<2>(*iter));
                double amount = get<1>(*iter);
                transList->erase(iter);
                make_heap((*transList).begin(), (*transList).end(), _schedulingAlgorithm);
                nodeToDestNodeStruct[destNode].totalAmtInQueue -= amount;
                _nodeToDebtQueue[myIndex()][destNode] -= amount;
                nodeToDestNodeStruct[destNode].totalNumTimedOut = 
                    nodeToDestNodeStruct[destNode].totalNumTimedOut + 1;
            }
            
            // go through all payment channels and remove from incoming and outgoing if present
            for (auto pIt = nodeToPaymentChannel.begin(); pIt != nodeToPaymentChannel.end(); ++pIt) {
                PaymentChannel* channel  = &(pIt->second);
                unordered_map<Id, double, hashId> *incomingTransUnits = 
                    &(channel->incomingTransUnits);
                auto iterIncoming = find_if((*incomingTransUnits).begin(),
                  (*incomingTransUnits).end(),
                  [&transactionId](const pair<tuple<int, int >, double> & p)
                  { return get<0>(p.first) == transactionId; });
                
                if (iterIncoming != (*incomingTransUnits).end()){
                    channel->totalAmtIncomingInflight -= iterIncoming->second;
                    iterIncoming = (*incomingTransUnits).erase(iterIncoming);
                }
            }
        }
               
       // remove from outgoing transUnits if present
       if (simTime() > msgArrivalTime + _maxTravelTime + _maxOneHopDelay){  
           for (auto pIt = nodeToPaymentChannel.begin(); pIt != nodeToPaymentChannel.end(); ++pIt) {
               PaymentChannel* channel  = &(pIt->second);
               unordered_map<Id, double, hashId> *outgoingTransUnits = 
                   &(channel->outgoingTransUnits);
               
               auto iterOutgoing = find_if((*outgoingTransUnits).begin(),
                 (*outgoingTransUnits).end(),
                 [&transactionId](const pair<tuple<int, int >, double> &q)
                 { return get<0>(q.first) == transactionId; });
               
               if (iterOutgoing != (*outgoingTransUnits).end()){
                   double amount = iterOutgoing->second;
                   channel->totalAmtOutgoingInflight -= amount;
                   iterOutgoing = (*outgoingTransUnits).erase(iterOutgoing);
                   
                   double updatedBalance = channel->balance + amount;
                   setPaymentChannelBalanceByNode(pIt->first, updatedBalance);
                   channel->balanceEWMA = (1 -_ewmaFactor) * channel->balanceEWMA + 
                       (_ewmaFactor) * updatedBalance;
               }
           }
           // all done, can remove txn and update stats
           it = canceledTransactions.erase(it);
       }
       else{
           it++;
       }
    }
}

/* main routine for handling transaction messages for celer
 * first adds transactions to the appropriate per destination queue at a router
 * and then processes transactions in order of the destination with highest CPI
 */
void routerNodeCeler::handleTransactionMessage(routerMsg* ttmsg){
    transactionMsg *transMsg = check_and_cast<transactionMsg *>(ttmsg->getEncapsulatedPacket());
    int hopcount = ttmsg->getHopCount();
    int destNode = transMsg->getReceiver();
    int transactionId = transMsg->getTransactionId();

    // ignore if txn is already cancelled
    auto iter = canceledTransactions.find(make_tuple(transactionId, 0, 0, 0, 0));
    if (iter != canceledTransactions.end() ){
        //delete yourself, message won't be encountered again
        ttmsg->decapsulate();
        delete transMsg;
        delete ttmsg;
        return;
    }

    // add to incoming trans units
    int prevNode = ttmsg->getRoute()[ttmsg->getHopCount()-1];
    unordered_map<Id, double, hashId> *incomingTransUnits =
            &(nodeToPaymentChannel[prevNode].incomingTransUnits);
    (*incomingTransUnits)[make_tuple(transMsg->getTransactionId(), transMsg->getHtlcIndex())] =
            transMsg->getAmount();
    nodeToPaymentChannel[prevNode].totalAmtIncomingInflight += transMsg->getAmount();
    
    // queue the transaction
    DestNodeStruct *destStruct = &(nodeToDestNodeStruct[destNode]);
    vector<tuple<int, double , routerMsg *, Id, simtime_t>> *q = &(destStruct->queuedTransUnits);
    tuple<int,int > key = make_tuple(transMsg->getTransactionId(), transMsg->getHtlcIndex());

    // add to queue and process in order of queue
    (*q).push_back(make_tuple(transMsg->getPriorityClass(), transMsg->getAmount(),
            ttmsg, key, simTime()));
    push_heap((*q).begin(), (*q).end(), _schedulingAlgorithm);
    
    // update debt queues and process according to celer
    destStruct->totalAmtInQueue += transMsg->getAmount();
    _nodeToDebtQueue[myIndex()][destNode] += transMsg->getAmount();
    celerProcessTransactions();
}

/* specialized ack handler that removes transaction information
 * from the transToNextHop map
 * NOTE: acks are on the reverse path relative to the original sender
 */
void routerNodeCeler::handleAckMessage(routerMsg* ttmsg) {
    ackMsg *aMsg = check_and_cast<ackMsg*>(ttmsg->getEncapsulatedPacket());
    int transactionId = aMsg->getTransactionId();
    int nextNode = ttmsg->getRoute()[ttmsg->getHopCount()+1];
    transToNextHop[transactionId].pop_back();
    if (transToNextHop[transactionId].size() == 0)
        transToNextHop.erase(transactionId);
    if (aMsg->getIsSuccess()) {
        nodeToPaymentChannel[nextNode].deltaAmtReceived +=  aMsg->getAmount();
        nodeToPaymentChannel[nextNode].totalAmtReceived +=  aMsg->getAmount();
    }
    routerNodeBase::handleAckMessage(ttmsg);
}

/* special type of time out message for celer designed for a specific path 
 * that is contructed dynamically or one hop at a time, until the transaction
 * is deleted at the router itself and then the message needs to go 
 * no further
 */
void routerNodeCeler::appendNextHopToTimeOutMessage(routerMsg* ttmsg, int nextNode) {
    vector<int> newRoute = ttmsg->getRoute();
    newRoute.push_back(nextNode);
    ttmsg->setRoute(newRoute);
}

/* helper function to process transactions to the neighboring node if there are transactions to 
 * be sent on this payment channel, if one is passed in
 * otherwise use any payment channel to send out transactions
 */
void routerNodeCeler::celerProcessTransactions(int neighborNode){
    unordered_set<int> exclude;
    if (neighborNode != -1){
        int kStar = findKStar(neighborNode, exclude);
        while (kStar >= 0){
            vector<tuple<int, double , routerMsg *, Id, simtime_t>> *q;
            q = &(nodeToDestNodeStruct[kStar].queuedTransUnits);
            if (!processTransUnits(neighborNode, *q))
                break;
            exclude.insert(kStar); // ignore this dest because queue is empty
            kStar = findKStar(neighborNode, exclude);
        }
    }
    else{
        // get all paymentChannels with positive balance
        vector<int> positiveKey = {};
        for (auto iter = nodeToPaymentChannel.begin(); iter != nodeToPaymentChannel.end(); ++iter){
           if (iter->second.balance > 0){
               positiveKey.push_back(iter->first);
           }
        }
        while (true){
            exclude.clear();
            if (positiveKey.size() == 0)
               break;
            
            //generate random channel with positive balance to process
            int randIdx = rand() % positiveKey.size();
            int key = positiveKey[randIdx]; //node
            positiveKey.erase(positiveKey.begin() + randIdx);
            
            if (key < _numHostNodes){ //adjacent host node payment channel
                vector<tuple<int, double, routerMsg *, Id, simtime_t>> *k;
                k = &(nodeToDestNodeStruct[key].queuedTransUnits);
                processTransUnits(key, *k);
            }
            else {
                // for each payment channel (nextNode), choose a k* or
                // destNode queue to use as q*, and send as much as possible to that dest
                // if no more transactions left, keep finding the next kStar for that channel
                // until it is exhausted or no more transactions in any dest queue
                int kStar = findKStar(key, exclude);
                while (kStar >= 0) {
                    vector<tuple<int, double, routerMsg *, Id, simtime_t>> *k;
                    k = &(nodeToDestNodeStruct[kStar].queuedTransUnits);
                    if (!processTransUnits(key, *k))
                        break;
                    exclude.insert(kStar); // ignore this dest because queue is empty
                    kStar = findKStar(key, exclude);
                }
                if (kStar == -1) // no more transactions in any dest queue
                    break;
            }
        }
    }
}

/* helper function to calculate the destination with the maximum CPI weight
 * that we should send transactions to on this payment channel
 */
int routerNodeCeler::findKStar(int neighborNode, unordered_set<int> exclude){
    int destNode = -1;
    int highestCPI = -1000000;
    for (int i = 0; i < _numHostNodes; ++i) { //initialize debt queues map
        if (adjacentHostNodes.count(i) == 0 && nodeToDestNodeStruct.count(i) > 0 && exclude.count(i) == 0) {
            double CPI = calculateCPI(i, neighborNode); 
            if (destNode == -1 || (CPI > highestCPI)){
                destNode = i;
                highestCPI = CPI;
            }
        }
    }
    return destNode;
}

/* helper function to calculate congestion plus imbalance price 
 */
double routerNodeCeler::calculateCPI(int destNode, int neighborNode){
    PaymentChannel *neighbor = &(nodeToPaymentChannel[neighborNode]);
    double channelImbalance = 0;
    accumulate(neighbor->channelImbalance.begin(), neighbor->channelImbalance.end(), channelImbalance);
    double Q_ik = _nodeToDebtQueue[myIndex()][destNode];
    double Q_jk = _nodeToDebtQueue[neighborNode][destNode];

    double W_ijk = Q_ik - Q_jk + _celerBeta*channelImbalance;
    neighbor->destToCPIValue[destNode] = W_ijk;
    return W_ijk;
}

/* updates debt queue information (removing from it) before performing the regular
 * routine of forwarding a transction only if there's balance on the payment channel
 */
int routerNodeCeler::forwardTransactionMessage(routerMsg *msg, int nextNode, simtime_t arrivalTime) {
    transactionMsg *transMsg = check_and_cast<transactionMsg *>(msg->getEncapsulatedPacket());
    int transactionId = transMsg->getTransactionId();
    PaymentChannel *neighbor = &(nodeToPaymentChannel[nextNode]);
    int dest = transMsg->getReceiver();
    int amt = transMsg->getAmount();
    Id thisTrans = make_tuple(transactionId, transMsg->getHtlcIndex());

    if (neighbor->balance <= 0 || transMsg->getAmount() > neighbor->balance){
        return 0;
    }
    else if (neighbor->incomingTransUnits.count(thisTrans) > 0 ||
            neighbor->outgoingTransUnits.count(thisTrans) > 0 || 
            (nextNode < _numHostNodes && nextNode != dest)) {
        // don't cause cycles, don't send to end host that's not destination
        return -1;
    }
    else {
        // if cancelled, remove it from queue calculations 
        auto iter = canceledTransactions.find(make_tuple(transactionId, 0, 0, 0, 0));
        if (iter != canceledTransactions.end()) {
            nodeToDestNodeStruct[dest].totalAmtInQueue -= transMsg->getAmount();
            _nodeToDebtQueue[myIndex()][dest] -= transMsg->getAmount();
            return routerNodeBase::forwardTransactionMessage(msg, nextNode, arrivalTime);
        }

        //append next node to the route of the routerMsg
        vector<int> newRoute = msg->getRoute();
        newRoute.push_back(nextNode);
        transToNextHop[transactionId].push_back(nextNode);
        msg->setRoute(newRoute);

        //decrement the total amount in queue to the destination
        nodeToDestNodeStruct[dest].totalAmtInQueue -= transMsg->getAmount();
        _nodeToDebtQueue[myIndex()][dest] -= transMsg->getAmount();

        //increment statAmtSent for channel in-balance calculations
        neighbor->deltaAmtSent+=  transMsg->getAmount();
        neighbor->totalAmtSent+=  transMsg->getAmount();
        return routerNodeBase::forwardTransactionMessage(msg, nextNode, arrivalTime);
    }
    return 1;
}


/* set balance of a payment channel to the passed in amount and if funds were added process
 * more payments that can be sent via celer
 */
void routerNodeCeler::setPaymentChannelBalanceByNode(int node, double amt){
    bool addedFunds = false;
    if (amt > nodeToPaymentChannel[node].balance){
        addedFunds = true;
    }
    nodeToPaymentChannel[node].balance = amt;
    if (addedFunds){
        celerProcessTransactions(node);
    }
}

############################################################################
/home/kush/Desktop/spiderproj/spider_omnet/hostNodeDCTCP.cc

#include "hostNodeDCTCP.h"

Define_Module(hostNodeDCTCP);

double _windowAlpha;
double _windowBeta;
double _qEcnThreshold;
double _qDelayEcnThreshold;
double _minDCTCPWindow;
double _balEcnThreshold;
bool _qDelayVersion;
bool _tcpVersion;
bool _isCubic;
double _cubicScalingConstant;

// knobs for enabling changing of paths
bool _changingPathsEnabled;
double _windowThresholdForChange;
int _maxPathsToConsider;
double _monitorRate;

/* generate path change trigger message every x seconds
 * that goes through all the paths and replaces the ones
 * with tiny windows
 */
routerMsg *hostNodeDCTCP::generateMonitorPathsMessage(){
    char msgname[MSGSIZE];
    sprintf(msgname, "node-%d monitorPathsMsg", myIndex());
    routerMsg *rMsg = new routerMsg(msgname);
    rMsg->setMessageType(MONITOR_PATHS_MSG);
    return rMsg;
}

/* initialization function to initialize parameters */
void hostNodeDCTCP::initialize(){
    hostNodeBase::initialize();
    
    if (myIndex() == 0) {
        // parameters
        _windowAlpha = par("windowAlpha");
        _windowBeta = par("windowBeta");
        _qEcnThreshold = par("queueThreshold");
        _qDelayEcnThreshold = par("queueDelayEcnThreshold");
        _qDelayVersion = par("DCTCPQEnabled");
        _tcpVersion = par("TCPEnabled");
        _balEcnThreshold = par("balanceThreshold");
        _minDCTCPWindow = par("minDCTCPWindow");
        _isCubic = par("cubicEnabled");
        _cubicScalingConstant = 0.4;

        // changing paths related
        _changingPathsEnabled = par("changingPathsEnabled");
        _maxPathsToConsider = par("maxPathsToConsider");
        _windowThresholdForChange = par("windowThresholdForChange");
        _monitorRate = par("pathMonitorRate");
    }

     //initialize signals with all other nodes in graph
    // that there is demand for
    for (int i = 0; i < _numHostNodes; ++i) {
        if (_destList[myIndex()].count(i) > 0) {
            simsignal_t signal;
            signal = registerSignalPerDest("demandEstimate", i, "");
            demandEstimatePerDestSignals[i] = signal;
            
            signal = registerSignalPerDest("numWaiting", i, "_Total");
            numWaitingPerDestSignals[i] = signal;
        }
    }

    //generate monitor paths messag to start a little later in the experiment
    if (_changingPathsEnabled) {
        routerMsg *monitorMsg = generateMonitorPathsMessage();
        scheduleAt(simTime() + 150, monitorMsg);
    }
}

/* overall controller for handling messages that dispatches the right function
 * based on message type in price Scheme
 */
void hostNodeDCTCP::handleMessage(cMessage *msg) {
    routerMsg *ttmsg = check_and_cast<routerMsg *>(msg);
    if (simTime() > _simulationLength){
        auto encapMsg = (ttmsg->getEncapsulatedPacket());
        ttmsg->decapsulate();
        delete ttmsg;
        delete encapMsg;
        return;
    } 

    switch(ttmsg->getMessageType()) {
        case MONITOR_PATHS_MSG:
             if (_loggingEnabled) cout<< "[HOST "<< myIndex() 
                 <<": RECEIVED MONITOR_PATHS_MSG] "<< ttmsg->getName() << endl;
             handleMonitorPathsMessage(ttmsg);
             if (_loggingEnabled) cout<< "[AFTER HANDLING:]  "<< endl;
             break;

        default:
             hostNodeBase::handleMessage(msg);

    }
}


/* specialized ack handler that does the routine if this is DCTCP
 * algorithm. In particular, collects/updates stats for this path alone
 * and updates the window for this path based on whether the packet is marked 
 * or not
 * NOTE: acks are on the reverse path relative to the original sender
 */
void hostNodeDCTCP::handleAckMessageSpecialized(routerMsg* ttmsg) {
    ackMsg *aMsg = check_and_cast<ackMsg*>(ttmsg->getEncapsulatedPacket());
    int pathIndex = aMsg->getPathIndex();
    int destNode = ttmsg->getRoute()[0];
    int transactionId = aMsg->getTransactionId();
    double largerTxnId = aMsg->getLargerTxnId();
    PathInfo *thisPathInfo = &(nodeToShortestPathsMap[destNode][pathIndex]);

    // window update based on marked or unmarked packet for non-cubic approach
    if (!_isCubic) {
        if (aMsg->getIsMarked()) {
            thisPathInfo->window  -= _windowBeta;
            thisPathInfo->window = max(_minDCTCPWindow, thisPathInfo->window);
            thisPathInfo->markedPackets += 1;
            thisPathInfo->inSlowStart = false;
        }
        else {
            double sumWindows = 0;
            for (auto p: nodeToShortestPathsMap[destNode])
                sumWindows += p.second.window;

            if (thisPathInfo->inSlowStart) {
                thisPathInfo->window += 1;
                if (thisPathInfo->window > thisPathInfo->windowThreshold) 
                    thisPathInfo->inSlowStart = false;
            }
            else if (!_tcpVersion)
                thisPathInfo->window += _windowAlpha / sumWindows;
            else if (_tcpVersion)
                thisPathInfo->window += _windowAlpha / thisPathInfo->window;
            thisPathInfo->unmarkedPackets += 1; 
        }
    }

    // cubic's window update based on successful or unsuccessful transmission
    if (_isCubic) {
        if (aMsg->getIsSuccess() == true) {
            if (thisPathInfo->inSlowStart) {
                thisPathInfo->window += 1;
                if (thisPathInfo->window > thisPathInfo->windowThreshold) 
                    thisPathInfo->inSlowStart = false;
            } else {
                double K = cbrt(thisPathInfo->windowMax * _windowBeta / _cubicScalingConstant);
                double timeElapsed = simTime().dbl() - thisPathInfo->lastWindowReductionTime;
                double timeTerm = (timeElapsed - K) * (timeElapsed - K) * (timeElapsed - K);
                thisPathInfo->window = _cubicScalingConstant * timeTerm + thisPathInfo->windowMax;
            }
        } else {
            /* lost packet */
            if (thisPathInfo->window < thisPathInfo->windowMax) {
                thisPathInfo->windowMax = thisPathInfo->window * ( 1 - _windowBeta/2.0);
            } else {
                thisPathInfo->windowMax = thisPathInfo->window;
            }
            thisPathInfo->lastWindowReductionTime = simTime().dbl();
            thisPathInfo->window *= (1 - _windowBeta);
            thisPathInfo->inSlowStart = false;
        }
    }
    thisPathInfo->window = min(thisPathInfo->window, thisPathInfo->windowThreshold);

    // general bookkeeping to track completion state
    if (aMsg->getIsSuccess() == false) {
        // make sure transaction isn't cancelled yet
        auto iter = canceledTransactions.find(make_tuple(transactionId, 0, 0, 0, 0));
        if (iter != canceledTransactions.end()) {
            if (aMsg->getTimeSent() >= _transStatStart && aMsg->getTimeSent() <= _transStatEnd)
                statAmtFailed[destNode] += aMsg->getAmount();
        } 
        else {
            // requeue transaction
            routerMsg *duplicateTrans = generateDuplicateTransactionMessage(aMsg);
            pushIntoSenderQueue(&(nodeToDestInfo[destNode]), duplicateTrans);
        }
    }
    else {
        SplitState* splitInfo = &(_numSplits[myIndex()][largerTxnId]);
        splitInfo->numReceived += 1;

        if (aMsg->getTimeSent() >= _transStatStart && 
                aMsg->getTimeSent() <= _transStatEnd) {
            statAmtCompleted[destNode] += aMsg->getAmount();

            if (splitInfo->numTotal == splitInfo->numReceived) {
                statRateCompleted[destNode] += 1;
                _transactionCompletionBySize[splitInfo->totalAmount] += 1;
                double timeTaken = simTime().dbl() - splitInfo->firstAttemptTime;
                statCompletionTimes[destNode] += timeTaken * 1000;
                _txnAvgCompTimeBySize[splitInfo->totalAmount] += timeTaken * 1000;
                recordTailCompletionTime(aMsg->getTimeSent(), splitInfo->totalAmount, timeTaken * 1000);
            }
        }
        if (splitInfo->numTotal == splitInfo->numReceived) 
            statNumCompleted[destNode] += 1; 
        thisPathInfo->statRateCompleted += 1;
        thisPathInfo->amtAcked += aMsg->getAmount();
    }

    //increment transaction amount ack on a path. 
    tuple<int,int> key = make_tuple(transactionId, pathIndex);
    if (transPathToAckState.count(key) > 0) {
        transPathToAckState[key].amtReceived += aMsg->getAmount();
        thisPathInfo->sumOfTransUnitsInFlight -= aMsg->getAmount();
    }
   
    destNodeToNumTransPending[destNode] -= 1;     
    hostNodeBase::handleAckMessage(ttmsg);
    sendMoreTransactionsOnPath(destNode, pathIndex);
}


/* handler for timeout messages that is responsible for removing messages from 
 * sender queues if they haven't been sent yet and sends explicit time out messages
 * for messages that have been sent on a path already
 */
void hostNodeDCTCP::handleTimeOutMessage(routerMsg* ttmsg) {
    timeOutMsg *toutMsg = check_and_cast<timeOutMsg *>(ttmsg->getEncapsulatedPacket());
    int destination = toutMsg->getReceiver();
    int transactionId = toutMsg->getTransactionId();
    multiset<routerMsg*, transCompare> *transList = &(nodeToDestInfo[destination].transWaitingToBeSent);
    
    if (ttmsg->isSelfMessage()) {
        // if transaction was successful don't do anything more
        if (successfulDoNotSendTimeOut.count(transactionId) > 0) {
            successfulDoNotSendTimeOut.erase(toutMsg->getTransactionId());
            ttmsg->decapsulate();
            delete toutMsg;
            delete ttmsg;
            return;
        }

        // check if txn is still in just sender queue, just delete and return then
        auto iter = find_if(transList->begin(),
           transList->end(),
           [&transactionId](const routerMsg* p)
           { transactionMsg *transMsg = check_and_cast<transactionMsg *>(p->getEncapsulatedPacket());
             return transMsg->getTransactionId()  == transactionId; });

        if (iter != transList->end()) {
            deleteTransaction(*iter);
            transList->erase(iter);
            ttmsg->decapsulate();
            delete toutMsg;
            delete ttmsg;
            return;
        }

        // send out a time out message on the path that hasn't acked all of it
        for (auto p : (nodeToShortestPathsMap[destination])){
            int pathIndex = p.first;
            tuple<int,int> key = make_tuple(transactionId, pathIndex);
                        
            if (transPathToAckState.count(key) > 0 && 
                    transPathToAckState[key].amtSent != transPathToAckState[key].amtReceived) {
                routerMsg* psMsg = generateTimeOutMessageForPath(
                    nodeToShortestPathsMap[destination][p.first].path, 
                    transactionId, destination);
                int nextNode = (psMsg->getRoute())[psMsg->getHopCount()+1];
                CanceledTrans ct = make_tuple(toutMsg->getTransactionId(), 
                        simTime(), -1, nextNode, destination);
                canceledTransactions.insert(ct);
                forwardMessage(psMsg);
            }
            else {
                transPathToAckState.erase(key);
            }
        }
        ttmsg->decapsulate();
        delete toutMsg;
        delete ttmsg;
    }
    else {
        // at the receiver
        CanceledTrans ct = make_tuple(toutMsg->getTransactionId(),simTime(),
                (ttmsg->getRoute())[ttmsg->getHopCount()-1], -1, toutMsg->getReceiver());
        canceledTransactions.insert(ct);
        ttmsg->decapsulate();
        delete toutMsg;
        delete ttmsg;
    }
}

/* initialize data for a particular path with path index to the dest supplied in the arguments
 * and also fix the paths for susbequent transactions to this destination
 * and register signals that are path specific
 */
void hostNodeDCTCP::initializeThisPath(vector<int> thisPath, int pathIdx, int destNode) {
    // initialize pathInfo
    PathInfo temp = {};
    temp.path = thisPath;
    temp.window = _minDCTCPWindow;
    temp.inSlowStart = true;
    temp.sumOfTransUnitsInFlight = 0;
    temp.inUse = true;
    temp.timeStartedUse = simTime().dbl();
    temp.windowThreshold = bottleneckCapacityOnPath(thisPath); 
    temp.rttMin = (thisPath.size() - 1) * 2 * _avgDelay/1000.0;
    nodeToShortestPathsMap[destNode][pathIdx] = temp;

    // update the index of the highest path found, if you've circled back to 0, then refresh
    // the max index back to 0
    if (pathIdx > nodeToDestInfo[destNode].maxPathId || pathIdx == 0)
        nodeToDestInfo[destNode].maxPathId = pathIdx;

    // initialize signals
    simsignal_t signal;
    signal = registerSignalPerDestPath("sumOfTransUnitsInFlight", pathIdx, destNode);
    nodeToShortestPathsMap[destNode][pathIdx].sumOfTransUnitsInFlightSignal = signal;

    signal = registerSignalPerDestPath("window", pathIdx, destNode);
    nodeToShortestPathsMap[destNode][pathIdx].windowSignal = signal;

    signal = registerSignalPerDestPath("rateOfAcks", pathIdx, destNode);
    nodeToShortestPathsMap[destNode][pathIdx].rateOfAcksSignal = signal;
    
    signal = registerSignalPerDestPath("fractionMarked", pathIdx, destNode);
    nodeToShortestPathsMap[destNode][pathIdx].fractionMarkedSignal = signal;
}


/* initialize data for for the paths supplied to the destination node
 * and also fix the paths for susbequent transactions to this destination
 * and register signals that are path specific
 */
void hostNodeDCTCP::initializePathInfo(vector<vector<int>> kShortestPaths, int destNode){
    for (int pathIdx = 0; pathIdx < kShortestPaths.size(); pathIdx++){
        initializeThisPath(kShortestPaths[pathIdx], pathIdx, destNode);
    }
}

/* get maximum window size from amongs the paths considered
 */
double hostNodeDCTCP::getMaxWindowSize(unordered_map<int, PathInfo> pathList) {
    double maxWindowSize = 0;
    for (auto p = pathList.begin(); p != pathList.end(); p++) {
        int pathIndex = p->first;
        PathInfo *pInfo = &(p->second);

        if (pInfo->inUse && pInfo->windowSum/_monitorRate > maxWindowSize)
            maxWindowSize = pInfo->windowSum/_monitorRate;
    }
    return max(maxWindowSize, _minDCTCPWindow);
}

/* routine to monitor paths periodically and change them out if need be
 * in particular, marks the path as "candidate" for changing if its window is small
 * next time, it actually changes the path out if its window is still small
 * next interval it tears down the old path altogether
 */
void hostNodeDCTCP::handleMonitorPathsMessage(routerMsg* ttmsg) {
    // reschedule this message to be sent again
    if (simTime() > _simulationLength || !_changingPathsEnabled){
        delete ttmsg;
    }
    else {
        scheduleAt(simTime() + _monitorRate, ttmsg);
    }

    for (auto it = 0; it < _numHostNodes; it++){ 
        if (it != getIndex() && _destList[myIndex()].count(it) > 0) {
            if (nodeToShortestPathsMap.count(it) > 0) {
                double maxWindowSize = getMaxWindowSize(nodeToShortestPathsMap[it]);

                for (auto p = nodeToShortestPathsMap[it].begin(); p != nodeToShortestPathsMap[it].end();){
                    int pathIndex = p->first;
                    PathInfo *pInfo = &(p->second);

                    // if path is in use currently and has had a very small window for a long period
                    // replace it unless you've gone through all K paths and have no options
                    if (pInfo->inUse) {
                        double timeSincePathUse = simTime().dbl() - pInfo->timeStartedUse;
                        if (pInfo->windowSum/_monitorRate <= _windowThresholdForChange * maxWindowSize / 100.0 
                                && nodeToDestInfo[it].sumTxnsWaiting/_monitorRate > 0 
                                && timeSincePathUse > 10.0) {
                            int maxK = nodeToDestInfo[it].maxPathId;
                            if (pInfo->candidate) {
                                tuple<int, vector<int>> nextPath;

                                while (true) {
                                    nextPath =  getNextPath(getIndex(), it, maxK);
                                    int nextPathIndex = get<0>(nextPath);
                                    maxK =  (nextPathIndex == 0) ? 0 : maxK + 1;
                                    
                                    // valid new path found
                                    if (nodeToShortestPathsMap[it].count(nextPathIndex) == 0)
                                       break;
                                    
                                    // implies we came a full circle, no eligible path
                                    if (nextPathIndex == pathIndex) {
                                        pInfo->candidate = false;
                                        break;
                                    }
                                }

                                // if this path is still a candidate, then find a new path
                                if (pInfo->candidate) {
                                    initializeThisPath(get<1>(nextPath), get<0>(nextPath), it);
                                    pInfo->inUse = false;
                                }
                            }
                            else {
                                pInfo->candidate = true;
                            }
                        } else {
                            pInfo->candidate = false;
                        }
                        pInfo->windowSum = 0;
                        p++;
                    } else {
                        p = nodeToShortestPathsMap[it].erase(p);
                    }
                } 
            }
            nodeToDestInfo[it].sumTxnsWaiting = 0;
        }
    } 
}

/* handles forwarding of  transactions out of the queue
 * the way other schemes' routers do except that it marks the packet
 * if the queue is larger than the threshold, therfore mostly similar to the base code */ 
int hostNodeDCTCP::forwardTransactionMessage(routerMsg *msg, int dest, simtime_t arrivalTime) {
    transactionMsg *transMsg = check_and_cast<transactionMsg *>(msg->getEncapsulatedPacket());
    int nextDest = msg->getRoute()[msg->getHopCount()+1];
    PaymentChannel *neighbor = &(nodeToPaymentChannel[nextDest]);
    
    //don't mark yet if the packet can't be sent out
    if (neighbor->balance <= 0 || transMsg->getAmount() > neighbor->balance){
        return 0;
    }
 
    // else mark before forwarding if need be
    vector<tuple<int, double , routerMsg *, Id, simtime_t>> *q;
    q = &(neighbor->queuedTransUnits);
    if (_qDelayVersion) {
        simtime_t curQueueingDelay = simTime()  - arrivalTime;
        if (curQueueingDelay.dbl() > _qDelayEcnThreshold) {
            transMsg->setIsMarked(true); 
        }
    } 
    else {
        if (getTotalAmount(nextDest) > _qEcnThreshold) {
            transMsg->setIsMarked(true); 
        }
    }
    return hostNodeBase::forwardTransactionMessage(msg, dest, arrivalTime);
}


/* main routine for handling a new transaction under DCTCP
 * In particular, only sends out transactions if the window permits it */
void hostNodeDCTCP::handleTransactionMessageSpecialized(routerMsg* ttmsg){
    transactionMsg *transMsg = check_and_cast<transactionMsg *>(ttmsg->getEncapsulatedPacket());
    int hopcount = ttmsg->getHopCount();
    int destNode = transMsg->getReceiver();
    int transactionId = transMsg->getTransactionId();

    SplitState* splitInfo = &(_numSplits[myIndex()][transMsg->getLargerTxnId()]);
    splitInfo->numArrived += 1;
    
    // first time seeing this transaction so add to d_ij computation
    // count the txn for accounting also
    if (simTime() == transMsg->getTimeSent()) {
        destNodeToNumTransPending[destNode]  += 1;
        nodeToDestInfo[destNode].transSinceLastInterval += transMsg->getAmount();
        if (splitInfo->numArrived == 1)
            splitInfo->firstAttemptTime = simTime().dbl();

        if (transMsg->getTimeSent() >= _transStatStart && 
            transMsg->getTimeSent() <= _transStatEnd) {
            statAmtArrived[destNode] += transMsg->getAmount();
            
            if (splitInfo->numArrived == 1) {       
                statRateArrived[destNode] += 1; 
            }
        }
        if (splitInfo->numArrived == 1) 
            statNumArrived[destNode] += 1;
    }

    // initiate paths and windows if it is a new destination
    if (nodeToShortestPathsMap.count(destNode) == 0 && ttmsg->isSelfMessage()){
        int kForPaths = _kValue;
        vector<vector<int>> kShortestRoutes = getKPaths(transMsg->getSender(),destNode, kForPaths);
        initializePathInfo(kShortestRoutes, destNode);
    }


    // at destination, add to incoming transUnits and trigger ack
    if (transMsg->getReceiver() == myIndex()) {
       handleTransactionMessage(ttmsg, false); 
    }
    else if (ttmsg->isSelfMessage()) {
        // at sender, either queue up or send on a path that allows you to send
        DestInfo* destInfo = &(nodeToDestInfo[destNode]);
            
        // use a random ordering on the path indices
        vector<int> pathIndices;
        for (auto p: nodeToShortestPathsMap[destNode]) pathIndices.push_back(p.first);
        random_shuffle(pathIndices.begin(), pathIndices.end());
       
        //send on a path if no txns queued up and timer was in the path
        if ((destInfo->transWaitingToBeSent).size() > 0) {
            pushIntoSenderQueue(destInfo, ttmsg);
            sendMoreTransactionsOnPath(destNode, -1);
        } else {
            // try the first path in this random ordering
            for (auto p : pathIndices) {
                int pathIndex = p;
                PathInfo *pathInfo = &(nodeToShortestPathsMap[destNode][pathIndex]);
                
                if (pathInfo->sumOfTransUnitsInFlight + transMsg->getAmount() <= pathInfo->window &&
                        pathInfo->inUse) {
                    ttmsg->setRoute(pathInfo->path);
                    ttmsg->setHopCount(0);
                    transMsg->setPathIndex(pathIndex);
                    handleTransactionMessage(ttmsg, true /*revisit*/);

                    // first attempt of larger txn
                    SplitState* splitInfo = &(_numSplits[myIndex()][transMsg->getLargerTxnId()]);
                    if (splitInfo->numAttempted == 0) {
                        splitInfo->numAttempted += 1;
                        if (transMsg->getTimeSent() >= _transStatStart && 
                            transMsg->getTimeSent() <= _transStatEnd) 
                            statRateAttempted[destNode] += 1;
                    }
                    
                    if (transMsg->getTimeSent() >= _transStatStart && 
                            transMsg->getTimeSent() <= _transStatEnd) {
                        statAmtAttempted[destNode] += transMsg->getAmount();
                    }
                    
                    // update stats
                    pathInfo->statRateAttempted += 1;
                    pathInfo->sumOfTransUnitsInFlight += transMsg->getAmount();

                    // necessary for knowing what path to remove transaction in flight funds from
                    tuple<int,int> key = make_tuple(transMsg->getTransactionId(), pathIndex); 
                    transPathToAckState[key].amtSent += transMsg->getAmount();
                    
                    return;
                }
            }
            
            //transaction cannot be sent on any of the paths, queue transaction
            pushIntoSenderQueue(destInfo, ttmsg);
        }
    }
}

/* handler for the statistic message triggered every x seconds to also
 * output DCTCP scheme stats in addition to the default
 */
void hostNodeDCTCP::handleStatMessage(routerMsg* ttmsg){
    if (_signalsEnabled) {
        for (auto it = 0; it < _numHostNodes; it++){ 
            if (it != getIndex() && _destList[myIndex()].count(it) > 0) {
                if (nodeToShortestPathsMap.count(it) > 0) {
                    for (auto& p: nodeToShortestPathsMap[it]){
                        int pathIndex = p.first;
                        PathInfo *pInfo = &(p.second);
                        emit(pInfo->sumOfTransUnitsInFlightSignal, 
                                pInfo->sumOfTransUnitsInFlight);
                        emit(pInfo->windowSignal, pInfo->window);
                        emit(pInfo->rateOfAcksSignal, pInfo->amtAcked/_statRate);
                        emit(pInfo->fractionMarkedSignal, 
                                pInfo->markedPackets/(pInfo->markedPackets + pInfo->unmarkedPackets));
                        pInfo->amtAcked = 0;
                        pInfo->unmarkedPackets = 0;
                        pInfo->markedPackets = 0;
                    }
                }
                emit(numWaitingPerDestSignals[it], 
                    nodeToDestInfo[it].transWaitingToBeSent.size());
                emit(demandEstimatePerDestSignals[it], nodeToDestInfo[it].transSinceLastInterval/_statRate);
                nodeToDestInfo[it].transSinceLastInterval = 0;
            }        
        }
    } 

    // call the base method to output rest of the stats
    hostNodeBase::handleStatMessage(ttmsg);
}

/* handler for the clear state message that deals with
 * transactions that will no longer be completed
 * In particular clears out the amount inn flight on the path
 */
void hostNodeDCTCP::handleClearStateMessage(routerMsg *ttmsg) {
    // average windows over the last second
    for (auto it = 0; it < _numHostNodes; it++){ 
        if (it != getIndex() && _destList[myIndex()].count(it) > 0) {
            nodeToDestInfo[it].sumTxnsWaiting += nodeToDestInfo[it].transWaitingToBeSent.size();
            if (nodeToShortestPathsMap.count(it) > 0) {
                for (auto& p: nodeToShortestPathsMap[it]){
                    int pathIndex = p.first;
                    PathInfo *pInfo = &(p.second);
                    pInfo->windowSum += pInfo->window;
                }
            }
        }
    }

    // handle cancellations
    for ( auto it = canceledTransactions.begin(); it!= canceledTransactions.end(); it++){
        int transactionId = get<0>(*it);
        simtime_t msgArrivalTime = get<1>(*it);
        int prevNode = get<2>(*it);
        int nextNode = get<3>(*it);
        int destNode = get<4>(*it);
        
        if (simTime() > (msgArrivalTime + _maxTravelTime + 0.001)){
            // ack was not received,safely can consider this txn done
            for (auto p : nodeToShortestPathsMap[destNode]) {
                int pathIndex = p.first;
                tuple<int,int> key = make_tuple(transactionId, pathIndex);
                if (transPathToAckState.count(key) != 0) {
                    nodeToShortestPathsMap[destNode][pathIndex].sumOfTransUnitsInFlight -= 
                        (transPathToAckState[key].amtSent - transPathToAckState[key].amtReceived);
                    transPathToAckState.erase(key);

                    // treat this basiscally as one marked packet
                    nodeToShortestPathsMap[destNode][pathIndex].window  -= _windowBeta;
                    nodeToShortestPathsMap[destNode][pathIndex].window = max(_minDCTCPWindow, 
                         nodeToShortestPathsMap[destNode][pathIndex].window);
                    nodeToShortestPathsMap[destNode][pathIndex].markedPackets += 1; 
                }
            }
        }
    }

    // works fine now because timeouts start per transaction only when
    // sent out and no txn splitting
    hostNodeBase::handleClearStateMessage(ttmsg);
}

/* helper method to remove a transaction from the sender queue and send it on a particular path
 * to the given destination (or multiplexes across all paths) */
void hostNodeDCTCP::sendMoreTransactionsOnPath(int destNode, int pathId) {
    transactionMsg *transMsg;
    routerMsg *msgToSend;
    int randomIndex;

    // construct a vector with the path indices
    vector<int> pathIndices;
    for (auto p: nodeToShortestPathsMap[destNode]) pathIndices.push_back(p.first);

    //remove the transaction $tu$ at the head of the queue if one exists
    while (nodeToDestInfo[destNode].transWaitingToBeSent.size() > 0) {
        auto firstPosition = nodeToDestInfo[destNode].transWaitingToBeSent.begin();
        routerMsg *msgToSend = *firstPosition;
        transMsg = check_and_cast<transactionMsg *>(msgToSend->getEncapsulatedPacket());

        // pick a path at random to try and send on unless a path is given
        int pathIndex;
        if (pathId != -1)
            pathIndex = pathId;
        else {
            randomIndex = rand() % pathIndices.size();
            pathIndex = pathIndices[randomIndex];
        }

        PathInfo *pathInfo = &(nodeToShortestPathsMap[destNode][pathIndex]);
        if (pathInfo->sumOfTransUnitsInFlight + transMsg->getAmount() <= pathInfo->window && pathInfo->inUse) {
            // remove the transaction from queue and send it on the path
            nodeToDestInfo[destNode].transWaitingToBeSent.erase(firstPosition);
            msgToSend->setRoute(pathInfo->path);
            msgToSend->setHopCount(0);
            transMsg->setPathIndex(pathIndex);
            handleTransactionMessage(msgToSend, true /*revisit*/);

            // first attempt of larger txn
            SplitState* splitInfo = &(_numSplits[myIndex()][transMsg->getLargerTxnId()]);
            if (splitInfo->numAttempted == 0) {
                splitInfo->numAttempted += 1;
                if (transMsg->getTimeSent() >= _transStatStart && 
                    transMsg->getTimeSent() <= _transStatEnd) 
                    statRateAttempted[destNode] += 1;
            }
            
            if (transMsg->getTimeSent() >= _transStatStart && 
                    transMsg->getTimeSent() <= _transStatEnd) {
                statAmtAttempted[destNode] += transMsg->getAmount();
            }
            
            // update stats
            pathInfo->statRateAttempted += 1;
            pathInfo->sumOfTransUnitsInFlight += transMsg->getAmount();

            // necessary for knowing what path to remove transaction in flight funds from
            tuple<int,int> key = make_tuple(transMsg->getTransactionId(), pathIndex); 
            transPathToAckState[key].amtSent += transMsg->getAmount();
        }
        else {
            // if this path is the only path you are trying and it is exhausted, return
            if (pathId != -1)
                return;

            // if no paths left to multiplex, return
            pathIndices.erase(pathIndices.begin() + randomIndex);
            if (pathIndices.size() == 0)
                return;
        }
    }
}


######################################################################
/home/kush/Desktop/spiderproj/spider_omnet/hostNodeLandmarkRouting.cc

#include "hostNodeLandmarkRouting.h"

// set of landmarks for landmark routing
vector<tuple<int,int>> _landmarksWithConnectivityList = {};
vector<int> _landmarks;

Define_Module(hostNodeLandmarkRouting);

/* generates the probe message for a particular destination and a particur path
 * identified by the list of hops and the path index
 */
routerMsg* hostNodeLandmarkRouting::generateProbeMessage(int destNode, int pathIdx, vector<int> path){
    char msgname[MSGSIZE];
    sprintf(msgname, "tic-%d-to-%d probeMsg [idx %d]", myIndex(), destNode, pathIdx);
    probeMsg *pMsg = new probeMsg(msgname);
    pMsg->setSender(myIndex());
    pMsg->setPathIndex(pathIdx);
    pMsg->setReceiver(destNode);
    pMsg->setIsReversed(false);
    vector<double> pathBalances;
    pMsg->setPathBalances(pathBalances);
    pMsg->setPath(path);

    sprintf(msgname, "tic-%d-to-%d router-probeMsg [idx %d]", myIndex(), destNode, pathIdx);
    routerMsg *rMsg = new routerMsg(msgname);
    rMsg->setRoute(path);

    rMsg->setHopCount(0);
    rMsg->setMessageType(PROBE_MSG);

    rMsg->encapsulate(pMsg);
    return rMsg;
}

/* forwards probe messages for waterfilling alone that appends the current balance
 * to the list of balances
 */
void hostNodeLandmarkRouting::forwardProbeMessage(routerMsg *msg){
    // Increment hop count.
    msg->setHopCount(msg->getHopCount()+1);
    //use hopCount to find next destination
    int nextDest = msg->getRoute()[msg->getHopCount()];

    probeMsg *pMsg = check_and_cast<probeMsg *>(msg->getEncapsulatedPacket());
    if (pMsg->getIsReversed() == false){
        vector<double> *pathBalances = & ( pMsg->getPathBalances());
        (*pathBalances).push_back(nodeToPaymentChannel[nextDest].balanceEWMA);
    }

   if (_loggingEnabled) cout << "forwarding " << msg->getMessageType() << " at " 
       << simTime() << endl;
   send(msg, nodeToPaymentChannel[nextDest].gate);
}


/* overall controller for handling messages that dispatches the right function
 * based on message type in Landmark Routing
 */
void hostNodeLandmarkRouting::handleMessage(cMessage *msg) {
    routerMsg *ttmsg = check_and_cast<routerMsg *>(msg);
    if (simTime() > _simulationLength){
        auto encapMsg = (ttmsg->getEncapsulatedPacket());
        ttmsg->decapsulate();
        delete ttmsg;
        delete encapMsg;
        return;
    } 

    switch(ttmsg->getMessageType()) {
        case PROBE_MSG:
             if (_loggingEnabled) cout<< "[HOST "<< myIndex() 
                 <<": RECEIVED PROBE MSG] "<< ttmsg->getName() << endl;
             handleProbeMessage(ttmsg);
             if (_loggingEnabled) cout<< "[AFTER HANDLING:]  "<< endl;
             break;

        default:
             hostNodeBase::handleMessage(msg);
    }
}

/* main routine for handling a new transaction under landmark routing 
 * In particular, initiate probes at sender and send acknowledgements
 * at the receiver
 */
void hostNodeLandmarkRouting::handleTransactionMessageSpecialized(routerMsg* ttmsg){
    transactionMsg *transMsg = check_and_cast<transactionMsg *>(ttmsg->getEncapsulatedPacket());
    int hopcount = ttmsg->getHopCount();
    vector<tuple<int, double , routerMsg *, Id>> *q;
    int destNode = transMsg->getReceiver();
    int destination = destNode;
    int transactionId = transMsg->getTransactionId();
    SplitState* splitInfo = &(_numSplits[myIndex()][transMsg->getLargerTxnId()]);

    // if its at the sender, initiate probes, when they come back,
    // call normal handleTransactionMessage
    if (ttmsg->isSelfMessage()) {
        if (transMsg->getTimeSent() >= _transStatStart && 
            transMsg->getTimeSent() <= _transStatEnd) {
            statRateArrived[destination] += 1;
            statAmtArrived[destination] += transMsg->getAmount();
            splitInfo->firstAttemptTime = simTime().dbl();
        }
        statNumArrived[destination] += 1; 

        AckState * s = new AckState();
        s->amtReceived = 0;
        s->amtSent = transMsg->getAmount();
        transToAmtLeftToComplete[transMsg->getTransactionId()] = *s;

        // if destination hasn't been encountered, find paths
        if (nodeToShortestPathsMap.count(destNode) == 0 ){
            vector<vector<int>> kShortestRoutes = getKShortestRoutesLandmarkRouting(transMsg->getSender(), 
                    destNode, _kValue);
            initializePathInfoLandmarkRouting(kShortestRoutes, destNode);
        }
        
        initializeLandmarkRoutingProbes(ttmsg, transMsg->getTransactionId(), destNode);
    }
    else if (ttmsg->getHopCount() ==  ttmsg->getRoute().size() - 1) { 
       handleTransactionMessage(ttmsg, false); 
    }
    else {
        cout << "entering this case " << endl;
        assert(false);
    }
}


/* handles the special time out mechanism for landmark routing which is responsible
 * for sending time out messages on all paths that may have seen this txn and 
 * marking the txn as cancelled
 */
void hostNodeLandmarkRouting::handleTimeOutMessage(routerMsg* ttmsg){
    timeOutMsg *toutMsg = check_and_cast<timeOutMsg *>(ttmsg->getEncapsulatedPacket());

    if (ttmsg->isSelfMessage()) { 
        //is at the sender
        int transactionId = toutMsg->getTransactionId();
        int destination = toutMsg->getReceiver();

        if (transToAmtLeftToComplete.count(transactionId) == 0) {
                delete ttmsg;
                return;
        }
        
        for (auto p : (nodeToShortestPathsMap[destination])){
            int pathIndex = p.first;
            tuple<int,int> key = make_tuple(transactionId, pathIndex);
            if(_loggingEnabled) {
                cout << "transPathToAckState.count(key): " 
                   << transPathToAckState.count(key) << endl;
                cout << "transactionId: " << transactionId 
                    << "; pathIndex: " << pathIndex << endl;
            }
            
            if (transPathToAckState[key].amtSent > transPathToAckState[key].amtReceived + _epsilon) {
                int nextNode = nodeToShortestPathsMap[destination][pathIndex].path[1];
                CanceledTrans ct = make_tuple(toutMsg->getTransactionId(), 
                        simTime(), -1, nextNode, destination);
                canceledTransactions.insert(ct);
            }
            else {
                transPathToAckState.erase(key);
            }
        }
        delete ttmsg;
    }
    else{
        // at the receiver
        CanceledTrans ct = make_tuple(toutMsg->getTransactionId(),simTime(),
                (ttmsg->getRoute())[ttmsg->getHopCount()-1], -1, toutMsg->getReceiver());
        canceledTransactions.insert(ct);
        ttmsg->decapsulate();
        delete toutMsg;
        delete ttmsg;
    }
}


/* handles to logic for ack messages in the presence of timeouts
 * in particular, removes the transaction from the cancelled txns
 * to mark that it has been received 
 * it uses the transAmtSent vs Received to detect if txn is complete
 * and therefore is different from the base class 
 */
void hostNodeLandmarkRouting::handleAckMessageTimeOut(routerMsg* ttmsg){
    ackMsg *aMsg = check_and_cast<ackMsg *>(ttmsg->getEncapsulatedPacket());
    int transactionId = aMsg->getTransactionId();

    if (aMsg->getIsSuccess()) {
        double totalAmtReceived = (transToAmtLeftToComplete[transactionId]).amtReceived +
            aMsg->getAmount();
        if (totalAmtReceived != transToAmtLeftToComplete[transactionId].amtSent) 
            return;
        
        auto iter = canceledTransactions.find(make_tuple(transactionId, 0, 0, 0, 0));
        if (iter!=canceledTransactions.end()) {
            canceledTransactions.erase(iter);
        }
    }
}


/* specialized ack handler that does the routine for handling acks
 * across paths. In particular, collects/updates stats for this path alone
 * NOTE: acks are on the reverse path relative to the original sender
 */
void hostNodeLandmarkRouting::handleAckMessageSpecialized(routerMsg* ttmsg) {
    ackMsg *aMsg = check_and_cast<ackMsg *>(ttmsg->getEncapsulatedPacket());
    int receiver = aMsg->getReceiver();
    int pathIndex = aMsg->getPathIndex();
    int transactionId = aMsg->getTransactionId();
    tuple<int,int> key = make_tuple(transactionId, pathIndex);
   
    if (aMsg->getIsSuccess()) { 
        if (transToAmtLeftToComplete.count(transactionId) == 0){
            cout << "error, transaction " << transactionId 
              <<" htlc index:" << aMsg->getHtlcIndex() 
              << " acknowledged at time " << simTime() 
              << " wasn't written to transToAmtLeftToComplete for amount " <<  aMsg->getAmount() << endl;
        }
        else {
            (transToAmtLeftToComplete[transactionId]).amtReceived += aMsg->getAmount();
            if (aMsg->getTimeSent() >= _transStatStart && aMsg->getTimeSent() <= _transStatEnd) {
                statAmtCompleted[receiver] += aMsg->getAmount();
            }
            
            double amtReceived = transToAmtLeftToComplete[transactionId].amtReceived;
            double amtSent = transToAmtLeftToComplete[transactionId].amtSent;

            if (amtReceived < amtSent + _epsilon && amtReceived > amtSent -_epsilon) {
                nodeToShortestPathsMap[receiver][pathIndex].statRateCompleted += 1;

                if (aMsg->getTimeSent() >= _transStatStart && aMsg->getTimeSent() <= _transStatEnd) {
                    statRateCompleted[receiver] += 1;
                    _transactionCompletionBySize[amtSent] += 1;

                    SplitState* splitInfo = &(_numSplits[myIndex()][aMsg->getLargerTxnId()]);
                    double timeTaken = simTime().dbl() - splitInfo->firstAttemptTime;
                    statCompletionTimes[receiver] += timeTaken * 1000;
                    _txnAvgCompTimeBySize[amtSent] += timeTaken * 1000;
                    recordTailCompletionTime(aMsg->getTimeSent(), amtSent, timeTaken * 1000);
                }
                statNumCompleted[receiver] += 1; 
                transToAmtLeftToComplete.erase(aMsg->getTransactionId());
            }
            transPathToAckState[key].amtReceived += aMsg->getAmount();
        }
    }
    hostNodeBase::handleAckMessage(ttmsg);
}


/* handler that clears additional state particular to lr 
 * when a cancelled transaction is deemed no longer completeable
 * in particular it clears the state that tracks how much of a
 * transaction is still pending
 * calls the base class's handler after its own handler
 */
void hostNodeLandmarkRouting::handleClearStateMessage(routerMsg *ttmsg) {
    for ( auto it = canceledTransactions.begin(); it!= canceledTransactions.end(); it++){
        int transactionId = get<0>(*it);
        simtime_t msgArrivalTime = get<1>(*it);
        int prevNode = get<2>(*it);
        int nextNode = get<3>(*it);
        int destNode = get<4>(*it);
        
        if (simTime() > (msgArrivalTime + _maxTravelTime + 1)){
            for (auto p : nodeToShortestPathsMap[destNode]) {
                int pathIndex = p.first;
                tuple<int,int> key = make_tuple(transactionId, pathIndex);
                if (transPathToAckState.count(key) != 0) {
                    transPathToAckState.erase(key);
                }
            }
        }
    }
    hostNodeBase::handleClearStateMessage(ttmsg);
}

/* handle Probe Message for Landmark Routing 
 * In essence, is waiting for all the probes, finding those paths 
 * with non-zero bottleneck balance and splitting the transaction
 * amongst them
 */
void hostNodeLandmarkRouting::handleProbeMessage(routerMsg* ttmsg){
    probeMsg *pMsg = check_and_cast<probeMsg *>(ttmsg->getEncapsulatedPacket());
    int transactionId = pMsg->getTransactionId();

    if (simTime() > _simulationLength ){
        ttmsg->decapsulate();
        delete pMsg;
        delete ttmsg;
        return;
    }

    bool isReversed = pMsg->getIsReversed();
    int nextDest = ttmsg->getRoute()[ttmsg->getHopCount()+1];
    
    if (isReversed == true){ 
       //store times into private map, delete message
       int pathIdx = pMsg->getPathIndex();
       int destNode = pMsg->getReceiver();
       double bottleneck = minVectorElemDouble(pMsg->getPathBalances());
       ProbeInfo *probeInfo = &(transactionIdToProbeInfoMap[transactionId]);
       
       probeInfo->probeReturnTimes[pathIdx] = simTime();
       probeInfo->numProbesWaiting -= 1; 
       probeInfo->probeBottlenecks[pathIdx] = bottleneck;

       // once all probes are back
       if (probeInfo->numProbesWaiting == 0){ 
           int numPathsPossible = 0;
           for (auto bottleneck: probeInfo->probeBottlenecks){
               if (bottleneck > 0){
                   numPathsPossible++;
               }
           }
           
           transactionMsg *transMsg = check_and_cast<transactionMsg*>(
                   probeInfo->messageToSend->getEncapsulatedPacket());
           vector<double> amtPerPath(probeInfo->probeBottlenecks.size());

           if (numPathsPossible > 0 && 
                   randomSplit(transMsg->getAmount(), probeInfo->probeBottlenecks, amtPerPath)) {
               if (transMsg->getTimeSent() >= _transStatStart && transMsg->getTimeSent() <= _transStatEnd) {
                   statRateAttempted[destNode] += 1;
                   statAmtAttempted[destNode] += transMsg->getAmount();
               }

               for (int i = 0; i < amtPerPath.size(); i++) {
                   double amt = amtPerPath[i];
                   if (amt > 0) {
                       tuple<int,int> key = make_tuple(transMsg->getTransactionId(), i); 
                       //update the data structure keeping track of how much sent and received on each path
                       if (transPathToAckState.count(key) == 0){
                           AckState temp = {};
                           temp.amtSent = amt;
                           temp.amtReceived = 0;
                           transPathToAckState[key] = temp;
                        }
                        else {
                            transPathToAckState[key].amtSent =  transPathToAckState[key].amtSent + amt;
                        }

                       // send a new transaction on that path with that amount
                       PathInfo *pathInfo = &(nodeToShortestPathsMap[destNode][i]);
                       routerMsg* lrMsg = generateTransactionMessageForPath(amt, 
                               pathInfo->path, i, transMsg); 
                       handleTransactionMessage(lrMsg, true /*revisit*/);
                   }
               }
           } 
           else {
                if (transMsg->getTimeSent() >= _transStatStart && transMsg->getTimeSent() <= _transStatEnd) {
                    statRateFailed[destNode] += 1;
                    statAmtFailed[destNode] += transMsg->getAmount();
                }
               transToAmtLeftToComplete.erase(transactionId);
           }

           probeInfo->messageToSend->decapsulate();
           delete transMsg;
           delete probeInfo->messageToSend;
           transactionIdToProbeInfoMap.erase(transactionId);
       }
       
       ttmsg->decapsulate();
       delete pMsg;
       delete ttmsg;
   }
   else { 
       //reverse and send message again from receiver
       pMsg->setIsReversed(true);
       ttmsg->setHopCount(0);
       vector<int> route = ttmsg->getRoute();
       reverse(route.begin(), route.end());
       ttmsg->setRoute(route);
       forwardMessage(ttmsg);
   }
}

/* function to compute a random split across all the paths for landmark routing
 */
bool hostNodeLandmarkRouting::randomSplit(double totalAmt, vector<double> bottlenecks, 
        vector<double> &amtPerPath) {
    vector<int> pathsPossible;
    vector<int> nextSet;
    double remainingAmt = totalAmt;

    // start with non-zero bottlneck paths
    for (int i = 0; i < bottlenecks.size(); i++)
        if (bottlenecks[i] > 0)
            pathsPossible.push_back(i);

    // keep allocating while you can
    while (remainingAmt > _epsilon - _epsilon && pathsPossible.size() > 0) {
        nextSet.clear();
        random_shuffle(pathsPossible.begin(), pathsPossible.end());
        for (int i : pathsPossible) {
            double amtToAdd = round((remainingAmt - 1) * (rand() / RAND_MAX + 1.) + 1);
            if (amtPerPath[i] + amtToAdd > bottlenecks[i])
                amtToAdd = bottlenecks[i] - amtPerPath[i];
            else 
                nextSet.push_back(i);
            amtPerPath[i] += amtToAdd;
            remainingAmt -= amtToAdd;
            if (remainingAmt <= _epsilon)
                break;
        }       
        pathsPossible = nextSet;
    }
    return (remainingAmt <= _epsilon);
}

/* initializes the table with the paths to use for Landmark Routing, everything else as 
 * to how many probes are in progress is initialized when probes are sent
 * This function only helps for memoization
 */
void hostNodeLandmarkRouting::initializePathInfoLandmarkRouting(vector<vector<int>> kShortestPaths, 
        int  destNode){ 
    nodeToShortestPathsMap[destNode] = {};
    for (int pathIdx = 0; pathIdx < kShortestPaths.size(); pathIdx++){
        PathInfo temp = {};
        nodeToShortestPathsMap[destNode][pathIdx] = temp;
        nodeToShortestPathsMap[destNode][pathIdx].path = kShortestPaths[pathIdx];
    }
    return;
}


/* initializes the actual balance queries for landmark routing and the state 
 * to keep track of how many of them are currently outstanding and how many of them
 * have returned with what balances
 * the msg passed is the transaction that triggered this set of probes which also
 * corresponds to the txn that needs to be sent out once all probes return
 */
void hostNodeLandmarkRouting::initializeLandmarkRoutingProbes(routerMsg * msg, 
        int transactionId, int destNode){
    ProbeInfo probeInfoTemp =  {};
    probeInfoTemp.messageToSend = msg; //message to send out once all probes return
    probeInfoTemp.probeReturnTimes = {}; //probeReturnTimes[0] is return time of first probe

    for (auto pTemp: nodeToShortestPathsMap[destNode]){
        int pathIndex = pTemp.first;
        PathInfo pInfo = pTemp.second;
        vector<int> path = pInfo.path;
        routerMsg * rMsg = generateProbeMessage(destNode , pathIndex, path);

        //set the transactionId in the generated message
        probeMsg *pMsg = check_and_cast<probeMsg *>(rMsg->getEncapsulatedPacket());
        pMsg->setTransactionId(transactionId);
        forwardProbeMessage(rMsg);

        probeInfoTemp.probeReturnTimes.push_back(-1);
        probeInfoTemp.probeBottlenecks.push_back(-1);
    }

    // set number of probes waiting on to be the number of total paths to 
    // this particular destination (might be less than k, so not safe to use that)
    probeInfoTemp.numProbesWaiting = nodeToShortestPathsMap[destNode].size();
    transactionIdToProbeInfoMap[transactionId] = probeInfoTemp;
    return;
}

/* function that is called at the end of the simulation that
 * deletes any remaining messages in transactionIdToProbeInfoMap
 */
void hostNodeLandmarkRouting::finish() {
    for (auto it = transactionIdToProbeInfoMap.begin(); it != transactionIdToProbeInfoMap.end(); it++) {
        ProbeInfo *probeInfo = &(it->second);
        transactionMsg *transMsg = check_and_cast<transactionMsg*>(
            probeInfo->messageToSend->getEncapsulatedPacket());
        probeInfo->messageToSend->decapsulate();
        delete transMsg;
        delete probeInfo->messageToSend;
    }
    hostNodeBase::finish();
}


#################################################################################
/home/kush/Desktop/spiderproj/spider_omnet/hostNodeLndBaseline.cc

#include "hostNodeLndBaseline.h"
Define_Module(hostNodeLndBaseline);

//instaniate global parameter for hostNodeLndBaseline
double _restorePeriod;
int _numAttemptsLndBaseline;
vector<int> succRetriesList, failRetriesList;

/* helper function for sorting heap by prune time */
bool sortPrunedChannelsFunction(tuple<simtime_t, tuple<int,int>> x, tuple<simtime_t, tuple<int, int>> y){
    simtime_t xTime = get<0>( x );
    simtime_t yTime = get<0>( y );
    return xTime > yTime; //makes smallest simtime_t appear first
} 


/* helper function to record the tail retries for successful and failed transactions
 * for LND */
void hostNodeLndBaseline::recordTailRetries(simtime_t timeSent, bool success, int retries){
    if (timeSent >= _transStatStart && timeSent <= _transStatEnd) {
        vector<int> *retryList = success ? &succRetriesList : &failRetriesList;
        retryList->push_back(retries);
        if (retryList->size() == 1000) {
            if (success) {
                for (auto const& time : *retryList) 
                    _succRetriesFile << time << " ";
                _succRetriesFile << endl;
            } else {
                for (auto const& time : *retryList) 
                    _failRetriesFile << time << " ";
                _failRetriesFile << endl;
            }
            retryList->clear();
        }
    }
}


/* makes local copy of global _channels data structure, without 
 * delay, as paths are calculated using BFS (not weighted edges)
 * use this to periodically prune channels */
void hostNodeLndBaseline::initializeMyChannels(){
    //not going to store delay, because using BFS to find shortest paths
    _activeChannels.clear();
    for (auto nodeIter: _channels){
        int node = nodeIter.first;
        _activeChannels[node] = {};
        vector<pair<int, int>> edgeDelayVec = nodeIter.second;
        for (auto edgeIter: edgeDelayVec){
            int destNode = edgeIter.first;
            _activeChannels[node].insert(destNode);
        }
    }
}

/* generates next path, but adding in the edges whose retore times are over, then running
 * BFS on _activeChannels */
vector<int>  hostNodeLndBaseline::generateNextPath(int destNodePath){
    if (_prunedChannelsList.size() > 0){
        tuple<simtime_t, tuple<int, int>> currentEdge =  _prunedChannelsList.front();
        while (_prunedChannelsList.size()>0 && (get<0>(currentEdge) + _restorePeriod < simTime())){
            int sourceNode = get<0>(get<1>(currentEdge));
            int destNode = get<1>(get<1>(currentEdge));
            _activeChannels[sourceNode].insert(destNode);
            _prunedChannelsList.pop_front();
            if (_prunedChannelsList.size() > 0)
                currentEdge =  _prunedChannelsList.front();
        }
    }
    vector<int> resultPath = breadthFirstSearchByGraph(myIndex(),destNodePath, _activeChannels);
    return resultPath;       
}

/* given source and destination, will remove edge from _activeChannels, or if already removed,
 * update the time pruned */
void hostNodeLndBaseline::pruneEdge(int sourceNode, int destNode){
    tuple<int, int> edgeTuple = make_tuple(sourceNode, destNode);        
    auto iter = _activeChannels[sourceNode].find(destNode); 
    if (iter != _activeChannels[sourceNode].end())
    {
        _activeChannels[sourceNode].erase(iter);
        _prunedChannelsList.push_back(make_tuple(simTime(), make_tuple(sourceNode, destNode)));
    }
    else {
        auto iterHeap = find_if(_prunedChannelsList.begin(),
            _prunedChannelsList.end(),
            [&edgeTuple](const tuple<simtime_t, tuple<int,int>>& p)
            { return ((get<0>(get<1>(p)) == get<0>(edgeTuple)) && 
                    (get<1>(get<1>(p)) == get<1>(edgeTuple))); });

        if (iterHeap != _prunedChannelsList.end()){
            _prunedChannelsList.erase(iterHeap);
            _prunedChannelsList.push_back(make_tuple(simTime(), make_tuple(sourceNode, destNode)));
        }
    }
}


/* initialization function to initialize parameters */
void hostNodeLndBaseline::initialize(){
    hostNodeBase::initialize();
    for (int i = 0; i < _numHostNodes; ++i) {
        if (_destList[myIndex()].count(i) > 0 && _signalsEnabled) { 
            simsignal_t signal;
            signal = registerSignalPerDest("numPathsPerTrans", i, "");
            numPathsPerTransPerDestSignals.push_back(signal);
        }
    }
    
    _restorePeriod = 5.0;
    _numAttemptsLndBaseline = 20;
    initializeMyChannels(); 
    _prunedChannelsList = {};
}


/* calls base method and also reports any unreported success and failure retries
 */
void hostNodeLndBaseline::finish(){
    hostNodeBase::finish();

    if (myIndex() == 0) {
        for (auto const& t : succRetriesList) 
            _succRetriesFile << t << " ";
        _succRetriesFile << endl;
        _succRetriesFile.close();

        for (auto const& t : failRetriesList) 
            _failRetriesFile << t << " ";
        _failRetriesFile << endl;
        _failRetriesFile.close();
    }
}

/* generateAckMessage that encapsulates transaction message to use for reattempts 
 * Note: this is different from the hostNodeBase version that will delete the 
 * passed in transaction message */
routerMsg *hostNodeLndBaseline::generateAckMessage(routerMsg* ttmsg, bool isSuccess) {
    int sender = (ttmsg->getRoute())[0];
    int receiver = (ttmsg->getRoute())[(ttmsg->getRoute()).size() - 1];
    vector<int> route = ttmsg->getRoute();

    transactionMsg *transMsg = check_and_cast<transactionMsg *>(ttmsg->getEncapsulatedPacket());
    int transactionId = transMsg->getTransactionId();
    double timeSent = transMsg->getTimeSent();
    double amount = transMsg->getAmount();
    bool hasTimeOut = transMsg->getHasTimeOut();

    char msgname[MSGSIZE];
    sprintf(msgname, "receiver-%d-to-sender-%d ackMsg", receiver, sender);
    routerMsg *msg = new routerMsg(msgname);
    ackMsg *aMsg = new ackMsg(msgname);
    aMsg->setTransactionId(transactionId);
    aMsg->setIsSuccess(isSuccess);
    aMsg->setTimeSent(timeSent);
    aMsg->setAmount(amount);
    aMsg->setReceiver(transMsg->getReceiver());
    aMsg->setHasTimeOut(hasTimeOut);
    aMsg->setHtlcIndex(transMsg->getHtlcIndex());
    aMsg->setPathIndex(transMsg->getPathIndex());
    aMsg->setLargerTxnId(transMsg->getLargerTxnId());
    aMsg->setIsMarked(transMsg->getIsMarked());
    if (!isSuccess){
        aMsg->setFailedHopNum((route.size() - 1) - ttmsg->getHopCount());
    }
    
    //no need to set secret - not modelled
    reverse(route.begin(), route.end());
    msg->setRoute(route);

    //need to reverse path from current hop number in case of partial failure
    msg->setHopCount((route.size() - 1) - ttmsg->getHopCount());

    msg->setMessageType(ACK_MSG);
    ttmsg->decapsulate();
    aMsg->encapsulate(transMsg);
    msg->encapsulate(aMsg);
    delete ttmsg;
    return msg;
}


/* main routine for handling a new transaction under lnd baseline 
 */
void hostNodeLndBaseline::handleTransactionMessageSpecialized(routerMsg* ttmsg){
    transactionMsg *transMsg = check_and_cast<transactionMsg *>(ttmsg->getEncapsulatedPacket());
    int hopcount = ttmsg->getHopCount();
    vector<tuple<int, double , routerMsg *, Id>> *q;
    int destNode = transMsg->getReceiver();
    int transactionId = transMsg->getTransactionId();

    SplitState* splitInfo = &(_numSplits[myIndex()][transMsg->getLargerTxnId()]);
    splitInfo->numArrived += 1;

    // if its at the sender, initiate probes, when they come back,
    // call normal handleTransactionMessage
    if (ttmsg->isSelfMessage()) {
        if (splitInfo->numArrived == 1)
            splitInfo->firstAttemptTime = simTime().dbl();

        if (transMsg->getTimeSent() >= _transStatStart && 
            transMsg->getTimeSent() <= _transStatEnd) {
            statAmtArrived[destNode] += transMsg->getAmount();
            statAmtAttempted[destNode] += transMsg->getAmount();
            
            if (splitInfo->numArrived == 1) {       
                statRateArrived[destNode] += 1; 
                statRateAttempted[destNode] += 1;
                splitInfo->numTries += 1; 
            }
        }
        if (splitInfo->numArrived == 1)     
            statNumArrived[destNode] += 1;

        vector<int> newRoute = generateNextPath(destNode);
        //note: number of paths attempted is calculated as pathIndex + 1, so if fails
        //without attempting any paths, want 0 = -1+1
        transMsg->setPathIndex(-1);
        if (newRoute.size() > 0) {
            transMsg->setPathIndex(0);
            ttmsg->setRoute(newRoute);
            handleTransactionMessage(ttmsg, true);
        }
        else {
            routerMsg * failedAckMsg = generateAckMessage(ttmsg, false);
            handleAckMessageNoMoreRoutes(failedAckMsg, true);
        }
    }
    else {
        handleTransactionMessage(ttmsg,true);
    }
}

/* handleAckMessageNoMoreRoute - increments failed statistics, and deletes all three messages:
 * ackMsg, transMsg, routerMsg */
void hostNodeLndBaseline::handleAckMessageNoMoreRoutes(routerMsg *msg, bool toDelete){
    ackMsg *aMsg = check_and_cast<ackMsg *>(msg->getEncapsulatedPacket());
    transactionMsg *transMsg = check_and_cast<transactionMsg *>(aMsg->getEncapsulatedPacket());
    int numPathsAttempted = aMsg->getPathIndex() + 1;
    SplitState* splitInfo = &(_numSplits[myIndex()][aMsg->getLargerTxnId()]);

    if (aMsg->getTimeSent() >= _transStatStart && aMsg->getTimeSent() <= _transStatEnd) {
        statRateFailed[aMsg->getReceiver()] = statRateFailed[aMsg->getReceiver()] + 1;
        statAmtFailed[aMsg->getReceiver()] += aMsg->getAmount();
        recordTailRetries(aMsg->getTimeSent(), false, splitInfo->numTries); 
    }

    if (toDelete) {
        aMsg->decapsulate();
        delete transMsg;
        msg->decapsulate();
        delete aMsg;
        delete msg;
    }
}

/* handles ack messages - guaranteed to be returning from an attempted path to the sender
 * if successful or no more attempts left, call hostNodeBase's handleAckMsgSpecialized
 * if need to reattempt:
 * - reset state by handlingAckMessage
 * - see if more routes possible: if possible, then generate new transaction msg, else fail it
 */
void hostNodeLndBaseline::handleAckMessageSpecialized(routerMsg *msg)
{
    ackMsg *aMsg = check_and_cast<ackMsg *>(msg->getEncapsulatedPacket());
    transactionMsg *transMsg = check_and_cast<transactionMsg *>(aMsg->getEncapsulatedPacket());
    int transactionId = transMsg->getTransactionId();
    int destNode = msg->getRoute()[0];
    double largerTxnId = aMsg->getLargerTxnId();
    SplitState* splitInfo = &(_numSplits[myIndex()][largerTxnId]);
    
    auto iter = canceledTransactions.find(make_tuple(transactionId, 0, 0, 0, 0));
    int numPathsAttempted = aMsg->getPathIndex() + 1;

    if (aMsg->getIsSuccess() || (numPathsAttempted == _numAttemptsLndBaseline || 
            (_timeoutEnabled && iter != canceledTransactions.end()))) {
        //no more attempts allowed
        if (iter != canceledTransactions.end())
            canceledTransactions.erase(iter);

        if (aMsg->getIsSuccess()) {
            splitInfo->numReceived += 1;
            if (transMsg->getTimeSent() >= _transStatStart && 
                    transMsg->getTimeSent() <= _transStatEnd) {
                statAmtCompleted[destNode] += aMsg->getAmount();

                if (splitInfo->numTotal == splitInfo->numReceived) {
                    statRateCompleted[destNode] += 1;
                    _transactionCompletionBySize[splitInfo->totalAmount] += 1;
                    double timeTaken = simTime().dbl() - splitInfo->firstAttemptTime;
                    statCompletionTimes[destNode] += timeTaken * 1000;
                    _txnAvgCompTimeBySize[splitInfo->totalAmount] += timeTaken * 1000;
                    recordTailCompletionTime(aMsg->getTimeSent(), splitInfo->totalAmount, timeTaken * 1000);
                    recordTailRetries(aMsg->getTimeSent(), true, splitInfo->numTries);
                }
            }
            if (splitInfo->numTotal == splitInfo->numReceived) 
                statNumCompleted[destNode] += 1; 
        }
        else {
            statNumTimedOut[destNode] += 1;
        }
        aMsg->decapsulate();
        delete transMsg;
        hostNodeBase::handleAckMessage(msg);
    }
    else { 
        // more attempts allowed
        int newIndex = aMsg->getPathIndex() + 1;
        transMsg->setPathIndex(newIndex);

        //prune edge
        int failedHopNum = aMsg->getFailedHopNum();
        int failedSource = msg->getRoute()[failedHopNum];
        int failedDest = msg->getRoute()[failedHopNum - 1];
        pruneEdge(failedSource, failedDest);

        vector<int> newRoute = generateNextPath(transMsg->getReceiver());
        if (newRoute.size() == 0) {
            handleAckMessageNoMoreRoutes(msg, false);
            hostNodeBase::handleAckMessage(msg);
        } else if (iter != canceledTransactions.end()) {
            canceledTransactions.erase(iter);
            handleAckMessageNoMoreRoutes(msg, false);
            hostNodeBase::handleAckMessage(msg);
        }
        else {
            //generate new router  message for transaction message
            char msgname[MSGSIZE];
            sprintf(msgname, "tic-%d-to-%d transactionMsg", transMsg->getSender(), transMsg->getReceiver());
            routerMsg* ttmsg = new routerMsg(msgname);
            ttmsg->setRoute(newRoute);
            ttmsg->setHopCount(0);
            ttmsg->setMessageType(TRANSACTION_MSG);
            aMsg->decapsulate();
            hostNodeBase::handleAckMessage(msg);
            ttmsg->encapsulate(transMsg);
            splitInfo->numTries += 1;
            handleTransactionMessage(ttmsg, true);
        }   
    }
}


#########################################################################
/home/kush/Desktop/spiderproj/spider_omnet/hostNodePriceScheme.cc

#include "hostNodePriceScheme.h"

// global parameters
double _tokenBucketCapacity = 1.0;
bool _reschedulingEnabled; // whether timers can be rescheduled
bool _nesterov;
bool _secondOrderOptimization;
bool _useQueueEquation;

// parameters for price scheme
double _eta; //for price computation
double _kappa; //for price computation
double _capacityFactor; //for price computation
double _tUpdate; //for triggering price updates at routers
double _tQuery; //for triggering price query probes at hosts
double _alpha;
double _zeta;
double _delta;
double _avgDelay;
double _minPriceRate;
double _rhoLambda;
double _rhoMu;
double _rho;
double _minWindow;
double _xi;
double _routerQueueDrainTime;
double _smallRate = pow(10, -6); // ensuring that timers are no more than 1000 seconds away


Define_Module(hostNodePriceScheme);
/* generate the trigger message to initiate price Updates periodically
 */
routerMsg *hostNodePriceScheme::generateTriggerPriceUpdateMessage(){
    char msgname[MSGSIZE];
    sprintf(msgname, "node-%d triggerPriceUpdateMsg", myIndex());
    routerMsg *rMsg = new routerMsg(msgname);
    rMsg->setMessageType(TRIGGER_PRICE_UPDATE_MSG);
    return rMsg;
}

/* generate the actual price Update message when a triggerPriceUpdate
 * tells you to update your price to be sent to your neighbor to tell
 * them your xLocal value
 */
routerMsg * hostNodePriceScheme::generatePriceUpdateMessage(double nLocal, double serviceRate, 
        double arrivalRate, double queueSize, int receiver){
    char msgname[MSGSIZE];
    sprintf(msgname, "tic-%d-to-%d priceUpdateMsg", myIndex(), receiver);
    routerMsg *rMsg = new routerMsg(msgname);
    vector<int> route={myIndex(),receiver};
   
    rMsg->setRoute(route);
    rMsg->setHopCount(0);
    rMsg->setMessageType(PRICE_UPDATE_MSG);
    
    priceUpdateMsg *puMsg = new priceUpdateMsg(msgname);
    puMsg->setNLocal(nLocal);
    puMsg->setServiceRate(serviceRate);
    puMsg->setQueueSize(queueSize);
    puMsg->setArrivalRate(arrivalRate);

    rMsg->encapsulate(puMsg);
    return rMsg;
}


/* generate the trigger message to initiate price queries periodically 
 */
routerMsg *hostNodePriceScheme::generateTriggerPriceQueryMessage(){
    char msgname[MSGSIZE];
    sprintf(msgname, "node-%d triggerPriceQueryMsg", myIndex());
    routerMsg *rMsg = new routerMsg(msgname);
    rMsg->setMessageType(TRIGGER_PRICE_QUERY_MSG);
    return rMsg;
}

/* create a price query message to be sent on a particular path identified
 * by the list of hops and the index for that path amongst other paths to 
 * that destination
 */
routerMsg * hostNodePriceScheme::generatePriceQueryMessage(vector<int> path, int pathIdx){
    int destNode = path[path.size()-1];
    char msgname[MSGSIZE];
    sprintf(msgname, "tic-%d-to-%d priceQueryMsg [idx %d]", myIndex(), destNode, pathIdx);
    priceQueryMsg *pqMsg = new priceQueryMsg(msgname);
    pqMsg->setPathIndex(pathIdx);
    pqMsg->setIsReversed(false);
    pqMsg->setZValue(0);

    sprintf(msgname, "tic-%d-to-%d router-priceQueryMsg [idx %d]", myIndex(), destNode, pathIdx);
    routerMsg *rMsg = new routerMsg(msgname);
    rMsg->setRoute(path);
    rMsg->setHopCount(0);
    rMsg->setMessageType(PRICE_QUERY_MSG);
    rMsg->encapsulate(pqMsg);
    return rMsg;
}


/* creates a message that is meant to signal the time when a transaction can
 * be sent on the path in context based on the current view of the rate
 */
routerMsg *hostNodePriceScheme::generateTriggerTransactionSendMessage(vector<int> path, 
        int pathIndex, int destNode) {
    char msgname[MSGSIZE];
    sprintf(msgname, "tic-%d-to-%d (path %d) transactionSendMsg", myIndex(), destNode, pathIndex);
    transactionSendMsg *tsMsg = new transactionSendMsg(msgname);
    tsMsg->setPathIndex(pathIndex);
    tsMsg->setTransactionPath(path);
    tsMsg->setReceiver(destNode);

    routerMsg *rMsg = new routerMsg(msgname);
    rMsg->setMessageType(TRIGGER_TRANSACTION_SEND_MSG);
    rMsg->encapsulate(tsMsg);
    return rMsg;
}

/* check if all the provided rates are non-negative and also verify
 *  that their sum is less than the demand, return false otherwise
 */
bool hostNodePriceScheme::ratesFeasible(vector<PathRateTuple> actualRates, double demand) {
    double sumRates = 0;
    for (auto a : actualRates) {
        double rate = get<1>(a);
        if (rate < (0 - _epsilon))
            return false;
        sumRates += rate;
    }
    return (sumRates <= (demand + _epsilon));
}

/* computes the projection of the given recommended rates onto the demand d_ij per source
 */
vector<PathRateTuple> hostNodePriceScheme::computeProjection(
        vector<PathRateTuple> recommendedRates, double demand) {
    auto compareRates = [](PathRateTuple rate1, PathRateTuple rate2) {
            return (get<1>(rate1) < get<1>(rate2));
    };

    auto nuFeasible = [](double nu, double nuLeft, double nuRight) {
            return (nu >= -1 * _epsilon && nu >= (nuLeft - _epsilon) && 
                    nu <= (nuRight + _epsilon));
    };

    sort(recommendedRates.begin(), recommendedRates.end(), compareRates);
    double nu = 0.0;
    vector<PathRateTuple> actualRates;

    // initialize all rates to zero
    vector<PathRateTuple> zeroRates;
    for (int i = 0; i < recommendedRates.size(); i++) {
        zeroRates.push_back(make_tuple(i, 0.0));
    }

    // if everything is negative (aka the largest is negative), just return 0s
    if (get<1>(recommendedRates[recommendedRates.size() - 1]) < 0 + _epsilon){
        return zeroRates;
    }

    // consider nu  = 0 and see if everything checks out
    actualRates = zeroRates;
    int firstPositiveIndex = -1;
    int i = 0;
    for (auto p : recommendedRates) {
        double rate = get<1>(p);
        int pathIndex = get<0>(p);
        if (rate > 0) {
            if (firstPositiveIndex == -1) 
                firstPositiveIndex = i;
            actualRates[pathIndex] = make_tuple(pathIndex, rate);
        }
        i++;
    }
    if (ratesFeasible(actualRates, demand))
        return actualRates;

    // atleast something should be positive if you got this far
    assert(firstPositiveIndex >= 0);


    // now go through all intervals between adjacent 2* recommended rates and see if any of them
    // can give you a valid assignment of actual rates and nu
    i = firstPositiveIndex; 
    double nuLeft = 0;
    double nuRight = 0;
    while (i < recommendedRates.size()) {
        // start over checking with a new nu interval
        actualRates = zeroRates;
        nuLeft = nuRight; 
        nuRight = 2*get<1>(recommendedRates[i]);

        // find sum of all elements that are to the right of nuRight
        double sumOfRightElements = 0.0;
        for (int j = i; j < recommendedRates.size(); j++)
            sumOfRightElements += get<1>(recommendedRates[j]);     
        nu = (sumOfRightElements - demand) * 2.0/(recommendedRates.size() - i);

        // given this nu, compute the actual rates for those elements to the right of nuRight
        for (auto p : recommendedRates) {
            double rate = get<1>(p);
            int pathIndex = get<0>(p);
            if (2*rate > nuLeft) {
                actualRates[pathIndex] = make_tuple(pathIndex, rate - nu/2.0);
            }
        }

        // check if these rates are feasible and nu actually falls in the right interval 
        if (ratesFeasible(actualRates, demand) && nuFeasible(nu, nuLeft, nuRight)) 
            return actualRates;

        // otherwise move on
        i++;
    }

    // should never be reached
    assert(false);
    return zeroRates;
}


/* overall controller for handling messages that dispatches the right function
 * based on message type in price Scheme
 */
void hostNodePriceScheme::handleMessage(cMessage *msg) {
    routerMsg *ttmsg = check_and_cast<routerMsg *>(msg);
 
    if (simTime() > _simulationLength){
        auto encapMsg = (ttmsg->getEncapsulatedPacket());
        ttmsg->decapsulate();
        delete ttmsg;
        delete encapMsg;
        return;
    } 

    switch(ttmsg->getMessageType()) {
        case TRIGGER_PRICE_UPDATE_MSG:
             if (_loggingEnabled) cout<< "[HOST "<< myIndex() 
                 <<": RECEIVED TRIGGER_PRICE_UPDATE MSG] "<< ttmsg->getName() << endl;
             handleTriggerPriceUpdateMessage(ttmsg);
             if (_loggingEnabled) cout<< "[AFTER HANDLING:]  "<< endl;
             break;

        case PRICE_UPDATE_MSG:
             if (_loggingEnabled) cout<< "[HOST "<< myIndex() 
                 <<": RECEIVED PRICE_UPDATE MSG] "<< ttmsg->getName() << endl;
             handlePriceUpdateMessage(ttmsg);
             if (_loggingEnabled) cout<< "[AFTER HANDLING:]  "<< endl;
             break;

        case TRIGGER_PRICE_QUERY_MSG:
             if (_loggingEnabled) cout<< "[HOST "<< myIndex() 
                 <<": RECEIVED TRIGGER_PRICE_QUERY MSG] "<< ttmsg->getName() << endl;
             handleTriggerPriceQueryMessage(ttmsg);
             if (_loggingEnabled) cout<< "[AFTER HANDLING:]  "<< endl;
             break;
        
        case PRICE_QUERY_MSG:
             if (_loggingEnabled) cout<< "[HOST "<< myIndex() 
                 <<": RECEIVED PRICE_QUERY MSG] "<< ttmsg->getName() << endl;
             handlePriceQueryMessage(ttmsg);
             if (_loggingEnabled) cout<< "[AFTER HANDLING:]  "<< endl;
             break;

        case TRIGGER_TRANSACTION_SEND_MSG:
             if (_loggingEnabled) cout<< "[HOST "<< myIndex() 
                 <<": RECEIVED TRIGGER_TXN_SEND MSG] "<< ttmsg->getName() << endl;
             handleTriggerTransactionSendMessage(ttmsg);
             if (_loggingEnabled) cout<< "[AFTER HANDLING:]  "<< endl;
             break;

        default:
             hostNodeBase::handleMessage(msg);

    }
}

/* handler for timeout messages that is responsible for removing messages from 
 * sender queues if they haven't been sent yet and sends explicit time out messages
 * for messages that have been sent on a path already
 */
void hostNodePriceScheme::handleTimeOutMessage(routerMsg* ttmsg) {
    timeOutMsg *toutMsg = check_and_cast<timeOutMsg *>(ttmsg->getEncapsulatedPacket());
    int destination = toutMsg->getReceiver();
    int transactionId = toutMsg->getTransactionId();
    multiset<routerMsg*, transCompare> *transList = &(nodeToDestInfo[destination].transWaitingToBeSent);
    
    if (ttmsg->isSelfMessage()) {
        // if transaction was successful don't do anything more
        if (successfulDoNotSendTimeOut.count(transactionId) > 0) {
            successfulDoNotSendTimeOut.erase(toutMsg->getTransactionId());
            ttmsg->decapsulate();
            delete toutMsg;
            delete ttmsg;
            return;
        }

        // check if txn is still in just sender queue, just delete and return then
        auto iter = find_if(transList->begin(),
           transList->end(),
           [&transactionId](const routerMsg* p)
           { transactionMsg *transMsg = check_and_cast<transactionMsg *>(p->getEncapsulatedPacket());
             return transMsg->getTransactionId()  == transactionId; });

        if (iter!=transList->end()) {
            deleteTransaction(*iter);
            transList->erase(iter);
            ttmsg->decapsulate();
            delete toutMsg;
            delete ttmsg;
            return;
        }

        // send out a time out message on the path that hasn't acked all of it
        for (auto p : (nodeToShortestPathsMap[destination])){
            int pathIndex = p.first;
            tuple<int,int> key = make_tuple(transactionId, pathIndex);
                        
            if (transPathToAckState.count(key) > 0 && 
                    transPathToAckState[key].amtSent != transPathToAckState[key].amtReceived) {
                routerMsg* psMsg = generateTimeOutMessageForPath(
                    nodeToShortestPathsMap[destination][p.first].path, 
                    transactionId, destination);
                int nextNode = (psMsg->getRoute())[psMsg->getHopCount()+1];
                CanceledTrans ct = make_tuple(toutMsg->getTransactionId(), 
                        simTime(), -1, nextNode, destination);
                canceledTransactions.insert(ct);
                forwardMessage(psMsg);
            }
            else {
                transPathToAckState.erase(key);
            }
        }
        ttmsg->decapsulate();
        delete toutMsg;
        delete ttmsg;
    }
    else {
        // at the receiver
        CanceledTrans ct = make_tuple(toutMsg->getTransactionId(),simTime(),
                (ttmsg->getRoute())[ttmsg->getHopCount()-1], -1, toutMsg->getReceiver());
        canceledTransactions.insert(ct);
        ttmsg->decapsulate();
        delete toutMsg;
        delete ttmsg;
    }
}


/* main routine for handling a new transaction under the pricing scheme
 * In particular, initiate price probes, assigns a txn to a path if the 
 * rate for that path allows it, otherwise queues it at the sender
 * until a timer fires on the next path allowing a txn to go through
 */
void hostNodePriceScheme::handleTransactionMessageSpecialized(routerMsg* ttmsg){
    transactionMsg *transMsg = check_and_cast<transactionMsg *>(ttmsg->getEncapsulatedPacket());
    int hopcount = ttmsg->getHopCount();
    int destNode = transMsg->getReceiver();
    int nextNode = ttmsg->getRoute()[hopcount + 1];
    int transactionId = transMsg->getTransactionId();

    SplitState* splitInfo = &(_numSplits[myIndex()][transMsg->getLargerTxnId()]);
    splitInfo->numArrived += 1;
    
    // first time seeing this transaction so add to d_ij computation
    // count the txn for accounting also
    if (simTime() == transMsg->getTimeSent()) {
        destNodeToNumTransPending[destNode]  += 1;
        nodeToDestInfo[destNode].transSinceLastInterval += transMsg->getAmount();
        if (splitInfo->numArrived == 1)
            splitInfo->firstAttemptTime = simTime().dbl();

        if (transMsg->getTimeSent() >= _transStatStart && 
            transMsg->getTimeSent() <= _transStatEnd) {
            statAmtArrived[destNode] += transMsg->getAmount();
            
            if (splitInfo->numArrived == 1) {       
                statNumArrived[destNode] += 1;
                statRateArrived[destNode] += 1; 
            }
        }
    }

    // initiate price probes if it is a new destination
    if (nodeToShortestPathsMap.count(destNode) == 0 && ttmsg->isSelfMessage()){
        vector<vector<int>> kShortestRoutes = getKPaths(transMsg->getSender(),destNode, _kValue);
        initializePriceProbes(kShortestRoutes, destNode);
        scheduleAt(simTime() + _maxTravelTime, ttmsg);
        return;
    }
    
    // at destination, add to incoming transUnits and trigger ack
    if (transMsg->getReceiver() == myIndex()) {
       handleTransactionMessage(ttmsg, false); 
    }
    else if (ttmsg->isSelfMessage()) {
        // at sender, either queue up or send on a path that allows you to send
        DestInfo* destInfo = &(nodeToDestInfo[destNode]);
       
        //send on a path if no txns queued up and timer was in the path
        if ((destInfo->transWaitingToBeSent).size() > 0) {
            pushIntoSenderQueue(destInfo, ttmsg);
        } else {
            for (auto p: nodeToShortestPathsMap[destNode]) {
                int pathIndex = p.first;
                PathInfo *pathInfo = &(nodeToShortestPathsMap[destNode][pathIndex]);
                pathInfo->window = max(pathInfo->rateToSendTrans * pathInfo->rttMin, _minWindow);
                
                if (pathInfo->rateToSendTrans > 0 && simTime() > pathInfo->timeToNextSend && 
                        (!_windowEnabled || 
                         (_windowEnabled && pathInfo->sumOfTransUnitsInFlight < pathInfo->window))) {
                    
                    ttmsg->setRoute(pathInfo->path);
                    ttmsg->setHopCount(0);
                    transMsg->setPathIndex(pathIndex);
                    handleTransactionMessage(ttmsg, true /*revisit*/);

                    // record stats on sent units for payment channel, destination and path
                    int nextNode = pathInfo->path[1];
                    nodeToPaymentChannel[nextNode].nValue += transMsg->getAmount();

                    // first attempt of larger txn
                    SplitState* splitInfo = &(_numSplits[myIndex()][transMsg->getLargerTxnId()]);
                    if (splitInfo->numAttempted == 0) {
                        splitInfo->numAttempted += 1;
                        if (transMsg->getTimeSent() >= _transStatStart && 
                            transMsg->getTimeSent() <= _transStatEnd) 
                            statRateAttempted[destNode] += 1;
                    }
                    
                    if (transMsg->getTimeSent() >= _transStatStart && 
                            transMsg->getTimeSent() <= _transStatEnd) {
                        statAmtAttempted[destNode] += transMsg->getAmount();
                    }
                    
                    pathInfo->nValue += transMsg->getAmount();
                    pathInfo->statRateAttempted += 1;
                    pathInfo->sumOfTransUnitsInFlight += transMsg->getAmount();
                    pathInfo->lastTransSize = transMsg->getAmount();
                    pathInfo->lastSendTime = simTime();
                    pathInfo->amtAllowedToSend = max(pathInfo->amtAllowedToSend - transMsg->getAmount(), 0.0);

                    // necessary for knowing what path to remove transaction in flight funds from
                    tuple<int,int> key = make_tuple(transMsg->getTransactionId(), pathIndex); 
                    transPathToAckState[key].amtSent += transMsg->getAmount();
                
                    // update the "time when the next transaction can be sent"
                    double rateToUse = max(pathInfo->rateToSendTrans, _smallRate);
                    double additional = min(max((transMsg->getAmount())/ rateToUse, _epsilon), 10000.0);
                    simtime_t newTimeToNextSend =  simTime() + additional;
                    pathInfo->timeToNextSend = newTimeToNextSend;
                    return;
                }
            }
            
            //transaction cannot be sent on any of the paths, queue transaction
            pushIntoSenderQueue(destInfo, ttmsg);

            for (auto p: nodeToShortestPathsMap[destNode]) {
                PathInfo *pInfo = &(nodeToShortestPathsMap[destNode][p.first]);
                if (pInfo->isSendTimerSet == false) {
                    simtime_t timeToNextSend = pInfo->timeToNextSend;
                    if (simTime() > timeToNextSend) {
                        timeToNextSend = simTime() + 0.0001;
                    }
                    cancelEvent(pInfo->triggerTransSendMsg);            
                    scheduleAt(timeToNextSend, pInfo->triggerTransSendMsg);
                    pInfo->isSendTimerSet = true;
                }
            }
        }
    }
}


/* handler for the statistic message triggered every x seconds to also
 * output the price based scheme stats in addition to the default
 */
void hostNodePriceScheme::handleStatMessage(routerMsg* ttmsg){
    if (_signalsEnabled) {
        // per payment channel stats
        for ( auto it = nodeToPaymentChannel.begin(); it!= nodeToPaymentChannel.end(); it++){ 
            int node = it->first; 
            PaymentChannel* p = &(nodeToPaymentChannel[node]);

            emit(p->nValueSignal, p->nValue);
            emit(p->lambdaSignal, p->lambda);
            emit(p->muLocalSignal, p->muLocal);
            emit(p->muRemoteSignal, p->muRemote);
        }
        
        // per destination statistics
        for (auto it = 0; it < _numHostNodes; it++){ 
            if (it != getIndex() && _destList[myIndex()].count(it) > 0) {
                if (nodeToShortestPathsMap.count(it) > 0) {
                    for (auto& p: nodeToShortestPathsMap[it]){
                        int pathIndex = p.first;
                        PathInfo *pInfo = &(p.second);
                        emit(pInfo->rateToSendTransSignal, pInfo->rateToSendTrans);
                        emit(pInfo->rateActuallySentSignal, pInfo->rateSentOnPath);
                        emit(pInfo->sumOfTransUnitsInFlightSignal, 
                                pInfo->sumOfTransUnitsInFlight);
                        emit(pInfo->windowSignal, pInfo->window);
                        emit(pInfo->priceLastSeenSignal, pInfo->priceLastSeen);
                        emit(pInfo->rateOfAcksSignal, pInfo->amtAcked/_statRate);
                        pInfo->amtAcked = 0;
                    }
                }

                emit(demandEstimatePerDestSignals[it], nodeToDestInfo[it].demand);
                emit(numWaitingPerDestSignals[it], 
                       nodeToDestInfo[it].transWaitingToBeSent.size()); 
            }        
        }
    } 

    // call the base method to output rest of the stats
    hostNodeBase::handleStatMessage(ttmsg);
}


/* specialized ack handler that does the routine if this is a price scheme 
 * algorithm. In particular, collects/updates stats for this path alone
 * NOTE: acks are on the reverse path relative to the original sender
 */
void hostNodePriceScheme::handleAckMessageSpecialized(routerMsg* ttmsg){
    ackMsg *aMsg = check_and_cast<ackMsg*>(ttmsg->getEncapsulatedPacket());
    int pathIndex = aMsg->getPathIndex();
    int destNode = ttmsg->getRoute()[0];
    int transactionId = aMsg->getTransactionId();
    double largerTxnId = aMsg->getLargerTxnId();

    if (aMsg->getIsSuccess() == false) {
        // make sure transaction isn't cancelled yet
        auto iter = canceledTransactions.find(make_tuple(transactionId, 0, 0, 0, 0));
    
        if (iter != canceledTransactions.end()) {
            if (aMsg->getTimeSent() >= _transStatStart && aMsg->getTimeSent() <= _transStatEnd)
                statAmtFailed[destNode] += aMsg->getAmount();
        } 
        else {
            // requeue transaction
            routerMsg *duplicateTrans = generateDuplicateTransactionMessage(aMsg);
            pushIntoSenderQueue(&(nodeToDestInfo[destNode]), duplicateTrans);
        }
 
    }
    else {
        SplitState* splitInfo = &(_numSplits[myIndex()][largerTxnId]);
        splitInfo->numReceived += 1;

        if (aMsg->getTimeSent() >= _transStatStart && 
                aMsg->getTimeSent() <= _transStatEnd) {
            statAmtCompleted[destNode] += aMsg->getAmount();

            if (splitInfo->numTotal == splitInfo->numReceived) {
                statNumCompleted[destNode] += 1; 
                statRateCompleted[destNode] += 1;
                _transactionCompletionBySize[splitInfo->totalAmount] += 1;
                double timeTaken = simTime().dbl() - splitInfo->firstAttemptTime;
                statCompletionTimes[destNode] += timeTaken * 1000;
                _txnAvgCompTimeBySize[splitInfo->totalAmount] += timeTaken * 1000;
                recordTailCompletionTime(aMsg->getTimeSent(), splitInfo->totalAmount, timeTaken * 1000);
            }
        }
        nodeToShortestPathsMap[destNode][pathIndex].statRateCompleted += 1;
        nodeToShortestPathsMap[destNode][pathIndex].amtAcked += aMsg->getAmount();
    }

    //increment transaction amount ack on a path. 
    tuple<int,int> key = make_tuple(transactionId, pathIndex);
    transPathToAckState[key].amtReceived += aMsg->getAmount();
    
    nodeToShortestPathsMap[destNode][pathIndex].sumOfTransUnitsInFlight -= aMsg->getAmount();
    destNodeToNumTransPending[destNode] -= 1;     
    hostNodeBase::handleAckMessage(ttmsg);
}

/* handler for the clear state message that deals with
 * transactions that will no longer be completed
 * In particular clears out the amount inn flight on the path
 */
void hostNodePriceScheme::handleClearStateMessage(routerMsg *ttmsg) {
    for ( auto it = canceledTransactions.begin(); it!= canceledTransactions.end(); it++){
        int transactionId = get<0>(*it);
        simtime_t msgArrivalTime = get<1>(*it);
        int prevNode = get<2>(*it);
        int nextNode = get<3>(*it);
        int destNode = get<4>(*it);
        
        if (simTime() > (msgArrivalTime + _maxTravelTime + 1)){
            // ack was not received,safely can consider this txn done
            for (auto p : nodeToShortestPathsMap[destNode]) {
                int pathIndex = p.first;
                tuple<int,int> key = make_tuple(transactionId, pathIndex);
                if (transPathToAckState.count(key) != 0) {
                    nodeToShortestPathsMap[destNode][pathIndex].sumOfTransUnitsInFlight -= 
                        (transPathToAckState[key].amtSent - transPathToAckState[key].amtReceived);
                    transPathToAckState.erase(key);
                }
            }
        }
    }

    // works fine now because timeouts start per transaction only when
    // sent out and no txn splitting
    hostNodeBase::handleClearStateMessage(ttmsg);
}


/* handler for the trigger message that regularly fires to indicate
 * that it is time to recompute prices for all payment channels 
 * and let your neighbors know about the latest rates of incoming 
 * transactions for every one of them and wait for them to send
 * you the same before recomputing prices
 */
void hostNodePriceScheme::handleTriggerPriceUpdateMessage(routerMsg* ttmsg) {
    // reschedule this to trigger again at intervals of _tUpdate
    if (simTime() > _simulationLength) {
        delete ttmsg;
    }
    else {
        scheduleAt(simTime()+_tUpdate, ttmsg);
    }

    // go through all the payment channels and recompute the arrival rate of 
    // transactions for all of them
    for ( auto it = nodeToPaymentChannel.begin(); it!= nodeToPaymentChannel.end(); it++ ) {
        PaymentChannel *neighborChannel = &(nodeToPaymentChannel[it->first]);      
        neighborChannel->xLocal = neighborChannel->nValue / _tUpdate;
        neighborChannel->updateRate = neighborChannel->numUpdateMessages/_tUpdate;
        if (it->first < 0){
            printNodeToPaymentChannel();
            endSimulation();
        }

        auto firstTransTimes = neighborChannel->serviceArrivalTimeStamps.front();
        auto lastTransTimes =  neighborChannel->serviceArrivalTimeStamps.back();
        double serviceTimeDiff = get<1>(lastTransTimes).dbl() - get<1>(firstTransTimes).dbl(); 
        double arrivalTimeDiff = get<1>(neighborChannel->arrivalTimeStamps.back()).dbl() - 
            get<1>(neighborChannel->arrivalTimeStamps.front()).dbl();
        
        neighborChannel->serviceRate = neighborChannel->sumServiceWindowTxns / serviceTimeDiff;
        neighborChannel->arrivalRate = neighborChannel->sumArrivalWindowTxns / arrivalTimeDiff;
        neighborChannel->lastQueueSize = getTotalAmount(it->first);

        routerMsg * priceUpdateMsg = generatePriceUpdateMessage(neighborChannel->nValue, 
                neighborChannel->serviceRate, neighborChannel->arrivalRate, 
            neighborChannel->lastQueueSize, it->first);
        neighborChannel->lastNValue = neighborChannel->nValue;
        neighborChannel->nValue = 0;
        neighborChannel->numUpdateMessages = 0;
        
        forwardMessage(priceUpdateMsg);
    }
    
    // also update the rate actually Sent for a given destination and path 
    // to be used for your next rate computation
    for ( auto it = nodeToShortestPathsMap.begin(); it != nodeToShortestPathsMap.end(); it++ ) {
        int destNode = it->first;
        for (auto p : nodeToShortestPathsMap[destNode]) {
            PathInfo *pInfo = &(nodeToShortestPathsMap[destNode][p.first]); 
            double latestRate = pInfo->nValue / _tUpdate;
            double oldRate = pInfo->rateSentOnPath; 
            
            pInfo->nValue = 0; 
            pInfo->rateSentOnPath = latestRate; 
        }

        // use this information to also update your demand for this destination
        DestInfo* destInfo = &(nodeToDestInfo[destNode]);
        double newDemand = destInfo->transSinceLastInterval/_tUpdate;
        destInfo->demand = (1 - _zeta) * destInfo->demand + _zeta * newDemand;
        destInfo->transSinceLastInterval = 0;
    }
}


/* handler that handles the receipt of a priceUpdateMessage from a neighboring 
 * node that causes this node to update its prices for this particular
 * payment channel
 * Nesterov and secondOrderOptimization are two means to speed up the convergence
 */
void hostNodePriceScheme::handlePriceUpdateMessage(routerMsg* ttmsg){
    priceUpdateMsg *puMsg = check_and_cast<priceUpdateMsg *>(ttmsg->getEncapsulatedPacket());
    double nRemote = puMsg->getNLocal();
    double serviceRateRemote = puMsg->getServiceRate();
    double arrivalRateRemote = puMsg->getArrivalRate();
    double qRemote = puMsg->getQueueSize();
    int sender = ttmsg->getRoute()[0];
    tuple<int, int> senderReceiverTuple = (sender < myIndex()) ? make_tuple(sender, myIndex()) :
                    make_tuple(myIndex(), sender);
    
    PaymentChannel *neighborChannel = &(nodeToPaymentChannel[sender]);
    double inflightRemote = min(getTotalAmountIncomingInflight(sender) + 
        serviceRateRemote * _avgDelay/1000, _capacities[senderReceiverTuple]); 
    double updateRateLocal = neighborChannel->updateRate;
    double nLocal = neighborChannel->lastNValue;
    double inflightLocal = min(getTotalAmountOutgoingInflight(sender) + 
        updateRateLocal* _avgDelay/1000.0, _capacities[senderReceiverTuple]);
    double qLocal = neighborChannel->lastQueueSize; 
    double serviceRateLocal = neighborChannel->serviceRate;
    double arrivalRateLocal = neighborChannel->arrivalRate;

    tuple<int, int> node1node2Pair = (myIndex() < sender) ? make_tuple(myIndex(), sender) : make_tuple(sender,
            myIndex());
    double cValue = _capacities[node1node2Pair]; 
    double oldLambda = neighborChannel->lambda;
    double oldMuLocal = neighborChannel->muLocal;
    double oldMuRemote = neighborChannel->muRemote;
 
    double newLambdaGrad;
    double newMuLocalGrad;
    if (_useQueueEquation) {
        newLambdaGrad	= inflightLocal*arrivalRateLocal/serviceRateLocal + 
            inflightRemote * arrivalRateRemote/serviceRateRemote + 
            2*_xi*min(qLocal, qRemote) - (_capacityFactor * cValue);
        
        newMuLocalGrad	= nLocal - nRemote + 
            qLocal*_tUpdate/_routerQueueDrainTime - 
            qRemote*_tUpdate/_routerQueueDrainTime;
    } else {
        newLambdaGrad = inflightLocal*arrivalRateLocal/serviceRateLocal + 
            inflightRemote * arrivalRateRemote/serviceRateRemote - (_capacityFactor * cValue);
        newMuLocalGrad	= nLocal - nRemote;
    }

    if (_rebalancingEnabled) {
        double amtAdded = (oldMuLocal - _gamma);
        double newSize = neighborChannel->fakeRebalancingQueue + amtAdded;
        neighborChannel->fakeRebalancingQueue = max(0.0, newSize);
        if (newSize > _maxGammaImbalanceQueueSize) {
            neighborChannel->balance += amtAdded;
            tuple<int, int> senderReceiverTuple = 
                    (sender < myIndex()) ? make_tuple(sender, myIndex()) :
                    make_tuple(myIndex(), sender);
            _capacities[senderReceiverTuple] += amtAdded;
            neighborChannel->amtAdded += amtAdded;
            neighborChannel->numRebalanceEvents += 1;
            neighborChannel->fakeRebalancingQueue = 0;
        }
    }

    // Nesterov's gradient descent equation
    // and other speeding up mechanisms
    double newLambda = 0.0;
    double newMuLocal = 0.0;
    double newMuRemote = 0.0;
    double myKappa = _kappa;
    double myEta = _eta;
    if (_nesterov) {
        double yLambda = neighborChannel->yLambda;
        double yMuLocal = neighborChannel->yMuLocal;
        double yMuRemote = neighborChannel->yMuRemote;

        double yLambdaNew = oldLambda + myEta*newLambdaGrad;
        newLambda = yLambdaNew + _rhoLambda*(yLambdaNew - yLambda); 
        neighborChannel->yLambda = yLambdaNew;

        double yMuLocalNew = oldMuLocal + myKappa*newMuLocalGrad;
        newMuLocal = yMuLocalNew + _rhoMu*(yMuLocalNew - yMuLocal);
        neighborChannel->yMuLocal = yMuLocalNew;

        double yMuRemoteNew = oldMuRemote - myKappa*newMuLocalGrad;
        newMuRemote = yMuRemoteNew + _rhoMu*(yMuRemoteNew - yMuRemote);
        neighborChannel->yMuRemote = yMuRemoteNew;
    } 
    else {
        newLambda = oldLambda +  myEta*newLambdaGrad;
        newMuLocal = oldMuLocal + myKappa*newMuLocalGrad;
        newMuRemote = oldMuRemote - myKappa*newMuLocalGrad; 
    }
    
    neighborChannel->lambda = maxDouble(newLambda, 0);
    neighborChannel->muLocal = maxDouble(newMuLocal, 0);
    neighborChannel->muRemote = maxDouble(newMuRemote, 0);
    
    //delete both messages
    ttmsg->decapsulate();
    delete puMsg;
    delete ttmsg;
}


/* handler for the trigger message that periodically fires indicating
 * that it is time to send a new price query on this path
 */
void hostNodePriceScheme::handleTriggerPriceQueryMessage(routerMsg* ttmsg){
    if (simTime() > _simulationLength){
        delete ttmsg;
    } else {
        scheduleAt(simTime()+_tQuery, ttmsg);
    }
    
    // go through all destinations that have pending transactions and send 
    // out a new probe to them
    for (auto it = destNodeToNumTransPending.begin(); it!=destNodeToNumTransPending.end(); it++){
        if (it->first == myIndex()){
            continue;
        }
        
        if (it->second>0){ 
            //if we have transactions pending
            for (auto p = nodeToShortestPathsMap[it->first].begin() ;
                    p!= nodeToShortestPathsMap[it->first].end(); p++){
                int routeIndex = p->first;
                PathInfo pInfo = nodeToShortestPathsMap[it->first][p->first];
                routerMsg * msg = generatePriceQueryMessage(pInfo.path, routeIndex);
                handlePriceQueryMessage(msg);
            }
        }
    }
}

/* handler for a price query message which when received back at the sender
 * is responsible for adjusting the sending rate on the associated paths
 * at all other points, it just forwards the messgae
 */
void hostNodePriceScheme::handlePriceQueryMessage(routerMsg* ttmsg){
    priceQueryMsg *pqMsg = check_and_cast<priceQueryMsg *>(ttmsg->getEncapsulatedPacket());
    
    bool isReversed = pqMsg->getIsReversed();
    if (!isReversed && ttmsg->getHopCount() == 0){ 
        // at the sender, compute my own price and send
        int nextNode = ttmsg->getRoute()[ttmsg->getHopCount()+1];
        double zOld = pqMsg->getZValue();
        double lambda = nodeToPaymentChannel[nextNode].lambda;
        double muLocal = nodeToPaymentChannel[nextNode].muLocal;
        double muRemote = nodeToPaymentChannel[nextNode].muRemote;
        double zNew =  zOld;// + (2 * lambda) + muLocal  - muRemote;
        pqMsg->setZValue(zNew);
        forwardMessage(ttmsg);
    }
    else if (!isReversed){ 
        //is at destination
        pqMsg->setIsReversed(true);
        vector<int> route = ttmsg->getRoute();
        reverse(route.begin(), route.end());
        ttmsg->setRoute(route);
        ttmsg->setHopCount(0);
        forwardMessage(ttmsg);
    }
    else { 
        //is back at sender with the price
        double demand;
        double zValue = pqMsg->getZValue();
        int destNode = ttmsg->getRoute()[0];
        int routeIndex = pqMsg->getPathIndex();

        PathInfo *p = &(nodeToShortestPathsMap[destNode][routeIndex]);
        double oldRate = p->rateToSendTrans;

        // Nesterov's gradient descent equation
        // and other speeding up mechanisms
        double sumOfRates = 0.0;
        double preProjectionRate = 0.0;
        if (_nesterov) {
          double yPrev = p->yRateToSendTrans;
          double yNew = oldRate + _alpha*(1 - zValue);
          p->yRateToSendTrans = yNew;
          preProjectionRate = yNew + _rho*(yNew - yPrev);
        } else if (_secondOrderOptimization) {
            for (auto curPath : nodeToShortestPathsMap[destNode]) {
                sumOfRates += curPath.second.rateToSendTrans;
            }
            double delta = _alpha*(1 - zValue) + _rho*(p->priceLastSeen - zValue);
            preProjectionRate = oldRate + delta ;
        }
        else {
          preProjectionRate  = oldRate + _alpha*(1 - zValue);
        }

        p->priceLastSeen = zValue;
        p->oldSumOfRates = sumOfRates;

        // compute the projection of this new rate along with old rates
        double sumRates = 0.0;
        vector<PathRateTuple> pathRateTuples;
        for (auto p : nodeToShortestPathsMap[destNode]) {
            int pathIndex = p.first;
            double rate;
            if (pathIndex != routeIndex) {
                rate =  p.second.rateToSendTrans;
            } else {
                rate = preProjectionRate;
            }
            sumRates += p.second.rateToSendTrans;
              
            PathRateTuple newTuple = make_tuple(pathIndex, rate);
            pathRateTuples.push_back(newTuple);
        }

        int queueSize = nodeToDestInfo[destNode].transWaitingToBeSent.size();
        double drainTime = 100.0; // seconds
        double bias = nodeToDestInfo[destNode].demand - sumRates; 
        vector<PathRateTuple> projectedRates = 
            computeProjection(pathRateTuples, 1.1*nodeToDestInfo[destNode].demand);

        // reassign all path's rates to the projected rates and 
        // make sure it is atleast minPriceRate for every path
        for (auto p : projectedRates) {
            int pathIndex = get<0>(p);
            double newRate = get<1>(p);

            if (_reschedulingEnabled) {
                updateTimers(destNode, pathIndex, newRate);
            }
            else
                nodeToShortestPathsMap[destNode][pathIndex].rateToSendTrans = 
                maxDouble(newRate, _minPriceRate);
        }
              
        //delete both messages
        ttmsg->decapsulate();
        delete pqMsg;
        delete ttmsg;
   }
}

/* handles the sending of transactions at a certain rate indirectly via timers 
 * going off to trigger the next send on a particular path. This responds to
 * that trigger and pulls the next transaction off the queue and sends that
 */
void hostNodePriceScheme::handleTriggerTransactionSendMessage(routerMsg* ttmsg){
    transactionSendMsg *tsMsg = 
        check_and_cast<transactionSendMsg *>(ttmsg->getEncapsulatedPacket());

    vector<int> path = tsMsg->getTransactionPath();
    int pathIndex = tsMsg->getPathIndex();
    int destNode = tsMsg->getReceiver();
    PathInfo* p = &(nodeToShortestPathsMap[destNode][pathIndex]);
    p->window = max(p->rateToSendTrans * p->rttMin, _minWindow);

    if (nodeToDestInfo[destNode].transWaitingToBeSent.size() > 0 && (!_windowEnabled || 
            (_windowEnabled && p->sumOfTransUnitsInFlight < p->window))){
        //remove the transaction $tu$ at the head of the queue
        auto firstPosition = nodeToDestInfo[destNode].transWaitingToBeSent.begin();
        routerMsg *msgToSend = *firstPosition;
        nodeToDestInfo[destNode].transWaitingToBeSent.erase(firstPosition);
        transactionMsg *transMsg = 
           check_and_cast<transactionMsg *>(msgToSend->getEncapsulatedPacket());
        
        // Send the transaction treansaction unit along the corresponding path.
        transMsg->setPathIndex(pathIndex);
        msgToSend->setRoute(path);
        msgToSend->setHopCount(0);

        // increment the number of units sent on a particular payment channel
        int nextNode = path[1];
        nodeToPaymentChannel[nextNode].nValue = nodeToPaymentChannel[nextNode].nValue + transMsg->getAmount();

        // increment amount in inflght and other info on last transaction on this path
        p->nValue += transMsg->getAmount();
        p->sumOfTransUnitsInFlight = p->sumOfTransUnitsInFlight + transMsg->getAmount();
        p->lastTransSize = transMsg->getAmount();
        p->lastSendTime = simTime();
        p->amtAllowedToSend = max(p->amtAllowedToSend - transMsg->getAmount(), 0.0);
        
        // necessary for knowing what path to remove transaction in flight funds from
        tuple<int,int> key = make_tuple(transMsg->getTransactionId(), pathIndex); 
        transPathToAckState[key].amtSent += transMsg->getAmount();

        // cannot be cancelled at this point
        handleTransactionMessage(msgToSend, 1/*revisiting*/);
        p->statRateAttempted = p->statRateAttempted + 1;

        // first attempt of larger txn
        SplitState* splitInfo = &(_numSplits[myIndex()][transMsg->getLargerTxnId()]);
        if (splitInfo->numAttempted == 0) {
            splitInfo->numAttempted += 1;
            if (transMsg->getTimeSent() >= _transStatStart && 
                transMsg->getTimeSent() <= _transStatEnd)
                statRateAttempted[destNode] += 1;
        }

        if (transMsg->getTimeSent() >= _transStatStart && 
            transMsg->getTimeSent() <= _transStatEnd){
            statAmtAttempted[destNode] += transMsg->getAmount();
        }

        //Update the  “time when next transaction can be sent” 
        double bound = _reschedulingEnabled ? _smallRate  : 1.0;
        double rateToSendTrans = max(p->rateToSendTrans, bound);
        double additional =  min(transMsg->getAmount()/rateToSendTrans, 10000.0);
        p->timeToNextSend = simTime() + additional; 
        cancelEvent(ttmsg);
        scheduleAt(p->timeToNextSend, ttmsg);
    }
    else { 
        //no trans to send
        // don't turn off timers
        PathInfo* p = &(nodeToShortestPathsMap[destNode][pathIndex]);
        double rateToSendTrans = max(p->rateToSendTrans, _smallRate);
        double lastTxnSize = p->lastTransSize;
        double additional = min(max(lastTxnSize/rateToSendTrans, _epsilon), 10000.0);
        p->timeToNextSend = simTime() + additional;
        cancelEvent(ttmsg);
        scheduleAt(p->timeToNextSend, ttmsg);
        p->amtAllowedToSend = 0.0;
    }
}

/* initialize data for for the paths supplied to the destination node
 * and also fix the paths for susbequent transactions to this destination
 * and register signals that are path specific
 */
void hostNodePriceScheme::initializePriceProbes(vector<vector<int>> kShortestPaths, int destNode){
    for (int pathIdx = 0; pathIdx < kShortestPaths.size(); pathIdx++){
        // initialize pathInfo
        PathInfo temp = {};
        temp.path = kShortestPaths[pathIdx];
        routerMsg * triggerTransSendMsg = 
          generateTriggerTransactionSendMessage(kShortestPaths[pathIdx], pathIdx, destNode);
        temp.triggerTransSendMsg = triggerTransSendMsg;
        temp.rateToSendTrans = _minPriceRate;
        temp.yRateToSendTrans = _minPriceRate;
        temp.rttMin = (kShortestPaths[pathIdx].size() - 1) * 2 * _avgDelay/1000.0;
        nodeToShortestPathsMap[destNode][pathIdx] = temp;

        //initialize signals
        simsignal_t signal;
        signal = registerSignalPerDestPath("rateToSendTrans", pathIdx, destNode);
        nodeToShortestPathsMap[destNode][pathIdx].rateToSendTransSignal = signal;

        signal = registerSignalPerDestPath("rateSent", pathIdx, destNode);
        nodeToShortestPathsMap[destNode][pathIdx].rateActuallySentSignal = signal;
        
        signal = registerSignalPerDestPath("timeToNextSend", pathIdx, destNode);
        nodeToShortestPathsMap[destNode][pathIdx].timeToNextSendSignal = signal;

        signal = registerSignalPerDestPath("sumOfTransUnitsInFlight", pathIdx, destNode);
        nodeToShortestPathsMap[destNode][pathIdx].sumOfTransUnitsInFlightSignal = signal;

        signal = registerSignalPerDestPath("window", pathIdx, destNode);
        nodeToShortestPathsMap[destNode][pathIdx].windowSignal = signal;
      
        signal = registerSignalPerDestPath("priceLastSeen", pathIdx, destNode);
        nodeToShortestPathsMap[destNode][pathIdx].priceLastSeenSignal = signal;

        signal = registerSignalPerDestPath("rateOfAcks", pathIdx, destNode);
        nodeToShortestPathsMap[destNode][pathIdx].rateOfAcksSignal = signal;
   }
}



/* helper method to reschedule the timer on a given path according to the new rate
 */
void hostNodePriceScheme::updateTimers(int destNode, int pathIndex, double newRate) {
    PathInfo *p = &(nodeToShortestPathsMap[destNode][pathIndex]);
    simtime_t lastSendTime = p->lastSendTime;
    double lastTxnSize = p->lastTransSize;
    simtime_t oldTime = p->timeToNextSend;

    // compute allowed to send
    double oldRate = p->rateToSendTrans; 
    simtime_t lastUpdateTime = p->lastRateUpdateTime; 
    simtime_t timeForThisRate = min(simTime() - lastUpdateTime, simTime() - lastSendTime);
    p->amtAllowedToSend += oldRate * timeForThisRate.dbl();
    p->amtAllowedToSend = min(lastTxnSize, p->amtAllowedToSend);

    // update the rate
    p->rateToSendTrans = newRate;
    double allowedToSend = p->amtAllowedToSend;
    p->lastRateUpdateTime = simTime();

    // Reschedule timer on this path according to this rate
    double rateToUse = max(newRate, _smallRate);
    double additional = min(max((lastTxnSize - allowedToSend)/ rateToUse, _epsilon), 10000.0);
    simtime_t newTimeToSend = simTime() + additional;

    cancelEvent(p->triggerTransSendMsg);
    p->timeToNextSend = newTimeToSend;
    scheduleAt(newTimeToSend, p->triggerTransSendMsg);
}

/* additional initalization that has to be done for the price based scheme
 * in particular set price variables to zero, initialize more signals
 * and schedule the first price update and price trigger
 */
void hostNodePriceScheme::initialize() {
    hostNodeBase::initialize();
    
    if (myIndex() == 0) {
        // price scheme parameters         
        _nesterov = false;
        _secondOrderOptimization = true;
        _reschedulingEnabled = true;
        _minWindow = 1.0;

        _eta = par("eta");
        _kappa = par("kappa");
        _capacityFactor = par("capacityFactor");
        _useQueueEquation = par("useQueueEquation");
        _tUpdate = par("updateQueryTime");
        _tQuery = par("updateQueryTime");
        _alpha = par("alpha");
        _zeta = par("zeta"); // ewma for d_ij every source dest demand
        _minPriceRate = par("minRate");
        _rho = _rhoLambda = _rhoMu = par("rhoValue");
        _xi = par("xi");
        _routerQueueDrainTime = par("routerQueueDrainTime"); // seconds
        _windowEnabled = par("windowEnabled");
    }

    for(auto iter = nodeToPaymentChannel.begin(); iter != nodeToPaymentChannel.end(); ++iter) {
        int key = iter->first;
        
        nodeToPaymentChannel[key].lambda = nodeToPaymentChannel[key].yLambda = 0;
        nodeToPaymentChannel[key].muLocal = nodeToPaymentChannel[key].yMuLocal = 0;
        nodeToPaymentChannel[key].muRemote = nodeToPaymentChannel[key].yMuRemote = 0;

        // register signals for price per payment channel
        simsignal_t signal;
        signal = registerSignalPerChannel("nValue", key);
        nodeToPaymentChannel[key].nValueSignal = signal;
        
        signal = registerSignalPerChannel("muLocal", key);
        nodeToPaymentChannel[key].muLocalSignal = signal;

        signal = registerSignalPerChannel("lambda", key);
        nodeToPaymentChannel[key].lambdaSignal = signal;
    }

    // initialize signals with all other nodes in graph
    // that there is demand for
    for (int i = 0; i < _numHostNodes; ++i) {
        if (_destList[myIndex()].count(i) > 0) {
            simsignal_t signal;
            signal = registerSignalPerDest("demandEstimate", i, "");
            demandEstimatePerDestSignals[i] = signal;
        
            signal = registerSignalPerDest("numWaiting", i, "_Total");
            numWaitingPerDestSignals[i] = signal;
        }
    }
    // trigger the first set of triggers for price update and queries
    routerMsg *triggerPriceUpdateMsg = generateTriggerPriceUpdateMessage();
    scheduleAt(simTime() + _tUpdate, triggerPriceUpdateMsg );

    routerMsg *triggerPriceQueryMsg = generateTriggerPriceQueryMessage();
    scheduleAt(simTime() + _tQuery, triggerPriceQueryMsg );
}


###########################################################################
/home/kush/Desktop/spiderproj/spider_omnet/hostNodePropFairPriceScheme.cc

#include "hostNodePropFairPriceScheme.h"
Define_Module(hostNodePropFairPriceScheme);

// global parameters
/* new variables */
double _cannonicalRTT = 0;
double _totalPaths = 0;
double computeDemandRate = 0.5;
double rateDecreaseFrequency = 5;


/* creates a message that is meant to signal the time when a transaction can
 * be sent on the path in context based on the current view of the rate
 */
routerMsg *hostNodePropFairPriceScheme::generateTriggerTransactionSendMessage(vector<int> path, 
        int pathIndex, int destNode) {
    char msgname[MSGSIZE];
    sprintf(msgname, "tic-%d-to-%d (path %d) transactionSendMsg", myIndex(), destNode, pathIndex);
    transactionSendMsg *tsMsg = new transactionSendMsg(msgname);
    tsMsg->setPathIndex(pathIndex);
    tsMsg->setTransactionPath(path);
    tsMsg->setReceiver(destNode);

    routerMsg *rMsg = new routerMsg(msgname);
    rMsg->setMessageType(TRIGGER_TRANSACTION_SEND_MSG);
    rMsg->encapsulate(tsMsg);
    return rMsg;
}

/* generate statistic trigger message every x seconds
 * to output all statistics which can then be plotted
 */
routerMsg *hostNodePropFairPriceScheme::generateComputeDemandMessage(){
    char msgname[MSGSIZE];
    sprintf(msgname, "node-%d computeDemandMsg", myIndex());
    routerMsg *rMsg = new routerMsg(msgname);
    rMsg->setMessageType(COMPUTE_DEMAND_MSG);
    return rMsg;
}

/* generate trigger message to compute the fraction of marked packets 
 * on all paths to all destinations every x seconds
 * then do the beta portion for all of those rates
 */
routerMsg *hostNodePropFairPriceScheme::generateTriggerRateDecreaseMessage(){
    char msgname[MSGSIZE];
    sprintf(msgname, "node-%d triggerRateDecreaseMsg", myIndex());
    routerMsg *rMsg = new routerMsg(msgname);
    rMsg->setMessageType(TRIGGER_RATE_DECREASE_MSG);
    return rMsg;
}

/* check if all the provided rates are non-negative and also verify
 *  that their sum is less than the demand, return false otherwise
 */
bool hostNodePropFairPriceScheme::ratesFeasible(vector<PathRateTuple> actualRates, double demand) {
    double sumRates = 0;
    for (auto a : actualRates) {
        double rate = get<1>(a);
        if (rate < (0 - _epsilon))
            return false;
        sumRates += rate;
    }
    return (sumRates <= (demand + _epsilon));
}

/* computes the projection of the given recommended rates onto the demand d_ij per source
 */
vector<PathRateTuple> hostNodePropFairPriceScheme::computeProjection(
        vector<PathRateTuple> recommendedRates, double demand) {
    /* to resolve issues very early on when demand hasn't been computed yet */
    if (demand == 0)
        demand = 1;

    auto compareRates = [](PathRateTuple rate1, PathRateTuple rate2) {
            return (get<1>(rate1) < get<1>(rate2));
    };

    auto nuFeasible = [](double nu, double nuLeft, double nuRight) {
            return (nu >= -1 * _epsilon && nu >= (nuLeft - _epsilon) && 
                    nu <= (nuRight + _epsilon));
    };

    sort(recommendedRates.begin(), recommendedRates.end(), compareRates);
    double nu = 0.0;
    vector<PathRateTuple> actualRates;

    // initialize all rates to zero
    vector<PathRateTuple> zeroRates;
    for (int i = 0; i < recommendedRates.size(); i++) {
        zeroRates.push_back(make_tuple(i, 0.0));
    }

    // if everything is negative (aka the largest is negative), just return 0s
    if (get<1>(recommendedRates[recommendedRates.size() - 1]) < 0 + _epsilon){
        return zeroRates;
    }

    // consider nu  = 0 and see if everything checks out
    actualRates = zeroRates;
    int firstPositiveIndex = -1;
    int i = 0;
    for (auto p : recommendedRates) {
        double rate = get<1>(p);
        int pathIndex = get<0>(p);
        if (rate > 0) {
            if (firstPositiveIndex == -1) 
                firstPositiveIndex = i;
            actualRates[pathIndex] = make_tuple(pathIndex, rate);
        }
        i++;
    }
    if (ratesFeasible(actualRates, demand))
        return actualRates;

    // atleast something should be positive if you got this far
    assert(firstPositiveIndex >= 0);


    // now go through all intervals between adjacent 2* recommended rates and see if any of them
    // can give you a valid assignment of actual rates and nu
    i = firstPositiveIndex; 
    double nuLeft = 0;
    double nuRight = 0;
    while (i < recommendedRates.size()) {
        // start over checking with a new nu interval
        actualRates = zeroRates;
        nuLeft = nuRight; 
        nuRight = 2*get<1>(recommendedRates[i]);

        // find sum of all elements that are to the right of nuRight
        double sumOfRightElements = 0.0;
        for (int j = i; j < recommendedRates.size(); j++)
            sumOfRightElements += get<1>(recommendedRates[j]);     
        nu = (sumOfRightElements - demand) * 2.0/(recommendedRates.size() - i);

        // given this nu, compute the actual rates for those elements to the right of nuRight
        for (auto p : recommendedRates) {
            double rate = get<1>(p);
            int pathIndex = get<0>(p);
            if (2*rate > nuLeft) {
                actualRates[pathIndex] = make_tuple(pathIndex, rate - nu/2.0);
            }
        }

        // check if these rates are feasible and nu actually falls in the right interval 
        if (ratesFeasible(actualRates, demand) && nuFeasible(nu, nuLeft, nuRight)) 
            return actualRates;

        // otherwise move on
        i++;
    }

    // should never be reached
    assert(false);
    return zeroRates;
}


/* overall controller for handling messages that dispatches the right function
 * based on message type in price Scheme
 */
void hostNodePropFairPriceScheme::handleMessage(cMessage *msg) {
    routerMsg *ttmsg = check_and_cast<routerMsg *>(msg);
    if (simTime() > _simulationLength){
        auto encapMsg = (ttmsg->getEncapsulatedPacket());
        ttmsg->decapsulate();
        delete ttmsg;
        delete encapMsg;
        return;
    } 

    switch(ttmsg->getMessageType()) {
        case TRIGGER_TRANSACTION_SEND_MSG:
             if (_loggingEnabled) cout<< "[HOST "<< myIndex() 
                 <<": RECEIVED TRIGGER_TXN_SEND MSG] "<< ttmsg->getName() << endl;
             handleTriggerTransactionSendMessage(ttmsg);
             if (_loggingEnabled) cout<< "[AFTER HANDLING:]  "<< endl;
             break;
         
        case TRIGGER_RATE_DECREASE_MSG:
             if (_loggingEnabled) cout<< "[HOST "<< myIndex() 
                 <<": RECEIVED TRIGGER_RATE_DECREASE MSG] "<< ttmsg->getName() << endl;
             handleTriggerRateDecreaseMessage(ttmsg);
             if (_loggingEnabled) cout<< "[AFTER HANDLING:]  "<< endl;
             break;   
        
        case COMPUTE_DEMAND_MSG:
             if (_loggingEnabled) cout<< "[HOST "<< myIndex() 
                 <<": RECEIVED COMPUTE_DEMAND MSG] "<< ttmsg->getName() << endl;
             handleComputeDemandMessage(ttmsg);
             if (_loggingEnabled) cout<< "[AFTER HANDLING:]  "<< endl;
             break;


        default:
             hostNodeBase::handleMessage(msg);

    }
}

/* handler for timeout messages that is responsible for removing messages from 
 * sender queues if they haven't been sent yet and sends explicit time out messages
 * for messages that have been sent on a path already
 */
void hostNodePropFairPriceScheme::handleTimeOutMessage(routerMsg* ttmsg) {
    timeOutMsg *toutMsg = check_and_cast<timeOutMsg *>(ttmsg->getEncapsulatedPacket());
    int destination = toutMsg->getReceiver();
    int transactionId = toutMsg->getTransactionId();
    multiset<routerMsg*, transCompare> *transList = &(nodeToDestInfo[destination].transWaitingToBeSent);
    
    if (ttmsg->isSelfMessage()) {
        // if transaction was successful don't do anything more
        if (successfulDoNotSendTimeOut.count(transactionId) > 0) {
            successfulDoNotSendTimeOut.erase(toutMsg->getTransactionId());
            ttmsg->decapsulate();
            delete toutMsg;
            delete ttmsg;
            return;
        }

        // check if txn is still in just sender queue, just delete and return then
        auto iter = find_if(transList->begin(),
           transList->end(),
           [&transactionId](const routerMsg* p)
           { transactionMsg *transMsg = check_and_cast<transactionMsg *>(p->getEncapsulatedPacket());
             return transMsg->getTransactionId()  == transactionId; });

        if (iter!=transList->end()) {
            deleteTransaction(*iter);
            transList->erase(iter);
            ttmsg->decapsulate();
            statNumTimedOut[destination] += 1;
            delete toutMsg;
            delete ttmsg;
            return;
        }

        // send out a time out message on the path that hasn't acked all of it
        for (auto p : (nodeToShortestPathsMap[destination])){
            int pathIndex = p.first;
            tuple<int,int> key = make_tuple(transactionId, pathIndex);
                        
            if (transPathToAckState.count(key) > 0 && 
                    transPathToAckState[key].amtSent != transPathToAckState[key].amtReceived) {
                routerMsg* psMsg = generateTimeOutMessageForPath(
                    nodeToShortestPathsMap[destination][p.first].path, 
                    transactionId, destination);
                int nextNode = (psMsg->getRoute())[psMsg->getHopCount()+1];
                CanceledTrans ct = make_tuple(toutMsg->getTransactionId(), 
                        simTime(), -1, nextNode, destination);
                canceledTransactions.insert(ct);
                forwardMessage(psMsg);
            }
            else {
                transPathToAckState.erase(key);
            }
        }
        ttmsg->decapsulate();
        delete toutMsg;
        delete ttmsg;
    }
    else {
        // at the receiver
        CanceledTrans ct = make_tuple(toutMsg->getTransactionId(),simTime(),
                (ttmsg->getRoute())[ttmsg->getHopCount()-1], -1, toutMsg->getReceiver());
        canceledTransactions.insert(ct);
        ttmsg->decapsulate();
        delete toutMsg;
        delete ttmsg;
    }
}


/* main routine for handling a new transaction under the pricing scheme
 * In particular, initiate price probes, assigns a txn to a path if the 
 * rate for that path allows it, otherwise queues it at the sender
 * until a timer fires on the next path allowing a txn to go through
 */
void hostNodePropFairPriceScheme::handleTransactionMessageSpecialized(routerMsg* ttmsg){
    transactionMsg *transMsg = check_and_cast<transactionMsg *>(ttmsg->getEncapsulatedPacket());
    int hopcount = ttmsg->getHopCount();
    int destNode = transMsg->getReceiver();
    int nextNode = ttmsg->getRoute()[hopcount + 1];
    int transactionId = transMsg->getTransactionId();

    SplitState* splitInfo = &(_numSplits[myIndex()][transMsg->getLargerTxnId()]);
    splitInfo->numArrived += 1;
    
    // first time seeing this transaction so add to d_ij computation
    // count the txn for accounting also
    if (simTime() == transMsg->getTimeSent()) {
        destNodeToNumTransPending[destNode]  += 1;
        nodeToDestInfo[destNode].transSinceLastInterval += transMsg->getAmount();
        if (splitInfo->numArrived == 1)
            splitInfo->firstAttemptTime = simTime().dbl();

        if (transMsg->getTimeSent() >= _transStatStart && 
            transMsg->getTimeSent() <= _transStatEnd) {
            statAmtArrived[destNode] += transMsg->getAmount();
            
            if (splitInfo->numArrived == 1) {       
                statNumArrived[destNode] += 1;
                statRateArrived[destNode] += 1; 
            }
        }
    }

    // initiate price probes if it is a new destination
    if (nodeToShortestPathsMap.count(destNode) == 0 && ttmsg->isSelfMessage()){
        vector<vector<int>> kShortestRoutes = getKPaths(transMsg->getSender(),destNode, _kValue);
        initializePathInfo(kShortestRoutes, destNode);
    }
    
    // at destination, add to incoming transUnits and trigger ack
    if (transMsg->getReceiver() == myIndex()) {
        int prevNode = ttmsg->getRoute()[ttmsg->getHopCount() - 1];
        unordered_map<Id, double, hashId> *incomingTransUnits = 
            &(nodeToPaymentChannel[prevNode].incomingTransUnits);
        (*incomingTransUnits)[make_tuple(transMsg->getTransactionId(), transMsg->getHtlcIndex())] =
            transMsg->getAmount();
        
        if (_timeoutEnabled) {
            auto iter = canceledTransactions.find(make_tuple(transactionId, 0, 0, 0, 0));
            if (iter != canceledTransactions.end()){
                canceledTransactions.erase(iter);
            }
        }
        routerMsg* newMsg =  generateAckMessage(ttmsg);
        forwardMessage(newMsg);
        return;
    }
    else if (ttmsg->isSelfMessage()) {
        // at sender, either queue up or send on a path that allows you to send
        DestInfo* destInfo = &(nodeToDestInfo[destNode]);

        // use a random ordering on the path indices
        vector<int> pathIndices;
        for (int i = 0; i < nodeToShortestPathsMap[destNode].size(); ++i) pathIndices.push_back(i);
        random_shuffle(pathIndices.begin(), pathIndices.end());
       
        //send on a path if no txns queued up and timer was in the path
        if ((destInfo->transWaitingToBeSent).size() > 0) {
            pushIntoSenderQueue(destInfo, ttmsg);
        } else {
            for (auto p: pathIndices) {
                int pathIndex = p;
                PathInfo *pathInfo = &(nodeToShortestPathsMap[destNode][pathIndex]);
                
                if (pathInfo->rateToSendTrans > 0 && simTime() > pathInfo->timeToNextSend && 
                        pathInfo->sumOfTransUnitsInFlight + transMsg->getAmount() <= pathInfo->window) {
                    ttmsg->setRoute(pathInfo->path);
                    ttmsg->setHopCount(0);
                    transMsg->setPathIndex(pathIndex);
                    handleTransactionMessage(ttmsg, true /*revisit*/);

                    // first attempt of larger txn
                    SplitState* splitInfo = &(_numSplits[myIndex()][transMsg->getLargerTxnId()]);
                    if (splitInfo->numAttempted == 0) {
                        splitInfo->numAttempted += 1;
                        if (transMsg->getTimeSent() >= _transStatStart && 
                            transMsg->getTimeSent() <= _transStatEnd) 
                            statRateAttempted[destNode] += 1;
                    }
                    
                    if (transMsg->getTimeSent() >= _transStatStart && 
                            transMsg->getTimeSent() <= _transStatEnd) {
                        statAmtAttempted[destNode] += transMsg->getAmount();
                    }
                    
                    // update stats
                    pathInfo->nValue += transMsg->getAmount();
                    pathInfo->statRateAttempted += 1;
                    pathInfo->sumOfTransUnitsInFlight += transMsg->getAmount();
                    pathInfo->lastTransSize = transMsg->getAmount();
                    pathInfo->lastSendTime = simTime();
                    pathInfo->amtAllowedToSend = max(pathInfo->amtAllowedToSend - transMsg->getAmount(), 0.0);

                    // necessary for knowing what path to remove transaction in flight funds from
                    tuple<int,int> key = make_tuple(transMsg->getTransactionId(), pathIndex); 
                    transPathToAckState[key].amtSent += transMsg->getAmount();
                
                    // update the "time when the next transaction can be sent"
                    double rateToUse = max(pathInfo->rateToSendTrans, _smallRate);
                    double additional = min(max((transMsg->getAmount())/ rateToUse, _epsilon), 10000.0);
                    simtime_t newTimeToNextSend =  simTime() + additional;
                    pathInfo->timeToNextSend = newTimeToNextSend;
                    
                    return;
                }
            }
            
            // transaction cannot be sent on any of the paths, queue transaction
            pushIntoSenderQueue(destInfo, ttmsg);

            for (auto p: nodeToShortestPathsMap[destNode]) {
                PathInfo *pInfo = &(nodeToShortestPathsMap[destNode][p.first]);
                if (pInfo->isSendTimerSet == false) {
                    simtime_t timeToNextSend = pInfo->timeToNextSend;
                    if (simTime() > timeToNextSend) {
                        timeToNextSend = simTime() + 0.0001;
                    }
                    cancelEvent(pInfo->triggerTransSendMsg);            
                    scheduleAt(timeToNextSend, pInfo->triggerTransSendMsg);
                    pInfo->isSendTimerSet = true;
                }
            }
        }
    }
}

/* handler for compute demand message triggered every y seconds simply to comput the demand per destination
 */
void hostNodePropFairPriceScheme::handleComputeDemandMessage(routerMsg* ttmsg){
    // reschedule this message to be sent again
    if (simTime() > _simulationLength){
        delete ttmsg;
    }
    else {
        scheduleAt(simTime() + computeDemandRate, ttmsg);
    }

    for (auto it = 0; it < _numHostNodes; it++){ 
        if (it != getIndex() && _destList[myIndex()].count(it) > 0) {
            //nodeToDestInfo[it].demand = (myIndex() == 0) ? 100 : 250 ;
            DestInfo* destInfo = &(nodeToDestInfo[it]);
            double newDemand = destInfo->transSinceLastInterval / computeDemandRate;
            destInfo->demand = (1 - _zeta) * destInfo->demand + _zeta * newDemand;
            destInfo->transSinceLastInterval = 0;
        }
    }
}


/* handler for trigger rate decrease message triggered every y seconds
 * to compute the fraction of marked packets on all paths to the destinations
 * and perform the decrease portion of the control algorithm
 */
void hostNodePropFairPriceScheme::handleTriggerRateDecreaseMessage(routerMsg* ttmsg){
    // reschedule this message to be sent again
    if (simTime() > _simulationLength){
        delete ttmsg;
    }
    else {
        scheduleAt(simTime() + rateDecreaseFrequency, ttmsg);
    }

    // go through all destinations that have pending transactions and 
    // update the fraction of marked packets over the last y seconds
    for (auto it = destNodeToNumTransPending.begin(); it!=destNodeToNumTransPending.end(); it++){
        if (it->first == myIndex()){
            continue;
        }
        
        if (it->second>0){ 
            //if we have transactions pending
            for (auto p = nodeToShortestPathsMap[it->first].begin() ;
                    p!= nodeToShortestPathsMap[it->first].end(); p++){
                // p is per path in the destNode
                int routeIndex = p->first;
                PathInfo *pInfo = &(nodeToShortestPathsMap[it->first][p->first]);
                pInfo->lastMarkedFraction = pInfo->totalMarkedPacketsForInterval/pInfo->totalPacketsForInterval;

                pInfo->totalMarkedPacketsForInterval = 0;
                pInfo->totalPacketsForInterval = 0;
                pInfo->rateToSendTrans  -= _windowBeta * pInfo->lastMarkedFraction * pInfo->rateToSendTrans;
                pInfo->rateToSendTrans = max(_minPriceRate, pInfo->rateToSendTrans);
            }
        }
    }
}


/* handler for the statistic message triggered every x seconds to also
 * output the price based scheme stats in addition to the default
 */
void hostNodePropFairPriceScheme::handleStatMessage(routerMsg* ttmsg){
    if (_signalsEnabled) {
        // per destination statistics
        for (auto it = 0; it < _numHostNodes; it++){ 
            if (it != getIndex() && _destList[myIndex()].count(it) > 0) {
                if (nodeToShortestPathsMap.count(it) > 0) {
                    for (auto& p: nodeToShortestPathsMap[it]){
                        int pathIndex = p.first;
                        PathInfo *pInfo = &(p.second);

                        //signals for price scheme per path
                        emit(pInfo->rateToSendTransSignal, pInfo->rateToSendTrans);
                        emit(pInfo->rateActuallySentSignal, pInfo->nValue /_statRate);
                        emit(pInfo->sumOfTransUnitsInFlightSignal, 
                                pInfo->sumOfTransUnitsInFlight);
                        emit(pInfo->windowSignal, pInfo->window);
                        emit(pInfo->rateOfAcksSignal, pInfo->amtAcked/_statRate);
                        emit(pInfo->measuredRTTSignal, pInfo->measuredRTT);
                        emit(pInfo->fractionMarkedSignal, 
                                pInfo->markedPackets/(pInfo->markedPackets + pInfo->unmarkedPackets));
                        emit(pInfo->smoothedFractionMarkedSignal, pInfo->lastMarkedFraction);
                        pInfo->amtAcked = 0;
                        pInfo->unmarkedPackets = 0;
                        pInfo->markedPackets = 0;
                        pInfo->nValue = 0;
                    }
                }

                emit(demandEstimatePerDestSignals[it], nodeToDestInfo[it].demand);
                emit(numWaitingPerDestSignals[it], 
                       nodeToDestInfo[it].transWaitingToBeSent.size()); 
            }        
        }
    } 

    // call the base method to output rest of the stats
    hostNodeBase::handleStatMessage(ttmsg);
}


/* specialized ack handler that does the routine if this is a price scheme 
 * algorithm. In particular, collects/updates stats for this path alone
 * NOTE: acks are on the reverse path relative to the original sender
 */
void hostNodePropFairPriceScheme::handleAckMessageSpecialized(routerMsg* ttmsg){
    ackMsg *aMsg = check_and_cast<ackMsg*>(ttmsg->getEncapsulatedPacket());
    int pathIndex = aMsg->getPathIndex();
    int destNode = ttmsg->getRoute()[0];
    int transactionId = aMsg->getTransactionId();
    double largerTxnId = aMsg->getLargerTxnId();
    PathInfo* thisPath = &(nodeToShortestPathsMap[destNode][pathIndex]);

    // rate update based on marked or unmarked packet
    if (aMsg->getIsMarked()) {
        thisPath->window  -= _windowBeta;
        thisPath->window = max(_minWindow, thisPath->window);
        thisPath->markedPackets += 1; 
        thisPath->totalMarkedPacketsForInterval += 1;
    }
    else {
        thisPath->unmarkedPackets += 1; 
        double sumWindows = 0; 
        for (auto p : nodeToShortestPathsMap[destNode]) 
            sumWindows += p.second.window;
        thisPath->window += _windowAlpha / sumWindows;
    }
    thisPath->totalPacketsForInterval += 1;
    double preProjectionRate = thisPath->window/(0.9 * thisPath->measuredRTT);
    
    vector<PathRateTuple> pathRateTuples;
    double sumRates = 0;
    for (auto p : nodeToShortestPathsMap[destNode]) {
        sumRates += p.second.rateToSendTrans;
        double rate;
        if (p.first != pathIndex)
            rate = p.second.rateToSendTrans;
        else
            rate = preProjectionRate;
        PathRateTuple newTuple = make_tuple(p.first, rate);
        pathRateTuples.push_back(newTuple);
    }
    vector<PathRateTuple> projectedRates = pathRateTuples; 

    // reassign all path's rates to the projected rates and 
    // make sure it is atleast minPriceRate for every path
    for (auto p : projectedRates) {
        int index = get<0>(p);
        double newRate = max(get<1>(p), _minPriceRate);
        updateTimers(destNode, index, newRate);
    }

    // react to success or failure
    if (aMsg->getIsSuccess() == false) {
        auto iter = canceledTransactions.find(make_tuple(transactionId, 0, 0, 0, 0));
        if (iter != canceledTransactions.end()) {
            if (aMsg->getTimeSent() >= _transStatStart && aMsg->getTimeSent() <= _transStatEnd)
                statAmtFailed[destNode] += aMsg->getAmount();
        } 
        else {
            // requeue transaction
            routerMsg *duplicateTrans = generateDuplicateTransactionMessage(aMsg);
            pushIntoSenderQueue(&(nodeToDestInfo[destNode]), duplicateTrans);
        }
    }
    else {
        SplitState* splitInfo = &(_numSplits[myIndex()][largerTxnId]);
        splitInfo->numReceived += 1;

        if (aMsg->getTimeSent() >= _transStatStart && 
                aMsg->getTimeSent() <= _transStatEnd) {
            statAmtCompleted[destNode] += aMsg->getAmount();
            if (splitInfo->numTotal == splitInfo->numReceived) {
                statNumCompleted[destNode] += 1; 
                statRateCompleted[destNode] += 1;
                _transactionCompletionBySize[splitInfo->totalAmount] += 1;
                double timeTaken = simTime().dbl() - splitInfo->firstAttemptTime;
                statCompletionTimes[destNode] += timeTaken * 1000;
                _txnAvgCompTimeBySize[splitInfo->totalAmount] += timeTaken * 1000;
                recordTailCompletionTime(aMsg->getTimeSent(), splitInfo->totalAmount, timeTaken * 1000);
            }
        }
        thisPath->statRateCompleted += 1;
        thisPath->amtAcked += aMsg->getAmount();
        double newRTT = simTime().dbl() - aMsg->getTimeAttempted();
        thisPath->measuredRTT = 0.01 * newRTT + 0.99 * thisPath->measuredRTT;
    }

    // increment transaction amount ack on a path. 
    tuple<int,int> key = make_tuple(transactionId, pathIndex);
    transPathToAckState[key].amtReceived += aMsg->getAmount();
    
    nodeToShortestPathsMap[destNode][pathIndex].sumOfTransUnitsInFlight -= aMsg->getAmount();
    destNodeToNumTransPending[destNode] -= 1;     
    hostNodeBase::handleAckMessage(ttmsg);
}

/* handler for the clear state message that deals with
 * transactions that will no longer be completed
 * In particular clears out the amount inn flight on the path
 */
void hostNodePropFairPriceScheme::handleClearStateMessage(routerMsg *ttmsg) {
    for ( auto it = canceledTransactions.begin(); it!= canceledTransactions.end(); it++){
        int transactionId = get<0>(*it);
        simtime_t msgArrivalTime = get<1>(*it);
        int prevNode = get<2>(*it);
        int nextNode = get<3>(*it);
        int destNode = get<4>(*it);
        
        if (simTime() > (msgArrivalTime + _maxTravelTime + 1)){
            // ack was not received,safely can consider this txn done
            for (auto p : nodeToShortestPathsMap[destNode]) {
                int pathIndex = p.first;
                PathInfo* thisPath = &(nodeToShortestPathsMap[destNode][pathIndex]);
                tuple<int,int> key = make_tuple(transactionId, pathIndex);
                if (transPathToAckState.count(key) != 0) {
                    thisPath->sumOfTransUnitsInFlight -= 
                        (transPathToAckState[key].amtSent - transPathToAckState[key].amtReceived);
                    transPathToAckState.erase(key);

                    // treat this basiscally as one marked packet
                    thisPath->window  -= _windowBeta;
                    thisPath->window = max(_minWindow, thisPath->window);
                    thisPath->markedPackets += 1; 
                    thisPath->totalMarkedPacketsForInterval += 1;
                    thisPath->rateToSendTrans = thisPath->window/(0.9 * thisPath->measuredRTT);
                }
            }
        }
    }

    // works fine now because timeouts start per transaction only when
    // sent out and no txn splitting
    hostNodeBase::handleClearStateMessage(ttmsg);
}


/* handles the sending of transactions at a certain rate indirectly via timers 
 * going off to trigger the next send on a particular path. This responds to
 * that trigger and pulls the next transaction off the queue and sends that
 */
void hostNodePropFairPriceScheme::handleTriggerTransactionSendMessage(routerMsg* ttmsg){
    transactionSendMsg *tsMsg = 
        check_and_cast<transactionSendMsg *>(ttmsg->getEncapsulatedPacket());

    vector<int> path = tsMsg->getTransactionPath();
    int pathIndex = tsMsg->getPathIndex();
    int destNode = tsMsg->getReceiver();
    PathInfo* p = &(nodeToShortestPathsMap[destNode][pathIndex]);

    bool sentSomething = false;
    if (nodeToDestInfo[destNode].transWaitingToBeSent.size() > 0) {
        auto firstPosition = nodeToDestInfo[destNode].transWaitingToBeSent.begin();
        routerMsg *msgToSend = *firstPosition;
        transactionMsg *transMsg = 
           check_and_cast<transactionMsg *>(msgToSend->getEncapsulatedPacket());
        
        if (p->sumOfTransUnitsInFlight + transMsg->getAmount() <= p->window){
            //remove the transaction $tu$ at the head of the queue
            nodeToDestInfo[destNode].transWaitingToBeSent.erase(firstPosition);
            sentSomething = true;
            
            //Send the transaction $tu$ along the corresponding path.
            transMsg->setPathIndex(pathIndex);
            msgToSend->setRoute(path);
            msgToSend->setHopCount(0);

            // increment amount in inflght and other info on last transaction on this path
            p->nValue += transMsg->getAmount();
            p->sumOfTransUnitsInFlight = p->sumOfTransUnitsInFlight + transMsg->getAmount();
            p->lastTransSize = transMsg->getAmount();
            p->lastSendTime = simTime();
            p->amtAllowedToSend = max(p->amtAllowedToSend - transMsg->getAmount(), 0.0);
            
            // necessary for knowing what path to remove transaction in flight funds from
            tuple<int,int> key = make_tuple(transMsg->getTransactionId(), pathIndex); 
            transPathToAckState[key].amtSent += transMsg->getAmount();

            // cannot be cancelled at this point
            handleTransactionMessage(msgToSend, 1/*revisiting*/);
            p->statRateAttempted = p->statRateAttempted + 1;

            // first attempt of larger txn
            SplitState* splitInfo = &(_numSplits[myIndex()][transMsg->getLargerTxnId()]);
            if (splitInfo->numAttempted == 0) {
                splitInfo->numAttempted += 1;
                if (transMsg->getTimeSent() >= _transStatStart && 
                    transMsg->getTimeSent() <= _transStatEnd)
                    statRateAttempted[destNode] += 1;
            }

            if (transMsg->getTimeSent() >= _transStatStart && 
                transMsg->getTimeSent() <= _transStatEnd){
                statAmtAttempted[destNode] += transMsg->getAmount();
            }

            //Update the  “time when next transaction can be sent” 
            double bound = _reschedulingEnabled ? _smallRate  : 1.0;
            double rateToSendTrans = max(p->rateToSendTrans, bound);
            double additional =  min(transMsg->getAmount()/rateToSendTrans, 10000.0);
            p->timeToNextSend = simTime() + additional; 

            //If there are more transactions queued up, reschedule timer
            cancelEvent(ttmsg);
            scheduleAt(p->timeToNextSend, ttmsg);
        }
    }

    if (!sentSomething) {
        // something shady here TODO 
        // no trans sendable, just reschedule timer in a little bit as if it had never happened
        PathInfo* p = &(nodeToShortestPathsMap[destNode][pathIndex]);
        double rateToSendTrans = max(p->rateToSendTrans, _smallRate);
        double lastTxnSize = p->lastTransSize;
        double additional = min(max(lastTxnSize/rateToSendTrans, _epsilon), 10000.0);
        p->timeToNextSend = simTime() + additional;
        cancelEvent(ttmsg);
        scheduleAt(p->timeToNextSend, ttmsg);
        p->amtAllowedToSend = 0.0;
    }
}

/* initialize data for for the paths supplied to the destination node
 * and also fix the paths for susbequent transactions to this destination
 * and register signals that are path specific
 */
void hostNodePropFairPriceScheme::initializePathInfo(vector<vector<int>> kShortestPaths, int destNode){
    for (int pathIdx = 0; pathIdx < kShortestPaths.size(); pathIdx++){
        // initialize pathInfo
        PathInfo temp = {};
        temp.path = kShortestPaths[pathIdx];
        routerMsg * triggerTransSendMsg = 
          generateTriggerTransactionSendMessage(kShortestPaths[pathIdx], pathIdx, destNode);
        temp.triggerTransSendMsg = triggerTransSendMsg;
        temp.rateToSendTrans = _minPriceRate;
        temp.window = _minWindow;
        temp.rttMin = (kShortestPaths[pathIdx].size() - 1) * 2 * _avgDelay/1000.0;
        nodeToShortestPathsMap[destNode][pathIdx] = temp;

        //initialize signals
        simsignal_t signal;
        signal = registerSignalPerDestPath("rateToSendTrans", pathIdx, destNode);
        nodeToShortestPathsMap[destNode][pathIdx].rateToSendTransSignal = signal;

        signal = registerSignalPerDestPath("rateSent", pathIdx, destNode);
        nodeToShortestPathsMap[destNode][pathIdx].rateActuallySentSignal = signal;
        
        signal = registerSignalPerDestPath("timeToNextSend", pathIdx, destNode);
        nodeToShortestPathsMap[destNode][pathIdx].timeToNextSendSignal = signal;

        signal = registerSignalPerDestPath("sumOfTransUnitsInFlight", pathIdx, destNode);
        nodeToShortestPathsMap[destNode][pathIdx].sumOfTransUnitsInFlightSignal = signal;

        signal = registerSignalPerDestPath("window", pathIdx, destNode);
        nodeToShortestPathsMap[destNode][pathIdx].windowSignal = signal;
              
        signal = registerSignalPerDestPath("fractionMarked", pathIdx, destNode);
        nodeToShortestPathsMap[destNode][pathIdx].fractionMarkedSignal = signal;
  
        signal = registerSignalPerDestPath("smoothedFractionMarked", pathIdx, destNode);
        nodeToShortestPathsMap[destNode][pathIdx].smoothedFractionMarkedSignal = signal;      
        
        signal = registerSignalPerDestPath("rateOfAcks", pathIdx, destNode);
        nodeToShortestPathsMap[destNode][pathIdx].rateOfAcksSignal = signal;

        signal = registerSignalPerDestPath("measuredRTT", pathIdx, destNode);
        nodeToShortestPathsMap[destNode][pathIdx].measuredRTTSignal = signal;
   }
}



/* helper method to reschedule the timer on a given path according to the new rate
 */
void hostNodePropFairPriceScheme::updateTimers(int destNode, int pathIndex, double newRate) {
    PathInfo *p = &(nodeToShortestPathsMap[destNode][pathIndex]);
    simtime_t lastSendTime = p->lastSendTime;
    double lastTxnSize = p->lastTransSize;
    simtime_t oldTime = p->timeToNextSend;

    // compute allowed to send
    double oldRate = p->rateToSendTrans; 
    simtime_t lastUpdateTime = p->lastRateUpdateTime; 
    simtime_t timeForThisRate = min(simTime() - lastUpdateTime, simTime() - lastSendTime);
    p->amtAllowedToSend += oldRate * timeForThisRate.dbl();
    p->amtAllowedToSend = min(lastTxnSize, p->amtAllowedToSend);

    // update the rate
    p->rateToSendTrans = newRate;
    double allowedToSend = p->amtAllowedToSend;
    p->lastRateUpdateTime = simTime();

    // Reschedule timer on this path according to this rate
    double rateToUse = max(newRate, _smallRate);
    double additional = min(max((lastTxnSize - allowedToSend)/ rateToUse, _epsilon), 10000.0);
    simtime_t newTimeToSend = simTime() + additional;

    cancelEvent(p->triggerTransSendMsg);
    p->timeToNextSend = newTimeToSend;
    scheduleAt(newTimeToSend, p->triggerTransSendMsg);
}

/* additional initalization that has to be done for the price based scheme
 * in particular set price variables to zero, initialize more signals
 * and schedule the first price update and price trigger
 */
void hostNodePropFairPriceScheme::initialize() {
    hostNodeBase::initialize();
    
    if (myIndex() == 0) {
        // price scheme parameters         
        _reschedulingEnabled = true;
        _minWindow = par("minDCTCPWindow");
        _windowEnabled = true;
        _windowAlpha = par("windowAlpha");
        _windowBeta = par("windowBeta");
        _qEcnThreshold = par("queueThreshold");
        _balEcnThreshold = par("balanceThreshold");
        _zeta = par("zeta"); // ewma for d_ij every source dest demand
        _minPriceRate = par("minRate");
        rateDecreaseFrequency = par("rateDecreaseFrequency");
    }

    //initialize signals with all other nodes in graph
    // that there is demand for
    for (int i = 0; i < _numHostNodes; ++i) {
        if (_destList[myIndex()].count(i) > 0) {
            simsignal_t signal;
            signal = registerSignalPerDest("demandEstimate", i, "");
            demandEstimatePerDestSignals[i] = signal;
        
            signal = registerSignalPerDest("numWaiting", i, "_Total");
            numWaitingPerDestSignals[i] = signal;
        }
    }

    // trigger the message to compute demand to all destinations periodically
    routerMsg *computeDemandMsg = generateComputeDemandMessage();
    scheduleAt(simTime() + 0, computeDemandMsg);
}


##############################################################################
/home/kush/Desktop/spiderproj/spider_omnet/hostNodeWaterfilling.cc

#include "hostNodeWaterfilling.h"

// global parameters
// set to 1 to report exact instantaneous balances
double _ewmaFactor;

// parameters for smooth waterfilling
double _Tau;
double _Normalizer;
bool _smoothWaterfillingEnabled;
#define SMALLEST_INDIVISIBLE_UNIT 1 

Define_Module(hostNodeWaterfilling);

/* initialization function to initialize parameters */
void hostNodeWaterfilling::initialize(){
    hostNodeBase::initialize();
    
    if (myIndex() == 0) {
        // smooth waterfilling parameters
        _Tau = par("tau");
        _Normalizer = par("normalizer"); // TODO: C from discussion with Mohammad)
        _ewmaFactor = 1; // EWMA factor for balance information on probes
        _smoothWaterfillingEnabled = par("smoothWaterfillingEnabled");
    }

    //initialize signals with all other nodes in graph
    // that there is demand for
    for (int i = 0; i < _numHostNodes; ++i) {
        if (_destList[myIndex()].count(i) > 0) {
            simsignal_t signal;
            signal = registerSignalPerDest("numWaiting", i, "_Total");
            numWaitingPerDestSignals[i] = signal;
        }
    }
}

/* generates the probe message for a particular destination and a particur path
 * identified by the list of hops and the path index
 */
routerMsg* hostNodeWaterfilling::generateProbeMessage(int destNode, int pathIdx, vector<int> path){
    char msgname[MSGSIZE];
    sprintf(msgname, "tic-%d-to-%d probeMsg [idx %d]", myIndex(), destNode, pathIdx);
    probeMsg *pMsg = new probeMsg(msgname);
    pMsg->setSender(myIndex());
    pMsg->setPathIndex(pathIdx);
    pMsg->setReceiver(destNode);
    pMsg->setIsReversed(false);
    vector<double> pathBalances;
    pMsg->setPathBalances(pathBalances);
    pMsg->setPath(path);

    sprintf(msgname, "tic-%d-to-%d router-probeMsg [idx %d]", myIndex(), destNode, pathIdx);
    routerMsg *rMsg = new routerMsg(msgname);
    rMsg->setRoute(path);
    rMsg->setHopCount(0);
    rMsg->setMessageType(PROBE_MSG);
    rMsg->encapsulate(pMsg);
    return rMsg;
}



/* overall controller for handling messages that dispatches the right function
 * based on message type in waterfilling
 */
void hostNodeWaterfilling::handleMessage(cMessage *msg) {
    routerMsg *ttmsg = check_and_cast<routerMsg *>(msg);
    if (simTime() > _simulationLength){
        auto encapMsg = (ttmsg->getEncapsulatedPacket());
        ttmsg->decapsulate();
        delete ttmsg;
        delete encapMsg;
        return;
    } 

    switch(ttmsg->getMessageType()) {
        case PROBE_MSG:
             if (_loggingEnabled) cout<< "[HOST "<< myIndex() 
                 <<": RECEIVED PROBE MSG] "<< ttmsg->getName() << endl;
             handleProbeMessage(ttmsg);
             if (_loggingEnabled) cout<< "[AFTER HANDLING:]  "<< endl;
             break;
        default:
             hostNodeBase::handleMessage(msg);
    }
}

/* main routine for handling transaction messages for waterfilling
 * that initiates probes and splits transactions according to latest probes
 */
void hostNodeWaterfilling::handleTransactionMessageSpecialized(routerMsg* ttmsg){
    transactionMsg *transMsg = check_and_cast<transactionMsg *>(ttmsg->getEncapsulatedPacket());
    int hopCount = ttmsg->getHopCount();
    int destNode = transMsg->getReceiver();
    int nextNode = ttmsg->getRoute()[hopCount+1];
    int transactionId = transMsg->getTransactionId();
    double waitTime = _maxTravelTime;
    
    // txn at receiver
    if (ttmsg->getRoute()[hopCount] == destNode) {
       handleTransactionMessage(ttmsg, false); 
    }
    else { 
        // transaction received at sender
        // If transaction seen for first time, update stats.
        if (simTime() == transMsg->getTimeSent()) {
            SplitState* splitInfo = &(_numSplits[myIndex()][transMsg->getLargerTxnId()]);
            splitInfo->numArrived += 1;
            
            if (transMsg->getTimeSent() >= _transStatStart && transMsg->getTimeSent() <= _transStatEnd) {
                if (transMsg->getIsAttempted() == false)
                    statAmtArrived[destNode] += transMsg->getAmount();
                if (splitInfo->numArrived == 1) {
                    statRateArrived[destNode] += 1;
                }
            }
            destNodeToNumTransPending[destNode] += 1; 
            if (splitInfo->numArrived == 1) 
                statNumArrived[destNode] += 1; 
            
            AckState * s = new AckState();
            s->amtReceived = 0;
            s->amtSent = transMsg->getAmount();
            transToAmtLeftToComplete[transMsg->getTransactionId()] = *s;
        }
        
        // Compute paths and initialize probes if destination hasn't been encountered
        if (nodeToShortestPathsMap.count(destNode) == 0 ){
            vector<vector<int>> kShortestRoutes = getKPaths(transMsg->getSender(),destNode, _kValue);
            initializeProbes(kShortestRoutes, destNode);
            scheduleAt(simTime() + waitTime, ttmsg);
            return;
        }
        else {
            // if all probes from destination are recent enough and txn hasn't timed out, 
            // send transaction on one or more paths.
            bool recent = probesRecent(nodeToShortestPathsMap[destNode]);
            if (recent){
                if ((!_timeoutEnabled) || (simTime() < (transMsg->getTimeSent() + transMsg->getTimeOut()))) { 
                    attemptTransactionOnBestPath(ttmsg, !transMsg->getIsAttempted());
                    double amtRemaining = transMsg->getAmount();
                    if (amtRemaining > 0 + _epsilon) {
                        pushIntoSenderQueue(&(nodeToDestInfo[destNode]), ttmsg);
                    }
                    else {
                        ttmsg->decapsulate();
                        delete transMsg;
                        delete ttmsg;
                    }
                }
                else {
                    // transaction timed out
                    statNumTimedOut[destNode] += 1;
                    statNumTimedOutAtSender[destNode] += 1; 
                    ttmsg->decapsulate();
                    delete transMsg;
                    delete ttmsg;
                }
                return;
            }
            else { 
                // need more recent probes
                if (destNodeToNumTransPending[destNode] > 0) {
                    restartProbes(destNode);
                }
                pushIntoSenderQueue(&(nodeToDestInfo[destNode]), ttmsg);
                return;
            }
        }
    }
}

/* handles the special time out mechanism for waterfilling which is responsible
 * for sending time out messages on all paths that may have seen this txn and 
 * marking the txn as cancelled
 */
void hostNodeWaterfilling::handleTimeOutMessage(routerMsg* ttmsg){
    timeOutMsg *toutMsg = check_and_cast<timeOutMsg *>(ttmsg->getEncapsulatedPacket());
    if (ttmsg->isSelfMessage()) { 
        //is at the sender
        int transactionId = toutMsg->getTransactionId();
        int destination = toutMsg->getReceiver();
        multiset<routerMsg*, transCompare>* transList = &(nodeToDestInfo[destination].transWaitingToBeSent);

        // if transaction was successful don't do anything more
        if (successfulDoNotSendTimeOut.count(transactionId) > 0) {
            successfulDoNotSendTimeOut.erase(toutMsg->getTransactionId());
            ttmsg->decapsulate();
            delete toutMsg;
            delete ttmsg;
            return;
        }

        // check if txn is still in just sender queue
        auto iter = find_if(transList->begin(),
           transList->end(),
           [&transactionId](const routerMsg* p)
           { transactionMsg *transMsg = check_and_cast<transactionMsg *>(p->getEncapsulatedPacket());
             return transMsg->getTransactionId()  == transactionId; });

        if (iter!=transList->end()) {
            deleteTransaction(*iter);
            transList->erase(iter);
            ttmsg->decapsulate();
            delete toutMsg;
            delete ttmsg;
            return;
        }
       
        for (auto p : (nodeToShortestPathsMap[destination])){
            int pathIndex = p.first;
            tuple<int,int> key = make_tuple(transactionId, pathIndex);
            
            if (transPathToAckState.count(key) > 0 && 
                    transPathToAckState[key].amtSent != transPathToAckState[key].amtReceived) {
                routerMsg* waterTimeOutMsg = generateTimeOutMessageForPath(
                    nodeToShortestPathsMap[destination][p.first].path, 
                    transactionId, destination);
                int nextNode = (waterTimeOutMsg->getRoute())[waterTimeOutMsg->getHopCount()+1];
                CanceledTrans ct = make_tuple(toutMsg->getTransactionId(), 
                        simTime(), -1, nextNode, destination);
                canceledTransactions.insert(ct);
                forwardMessage(waterTimeOutMsg);
            }
            else {
                transPathToAckState.erase(key);
            }
        }
        delete ttmsg;
    }
    else{
        // at the receiver
        CanceledTrans ct = make_tuple(toutMsg->getTransactionId(),simTime(),
                (ttmsg->getRoute())[ttmsg->getHopCount()-1], -1, toutMsg->getReceiver());
        canceledTransactions.insert(ct);
        ttmsg->decapsulate();
        delete toutMsg;
        delete ttmsg;
    }
}



/* handle Waterfilling probe Message
 * if it back at the sender, then update the bottleneck balances for this path 
 * otherwise, reverse and send back to sender
 */
void hostNodeWaterfilling::handleProbeMessage(routerMsg* ttmsg){
    probeMsg *pMsg = check_and_cast<probeMsg *>(ttmsg->getEncapsulatedPacket());
    if (simTime()> _simulationLength ){
       ttmsg->decapsulate();
       delete pMsg;
       delete ttmsg;
       return;
    }

    bool isReversed = pMsg->getIsReversed();
    int nextDest = ttmsg->getRoute()[ttmsg->getHopCount()+1];
    
    if (isReversed == true) { 
        int pathIdx = pMsg->getPathIndex();
        int destNode = pMsg->getReceiver();
        vector<double> pathBalances = pMsg->getPathBalances();
        double bottleneck = minVectorElemDouble(pathBalances);

        PathInfo* p = &(nodeToShortestPathsMap[destNode][pathIdx]);
        assert(p->path == pMsg->getPath());
        
        // update state for this path -time of probe, balance
        p->lastUpdated = simTime();
        p->bottleneck = bottleneck;
        p->pathBalances = pathBalances;
        p->isProbeOutstanding = false;

        // see if this is the path with the new max available balance
        double availBal = p->bottleneck - p->sumOfTransUnitsInFlight;
        if (availBal > nodeToDestInfo[destNode].highestBottleneckBalance) {
            nodeToDestInfo[destNode].highestBottleneckBalance = availBal;
            nodeToDestInfo[destNode].highestBottleneckPathIndex = pathIdx;
        }
        
        if (destNodeToNumTransPending[destNode] > 0){
            // service first transaction on path
            if (nodeToDestInfo[destNode].transWaitingToBeSent.size() > 0 && availBal > 0) {
                auto firstPosition = nodeToDestInfo[destNode].transWaitingToBeSent.begin();
                routerMsg *nextTrans = *firstPosition;
                nodeToDestInfo[destNode].transWaitingToBeSent.erase(firstPosition);
                handleTransactionMessageSpecialized(nextTrans);
            }
            
            //reset the probe message to send again
           nodeToShortestPathsMap[destNode][pathIdx].isProbeOutstanding = true;
           vector<int> route = ttmsg->getRoute();
           reverse(route.begin(), route.end());
           vector<double> tempPathBal = {};

           ttmsg->setRoute(route);
           ttmsg->setHopCount(0);
           pMsg->setIsReversed(false); 
           pMsg->setPathBalances(tempPathBal);
           forwardProbeMessage(ttmsg);
        }
        else{
           ttmsg->decapsulate();
           delete pMsg;
           delete ttmsg;
        }
    }
    else {
        //reverse and send message again
        pMsg->setIsReversed(true);
        ttmsg->setHopCount(0);
        vector<int> route = ttmsg->getRoute();
        reverse(route.begin(), route.end());
        ttmsg->setRoute(route);
        forwardProbeMessage(ttmsg);
    }
}

/* handler that clears additional state particular to waterfilling 
 * when a cancelled transaction is deemed no longer completeable
 * in particular it clears the state that tracks how much of a
 * transaction is still pending
 * calls the base class's handler after its own handler
 */
void hostNodeWaterfilling::handleClearStateMessage(routerMsg *ttmsg) {
    for ( auto it = canceledTransactions.begin(); it!= canceledTransactions.end(); it++){
        int transactionId = get<0>(*it);
        simtime_t msgArrivalTime = get<1>(*it);
        int prevNode = get<2>(*it);
        int nextNode = get<3>(*it);
        int destNode = get<4>(*it);
        
        if (simTime() > (msgArrivalTime + _maxTravelTime + 1)){
            for (auto p : nodeToShortestPathsMap[destNode]) {
                int pathIndex = p.first;
                tuple<int,int> key = make_tuple(transactionId, pathIndex);
                if (transPathToAckState.count(key) != 0) {
                    PathInfo *pathInfo = &(nodeToShortestPathsMap[destNode][pathIndex]);
                    pathInfo->sumOfTransUnitsInFlight -= 
                        (transPathToAckState[key].amtSent - transPathToAckState[key].amtReceived);
                    transPathToAckState.erase(key);
                    
                    double availBal = pathInfo->bottleneck - pathInfo->sumOfTransUnitsInFlight;
                    if (availBal > nodeToDestInfo[destNode].highestBottleneckBalance) {
                        nodeToDestInfo[destNode].highestBottleneckBalance = availBal;
                        nodeToDestInfo[destNode].highestBottleneckPathIndex = pathIndex;
                    }
                }
            }
        }
    }
    hostNodeBase::handleClearStateMessage(ttmsg);
}


/* handles to logic for ack messages in the presence of timeouts
 * in particular, removes the transaction from the cancelled txns
 * to mark that it has been received 
 * it uses the transAmtSent vs Received to detect if txn is complete
 * and therefore is different from the base class 
 */
void hostNodeWaterfilling::handleAckMessageTimeOut(routerMsg* ttmsg){
    ackMsg *aMsg = check_and_cast<ackMsg *>(ttmsg->getEncapsulatedPacket());
    int transactionId = aMsg->getTransactionId();

    if (aMsg->getIsSuccess()) {
        double totalAmtReceived = (transToAmtLeftToComplete[transactionId]).amtReceived +
            aMsg->getAmount();
        if (totalAmtReceived != transToAmtLeftToComplete[transactionId].amtSent) 
            return;
        auto iter = canceledTransactions.find(make_tuple(transactionId, 0, 0, 0, 0));
        if (iter != canceledTransactions.end()) {
            canceledTransactions.erase(iter);
        }
        successfulDoNotSendTimeOut.insert(aMsg->getTransactionId());
    }
}

/* specialized ack handler that does the routine if this is waterfilling 
 * algorithm. In particular, collects/updates stats for this path alone
 * NOTE: acks are on the reverse path relative to the original sender
 */
void hostNodeWaterfilling::handleAckMessageSpecialized(routerMsg* ttmsg) {
    ackMsg *aMsg = check_and_cast<ackMsg *>(ttmsg->getEncapsulatedPacket());
    int receiver = aMsg->getReceiver();
    int pathIndex = aMsg->getPathIndex();
    int transactionId = aMsg->getTransactionId();
    double largerTxnId = aMsg->getLargerTxnId();
    PathInfo *pathInfo = &(nodeToShortestPathsMap[receiver][pathIndex]);
    
    if (transToAmtLeftToComplete.count(transactionId) == 0){
        cout << "error, transaction " << transactionId 
          <<" htlc index:" << aMsg->getHtlcIndex() 
          << " acknowledged at time " << simTime() 
          << " wasn't written to transToAmtLeftToComplete" << endl;
    }
    else {
        // update stats if successful
        if (aMsg->getIsSuccess()) { 
            pathInfo->statRateCompleted += 1;
            (transToAmtLeftToComplete[transactionId]).amtReceived += aMsg->getAmount();
            if (aMsg->getTimeSent() >= _transStatStart 
                    && aMsg->getTimeSent() <= _transStatEnd) { 
                statAmtCompleted[receiver] += aMsg->getAmount();
            }

            if (transToAmtLeftToComplete[transactionId].amtReceived > 
                    transToAmtLeftToComplete[transactionId].amtSent - _epsilon) {
                SplitState* splitInfo = &(_numSplits[myIndex()][largerTxnId]);
                splitInfo->numReceived += 1;

                if (aMsg->getTimeSent() >= _transStatStart && 
                        aMsg->getTimeSent() <= _transStatEnd) {
                    if (splitInfo->numTotal == splitInfo->numReceived) {
                        statRateCompleted[receiver] += 1;
                        _transactionCompletionBySize[splitInfo->totalAmount] += 1;
                        double timeTaken = simTime().dbl() - splitInfo->firstAttemptTime;
                        statCompletionTimes[receiver] += timeTaken * 1000;
                        _txnAvgCompTimeBySize[splitInfo->totalAmount] += timeTaken * 1000;
                        recordTailCompletionTime(aMsg->getTimeSent(), splitInfo->totalAmount, timeTaken * 1000);
                    }
                }
                if (splitInfo->numTotal == splitInfo->numReceived) 
                    statNumCompleted[receiver] += 1;
                
                // erase transaction from map 
                // NOTE: still keeping it in the per path map (transPathToAckState)
                // to identify that timeout needn't be sent
                transToAmtLeftToComplete.erase(aMsg->getTransactionId());
                destNodeToNumTransPending[receiver] -= 1;
            }
        } 
        else {
            // make sure transaction isn't cancelled yet
            auto iter = canceledTransactions.find(make_tuple(transactionId, 0, 0, 0, 0));
            if (iter != canceledTransactions.end()) {
                if (aMsg->getTimeSent() >= _transStatStart && aMsg->getTimeSent() <= _transStatEnd)
                    statAmtFailed[receiver] += aMsg->getAmount();
            } 
            else {
                // requeue transaction
                routerMsg *duplicateTrans = generateDuplicateTransactionMessage(aMsg);
                pushIntoSenderQueue(&(nodeToDestInfo[receiver]), duplicateTrans);
            }
        }
       
        //increment transaction amount acked on a path, so that we know not to send timeouts 
        // if nothing is in excess on the path
        tuple<int,int> key = make_tuple(transactionId, pathIndex);
        transPathToAckState[key].amtReceived += aMsg->getAmount();
        pathInfo->sumOfTransUnitsInFlight -= aMsg->getAmount();

        // update highest bottleneck balance path
        double thisPathAvailBal = pathInfo->bottleneck - pathInfo->sumOfTransUnitsInFlight;
        if (thisPathAvailBal > nodeToDestInfo[receiver].highestBottleneckBalance) {
            nodeToDestInfo[receiver].highestBottleneckBalance = thisPathAvailBal;
            nodeToDestInfo[receiver].highestBottleneckPathIndex = pathIndex;
        }
    }
    hostNodeBase::handleAckMessage(ttmsg);
}

/* handler for the statistic message triggered every x seconds to also
 * output the wf stats in addition to the default
 */
void hostNodeWaterfilling::handleStatMessage(routerMsg* ttmsg){
    if (_signalsEnabled) {     
        // per destination statistics
        for (auto it = 0; it < _numHostNodes; it++){ 
            if (it != getIndex() && _destList[myIndex()].count(it) > 0) {
                if (nodeToShortestPathsMap.count(it) > 0) {
                    for (auto p: nodeToShortestPathsMap[it]){
                        int pathIndex = p.first;
                        PathInfo *pInfo = &(p.second);
                        emit(pInfo->bottleneckPerDestPerPathSignal, pInfo->bottleneck);
                        emit(pInfo->windowSignal, pInfo->bottleneck);
                        if (_smoothWaterfillingEnabled)
                            emit(pInfo->probabilityPerDestPerPathSignal, pInfo->probability);
                        emit(pInfo->sumOfTransUnitsInFlightSignal, 
                                pInfo->sumOfTransUnitsInFlight);
                    }
                }
                emit(numWaitingPerDestSignals[it], 
                    nodeToDestInfo[it].transWaitingToBeSent.size());
            }        
        }
    } 
    hostNodeBase::handleStatMessage(ttmsg);
}


/* initialize probes along the paths specified to the destination node
 * and set up all the state in the table that maintains bottleneck balance
 * information across all paths to all destinations
 * also responsible for initializing signals
 */
void hostNodeWaterfilling::initializeProbes(vector<vector<int>> kShortestPaths, int destNode){ 
    destNodeToLastMeasurementTime[destNode] = 0.0;

    for (int pathIdx = 0; pathIdx < kShortestPaths.size(); pathIdx++){
        PathInfo temp = {};
        nodeToShortestPathsMap[destNode][pathIdx] = temp;
        nodeToShortestPathsMap[destNode][pathIdx].path = kShortestPaths[pathIdx];
        nodeToShortestPathsMap[destNode][pathIdx].probability = 1.0 / kShortestPaths.size();

        //initialize signals
        if (_signalsEnabled) {
            simsignal_t signal;
            signal = registerSignalPerDestPath("rateCompleted", pathIdx, destNode);
            nodeToShortestPathsMap[destNode][pathIdx].rateCompletedPerDestPerPathSignal = signal;

            if (_smoothWaterfillingEnabled) {
                signal = registerSignalPerDestPath("probability", pathIdx, destNode);
                nodeToShortestPathsMap[destNode][pathIdx].probabilityPerDestPerPathSignal = signal;
            }

            signal = registerSignalPerDestPath("rateAttempted", pathIdx, destNode);
            nodeToShortestPathsMap[destNode][pathIdx].rateAttemptedPerDestPerPathSignal = signal;
    
            signal = registerSignalPerDestPath("sumOfTransUnitsInFlight", pathIdx, destNode);
            nodeToShortestPathsMap[destNode][pathIdx].sumOfTransUnitsInFlightSignal = signal;
    
            signal = registerSignalPerDestPath("window", pathIdx, destNode);
            nodeToShortestPathsMap[destNode][pathIdx].windowSignal = signal;
            
            signal = registerSignalPerDestPath("bottleneck", pathIdx, destNode);
            nodeToShortestPathsMap[destNode][pathIdx].bottleneckPerDestPerPathSignal = signal;
        }

        // generate a probe message on this path
        routerMsg * msg = generateProbeMessage(destNode, pathIdx, kShortestPaths[pathIdx]);
        nodeToShortestPathsMap[destNode][pathIdx].isProbeOutstanding = true;
        forwardProbeMessage(msg);
    }
}



/* restart waterfilling probes once they have been stopped to a particular destination
 * TODO: might end up leaving multiple probes in flight to some destinations, but that's okay 
 * for now.
 */
void hostNodeWaterfilling::restartProbes(int destNode) {
    for (auto p: nodeToShortestPathsMap[destNode] ){
        PathInfo pathInformation = p.second;
        if (nodeToShortestPathsMap[destNode][p.first].isProbeOutstanding == false) {
            nodeToShortestPathsMap[destNode][p.first].isProbeOutstanding = true;
            routerMsg * msg = generateProbeMessage(destNode, p.first, p.second.path);
            forwardProbeMessage(msg);
        }
    }
}


/* forwards probe messages for waterfilling alone that appends the current balance
 * to the list of balances
 */
void hostNodeWaterfilling::forwardProbeMessage(routerMsg *msg){
    // Increment hop count.
    msg->setHopCount(msg->getHopCount()+1);
    //use hopCount to find next destination
    int nextDest = msg->getRoute()[msg->getHopCount()];

    probeMsg *pMsg = check_and_cast<probeMsg *>(msg->getEncapsulatedPacket());
    if (pMsg->getIsReversed() == false && !_rebalancingEnabled){
        vector<double> *pathBalances = & ( pMsg->getPathBalances());
        (*pathBalances).push_back(nodeToPaymentChannel[nextDest].balanceEWMA);
    }

   if (_loggingEnabled) cout << "forwarding " << msg->getMessageType() << " at " 
       << simTime() << endl;
   send(msg, nodeToPaymentChannel[nextDest].gate);
}

/* simplified waterfilling logic that assumes transaction is already split and sends it fully only on the 
 * path with maximum bottleneck - inflight balance */
void hostNodeWaterfilling::attemptTransactionOnBestPath(routerMsg * ttmsg, bool firstAttempt){
    transactionMsg *transMsg = check_and_cast<transactionMsg *>(ttmsg->getEncapsulatedPacket());
    int destNode = transMsg->getReceiver();
    double remainingAmt = transMsg->getAmount();
    transMsg->setIsAttempted(true);
    
    // fill up priority queue of balances
    double highestBal = -1;
    int highestBalIdx = -1;
    for (auto iter: nodeToShortestPathsMap[destNode] ){
        int key = iter.first;
        double bottleneck = (iter.second).bottleneck;
        double inflight = (iter.second).sumOfTransUnitsInFlight;
        double availBal = bottleneck - inflight;

        if (availBal >= highestBal) {
            highestBal = availBal;
            highestBalIdx = key;
        }
    }

    // accounting
    SplitState* splitInfo = &(_numSplits[myIndex()][transMsg->getLargerTxnId()]);
    bool firstLargerAttempt = (splitInfo->firstAttemptTime == -1);
    if (splitInfo->firstAttemptTime == -1) {
        splitInfo->firstAttemptTime = simTime().dbl();
    }
    if (transMsg->getTimeSent() >= _transStatStart && 
            transMsg->getTimeSent() <= _transStatEnd) {
        if (firstAttempt && firstLargerAttempt) { 
            statRateAttempted[destNode] = statRateAttempted[destNode] + 1;
        }
        if (firstAttempt) {
            statAmtAttempted[destNode] += transMsg->getAmount();
        }
    }

    if (highestBal <= 0) 
        return;

    // update state and send this out on the path with higest bal
    tuple<int,int> key = make_tuple(transMsg->getTransactionId(), highestBalIdx);
            
    //update the data structure keeping track of how much sent and received on each path
    if (transPathToAckState.count(key) == 0) {
        AckState temp = {};
        temp.amtSent = remainingAmt;
        temp.amtReceived = 0;
        transPathToAckState[key] = temp;
    }
    else {
        transPathToAckState[key].amtSent =  transPathToAckState[key].amtSent + remainingAmt;
    }
            
    PathInfo *pathInfo = &(nodeToShortestPathsMap[destNode][highestBalIdx]);
    routerMsg* waterMsg = generateTransactionMessageForPath(remainingAmt, 
         pathInfo->path, highestBalIdx, transMsg);
    
    // increment numAttempted per path
    pathInfo->statRateAttempted += 1;
    handleTransactionMessage(waterMsg, true/*revisit*/);
    
    // incrementInFlight balance and update highest bal index
    pathInfo->sumOfTransUnitsInFlight += remainingAmt;
    nodeToDestInfo[destNode].highestBottleneckBalance -= remainingAmt;
    transMsg->setAmount(0);
}



/* core waterfilling logic in deciding how to split a transaction across paths
 * based on the bottleneck balances on each of those paths
 * For now, in hte absence of splitting, transaction is sent in entirety either
 * on the path with highest bottleneck balance or the paths are sampled with a
 * probabilities based on the bottleneck balances in the smooth waterfilling 
 * case
 */
void hostNodeWaterfilling::splitTransactionForWaterfilling(routerMsg * ttmsg, bool firstAttempt){
    transactionMsg *transMsg = check_and_cast<transactionMsg *>(ttmsg->getEncapsulatedPacket());
    int destNode = transMsg->getReceiver();
    double remainingAmt = transMsg->getAmount();
    transMsg->setIsAttempted(true);
    
    // if you want to choose at random between paths
    bool randomChoice = false; 
    
    unordered_map<int, double> pathMap = {}; //key is pathIdx, double is amt
    vector<double> bottleneckList;
   
    // fill up priority queue of balances
    priority_queue<pair<double,int>> pq;
    if (_loggingEnabled) cout << "bottleneck for node " <<  getIndex();
    for (auto iter: nodeToShortestPathsMap[destNode] ){
        int key = iter.first;
        double bottleneck = (iter.second).bottleneck;
        double inflight = (iter.second).sumOfTransUnitsInFlight;
        bottleneckList.push_back(bottleneck);
        if (_loggingEnabled) cout << bottleneck - inflight << " (" << key  << "," 
            << iter.second.lastUpdated<<"), ";
        
        pq.push(make_pair(bottleneck - inflight, key)); 
    }
    if (_loggingEnabled) cout << endl;

    double highestBal;
    double secHighestBal;
    double diffToSend;
    double amtToSend;
    int highestBalIdx;
    int numPaths = nodeToShortestPathsMap[destNode].size();

    if (randomChoice) {
        vector<double> probabilities (numPaths, 1.0/numPaths);
        int pathIndex = sampleFromDistribution(probabilities);
        pathMap[pathIndex] = pathMap[pathIndex] + remainingAmt;
        remainingAmt = 0;
    } 
    else if (_smoothWaterfillingEnabled) {
        highestBal = get<0>(pq.top());
        if (highestBal >= 0) {
            int pathIndex = updatePathProbabilities(bottleneckList, destNode);
            pathMap[pathIndex] = pathMap[pathIndex] + remainingAmt;
            remainingAmt = 0;
        }
    }
    else {
        // normal waterfilling - start filling with the path
        // with highest bottleneck balance and fill it till you get to 
        // the next path and so on
        if (pq.size() == 0) {
            cout << "PATHS NOT FOUND to " << destNode << "WHEN IT SHOULD HAVE BEEN";
            throw std::exception();
        }
        
        while(pq.size() > 0 && remainingAmt >= SMALLEST_INDIVISIBLE_UNIT){
            highestBal = get<0>(pq.top());
            if (highestBal <= 0) 
                break;
            
            highestBalIdx = get<1>(pq.top());
            pq.pop();

            if (pq.size() == 0) {
                secHighestBal = 0;
            }
            else {
                secHighestBal = get<0>(pq.top());
            }
            diffToSend = highestBal - secHighestBal;
            pathMap[highestBalIdx] = 0.0;
            
            double amtAddedInThisRound = 0.0;
            double maxForThisRound = pathMap.size() * diffToSend;
            while (remainingAmt > _epsilon && amtAddedInThisRound < maxForThisRound) {
                for (auto p: pathMap){
                    pathMap[p.first] += SMALLEST_INDIVISIBLE_UNIT;
                    remainingAmt = remainingAmt - SMALLEST_INDIVISIBLE_UNIT;
                    amtAddedInThisRound += SMALLEST_INDIVISIBLE_UNIT;
                    if (remainingAmt < _epsilon || amtAddedInThisRound >= maxForThisRound) {
                        break;
                    }
                }
            }
        }
   
        // send all of the remaining amount beyond the indivisible unit on one path
        // the highest bal path as long as it has non zero balance
        if (remainingAmt > _epsilon  && pq.size()>0 ) {
            highestBal = get<0>(pq.top());
            highestBalIdx = get<1>(pq.top());
               
            if (highestBal > 0) {
                pathMap[highestBalIdx] = pathMap[highestBalIdx] + remainingAmt;
                remainingAmt = 0;
            }
        }

        if (remainingAmt < _epsilon) 
            remainingAmt = 0;
    }

    // accounting
    SplitState* splitInfo = &(_numSplits[myIndex()][transMsg->getLargerTxnId()]);
    bool firstLargerAttempt = (splitInfo->firstAttemptTime == -1);
    if (splitInfo->firstAttemptTime == -1) {
        splitInfo->firstAttemptTime = simTime().dbl();
    }
    if (transMsg->getTimeSent() >= _transStatStart && 
            transMsg->getTimeSent() <= _transStatEnd) {
        if (firstAttempt && firstLargerAttempt) { 
            statRateAttempted[destNode] = statRateAttempted[destNode] + 1;
        }
        if (firstAttempt) {
            statAmtAttempted[destNode] += transMsg->getAmount();
        }
    }

    if (remainingAmt > transMsg->getAmount()) {
        cout << "remaining amount magically became higher than original amount" << endl;
        throw std::exception();
    }
    transMsg->setAmount(remainingAmt);
    
    for (auto p: pathMap){
        int pathIndex = p.first;
        double amtOnPath = p.second;
        if (amtOnPath > 0){
            tuple<int,int> key = make_tuple(transMsg->getTransactionId(), pathIndex);
            //update the data structure keeping track of how much sent and received on each path
            if (transPathToAckState.count(key) == 0) {
                AckState temp = {};
                temp.amtSent = amtOnPath;
                temp.amtReceived = 0;
                transPathToAckState[key] = temp;
            }
            else {
                transPathToAckState[key].amtSent =  transPathToAckState[key].amtSent + amtOnPath;
            }
            PathInfo *pathInfo = &(nodeToShortestPathsMap[destNode][pathIndex]);
            routerMsg* waterMsg = generateTransactionMessageForPath(amtOnPath, 
                 pathInfo->path, pathIndex, transMsg);
            pathInfo->statRateAttempted += 1;
            pathInfo->sumOfTransUnitsInFlight += p.second;

            handleTransactionMessage(waterMsg, true/*revisit*/);
        }
    }
}


/* computes the updated path probabilities based on the current state of 
 * bottleneck link balances and returns the next path index to send the transaction 
 * on in accordance to the latest rate
 * acts as a helper for smooth waterfilling
 */
int hostNodeWaterfilling::updatePathProbabilities(vector<double> bottleneckBalances, int destNode) {
    double averageBottleneck = accumulate(bottleneckBalances.begin(), 
            bottleneckBalances.end(), 0.0)/bottleneckBalances.size(); 
                
    double timeSinceLastTxn = simTime().dbl() - destNodeToLastMeasurementTime[destNode];
    destNodeToLastMeasurementTime[destNode] = simTime().dbl();

    // compute new porbabailities based on adjustment factor and expression
    vector<double> probabilities;
    int i = 0;
    for (auto b : bottleneckBalances) {
        probabilities.push_back(nodeToShortestPathsMap[destNode][i].probability + 
            (1 - exp(-1 * timeSinceLastTxn/_Tau))*(b - averageBottleneck)/_Normalizer);
        probabilities[i] = max(0.0, probabilities[i]);
        i++;
    }
    double sumProbabilities = accumulate(probabilities.begin(), probabilities.end(), 0.0); 
    
    // normalize them to 1 and update the stored probabilities
    for (i = 0; i < probabilities.size(); i++) {
        probabilities[i] /= sumProbabilities;
        nodeToShortestPathsMap[destNode][i].probability = probabilities[i];
    }
    return sampleFromDistribution(probabilities);
}

SUMMARY ##################################################

# Developer Manual: Host Node Implementation in Spider Project

This manual provides a detailed and comprehensive understanding of the implementation of host nodes in the Spider project. It focuses on the `.cc` (implementation) files, which contain the core logic for transaction processing, routing algorithms, signal handling, and other functionalities. Developers reading this manual will gain an in-depth understanding of how these components work together to achieve the desired behavior.

---

## 1. **Overview of Host Node Implementation**

The `.cc` files define the behavior of host nodes, which are responsible for initiating transactions, managing payment channels, and interacting with routers. These files implement the methods declared in the corresponding `.h` header files. The key responsibilities of host nodes include:

- **Transaction Management**: Generating, sending, and tracking transactions.
- **Routing Logic**: Implementing various routing algorithms (e.g., Celer, DCTCP, Landmark Routing, etc.).
- **Signal Handling**: Collecting statistics and emitting signals for monitoring performance.
- **Error Handling**: Managing timeouts, retries, and failures during transaction processing.

---

## 2. **Key Components and Methods**

### 2.1 **Initialization Functions**
These functions set up the initial state of the host node, including payment channels, paths, and global parameters.

#### `initialize()`
- **Purpose**: Initializes global parameters, payment channels, and signals for statistics collection.
- **Details**:
  - Reads configuration parameters such as `_simulationLength`, `_statRate`, `_timeoutEnabled`, etc.
  - Sets up payment channels using `initializeMyChannels()`.
  - Registers signals for monitoring metrics like demand estimates, wallet balances, and transaction success rates.
  - Example:
    ```cpp
    void hostNodeBase::initialize() {
        _simulationLength = par("simulationLength");
        _statRate = par("statRate");
        initializeMyChannels();
        registerSignals();
    }
    ```

#### `initializePathInfo()`
- **Purpose**: Initializes path-specific data structures for routing algorithms.
- **Details**:
  - Stores K-shortest paths for each destination.
  - Sets up path-specific signals for monitoring metrics like window size, bottleneck capacity, and acknowledgment rates.
  - Example:
    ```cpp
    void hostNodeDCTCP::initializePathInfo(vector<vector<int>> kShortestPaths, int destNode) {
        for (int pathIdx = 0; pathIdx < kShortestPaths.size(); pathIdx++) {
            PathInfo temp = {};
            temp.path = kShortestPaths[pathIdx];
            temp.window = _minDCTCPWindow;
            nodeToShortestPathsMap[destNode][pathIdx] = temp;
        }
    }
    ```

---

### 2.2 **Transaction Generation and Handling**

#### `generateNextTransaction()`
- **Purpose**: Generates the next transaction based on the workload file.
- **Details**:
  - Retrieves the next transaction from `_transUnitList`.
  - Creates a transaction message (`routerMsg`) and schedules it for transmission.
  - Example:
    ```cpp
    void hostNodeBase::generateNextTransaction() {
        if (_transUnitList[myIndex()].empty()) return;
        TransUnit j = _transUnitList[myIndex()].top();
        routerMsg *msg = generateTransactionMessage(j);
        scheduleAt(j.timeSent, msg);
    }
    ```

#### `handleTransactionMessageSpecialized()`
- **Purpose**: Processes incoming transaction messages specific to the routing algorithm.
- **Details**:
  - Validates the transaction and checks if it can be processed.
  - Updates payment channel balances and emits relevant signals.
  - Example:
    ```cpp
    void hostNodeCeler::handleTransactionMessageSpecialized(routerMsg *msg) {
        pushIntoPerDestQueue(msg, msg->getReceiver());
        celerProcessTransactions();
    }
    ```

---

### 2.3 **Routing Algorithms**

#### **Celer Protocol**
- **Key Methods**:
  - `calculateCPI()`: Computes the Cost Performance Index (CPI) for selecting optimal paths.
  - `findKStar()`: Determines the best path index (`k*`) for forwarding transactions.
  - `celerProcessTransactions()`: Processes transactions based on CPI and updates queues.

#### **DCTCP Protocol**
- **Key Methods**:
  - `getMaxWindowSize()`: Calculates the maximum allowable window size for congestion control.
  - `forwardTransactionMessage()`: Forwards transactions while considering queue thresholds.
  - `handleAckMessageSpecialized()`: Updates window sizes and acknowledgment rates based on feedback.

#### **Landmark Routing**
- **Key Methods**:
  - `generateProbeMessage()`: Creates probe messages to measure path conditions.
  - `randomSplit()`: Divides transactions across multiple paths based on bottleneck capacities.
  - `initializePathInfoLandmarkRouting()`: Configures paths using landmark routing strategies.

#### **Price Scheme**
- **Key Methods**:
  - `computeProjection()`: Adjusts rates to ensure feasibility.
  - `handleTriggerRateDecreaseMessage()`: Manages rate decrease triggers.
  - `handleTriggerTransactionSendMessage()`: Sends transactions based on updated rates.

---

### 2.4 **Signal Handling and Statistics Collection**

#### `registerSignalPerChannel()`
- **Purpose**: Registers signals for monitoring payment channel metrics.
- **Details**:
  - Creates unique signal names for each channel and registers them with the simulation environment.
  - Example:
    ```cpp
    simsignal_t hostNodeBase::registerSignalPerChannel(string signalStart, int id) {
        char signalName[64];
        sprintf(signalName, "%s(host %d)", signalStart.c_str(), id);
        simsignal_t signal = registerSignal(signalName);
        getEnvir()->addResultRecorders(this, signal, signalName, statisticTemplate);
        return signal;
    }
    ```

#### `handleStatMessage()`
- **Purpose**: Handles periodic statistic messages to update metrics.
- **Details**:
  - Emits signals for per-path and per-destination statistics.
  - Example:
    ```cpp
    void hostNodeWaterfilling::handleStatMessage(routerMsg *ttmsg) {
        if (_signalsEnabled) {
            for (auto &p : nodeToShortestPathsMap) {
                emit(p.second.bottleneckSignal, p.second.bottleneck);
            }
        }
    }
    ```

---

### 2.5 **Error Handling and Timeouts**

#### `handleTransactionMessageTimeOut()`
- **Purpose**: Handles timeouts for transactions that fail to complete.
- **Details**:
  - Removes timed-out transactions from the queue.
  - Logs failure statistics and retries if enabled.
  - Example:
    ```cpp
    bool hostNodeBase::handleTransactionMessageTimeOut(routerMsg *msg) {
        if (_timeoutEnabled) {
            // Remove transaction from queue and log failure
            return true;
        }
        return false;
    }
    ```

#### `handleAckMessage()`
- **Purpose**: Processes acknowledgment messages for completed transactions.
- **Details**:
  - Updates payment channel balances and acknowledgment states.
  - Triggers update messages for subsequent nodes on the path.
  - Example:
    ```cpp
    void hostNodeBase::handleAckMessage(routerMsg *ttmsg) {
        ackMsg *aMsg = check_and_cast<ackMsg *>(ttmsg->getEncapsulatedPacket());
        updatePaymentChannelBalances(aMsg);
        triggerUpdateMessage(aMsg);
    }
    ```

---

## 3. **Advanced Features**

### 3.1 **Rebalancing Operations**
- **Purpose**: Maintains optimal channel states by rebalancing funds.
- **Details**:
  - Replenishes depleted channels by transferring funds from underutilized ones.
  - Example:
    ```cpp
    void hostNodeBase::performRebalancing() {
        for (auto &channel : nodeToPaymentChannel) {
            if (channel.balance < threshold) {
                replenishChannel(channel.id);
            }
        }
    }
    ```

### 3.2 **Dynamic Path Changes**
- **Purpose**: Adapts to changing network conditions by updating paths.
- **Details**:
  - Monitors path performance and switches to better paths when necessary.
  - Example:
    ```cpp
    void hostNodeBase::updatePaths(int destNode) {
        vector<vector<int>> newPaths = getKShortestRoutes(myIndex(), destNode, _maxPathsToConsider);
        initializePathInfo(newPaths, destNode);
    }
    ```

---

## 4. **Conclusion**

The `.cc` files for host nodes in the Spider project encapsulate the core logic for transaction processing, routing, and monitoring. By understanding the initialization, transaction handling, routing algorithms, and signal management implemented in these files, developers can effectively contribute to and maintain the project. This manual provides a solid foundation for navigating and extending the functionality of host nodes in the Spider project.