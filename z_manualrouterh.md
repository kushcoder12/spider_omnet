/home/kush/Desktop/spiderproj/spider_omnet/routerNodeBase.h

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
#include "probeMsg_m.h"
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

#ifndef ROUTER_NODE_BASE_H
#define ROUTER_NODE_BASE_H

#include "global.h"
#include <memory>

class RouterNodeBase : public cSimpleModule {
protected:
    int index;  // router node identifier
    std::shared_ptr<WalletSystem> wallet_system;
    unordered_map<int, PaymentChannel> nodeToPaymentChannel;
    double amtSuccessfulSoFar;

    // Wallet-related signals for statistics
    simsignal_t routerWalletBalanceSignal;
    simsignal_t routerWalletStakedSignal;
    simsignal_t routerWalletLiquiditySignal;
    simsignal_t routerTransitAmountSignal;

public:
    RouterNodeBase() : 
        wallet_system(std::make_shared<WalletSystem>(REASONABLE_ROUTER_BALANCE)),
        amtSuccessfulSoFar(0) {}
    
    virtual ~RouterNodeBase() {}

    // Initialize router wallet
    bool initializeRouterWallet() {
        if (!wallet_system) return false;
        
        // Initialize wallet with router-specific parameters
        wallet_system->initializeWallets(nodeToPaymentChannel);
        
        // Register wallet-related signals
        routerWalletBalanceSignal = registerSignal("routerWalletBalance");
        routerWalletStakedSignal = registerSignal("routerWalletStaked");
        routerWalletLiquiditySignal = registerSignal("routerWalletLiquidity");
        routerTransitAmountSignal = registerSignal("routerTransitAmount");
        
        // Emit initial values
        emitWalletStatistics();
        return true;
    }

    // Emit wallet statistics
    void emitWalletStatistics() {
        if (!wallet_system) return;
        emit(routerWalletBalanceSignal, wallet_system->getWalletBalance(index));
        emit(routerWalletStakedSignal, wallet_system->getTotalStaked(index));
        emit(routerWalletLiquiditySignal, wallet_system->getAvailableLiquidity(index));
        emit(routerTransitAmountSignal, amtSuccessfulSoFar);
    }

    // Check if router can process transaction
    bool canRouteTransaction(double amount) const {
        return wallet_system && wallet_system->canProcessTransaction(index, amount);
    }

    // Process transaction forwarding
    bool processTransitPayment(int from_node, int to_node, double amount) {
        if (!wallet_system) return false;
        
        // First receive from source
        if (!wallet_system->processTransaction(from_node, index, amount)) {
            return false;
        }
        
        // Then forward to destination
        if (!wallet_system->processTransaction(index, to_node, amount)) {
            // Rollback the received amount if forwarding fails
            wallet_system->processTransaction(index, from_node, amount);
            return false;
        }
        
        amtSuccessfulSoFar += amount;
        emitWalletStatistics();
        return true;
    }

    // Modified transaction handling
    virtual void handleTransactionMessage(routerMsg *msg) {
        if (!msg) return;
        
        transactionMsg* trans_msg = check_and_cast<transactionMsg*>(msg);
        double amount = trans_msg->getAmount();
        
        // Check if router can handle this transaction
        if (!canRouteTransaction(amount)) {
            // Generate failure ACK
            routerMsg* ack = generateAckMessage(msg, false);
            forwardMessage(ack);
            delete msg;
            return;
        }
        
        int from_node = trans_msg->getSource();
        int to_node = trans_msg->getReceiver();
        
        // Process the transit payment
        if (processTransitPayment(from_node, to_node, amount)) {
            // Forward the transaction message
            forwardTransactionMessage(msg, to_node, simTime());
        } else {
            // Generate failure ACK
            routerMsg* ack = generateAckMessage(msg, false);
            forwardMessage(ack);
            delete msg;
        }
    }

    // Handle rebalancing operations
    virtual void performRebalancing() override {
        if (!wallet_system) return;
        
        // Get current channel states
        for (auto& channel : nodeToPaymentChannel) {
            int other_node = channel.first;
            double current_balance = channel.second.getBalance();
            double capacity = channel.second.getCapacity();
            
            // Check if rebalancing is needed
            if (current_balance < capacity * _rebalancingUpFactor) {
                double rebalance_amount = capacity - current_balance;
                if (canRouteTransaction(rebalance_amount)) {
                    processTransitPayment(index, other_node, rebalance_amount);
                }
            }
        }
        
        emitWalletStatistics();
    }

