/home/kush/Desktop/spiderproj/spider_omnet/hostInitialize.h

#include "routerNodeBase.h"


//initialization functions
bool probesRecent(unordered_map<int, PathInfo> probes);
void generateChannelsBalancesMap(string);
void setNumNodes(string);
void updateMaxTravelTime(vector<int> route);
void updateCannonicalRTT(vector<int> route);

//get path functions
vector<int> getRoute(int sender, int receiver);
vector<vector<int>> getKShortestRoutes(int sender, int receiver, int k);
vector<vector<int>> getKShortestRoutesLandmarkRouting(int sender, int receiver, int k);
vector<vector<int>> getKPaths(int sender, int receiver, int k);
tuple<int, vector<int>> getNextPath(int sender, int receiver, int k);
void initializePathMaps(string filename);

//search functions
vector<int> breadthFirstSearch(int sender, int receiver);
vector<int> breadthFirstSearchByGraph(int sender, int receiver, unordered_map<int, set<int>> graph);
vector<int> dijkstra(int src, int dest);

void generateTransUnitList(string);
vector<string> split(string str, char delimiter);
vector<int> getRoute(int sender, int receiver);
double bottleneckCapacityOnPath(vector<int> thisPath);

// scheudling functions for the queue 
bool sortPriorityThenAmtFunction(const tuple<int,double, routerMsg*, Id, simtime_t> &a,
      const tuple<int,double, routerMsg*, Id, simtime_t> &b);
bool sortFIFO(const tuple<int,double, routerMsg*, Id, simtime_t> &a,
      const tuple<int,double, routerMsg*, Id, simtime_t> &b);
bool sortLIFO(const tuple<int,double, routerMsg*, Id, simtime_t> &a,
      const tuple<int,double, routerMsg*, Id, simtime_t> &b);
bool sortSPF(const tuple<int,double, routerMsg*, Id, simtime_t> &a,
      const tuple<int,double, routerMsg*, Id, simtime_t> &b);
bool sortEDF(const tuple<int,double, routerMsg*, Id, simtime_t> &a,
      const tuple<int,double, routerMsg*, Id, simtime_t> &b);

unordered_map<int, vector<pair<int,int>>> removeRoute( unordered_map<int, vector<pair<int,int>>> channels, vector<int> route);
vector<int> dijkstraInputGraph(int src,  int dest, unordered_map<int, vector<pair<int,int>>> channels);
double minVectorElemDouble(vector<double> v);
double maxDouble(double x, double y);

//logging functions
void printChannels();
void printVector(vector<int> v);
void printVectorReverse(vector<int> v);
void printChannels(unordered_map<int, vector<int>> channels);
int minInt(int x, int y);

double getTotalAmount(unordered_map<Id, double> v);
double getTotalAmount(vector<tuple<int, double, routerMsg*, Id, simtime_t >> queue);

/home/kush/Desktop/spiderproj/spider_omnet/hostNodeBase.h

#ifndef ROUTERNODE_H
#define ROUTERNODE_H

#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include <vector>
#include "routerMsg_m.h"
#include "transactionMsg_m.h"
#include "ackMsg_m.h"
#include "updateMsg_m.h"
#include "timeOutMsg_m.h"
#include <iostream>
#include <algorithm>
#include <string>
#include <sstream>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <fstream>
#include "global.h"
#include "hostInitialize.h"

using namespace std;
using namespace omnetpp;

#ifndef HOST_NODE_BASE_H
#define HOST_NODE_BASE_H

#include "global.h"
#include <memory>

class HostNodeBase : public cSimpleModule {
protected:
    int index;  // node identifier
    std::shared_ptr<WalletSystem> wallet_system;  // Wallet management system
    unordered_map<int, PaymentChannel> nodeToPaymentChannel;
    // ... (keep other existing members)

    // New wallet-related signals for statistics
    simsignal_t walletBalanceSignal;
    simsignal_t walletStakedAmountSignal;
    simsignal_t walletLiquiditySignal;

public:
    // Constructor and destructor
    HostNodeBase() : wallet_system(std::make_shared<WalletSystem>()) {}
    virtual ~HostNodeBase() {}

