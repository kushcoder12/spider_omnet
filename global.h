#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include <vector>
#include <queue>
#include <stack>
#include "routerMsg_m.h"
#include "transactionMsg_m.h"
#include "ackMsg_m.h"
#include "updateMsg_m.h"
#include "timeOutMsg_m.h"
#include "probeMsg_m.h"
#include "priceUpdateMsg_m.h"
#include "priceQueryMsg_m.h"
#include "transactionSendMsg_m.h"
#include "addFundsMsg_m.h"
#include <iostream>
#include <algorithm>
#include <numeric>
#include <string>
#include <sstream>
#include <deque>
#include <unordered_map>
#include <fstream>
#include <iostream>
#include <list>
#include "structs/PaymentChannel.h"
#include "structs/PathInfo.h"
#include "structs/TransUnit.h"
#include "structs/CanceledTrans.h"
#include "structs/AckState.h"
#include "structs/DestInfo.h"
#include "structs/PathRateTuple.h"
#include "structs/ProbeInfo.h"
#include "structs/DestNodeStruct.h"

#define MSGSIZE 100
using namespace std;

struct LaterTransUnit
{
  bool operator()(const TransUnit& lhs, const TransUnit& rhs) const
  {
    return lhs.timeSent > rhs.timeSent;
  }
};

#ifndef GLOBAL_H
#define GLOBAL_H

#include <unordered_map>
#include <set>
#include <utility>
#include <tuple>

// Forward declarations of existing structures if needed
class PaymentChannel;

// Channel information structure
class ChannelInfo {
public:
    double staked_amount;
    double available_balance;

    ChannelInfo(double staked) : staked_amount(staked), available_balance(staked) {}
};

// Custom hash function for channel key pairs
struct ChannelKeyHash {
    template <class T1, class T2>
    std::size_t operator() (const std::pair<T1, T2>& pair) const {
        return std::hash<T1>()(pair.first) ^ std::hash<T2>()(pair.second);
    }
};

class WalletSystem {
private:
    double initial_balance;
    std::unordered_map<int, double> wallet_balances;
    std::unordered_map<std::pair<int, int>, ChannelInfo, ChannelKeyHash> channel_info;
    std::unordered_map<int, std::set<std::pair<int, int>>> node_channels;
    std::unordered_map<int, double> total_staked;

    // Helper function to get consistent channel key
    std::pair<int, int> getChannelKey(int node1, int node2) {
        return std::make_pair(std::min(node1, node2), std::max(node1, node2));
    }

public:
    WalletSystem(double initialBalance = 10000) : initial_balance(initialBalance) {}

    // Initialize wallets based on network topology
    void initializeWallets(const std::unordered_map<int, std::vector<std::pair<int, int>>>& network) {
        wallet_balances.clear();
        channel_info.clear();
        node_channels.clear();
        total_staked.clear();

        // First pass: Initialize basic structures
        for (const auto& node_entry : network) {
            int node = node_entry.first;
            wallet_balances[node] = initial_balance;
            total_staked[node] = 0;
        }

        // Second pass: Process channels and stakes
        for (const auto& node_entry : network) {
            int node1 = node_entry.first;
            for (const auto& edge : node_entry.second) {
                int node2 = edge.first;
                double staked = edge.second; // Assuming second value is stake amount

                auto channel_key = getChannelKey(node1, node2);
                channel_info.emplace(channel_key, ChannelInfo(staked));
                node_channels[node1].insert(channel_key);
                node_channels[node2].insert(channel_key);

                // Update total staked amounts
                total_staked[node1] += staked;
                total_staked[node2] += staked;
            }
        }

        // Third pass: Ensure sufficient wallet balances
        for (auto& balance : wallet_balances) {
            int node = balance.first;
            if (total_staked[node] > balance.second) {
                balance.second = total_staked[node];
            }
        }
    }

    // Check if transaction is possible
    bool canProcessTransaction(int from_node, double amount) const {
        auto it = wallet_balances.find(from_node);
        if (it == wallet_balances.end()) return false;
        return amount <= it->second;
    }

    // Process a transaction between nodes
    bool processTransaction(int from_node, int to_node, double amount) {
        if (amount <= 0) return false;

        // Handle credit operation
        if (from_node == -1 && to_node != -1) {
            auto it = wallet_balances.find(to_node);
            if (it == wallet_balances.end()) return false;
            it->second += amount;
            return true;
        }

        // Handle debit operation
        if (to_node == -1 && from_node != -1) {
            if (!canProcessTransaction(from_node, amount)) return false;
            wallet_balances[from_node] -= amount;
            return true;
        }

        // Handle transfer between nodes
        if (!canProcessTransaction(from_node, amount)) return false;
        wallet_balances[from_node] -= amount;
        wallet_balances[to_node] += amount;
        return true;
    }

    // Get wallet balance
    double getWalletBalance(int node) const {
        auto it = wallet_balances.find(node);
        return it != wallet_balances.end() ? it->second : 0.0;
    }

    // Get available liquidity
    double getAvailableLiquidity(int node) const {
        auto balance_it = wallet_balances.find(node);
        auto staked_it = total_staked.find(node);
        if (balance_it == wallet_balances.end()) return 0.0;
        if (staked_it == total_staked.end()) return balance_it->second;
        return balance_it->second - staked_it->second;
    }

    // Get total staked amount
    double getTotalStaked(int node) const {
        auto it = total_staked.find(node);
        return it != total_staked.end() ? it->second : 0.0;
    }
};

// Global wallet system instance
extern WalletSystem* globalWalletSystem;

// Convenience functions
inline void initializeWallets(const std::unordered_map<int, std::vector<std::pair<int, int>>>& network) {
    if (globalWalletSystem) globalWalletSystem->initializeWallets(network);
}

