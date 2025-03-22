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