    // Wallet management functions
    bool initializeWallet(double initial_balance) {
        if (!wallet_system) return false;
        wallet_system->initializeWallets(nodeToPaymentChannel);
        
        // Register wallet-related signals
        walletBalanceSignal = registerSignal("walletBalance");
        walletStakedAmountSignal = registerSignal("walletStaked");
        walletLiquiditySignal = registerSignal("walletLiquidity");
        
        // Emit initial values
        emit(walletBalanceSignal, wallet_system->getWalletBalance(index));
        emit(walletStakedAmountSignal, wallet_system->getTotalStaked(index));
        emit(walletLiquiditySignal, wallet_system->getAvailableLiquidity(index));
        
        return true;
    }

    // Transaction validation
    bool canProcessTransaction(double amount) const {
        return wallet_system && wallet_system->canProcessTransaction(index, amount);
    }

    // Process outgoing payment
    bool processOutgoingPayment(int to_node, double amount) {
        if (!wallet_system) return false;
        bool success = wallet_system->processTransaction(index, to_node, amount);
        if (success) {
            // Update statistics
            emit(walletBalanceSignal, wallet_system->getWalletBalance(index));
            emit(walletLiquiditySignal, wallet_system->getAvailableLiquidity(index));
        }
        return success;
    }

    // Process incoming payment
    bool processIncomingPayment(int from_node, double amount) {
        if (!wallet_system) return false;
        bool success = wallet_system->processTransaction(from_node, index, amount);
        if (success) {
            // Update statistics
            emit(walletBalanceSignal, wallet_system->getWalletBalance(index));
            emit(walletLiquiditySignal, wallet_system->getAvailableLiquidity(index));
        }
        return success;
    }

    // Modified transaction handling to include wallet checks
    virtual void handleTransactionMessage(routerMsg *msg, bool revisit=false) {
        if (!msg) return;
        
        transactionMsg* trans_msg = check_and_cast<transactionMsg*>(msg);
        double amount = trans_msg->getAmount();
        
        // Check if we have enough balance to process
        if (!canProcessTransaction(amount)) {
            // Handle insufficient funds
            routerMsg* ack = generateAckMessage(msg, false);
            forwardMessage(ack);
            delete msg;
            return;
        }
        
        // Process the transaction
        if (processOutgoingPayment(trans_msg->getReceiver(), amount)) {
            handleTransactionMessageSpecialized(msg);
        } else {
            // Handle transaction failure
            routerMsg* ack = generateAckMessage(msg, false);
            forwardMessage(ack);
            delete msg;
        }
    }

    // Modified ACK handling to update wallet on successful transactions
    virtual void handleAckMessage(routerMsg *msg) {
        if (!msg) return;
        
        ackMsg* ack_msg = check_and_cast<ackMsg*>(msg);
        if (ack_msg->getIsSuccess()) {
            // If successful, update wallet statistics
            emit(walletBalanceSignal, wallet_system->getWalletBalance(index));
            emit(walletLiquiditySignal, wallet_system->getAvailableLiquidity(index));
        }
        
        handleAckMessageSpecialized(msg);
    }

    // Getters for wallet information
    double getWalletBalance() const {
        return wallet_system ? wallet_system->getWalletBalance(index) : 0.0;
    }

    double getAvailableLiquidity() const {
        return wallet_system ? wallet_system->getAvailableLiquidity(index) : 0.0;
    }

    double getTotalStaked() const {
        return wallet_system ? wallet_system->getTotalStaked(index) : 0.0;
    }

    // ... (keep other existing methods)
};

#endif // HOST_NODE_BASE_H



class hostNodeBase : public cSimpleModule {
    protected:
        int index; // node identifier
        unordered_map<int, PaymentChannel> nodeToPaymentChannel;
        unordered_map<int, DestInfo> nodeToDestInfo; //one structure per destination;
        unordered_set<int> destList; // list of destinations with non zero demand
             