    // Getters for wallet information
    double getRouterBalance() const {
        return wallet_system ? wallet_system->getWalletBalance(index) : 0.0;
    }

    double getRouterLiquidity() const {
        return wallet_system ? wallet_system->getAvailableLiquidity(index) : 0.0;
    }

    double getRouterStaked() const {
        return wallet_system ? wallet_system->getTotalStaked(index) : 0.0;
    }

    double getTransitAmount() const {
        return amtSuccessfulSoFar;
    }

    // ... (keep other existing methods)
};

#endif // ROUTER_NODE_BASE_H

class routerNodeBase : public cSimpleModule
{
    protected:
        unordered_map<int, PaymentChannel> nodeToPaymentChannel = {};
        unordered_set<CanceledTrans, hashCanceledTrans, equalCanceledTrans> canceledTransactions = {};
        double amtSuccessfulSoFar = 0;

        // helper methods
        virtual int myIndex();
        virtual void checkQueuedTransUnits(vector<tuple<int, double, routerMsg*,  Id, simtime_t >> 
                queuedTransUnits, int node);
        virtual void printNodeToPaymentChannel();
        virtual simsignal_t registerSignalPerChannel(string signalStart, int id);
        virtual simsignal_t registerSignalPerDest(string signalStart, int id, string suffix);
        virtual simsignal_t registerSignalPerChannelPerDest(string signalStart,
              int pathIdx, int destNode);
        virtual double getTotalAmount(int x);
        virtual double getTotalAmountIncomingInflight(int x);
        virtual double getTotalAmountOutgoingInflight(int x);
        virtual void performRebalancing();
        virtual void setPaymentChannelBalanceByNode(int node, double balance);
        virtual void deleteTransaction(routerMsg* ttmsg);
        virtual void addFunds(map<int, double> pcsNeedingFunds);

        // core simulator functions 
        virtual void initialize() override;
        virtual void finish() override;
        virtual bool processTransUnits(int dest, vector<tuple<int, double , routerMsg *, Id, simtime_t>>& q);
        virtual void deleteMessagesInQueues();

        // generators for standard messages
        virtual routerMsg *generateUpdateMessage(int transId, int receiver, double amount, int htlcIndex);
        virtual routerMsg *generateStatMessage();
        virtual routerMsg *generateComputeMinBalanceMessage();
        virtual routerMsg *generateTriggerRebalancingMessage();
        virtual routerMsg *generateAddFundsMessage(map<int,double> fundsToBeAdded);
        virtual routerMsg *generateClearStateMessage();
        virtual routerMsg *generateAckMessage(routerMsg *msg, bool isSuccess = true);

        // message forwarders
        virtual void forwardMessage(routerMsg *msg);
        virtual int forwardTransactionMessage(routerMsg *msg, int dest, simtime_t);

        // message handlers
        virtual void handleMessage(cMessage *msg) override;
        virtual void handleTransactionMessage(routerMsg *msg);
        virtual void handleComputeMinAvailableBalanceMessage(routerMsg* ttmsg);
        virtual bool handleTransactionMessageTimeOut(routerMsg *msg);
        virtual void handleTimeOutMessage(routerMsg *msg);
        virtual void handleAckMessage(routerMsg *msg);
        virtual void handleAckMessageTimeOut(routerMsg *msg);
        virtual void handleUpdateMessage(routerMsg *msg);
        virtual void handleStatMessage(routerMsg *msg);
        virtual void handleClearStateMessage(routerMsg *msg);
};
#endif

/home/kush/Desktop/spiderproj/spider_omnet/routerNodeCeler.h

#ifndef ROUTERNODE_CELER_H
#define ROUTERNODE_CELER_H

#include "routerNodeBase.h"

using namespace std;
using namespace omnetpp;

class routerNodeCeler : public routerNodeBase {
    private:
        unordered_map<int, DestNodeStruct> nodeToDestNodeStruct;
        unordered_map<int, deque<int>> transToNextHop; 
        unordered_set<int> adjacentHostNodes = {};