inline bool creditWallet(int node, double amount) {
    return globalWalletSystem ? globalWalletSystem->processTransaction(-1, node, amount) : false;
}

inline bool debitWallet(int node, double amount) {
    return globalWalletSystem ? globalWalletSystem->processTransaction(node, -1, amount) : false;
}

inline double getWalletBalance(int node) {
    return globalWalletSystem ? globalWalletSystem->getWalletBalance(node) : 0.0;
}

#endif // GLOBAL_H

//global parameters
extern unordered_map<int, priority_queue<TransUnit, vector<TransUnit>, LaterTransUnit>> _transUnitList;
// extern unordered_map<int, deque<TransUnit>> _transUnitList;
extern unordered_map<int, set<int>> _destList;
extern unordered_map<int, unordered_map<int, vector<vector<int>>>> _pathsMap;
extern int _numNodes;
//number of nodes in network
extern int _numRouterNodes;
extern int _numHostNodes;
extern unordered_map<int, vector<pair<int,int>>> _channels; //adjacency list format of graph edges of network
extern unordered_map<tuple<int,int>,double, hashId> _balances;
extern unordered_map<tuple<int,int>, double, hashId> _capacities;
extern double _statRate;
extern double _clearRate;
extern double _maxTravelTime;
extern double _maxOneHopDelay;
extern double _percentile;

//unordered_map of balances for each edge; key = <int,int> is <source, destination>
//extern bool withFailures;
extern bool _waterfillingEnabled;
extern bool _timeoutEnabled;
extern int _kValue; //for k shortest paths
extern double _simulationLength;
extern bool _landmarkRoutingEnabled;
extern bool _lndBaselineEnabled;
extern int _numAttemptsLNDBaseline;
//for lndbaseline
extern double _restorePeriod; 

extern bool _windowEnabled;

extern vector<tuple<int,int>> _landmarksWithConnectivityList;//pair: (number of edges, node number)
extern unordered_map<double, int> _transactionCompletionBySize;
extern unordered_map<double, int> _transactionArrivalBySize;
extern unordered_map<double, double> _txnAvgCompTimeBySize;
extern unordered_map<double, vector<double>> _txnTailCompTimesBySize;
extern ofstream _succRetriesFile, _failRetriesFile;

// for silentWhispers
extern vector<int> _landmarks;

//parameters for price scheme
extern bool _priceSchemeEnabled;
extern bool _splittingEnabled;

extern double _rho; // for nesterov computation
extern double _rhoLambda; // and accelerated Gradients
extern double _rhoMu;

extern double _eta; //for price computation
extern double _kappa; //for price computation
extern double _capacityFactor; //for price computation
extern bool _useQueueEquation;
extern double _tUpdate; //for triggering price updates at routers
extern double _tQuery; //for triggering price query probes
extern double _alpha; //parameter for rate updates
extern double _minPriceRate; // minimum rate to assign to all nodes when computing projections and such
extern double _delta; // round trip propagation delay
extern double _avgDelay;
extern double _xi; // how fast you want to drain the queue relative to network rtt - want this to be less than 1
extern double _routerQueueDrainTime;
extern int _serviceArrivalWindow;
extern double _minWindow;
extern bool _reschedulingEnabled;
extern double _smallRate;
extern double _tokenBucketCapacity;
extern double _zeta;

// overall knobs
extern bool _signalsEnabled;
extern bool _loggingEnabled;
extern double _ewmaFactor;

// path choices knob
extern bool _widestPathsEnabled;
extern bool _heuristicPathsEnabled;
extern bool _kspYenEnabled;
extern bool _obliviousRoutingEnabled;

// queue knobs
extern bool _hasQueueCapacity;
extern int _queueCapacity;
extern double _epsilon;

// speeding up price scheme
extern bool _nesterov;
extern bool _secondOrderOptimization;

// experiments related parameters to control statistics
extern double _transStatStart;
extern double _transStatEnd;
extern double _waterfillingStartTime;
extern double _landmarkRoutingStartTime;
extern double _shortestPathStartTime;
extern double _shortestPathEndTime;
extern double _splitSize;

// rebalancing related parameters
extern bool _rebalancingEnabled;
extern double _rebalancingUpFactor;
extern double _queueDelayThreshold;
extern double _gamma;
extern double _maxGammaImbalanceQueueSize;
extern double _delayForAddingFunds;
extern double _rebalanceRate;
extern double _computeBalanceRate;
extern double _bank; // global accounter of all money in the system

// DCTCP params
extern double _windowAlpha;
extern double _windowBeta;
extern double _qEcnThreshold;
extern double _qDelayEcnThreshold;
extern bool _qDelayVersion;
extern bool _tcpVersion;
extern double _balEcnThreshold;
extern bool _dctcpEnabled;
extern double _cannonicalRTT; // for prop fair price scheme version - specific to a topology
extern double _totalPaths; // total number of paths over which cannonical RTT was computed
extern bool _isCubic;
extern double _cubicScalingConstant;

// knobs to enable the changing of paths
extern bool _changingPathsEnabled;
extern int _maxPathsToConsider;
extern double _windowThresholdForChange; 

// celer network params
extern bool _celerEnabled;
extern int _maxImbalanceWindow;
//global debt queues 
extern unordered_map<int, unordered_map<int, double>> _nodeToDebtQueue; // = {};
// (key1, (key2, value)) where key1 is nodeA (whose debt queue), and key2 is nodeB (dest node),
 // and value is amt needs to be transferred from nodeA to nodeB
extern double _celerBeta;  // denotes how much to weigh the imbalance in the per node weight