        // statistic collection related variables
        unordered_map<int, int> statNumFailed = {};
        unordered_map<int, double> statAmtFailed = {};
        unordered_map<int, int> statRateFailed = {};
        unordered_map<int, int> statNumCompleted = {};
        unordered_map<int, double> statAmtCompleted = {};
        unordered_map<int, int> statNumArrived = {};
        unordered_map<int, int> statRateCompleted = {};
        unordered_map<int, double> statAmtArrived = {};
        unordered_map<int, double> statAmtAttempted = {};
        unordered_map<int, int> statRateAttempted = {};
        unordered_map<int, int> statNumTimedOut = {};
        unordered_map<int, int> statNumTimedOutAtSender = {};
        unordered_map<int, int> statRateArrived = {};
        unordered_map<int, double> statProbabilities = {};
        unordered_map<int, double> statCompletionTimes = {};
        priority_queue<double, vector<double>, greater<double>> statNumTries;
        double maxPercentileHeapSize;
        int numCleared = 0;

        //store shortest paths 
        unordered_map<int, vector<int>> destNodeToPath = {};
        unordered_map<int, unordered_map<int, PathInfo>> nodeToShortestPathsMap = {};  

        // number of transactions pending to a given destination
        unordered_map<int, int> destNodeToNumTransPending = {};

        // signals for recording statistics above
        simsignal_t completionTimeSignal;
        simsignal_t numClearedSignal;

        unordered_map<int, simsignal_t> rateCompletedPerDestSignals = {};
        unordered_map<int, simsignal_t> rateAttemptedPerDestSignals = {};
        unordered_map<int, simsignal_t> rateArrivedPerDestSignals = {};
        unordered_map<int, simsignal_t> numCompletedPerDestSignals = {};
        unordered_map<int, simsignal_t> numArrivedPerDestSignals = {};
        unordered_map<int, simsignal_t> numTimedOutPerDestSignals = {};
        unordered_map<int, simsignal_t> numPendingPerDestSignals = {};       
        unordered_map<int, simsignal_t> rateFailedPerDestSignals = {};
        unordered_map<int, simsignal_t> numFailedPerDestSignals = {};
        unordered_map<int, simsignal_t> fracSuccessfulPerDestSignals = {};

        //set of transaction units WITH timeouts, that we already received acks for
        set<int> successfulDoNotSendTimeOut = {};   
        unordered_set<CanceledTrans, hashCanceledTrans, equalCanceledTrans> canceledTransactions = {};

        /****** state for splitting transactions into many HTLCs ********/
        // structure that tracks how much of a transaction has been completed 
        // this tracks it for every path, key is (transactionId, routeIndex)
        unordered_map<tuple<int,int>,AckState, hashId> transPathToAckState = {}; 
        // this is per transaction across all paths
        unordered_map<int, AckState> transToAmtLeftToComplete = {};
        //allows us to calculate the htlcIndex number
        unordered_map<int, int> transactionIdToNumHtlc = {};       

        // structure to enable rebalancing - keeps track of how much each sender has sent to you, so you can refund 
        // accordingly and how muh you've sent to each receiver
        unordered_map<int, double> senderToAmtRefundable = {};  
        unordered_map<int, double> receiverToAmtRefunded = {};  
    
    protected:

    public:
        //if host node, return getIndex(), if routerNode, return getIndex()+numHostNodes
        virtual int myIndex();
        virtual void setIndex(int index);  


        //helper functions common to all algorithms
        virtual bool printNodeToPaymentChannel();
        virtual int sampleFromDistribution(vector<double> probabilities);
        virtual double getTotalAmount(int x);
        virtual double getTotalAmountIncomingInflight(int x);
        virtual double getTotalAmountOutgoingInflight(int x);
        virtual void setPaymentChannelBalanceByNode(int node, double balance);

        virtual void pushIntoSenderQueue(DestInfo* destInfo, routerMsg* msg);
        virtual void deleteTransaction(routerMsg* msg);
        virtual void generateNextTransaction();
        virtual simsignal_t registerSignalPerDestPath(string signalStart, 
              int pathIdx, int destNode);
        virtual simsignal_t registerSignalPerChannel(string signalStart, int id);
        virtual simsignal_t registerSignalPerDest(string signalStart, int destNode, 
                string suffix);
        virtual simsignal_t registerSignalPerChannelPerDest(string signalStart,
              int pathIdx, int destNode);
        virtual void recordTailCompletionTime(simtime_t timeSent, double amount, double completionTime);