    protected:
        virtual void initialize() override; //initialize global debtQueues
        virtual void finish() override; // remove messages from the dest queues
        
        // message handlers
        virtual void handleStatMessage(routerMsg *msg) override;
        virtual void handleTimeOutMessage(routerMsg *msg) override;
        virtual void handleClearStateMessage(routerMsg *msg) override; 
        virtual void handleTransactionMessage(routerMsg *msg) override; 
        virtual void handleAckMessage(routerMsg *msg) override;

        // helper functions
        virtual void appendNextHopToTimeOutMessage(routerMsg* ttmsg, int nextNode);
        virtual void celerProcessTransactions(int endLinkNode = -1);
        virtual int findKStar(int endLinkNode, unordered_set<int> exclude);
        virtual double calculateCPI(int destNode, int neighborNode);
        virtual void setPaymentChannelBalanceByNode(int node, double balance) override;
        virtual int forwardTransactionMessage(routerMsg *msg, int dest, simtime_t arrivalTime) override; 
};
#endif

/home/kush/Desktop/spiderproj/spider_omnet/routerNodeDCTCP.h

#ifndef ROUTERNODE_DCTCP_H
#define ROUTERNODE_DCTCP_H

#include "routerNodeBase.h"

using namespace std;
using namespace omnetpp;

class routerNodeDCTCP : public routerNodeBase {
    protected:
        // messsage helpers
        virtual void handleStatMessage(routerMsg* ttmsg) override;
        virtual int forwardTransactionMessage(routerMsg *msg, int dest, simtime_t arrivalTime) override;
};
#endif

/home/kush/Desktop/spiderproj/spider_omnet/routerNodeDCTCPBal.h

#ifndef ROUTERNODE_DCTCP_BAL_H
#define ROUTERNODE_DCTCP_BAL_H

#include "routerNodeBase.h"

using namespace std;
using namespace omnetpp;

class routerNodeDCTCPBal : public routerNodeBase {
    protected:
        // messsage helpers
        virtual void handleTransactionMessage(routerMsg *msg) override;
};
#endif

/home/kush/Desktop/spiderproj/spider_omnet/routerNodeLndBaseline.h

#ifndef ROUTERNODE_LNDB_H
#define ROUTERNODE_LNDB_H

#include "routerNodeBase.h"

using namespace std;
using namespace omnetpp;

class routerNodeLndBaseline : public routerNodeBase {
    protected:
        // messsage helpers
        virtual routerMsg *generateAckMessage(routerMsg *msg, bool isSuccess = true) override;
};
#endif

/home/kush/Desktop/spiderproj/spider_omnet/routerNodePriceScheme.h

#ifndef ROUTERNODE_PS_H
#define ROUTERNODE_PS_H

#include "priceQueryMsg_m.h"
#include "priceUpdateMsg_m.h"
#include "routerNodeBase.h"

using namespace std;
using namespace omnetpp;

class routerNodePriceScheme : public routerNodeBase {
    protected:
        virtual void initialize() override;
        virtual routerMsg *generateTriggerPriceUpdateMessage();
        virtual routerMsg *generatePriceUpdateMessage(double nLocal, double serviceRate, double arrivalRate, 
                double queueSize, int reciever);

        // modified message handlers
        virtual void handleMessage(cMessage *msg) override;
        virtual void handleStatMessage(routerMsg *msg) override;
        virtual void handleTransactionMessage(routerMsg *msg) override;

        // special message handlers for price scheme
        virtual void handleTriggerPriceUpdateMessage(routerMsg *msg);
        virtual void handlePriceUpdateMessage(routerMsg* ttmsg);
        virtual void handlePriceQueryMessage(routerMsg* ttmsg);
};
#endif

/home/kush/Desktop/spiderproj/spider_omnet/routerNodeWaterfilling.h

#ifndef ROUTERNODE_WF_H
#define ROUTERNODE_WF_H

#include "probeMsg_m.h"
#include "routerNodeBase.h"

using namespace std;
using namespace omnetpp;

class routerNodeWaterfilling : public routerNodeBase {
    protected:
        // message handlers
        virtual void handleMessage(cMessage *msg) override;
        virtual void handleProbeMessage(routerMsg *msg);
      
        // messsage helpers
        virtual void forwardProbeMessage(routerMsg *msg);
};
#endif