        // generators for the standard messages
        virtual routerMsg* generateTransactionMessageForPath(double amt, 
                vector<int> path, int pathIndex, transactionMsg* transMsg);
        virtual routerMsg *generateTransactionMessage(TransUnit TransUnit);
        virtual routerMsg *generateDuplicateTransactionMessage(ackMsg* aMsg);
        virtual routerMsg *generateAckMessage(routerMsg *msg, bool isSuccess = true);
        virtual routerMsg *generateUpdateMessage(int transId, 
                int receiver, double amount, int htlcIndex);        
        virtual routerMsg *generateStatMessage();
        virtual routerMsg *generateComputeMinBalanceMessage();
        virtual routerMsg *generateClearStateMessage();
        virtual routerMsg* generateTimeOutMessageForPath(vector<int> path, int transactionId, int receiver);
        virtual routerMsg *generateTimeOutMessage(routerMsg *transMsg);
        virtual routerMsg *generateTriggerRebalancingMessage();
        virtual routerMsg *generateAddFundsMessage(map<int,double> fundsToBeAdded);
      

        // message handlers
        virtual void handleMessage(cMessage *msg) override;
        virtual void handleTransactionMessage(routerMsg *msg, bool revisit=false);
        virtual void handleTransactionMessageSpecialized(routerMsg *msg);
        //returns true if message was deleted
        virtual bool handleTransactionMessageTimeOut(routerMsg *msg); 
        virtual void handleTimeOutMessage(routerMsg *msg);
        virtual void handleAckMessageSpecialized(routerMsg *msg);
        virtual void handleTriggerRebalancingMessage(routerMsg* ttmsg);
        virtual void handleComputeMinAvailableBalanceMessage(routerMsg* ttmsg);
        virtual void handleAddFundsMessage(routerMsg* ttmsg);

        
        virtual void handleAckMessage(routerMsg *msg);
        virtual void handleAckMessageTimeOut(routerMsg *msg);
        virtual void handleUpdateMessage(routerMsg *msg);
        virtual void handleStatMessage(routerMsg *msg);
        virtual void handleClearStateMessage(routerMsg *msg);
        
        // message forwarders
        virtual int forwardTransactionMessage(routerMsg *msg, int dest, simtime_t arrivalTime);
        virtual void forwardMessage(routerMsg *msg);

        // core simulator functions
        virtual void initialize() override;
        virtual void finish() override;
        virtual bool processTransUnits(int dest, 
                vector<tuple<int, double , routerMsg *, Id, simtime_t>>& q);
        virtual void deleteMessagesInQueues();
};

#endif

/home/kush/Desktop/spiderproj/spider_omnet/hostNodeCeler.h

#ifndef ROUTERNODE_CELER_H
#define ROUTERNODE_CELER_H

#include "hostNodeBase.h"

using namespace std;
using namespace omnetpp;

class hostNodeCeler : public hostNodeBase {

    private:
        unordered_map<int, DestNodeStruct> nodeToDestNodeStruct;
        unordered_map<int, int> transToNextHop; 

    protected:
        virtual void initialize() override; //initialize global debtQueues
        virtual void finish() override; // remove messages from the dest queues

        // message handlers
        virtual void handleStatMessage(routerMsg *msg) override;
        virtual void handleClearStateMessage(routerMsg *msg) override;
        virtual void handleTimeOutMessage(routerMsg *msg) override;
        virtual void handleTransactionMessageSpecialized(routerMsg *msg) override; 
        virtual void handleAckMessageSpecialized(routerMsg *msg) override; 

        // helper methods
        virtual void pushIntoPerDestQueue(routerMsg* rMsg, int destNode);
        virtual void celerProcessTransactions(int endLinkNode = -1);
        virtual double calculateCPI(int destNode, int neighborNode);
        virtual int findKStar(int endLinkNode, unordered_set<int> exclude); // find k* for each paymen channel
        virtual int forwardTransactionMessage(routerMsg *msg, int dest, simtime_t arrivalTime) override; 
        virtual void setPaymentChannelBalanceByNode(int node, double balance) override;


};
#endif

/home/kush/Desktop/spiderproj/spider_omnet/hostNodeDCTCP.h

#ifndef ROUTERNODE_DCTCP_H
#define ROUTERNODE_DCTCP_H

#include "priceQueryMsg_m.h"
#include "priceUpdateMsg_m.h"
#include "hostNodeBase.h"

using namespace std;
using namespace omnetpp;

class hostNodeDCTCP : public hostNodeBase {
    private:
        // DCTCP signals
        unordered_map<int, simsignal_t> numWaitingPerDestSignals = {};
        unordered_map<int, simsignal_t> demandEstimatePerDestSignals = {};

    protected:
        virtual void initialize() override;
        virtual void handleMessage(cMessage *msg) override;
        virtual void handleAckMessageSpecialized(routerMsg *msg) override;
        virtual void handleTimeOutMessage(routerMsg *msg) override;
        virtual void handleTransactionMessageSpecialized(routerMsg *msg) override;
        virtual void handleStatMessage(routerMsg *msg) override;
        virtual void handleMonitorPathsMessage(routerMsg *msg);
        virtual void handleClearStateMessage(routerMsg *msg) override;

        // helper method
        virtual double getMaxWindowSize(unordered_map<int, PathInfo> pathList); 
        virtual void initializePathInfo(vector<vector<int>> kShortestPaths, int destNode);
        virtual void initializeThisPath(vector<int> thisPath, int pathIdx, int destNode);
        virtual void sendMoreTransactionsOnPath(int destNode, int pathIndex);
        virtual routerMsg *generateMonitorPathsMessage();
        virtual int forwardTransactionMessage(routerMsg *msg, int dest, simtime_t arrivalTime) override;
};
#endif

/home/kush/Desktop/spiderproj/spider_omnet/hostNodeLandmarkRouting.h

 #ifndef ROUTERNODE_LR_H
#define ROUTERNODE_LR_H

#include "probeMsg_m.h"
#include "hostNodeBase.h"

using namespace std;
using namespace omnetpp;

class hostNodeLandmarkRouting : public hostNodeBase {
    private:
        map<int, ProbeInfo> transactionIdToProbeInfoMap = {}; //used only for landmark routing

    protected:
        virtual routerMsg *generateProbeMessage(int destNode, int pathIdx, vector<int> path);
        virtual void forwardProbeMessage(routerMsg *msg);

        // message handlers
        virtual void handleMessage(cMessage* msg) override;
        virtual void handleTransactionMessageSpecialized(routerMsg *msg) override;
        virtual void handleTimeOutMessage(routerMsg *msg) override;
        virtual void handleAckMessageTimeOut(routerMsg *msg) override;
        virtual void handleAckMessageSpecialized(routerMsg *msg) override;
        virtual void handleClearStateMessage(routerMsg *msg) override;
        virtual void handleProbeMessage(routerMsg *msg);
      
        // core logic
        virtual bool randomSplit(double totalAmt, vector<double> bottlenecks, vector<double> &amtPerPath);
        virtual void initializePathInfoLandmarkRouting(vector<vector<int>> kShortestRoutes, 
               int  destNode);
        virtual void initializeLandmarkRoutingProbes(routerMsg * msg, 
               int transactionId, int destNode);

        virtual void finish() override;
};
#endif

/home/kush/Desktop/spiderproj/spider_omnet/hostNodeLndBaseline.h

#ifndef ROUTERNODE_LNDB_H
#define ROUTERNODE_LNDB_H

#include "hostNodeBase.h"

using namespace std;
using namespace omnetpp;

class hostNodeLndBaseline : public hostNodeBase {

    private:
        vector<simsignal_t> numPathsPerTransPerDestSignals = {};  
        unordered_map<int, set<int>> _activeChannels;
        
        //list order oldest pruned channels in the front, most newly pruned in the back
        list <tuple<simtime_t, tuple<int, int>>> _prunedChannelsList;
    
    protected:
        virtual void initialize() override;
        virtual void handleTransactionMessageSpecialized(routerMsg *msg) override;
        virtual void handleAckMessageSpecialized(routerMsg *msg) override;
        virtual void handleAckMessageNoMoreRoutes(routerMsg *msg, bool toDelete);
        virtual routerMsg *generateAckMessage(routerMsg *msg, bool isSuccess = true) override;
        virtual void finish() override;