SUMMARY ############################################

# Developer Manual: Router Classes in Spider Project

This manual provides a comprehensive overview of the various router classes implemented in the Spider project. These classes extend the functionality of `RouterNodeBase` to implement different routing and transaction processing algorithms. Each class is designed to handle specific network behaviors and optimizations.

## 1. RouterNodeBase

### Overview
`RouterNodeBase` serves as the foundational class for all router nodes. It includes essential functionalities for managing payment channels, handling transactions, rebalancing operations, and collecting statistics.

### Key Components

#### Wallet Management
- **Initialization**: The `initializeRouterWallet` function sets up the wallet system with initial balances tailored for routers.
- **Transaction Validation**: `canRouteTransaction` checks if a router has sufficient balance to process a transaction.
- **Processing Payments**:
  - `processTransitPayment`: Manages the receipt and forwarding of payments through the router.
  - `performRebalancing`: Executes rebalancing operations to maintain optimal channel states.

#### Transaction Handling
- **Message Processing**:
  - `handleTransactionMessage`: Validates and processes incoming transaction messages.
  - `handleAckMessage`: Handles acknowledgment messages for transactions.

#### Statistics Collection
Various signals are registered to collect statistics on transaction completions, failures, and transit amounts.

## 2. RouterNodeCeler

### Overview
Implements the Celer protocol for efficient transaction routing.

### Key Features
- **Per-Destination Queue Management**: Manages queues for each destination node.
- **CPI Calculation**: Computes the Cost Performance Index (CPI) for routing decisions.
- **K-Star Selection**: Determines optimal paths for transactions.

### Methods
- `appendNextHopToTimeOutMessage`: Adds next hop information to timeout messages.
- `celerProcessTransactions`: Processes transactions based on CPI.
- `calculateCPI`: Evaluates the cost-effectiveness of paths.
- `findKStar`: Identifies the best path index (`k*`) for transaction forwarding.

## 3. RouterNodeDCTCP

### Overview
Incorporates DCTCP (Data Center TCP) principles for congestion control.

### Key Features
- **Window Size Management**: Adjusts the window size based on network conditions.
- **Transaction Forwarding**: Sends more transactions on underutilized paths.

### Methods
- `forwardTransactionMessage`: Forwards transaction messages with consideration for DCTCP principles.

## 4. RouterNodeDCTCPBal

### Overview
Extends DCTCP with additional balancing mechanisms.

### Key Features
- **Transaction Handling**: Specializes in handling transaction messages with enhanced balancing logic.

### Methods
- `handleTransactionMessage`: Processes incoming transaction messages with balancing considerations.

## 5. RouterNodeLndBaseline

### Overview
Provides a baseline implementation for Lightning Network Daemon (LND) routing.

### Key Features
- **ACK Generation**: Creates acknowledgment messages for transaction outcomes.

### Methods
- `generateAckMessage`: Generates acknowledgment messages indicating success or failure.

## 6. RouterNodePriceScheme

### Overview
Implements a price-based scheme for transaction routing.

### Key Features
- **Rate Updates**: Manages price updates and queries.
- **Trigger Messages**: Generates messages to trigger price updates and queries.

### Methods
- `generateTriggerPriceUpdateMessage`: Creates messages to initiate price updates.
- `generatePriceUpdateMessage`: Generates messages containing updated price information.
- `handleTriggerPriceUpdateMessage`: Processes trigger messages for price updates.
- `handlePriceUpdateMessage`: Handles incoming price update messages.
- `handlePriceQueryMessage`: Processes price query messages.

## 7. RouterNodeWaterfilling

### Overview
Utilizes water-filling algorithms for load balancing.

### Key Features
- **Probe Management**: Sends probes to measure path conditions.
- **Path Probability Updates**: Adjusts probabilities based on measured balances.
- **Transaction Forwarding**: Forwards probe messages for path assessment.

### Methods
- `handleProbeMessage`: Processes incoming probe messages.
- `forwardProbeMessage`: Forwards probe messages to assess path conditions.

## Conclusion
These classes provide a robust framework for implementing various routing and transaction processing algorithms in the Spider project. Each class extends the base functionality with specialized methods tailored to specific protocols and strategies. Understanding these components allows developers to effectively contribute to and maintain the project.