        // helpers
        virtual void initializeMyChannels();
        virtual void pruneEdge(int sourceNode, int destNode);
        virtual vector<int> generateNextPath(int destNode);
        virtual void recordTailRetries(simtime_t timeSent, bool success, int retries);


};
#endif

/home/kush/Desktop/spiderproj/spider_omnet/hostNodePriceScheme.h

#ifndef ROUTERNODE_PS_H
#define ROUTERNODE_PS_H

#include "priceQueryMsg_m.h"
#include "priceUpdateMsg_m.h"
#include "hostNodeBase.h"

using namespace std;
using namespace omnetpp;

class hostNodePriceScheme : public hostNodeBase {

    private:
        // price scheme specific signals
        vector<simsignal_t> probabilityPerDestSignals = {};
        map<int, simsignal_t> numWaitingPerDestSignals = {};
        vector<simsignal_t> numTimedOutAtSenderSignals = {};
        vector<simsignal_t> pathPerTransPerDestSignals = {};
        map<int, simsignal_t> demandEstimatePerDestSignals = {};


    protected:
        // message generators
        virtual routerMsg *generateTriggerPriceUpdateMessage();
        virtual routerMsg *generatePriceUpdateMessage(double nLocal, double serviceRate, double arrivalRate, 
                double queueSize, int reciever);
        virtual routerMsg *generateTriggerPriceQueryMessage();
        virtual routerMsg *generatePriceQueryMessage(vector<int> route, int routeIndex);
        virtual routerMsg *generateTriggerTransactionSendMessage(vector<int> route, 
                int routeIndex, int destNode);

        // helpers
        // functions to compute projections while ensure rates are feasible
        virtual bool ratesFeasible(vector<PathRateTuple> actualRates, double demand);
        virtual vector<PathRateTuple> computeProjection(vector<PathRateTuple> recommendedRates, 
                double demand);

        // modified message handlers
        virtual void handleMessage(cMessage *msg) override;
        virtual void handleTimeOutMessage(routerMsg *msg) override;
        virtual void handleTransactionMessageSpecialized(routerMsg *msg) override;
        virtual void handleStatMessage(routerMsg *msg) override;
        virtual void handleAckMessageSpecialized(routerMsg* ttmsg) override;
        virtual void handleClearStateMessage(routerMsg *msg) override;

        // special messages for priceScheme
        virtual void handleTriggerPriceUpdateMessage(routerMsg *msg);
        virtual void handlePriceUpdateMessage(routerMsg* ttmsg);
        virtual void handleTriggerPriceQueryMessage(routerMsg *msg);
        virtual void handlePriceQueryMessage(routerMsg* ttmsg);
        virtual void handleTriggerTransactionSendMessage(routerMsg* ttmsg);


        /**** CORE LOGIC ******/
        // initialize price probes for a given set of paths to a destination and store the paths
        // for that destination
        virtual void initializePriceProbes(vector<vector<int>> kShortestPaths, int destNode);
        // updates timers once rates have been updated on a certain path
        virtual void updateTimers(int destNode, int pathIndex, double newRate);
        virtual void initialize() override;
};
#endif

/home/kush/Desktop/spiderproj/spider_omnet/hostNodePropFairPriceScheme.h

#ifndef ROUTERNODE_PRS_H
#define ROUTERNODE_PRS_H

#include "priceQueryMsg_m.h"
#include "priceUpdateMsg_m.h"
#include "hostNodeBase.h"

using namespace std;
using namespace omnetpp;

class hostNodePropFairPriceScheme : public hostNodeBase {

    private:
        // price scheme specific signals
        map<int, simsignal_t> numWaitingPerDestSignals = {};
        vector<simsignal_t> numTimedOutAtSenderSignals = {};
        map<int, simsignal_t> demandEstimatePerDestSignals = {};

    protected:
        // message generators
        virtual routerMsg *generateTriggerTransactionSendMessage(vector<int> route, 
                int routeIndex, int destNode);
        virtual routerMsg *generateComputeDemandMessage();
        virtual routerMsg *generateTriggerRateDecreaseMessage();

        // helpers
        // functions to compute projections while ensure rates are feasible
        virtual bool ratesFeasible(vector<PathRateTuple> actualRates, double demand);
        virtual vector<PathRateTuple> computeProjection(vector<PathRateTuple> recommendedRates, 
                double demand);

        // modified message handlers
        virtual void handleMessage(cMessage *msg) override;
        virtual void handleTimeOutMessage(routerMsg *msg) override;
        virtual void handleTransactionMessageSpecialized(routerMsg *msg) override;
        virtual void handleComputeDemandMessage(routerMsg *msg);
        virtual void handleTriggerRateDecreaseMessage(routerMsg *msg);
        virtual void handleStatMessage(routerMsg *msg) override;
        virtual void handleAckMessageSpecialized(routerMsg* ttmsg) override;
        virtual void handleClearStateMessage(routerMsg *msg) override;

        // special messages for priceScheme
        virtual void handleTriggerTransactionSendMessage(routerMsg* ttmsg);

        /**** CORE LOGIC ******/
        // initialize price probes for a given set of paths to a destination and store the paths
        // for that destination
        virtual void initializePathInfo(vector<vector<int>> kShortestPaths, int destNode);

        // updates timers once rates have been updated on a certain path
        virtual void updateTimers(int destNode, int pathIndex, double newRate);
        virtual void initialize() override;
};
#endif

/home/kush/Desktop/spiderproj/spider_omnet/hostNodeWaterfilling.h

#ifndef ROUTERNODE_WF_H
#define ROUTERNODE_WF_H

#include "probeMsg_m.h"
#include "hostNodeBase.h"

using namespace std;
using namespace omnetpp;

class hostNodeWaterfilling : public hostNodeBase {

    private:
        // time since the last measurement for balances was made 
        // used to update path probabilities in smooth waterfilling
        unordered_map<int, double> destNodeToLastMeasurementTime = {};

        // waterfilling specific signals
        unordered_map<int, simsignal_t> probabilityPerDestSignals = {};
        unordered_map<int, simsignal_t> numWaitingPerDestSignals = {};

    protected:
        virtual void initialize() override;
        // message generating functions
        virtual routerMsg *generateProbeMessage(int destNode, int pathIdx, vector<int> path);

        // message handlers
        virtual void handleMessage(cMessage *msg) override;
        virtual void handleTransactionMessageSpecialized(routerMsg *msg) override;
        virtual void handleTimeOutMessage(routerMsg *msg) override;
        virtual void handleProbeMessage(routerMsg *msg);
        virtual void handleClearStateMessage(routerMsg *msg) override;
        virtual void handleAckMessageTimeOut(routerMsg *msg) override;
        virtual void handleAckMessageSpecialized(routerMsg *msg) override;
        virtual void handleStatMessage(routerMsg *msg) override;
        
        /**** CORE LOGIC ****/
        virtual void initializeProbes(vector<vector<int>> kShortestPaths, int destNode);
        virtual void restartProbes(int destNode);
        virtual void forwardProbeMessage(routerMsg *msg);
        virtual void splitTransactionForWaterfilling(routerMsg * ttMsg, bool firstAttempt);
        virtual void attemptTransactionOnBestPath(routerMsg * ttMsg, bool firstAttempt);
        virtual int updatePathProbabilities(vector<double> bottleneckBalances, int destNode);


};
#endif


SUMMARY ###############

# Developer Manual: HostNode Classes in Spider Project

This manual provides a detailed overview of the various host node classes implemented in the Spider project. These classes extend the functionality of `hostNodeBase` to implement different routing and transaction processing algorithms. Each class is designed to handle specific network behaviors and optimizations.

## 1. HostNodeBase

### Overview
`HostNodeBase` is the foundational class for all host nodes. It includes essential functionalities for managing payment channels, handling transactions, and collecting statistics.

### Key Components

#### Wallet Management
- **Initialization**: The `initializeWallet` function sets up the wallet system with initial balances.
- **Transaction Validation**: `canProcessTransaction` checks if a node has sufficient balance to process a transaction.
- **Processing Payments**: 
  - `processOutgoingPayment` handles outgoing transactions.
  - `processIncomingPayment` manages incoming payments.

#### Transaction Handling
- **Message Processing**:
  - `handleTransactionMessage`: Validates and processes incoming transaction messages.
  - `handleAckMessage`: Handles acknowledgment messages for transactions.

#### Statistics Collection
Various signals are registered to collect statistics on transaction completions, failures, and timeouts.

## 2. HostNodeCeler

### Overview
Implements the Celer protocol for efficient transaction processing.

### Key Features
- **Per-Destination Queue Management**: Manages queues for each destination node.
- **CPI Calculation**: Computes the Cost Performance Index (CPI) for routing decisions.
- **K-Star Selection**: Determines optimal paths for transactions.

### Methods
- `pushIntoPerDestQueue`: Adds messages to destination-specific queues.
- `celerProcessTransactions`: Processes transactions based on CPI.
- `calculateCPI`: Evaluates the cost-effectiveness of paths.
- `findKStar`: Identifies the best path index (`k*`) for transaction forwarding.

## 3. HostNodeDCTCP

### Overview
Incorporates DCTCP (Data Center TCP) principles for congestion control.

### Key Features
- **Window Size Management**: Adjusts the window size based on network conditions.
- **Path Initialization**: Sets up paths for efficient data transmission.
- **Transaction Sending**: Sends more transactions on underutilized paths.

### Methods
- `getMaxWindowSize`: Determines the maximum allowable window size.
- `initializePathInfo`: Prepares paths for transaction routing.
- `sendMoreTransactionsOnPath`: Increases transaction flow on selected paths.

## 4. HostNodeLandmarkRouting

### Overview
Utilizes landmark routing for improved path selection.

### Key Features
- **Probe Management**: Sends and processes probe messages to assess path conditions.
- **Random Splitting**: Divides transactions across multiple paths based on bottleneck capacities.
- **Path Initialization**: Configures paths using landmark routing strategies.

### Methods
- `generateProbeMessage`: Creates probe messages for path assessment.
- `randomSplit`: Allocates transaction amounts across paths.
- `initializePathInfoLandmarkRouting`: Sets up paths using landmark routing.

## 5. HostNodeLndBaseline

### Overview
Provides a baseline implementation for Lightning Network Daemon (LND) routing.

### Key Features
- **Channel Pruning**: Manages active and pruned channels.
- **Path Generation**: Creates new paths dynamically.
- **Retry Logic**: Handles retries for failed transactions.

### Methods
- `pruneEdge`: Removes underperforming channels.
- `generateNextPath`: Generates alternative paths for transactions.
- `recordTailRetries`: Logs retry attempts for analysis.

## 6. HostNodePriceScheme

### Overview
Implements a price-based scheme for transaction routing.

### Key Features
- **Rate Feasibility**: Ensures proposed rates are achievable within network constraints.
- **Projection Computation**: Adjusts rates to maintain feasibility.
- **Price Updates**: Manages price updates and queries.

### Methods
- `ratesFeasible`: Checks if proposed rates are feasible.
- `computeProjection`: Adjusts rates to ensure feasibility.
- `generatePriceUpdateMessage`: Creates messages for price updates.

## 7. HostNodePropFairPriceScheme

### Overview
Extends the price scheme with proportional fairness.

### Key Features
- **Demand Estimation**: Tracks demand estimates per destination.
- **Rate Decrease Triggering**: Initiates rate decreases when necessary.
- **Transaction Triggering**: Sends transactions based on updated rates.

### Methods
- `computeProjection`: Similar to `HostNodePriceScheme`.
- `handleComputeDemandMessage`: Processes demand computation messages.
- `handleTriggerRateDecreaseMessage`: Manages rate decrease triggers.

## 8. HostNodeWaterfilling

### Overview
Utilizes water-filling algorithms for load balancing.

### Key Features
- **Probe Management**: Sends probes to measure path conditions.
- **Path Probability Updates**: Adjusts probabilities based on measured balances.
- **Transaction Splitting**: Divides transactions across best paths.

### Methods
- `initializeProbes`: Sets up initial probes for path measurement.
- `updatePathProbabilities`: Updates probabilities based on latest measurements.
- `splitTransactionForWaterfilling`: Splits transactions for optimal distribution.

## Conclusion
These classes provide a robust framework for implementing various routing and transaction processing algorithms in the Spider project. Each class extends the base functionality with specialized methods tailored to specific protocols and strategies. Understanding these components allows developers to effectively contribute to and maintain the project.