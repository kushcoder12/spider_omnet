/home/kush/Desktop/spiderproj/spider_omnet/routerNodeBase.cc

#include "routerNodeBase.h"
#include "hostInitialize.h"
#include <queue>

Define_Module(routerNodeBase);

int routerNodeBase::myIndex(){
   return getIndex() + _numHostNodes;
}

/* helper function to go through all transactions in a queue and print their details out
 * meant for debugging 
 */
void routerNodeBase::checkQueuedTransUnits(vector<tuple<int, double, routerMsg*,  Id, simtime_t>> 
        queuedTransUnits, int nextNode){
    if (queuedTransUnits.size() > 0) {
        cout << "simTime(): " << simTime() << endl;
        cout << "queuedTransUnits size: " << queuedTransUnits.size() << endl;
        for (auto q: queuedTransUnits) {
            routerMsg* msg = get<2>(q);
            transactionMsg *transMsg = check_and_cast<transactionMsg *>(msg->getEncapsulatedPacket());

            if (transMsg->getHasTimeOut()){
                cout << "(" << (transMsg->getTimeSent() + transMsg->getTimeOut()) 
                     << "," << transMsg->getTransactionId() << "," << nextNode << ") ";
            }
            else {
                cout << "(-1) ";
            }
        }
        cout << endl;
        for (auto c: canceledTransactions) {
            cout << "(" << get<0>(c) << "," <<get<1>(c) <<","<< get<2>(c)<< "," << get<3>(c)<< ") ";
        }
        cout << endl;
    }
}


/* helper method to print channel balances 
 */
void routerNodeBase:: printNodeToPaymentChannel(){
    printf("print of channels\n" );
    for (auto i : nodeToPaymentChannel){
        printf("(key: %d, %f )",i.first, i.second.balance);
    }
    cout<<endl;
}

/* get total amount on queue to node x */
double routerNodeBase::getTotalAmount(int x) {
    if (_hasQueueCapacity && _queueCapacity == 0)
        return 0;
    return nodeToPaymentChannel[x].totalAmtInQueue;
} 

/* get total amount inflight incoming node x */
double routerNodeBase::getTotalAmountIncomingInflight(int x) {
    return nodeToPaymentChannel[x].totalAmtIncomingInflight;
} 

/* get total amount inflight outgoing node x */
double routerNodeBase::getTotalAmountOutgoingInflight(int x) {
    return nodeToPaymentChannel[x].totalAmtOutgoingInflight;
} 

/* helper function to delete router message and encapsulated transaction message
 */
void routerNodeBase::deleteTransaction(routerMsg* ttmsg) {
    transactionMsg *transMsg = check_and_cast<transactionMsg *>(ttmsg->getEncapsulatedPacket());
    ttmsg->decapsulate();
    delete transMsg;
    delete ttmsg;
}


/* register a signal per channel of the particular type passed in
 * and return the signal created
 */
simsignal_t routerNodeBase::registerSignalPerChannel(string signalStart, int id) {
    char signalName[64];
    string signalPrefix = signalStart + "PerChannel";
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

/* register a signal per destination for this path of the particular type passed in
 * and return the signal created
 */
simsignal_t routerNodeBase::registerSignalPerChannelPerDest(string signalStart, int chnlEndNode, int destNode) {
    char signalName[64];
    string signalPrefix = signalStart + "PerChannelPerDest";
    string templateString = signalPrefix + "Template";
    
    if (chnlEndNode < _numHostNodes){
        sprintf(signalName, "%s_%d(host %d)", signalPrefix.c_str(), destNode, chnlEndNode);
    } else {
        sprintf(signalName, "%s_%d(router %d [%d] )", signalPrefix.c_str(),
             destNode, chnlEndNode - _numHostNodes, chnlEndNode);
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
simsignal_t routerNodeBase::registerSignalPerDest(string signalStart, int destNode, string suffix) {
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

/* initialize  basic
 * per channel information as well as default signals for all
 * payment channels 
 * */
void routerNodeBase::initialize()
{
    // Assign gates to all payment channels
    const char * gateName = "out";
    cGate *destGate = NULL;
    int i = 0;
    int gateSize = gate(gateName, 0)->size();

    do {
        destGate = gate(gateName, i++);
        cGate *nextGate = destGate->getNextGate();
        if (nextGate ) {
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
        int key =  iter->first; 
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

            signal = registerSignalPerChannel("numSent", key);
            nodeToPaymentChannel[key].numSentPerChannelSignal = signal;

	    signal = registerSignalPerChannel("balance", key);
            nodeToPaymentChannel[key].balancePerChannelSignal = signal;

            signal = registerSignalPerChannel("capacity", key);
            nodeToPaymentChannel[key].capacityPerChannelSignal = signal;
            
            signal = registerSignalPerChannel("queueDelayEWMA", key);
            nodeToPaymentChannel[key].queueDelayEWMASignal = signal;

            signal = registerSignalPerChannel("numInflight", key);
            nodeToPaymentChannel[key].numInflightPerChannelSignal = signal;

            signal = registerSignalPerChannel("timeInFlight", key);
            nodeToPaymentChannel[key].timeInFlightPerChannelSignal = signal;
            
            signal = registerSignalPerChannel("implicitRebalancingAmt", key);
            nodeToPaymentChannel[key].implicitRebalancingAmtPerChannelSignal = signal;
            
            signal = registerSignalPerChannel("explicitRebalancingAmt", key);
            nodeToPaymentChannel[key].explicitRebalancingAmtPerChannelSignal = signal;
        }
    }
    
    // generate statistic message
    routerMsg *statMsg = generateStatMessage();
    scheduleAt(simTime() + 0, statMsg);

    // generate time out clearing messages
    if (_timeoutEnabled) {
        routerMsg *clearStateMsg = generateClearStateMessage();
        scheduleAt(simTime() + _clearRate, clearStateMsg);
    }
}

/* function that is called at the end of the simulation that
 * deletes any remaining messages and records scalars
 */
void routerNodeBase::finish(){
    deleteMessagesInQueues();
    double numPoints = (_transStatEnd - _transStatStart)/(double) _statRate;
    double sumRebalancing = 0;
    double sumAmtAdded = 0;

    // iterate through all payment channels and print last summary statistics for queues
    for ( auto it = nodeToPaymentChannel.begin(); it!= nodeToPaymentChannel.end(); it++){ 
        int node = it->first; //key
        char buffer[30];
        sprintf(buffer, "queueSize %d -> %d", myIndex(), node);
        recordScalar(buffer, nodeToPaymentChannel[node].queueSizeSum/numPoints);

        sumRebalancing += nodeToPaymentChannel[node].numRebalanceEvents;
        sumAmtAdded += nodeToPaymentChannel[node].amtAdded;
    }
    
    // total amount of rebalancing incurred by this node
    char buffer[60];
    sprintf(buffer, "totalNumRebalancingEvents %d blah 1", myIndex());
    recordScalar(buffer, sumRebalancing/(_transStatEnd - _transStatStart));
    sprintf(buffer, "totalAmtAdded %d blah 2", myIndex());
    recordScalar(buffer, sumAmtAdded/(_transStatEnd - _transStatStart));
}

/*  given an adjacent node, and TransUnit queue of things to send to that node, sends
 *  TransUnits until channel funds are too low
 *  calls forwardTransactionMessage on every individual TransUnit
 *  returns true when it still can continue processing more transactions
 */
bool routerNodeBase:: processTransUnits(int neighbor, 
        vector<tuple<int, double , routerMsg *, Id, simtime_t>>& q) {
    int successful = true;
    while ((int)q.size() > 0 && successful == 1) {
        pop_heap(q.begin(), q.end(), _schedulingAlgorithm);
        successful = forwardTransactionMessage(get<2>(q.back()), neighbor, get<4>(q.back()));
        if (successful == 1){
            q.pop_back();
        }
    }
    return (successful != 0); // anything other than balance exhausted implies you can go on
}


/* helper method to delete the messages in any payment channel queues 
 * and per dest queues at the end of the experiment 
 */
void routerNodeBase::deleteMessagesInQueues(){
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
}


/********** MESSAGE GENERATOR ********************/
/* generates messages responsible for recognizing that a txn is complete
 * and funds have been securely transferred from a previous node to a 
 * neighboring node after the ack/secret has been received
 * Always goes only one hop, no more
 */
routerMsg *routerNodeBase::generateUpdateMessage(int transId, 
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
routerMsg *routerNodeBase::generateTriggerRebalancingMessage(){
    char msgname[MSGSIZE];
    sprintf(msgname, "node-%d rebalancingMsg", myIndex());
    routerMsg *rMsg = new routerMsg(msgname);
    rMsg->setMessageType(TRIGGER_REBALANCING_MSG);
    return rMsg;
}


/* generate statistic trigger message every x seconds
 * to output all statistics which can then be plotted
 */
routerMsg *routerNodeBase::generateStatMessage(){
    char msgname[MSGSIZE];
    sprintf(msgname, "node-%d statMsg", myIndex());
    routerMsg *rMsg = new routerMsg(msgname);
    rMsg->setMessageType(STAT_MSG);
    return rMsg;
}

/* generate message trigger t generate balances for all the payment channels
 */
routerMsg *routerNodeBase::generateComputeMinBalanceMessage(){
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
routerMsg *routerNodeBase::generateClearStateMessage(){
    char msgname[MSGSIZE];
    sprintf(msgname, "node-%d clearStateMsg", myIndex());
    routerMsg *rMsg = new routerMsg(msgname);
    rMsg->setMessageType(CLEAR_STATE_MSG);
    return rMsg;
}

/* called only when a router in between the sender-receiver path
 * wants to send a failure ack due to insfufficent funds
 * similar to the endhost ack
 * isSuccess denotes whether the ack is in response to a transaction
 * that succeeded or failed.
 */
routerMsg *routerNodeBase::generateAckMessage(routerMsg* ttmsg, bool isSuccess) {
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


/* generate a message that designates which payment channels at this router need funds
 * and how much funds they need, will be processed a few seconds/minutes later to 
 * actually add the funds to those payment channels */
routerMsg *routerNodeBase::generateAddFundsMessage(map<int, double> fundsToBeAdded) {
    char msgname[MSGSIZE];
    map<int,double> pcsNeedingFunds = fundsToBeAdded;
    sprintf(msgname, "addfundmessage-at-%d", myIndex());
    
    routerMsg *msg = new routerMsg(msgname);
    addFundsMsg *afMsg = new addFundsMsg(msgname);
    afMsg->setPcsNeedingFunds(pcsNeedingFunds);
    msg->setMessageType(ADD_FUNDS_MSG); 
    msg->encapsulate(afMsg);
    return msg;
}



/****** MESSAGE FORWARDERS ********/
/* responsible for forwarding all messages but transactions which need special care
 * in particular, looks up the next node's interface and sends out the message
 */
void routerNodeBase::forwardMessage(routerMsg* msg){
   msg->setHopCount(msg->getHopCount()+1);
   int nextDest = msg->getRoute()[msg->getHopCount()];
   if (_loggingEnabled) cout << "forwarding " << msg->getMessageType() << " at " 
       << simTime() << endl;
   send(msg, nodeToPaymentChannel[nextDest].gate);
}

/*
 *  Given a message representing a TransUnit, increments hopCount, finds next destination,
 *  adjusts (decrements) channel balance, sends message to next node on route
 *  as long as it isn't cancelled
 */
int routerNodeBase::forwardTransactionMessage(routerMsg *msg, int neighborIdx, simtime_t arrivalTime) {
    transactionMsg *transMsg = check_and_cast<transactionMsg *>(msg->getEncapsulatedPacket());
    int nextDest = neighborIdx; 
    int transactionId = transMsg->getTransactionId();
    PaymentChannel *neighbor = &(nodeToPaymentChannel[nextDest]);
    int amt = transMsg->getAmount();

    // return true directly if txn has been cancelled
    // so that you waste not resources on this and move on to a new txn
    // if you return false processTransUnits won't look for more txns
    auto iter = canceledTransactions.find(make_tuple(transactionId, 0, 0, 0, 0));
    if (iter != canceledTransactions.end()) {
        msg->decapsulate();
        delete transMsg;
        delete msg;
        neighbor->totalAmtInQueue -= amt;
        return 1;
    }

    if (neighbor->balance <= 0 || transMsg->getAmount() > neighbor->balance){
        return 0;
    }
    else {
        // update state to send transaction out
        msg->setHopCount(msg->getHopCount()+1);

        // update service arrival times
        neighbor->serviceArrivalTimeStamps.push_back(make_tuple(transMsg->getAmount(), simTime(), arrivalTime));
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
        double newBalance = neighbor->balance - amt;
        setPaymentChannelBalanceByNode(nextDest, newBalance); 
        neighbor-> balanceEWMA = (1 -_ewmaFactor) * neighbor->balanceEWMA + 
            (_ewmaFactor) * newBalance;
        neighbor->totalAmtInQueue -= amt;

        if (_loggingEnabled) cout << "forwardTransactionMsg send: " << simTime() << endl;
        send(msg, nodeToPaymentChannel[nextDest].gate);
        return 1;
    } 
}

/* overall controller for handling messages that dispatches the right function
 * based on message type
 */
void routerNodeBase::handleMessage(cMessage *msg){
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
                cout << "[ROUTER "<< myIndex() <<": RECEIVED ACK MSG] " << msg->getName() << endl;
            if (_timeoutEnabled)
                handleAckMessageTimeOut(ttmsg);
            handleAckMessage(ttmsg);
            if (_loggingEnabled) cout << "[AFTER HANDLING:]" <<endl;
            break;

        case TRANSACTION_MSG: 
            { 
                if (_loggingEnabled) 
                    cout<< "[ROUTER "<< myIndex() <<": RECEIVED TRANSACTION MSG]  "
                     << msg->getName() <<endl;
             
                if (_timeoutEnabled && handleTransactionMessageTimeOut(ttmsg)){
                    return;
                }
                handleTransactionMessage(ttmsg);
                if (_loggingEnabled) cout << "[AFTER HANDLING:]" << endl;
            }
            break;

        case UPDATE_MSG:
            if (_loggingEnabled) cout<< "[ROUTER "<< myIndex() 
                <<": RECEIVED UPDATE MSG] "<< msg->getName() << endl;
                handleUpdateMessage(ttmsg);
            if (_loggingEnabled) cout<< "[AFTER HANDLING:]  "<< endl;
            break;

        case STAT_MSG:
            if (_loggingEnabled) cout<< "[ROUTER "<< myIndex() 
                <<": RECEIVED STAT MSG] "<< msg->getName() << endl;
                handleStatMessage(ttmsg);
            if (_loggingEnabled) cout<< "[AFTER HANDLING:]  "<< endl;
            break;

        case TIME_OUT_MSG:
            if (_loggingEnabled) cout<< "[ROUTER "<< myIndex() 
                <<": RECEIVED TIME_OUT_MSG] "<< msg->getName() << endl;
       
            if (!_timeoutEnabled){
                cout << "timeout message generated when it shouldn't have" << endl;
                return;
            }

                handleTimeOutMessage(ttmsg);
            if (_loggingEnabled) cout<< "[AFTER HANDLING:]  "<< endl;
            break;
        
        case CLEAR_STATE_MSG:
            if (_loggingEnabled) cout<< "[ROUTER "<< myIndex() 
                <<": RECEIVED CLEAR_STATE_MSG] "<< msg->getName() << endl;
                handleClearStateMessage(ttmsg);
            if (_loggingEnabled) cout<< "[AFTER HANDLING:]  "<< endl;
            break;
        
        case COMPUTE_BALANCE_MSG:
            if (_loggingEnabled) cout<< "[ROUTER "<< myIndex() 
                <<": RECEIVED COMPUTE BALANCE MSG] "<< msg->getName() << endl;
                handleComputeMinAvailableBalanceMessage(ttmsg);
            if (_loggingEnabled) cout<< "[AFTER HANDLING:]  "<< endl;
            break;

        default:
                handleMessage(ttmsg);

    }

}

/* Main handler for normal processing of a transaction
 * checks if message has reached sender
 *      1. has reached (is in rev direction)  - turn transactionMsg into ackMsg, forward ackMsg
 *      2. has not reached yet - add to appropriate job queue q, process q as
 *          much as we have funds for
 */
void routerNodeBase::handleTransactionMessage(routerMsg* ttmsg) {
    transactionMsg *transMsg = check_and_cast<transactionMsg *>(ttmsg->getEncapsulatedPacket());
    int hopcount = ttmsg->getHopCount();
    vector<tuple<int, double , routerMsg *, Id, simtime_t>> *q;
    int destination = transMsg->getReceiver();
    int transactionId = transMsg->getTransactionId();

    // ignore if txn is already cancelled
    auto iter = canceledTransactions.find(make_tuple(transactionId, 0, 0, 0, 0));
    if ( iter!=canceledTransactions.end() ){
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

    // find the outgoing channel to check capacity/ability to send on it
    int nextNode = ttmsg->getRoute()[hopcount+1];
    PaymentChannel *neighbor = &(nodeToPaymentChannel[nextNode]);
    q = &(neighbor->queuedTransUnits);

    // mark the arrival on this payment channel
    neighbor->arrivalTimeStamps.push_back(make_tuple(transMsg->getAmount(), simTime()));
    neighbor->sumArrivalWindowTxns += transMsg->getAmount();
    if (neighbor->arrivalTimeStamps.size() > _serviceArrivalWindow) {
        double frontAmt = get<0>(neighbor->serviceArrivalTimeStamps.front());
        neighbor->arrivalTimeStamps.pop_front(); 
        neighbor->sumArrivalWindowTxns -= frontAmt;
    }

    // if balance is insufficient at the first node, return failure ack
    if (_hasQueueCapacity && _queueCapacity == 0) {
        if (forwardTransactionMessage(ttmsg, nextNode, simTime()) == 0) {
            routerMsg * failedAckMsg = generateAckMessage(ttmsg, false);
            handleAckMessage(failedAckMsg);
        }
    } else if (_hasQueueCapacity && _queueCapacity<= getTotalAmount(nextNode)) { 
        //failed transaction, queue at capacity, others are in queue so no point trying this txn
        routerMsg * failedAckMsg = generateAckMessage(ttmsg, false);
        handleAckMessage(failedAckMsg);
    } else {
        // add to queue and process in order of priority
        (*q).push_back(make_tuple(transMsg->getPriorityClass(), transMsg->getAmount(),
               ttmsg, make_tuple(transMsg->getTransactionId(), transMsg->getHtlcIndex()), simTime()));
        neighbor->totalAmtInQueue += transMsg->getAmount();
        push_heap((*q).begin(), (*q).end(), _schedulingAlgorithm);
        processTransUnits(nextNode, *q);
    }
}

/* handler responsible for prematurely terminating the processing
 * of a transaction if it has timed out and deleteing it. Returns
 * true if the transaction is timed out so that no special handlers
 * are called after
 */
bool routerNodeBase::handleTransactionMessageTimeOut(routerMsg* ttmsg) {
    transactionMsg *transMsg = check_and_cast<transactionMsg *>(ttmsg->getEncapsulatedPacket());
    int transactionId = transMsg->getTransactionId();
    auto iter = canceledTransactions.find(make_tuple(transactionId, 0, 0, 0, 0));
    
    if (iter != canceledTransactions.end() ){
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
 *  Will only be encountered in forward direction
 */
void routerNodeBase::handleTimeOutMessage(routerMsg* ttmsg){
    timeOutMsg *toutMsg = check_and_cast<timeOutMsg *>(ttmsg->getEncapsulatedPacket());
    int nextNode = (ttmsg->getRoute())[ttmsg->getHopCount()+1];
    int prevNode = (ttmsg->getRoute())[ttmsg->getHopCount()-1];
    CanceledTrans ct = make_tuple(toutMsg->getTransactionId(), simTime(), 
            prevNode, nextNode, toutMsg->getReceiver());
    canceledTransactions.insert(ct);
    forwardMessage(ttmsg);
}

/* default routine for handling an ack that is responsible for 
 * updating outgoing transunits and incoming trans units 
 * and triggering an update message to the next node on the path
 * before forwarding the ack back to the previous node
 */
void routerNodeBase::handleAckMessage(routerMsg* ttmsg){
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
    int transactionId = aMsg->getTransactionId();
   
    if (aMsg->getIsSuccess() == false){
        // increment funds on this channel unless this is the node that caused the fauilure
        // in which case funds were never decremented in the first place
        if (aMsg->getFailedHopNum() < ttmsg->getHopCount()) {
            double updatedBalance = prevChannel->balance + aMsg->getAmount();
            prevChannel->balanceEWMA = 
                (1 -_ewmaFactor) * prevChannel->balanceEWMA + (_ewmaFactor) * updatedBalance; 
            prevChannel->totalAmtOutgoingInflight -= aMsg->getAmount();
            setPaymentChannelBalanceByNode(prevNode, updatedBalance);
        }
        
        // this is nextNode on the ack path and so prev node in the forward path or rather
        // node sending you mayments
        int nextNode = ttmsg->getRoute()[ttmsg->getHopCount()+1];
        unordered_map<Id, double, hashId> *incomingTransUnits = 
            &(nodeToPaymentChannel[nextNode].incomingTransUnits);
        (*incomingTransUnits).erase(make_tuple(aMsg->getTransactionId(), aMsg->getHtlcIndex()));
        nodeToPaymentChannel[nextNode].totalAmtIncomingInflight -= aMsg->getAmount();
    }
    else { 
        // mark the time it spent inflight
        routerMsg* uMsg =  generateUpdateMessage(aMsg->getTransactionId(), prevNode, 
                aMsg->getAmount(), aMsg->getHtlcIndex() );
        prevChannel->sumTimeInFlight += timeInflight;
        prevChannel->timeInFlightSamples += 1;
        prevChannel->totalAmtOutgoingInflight -= aMsg->getAmount();
        prevChannel->numUpdateMessages += 1;
        forwardMessage(uMsg);
        
        amtSuccessfulSoFar += aMsg->getAmount();
        if (amtSuccessfulSoFar > _rebalanceRate && _rebalancingEnabled) {
            amtSuccessfulSoFar = 0;
            performRebalancing();
        }
    }
    forwardMessage(ttmsg);
}

/* handles the logic for ack messages in the presence of timeouts
 * in particular, removes the transaction from the cancelled txns
 * to mark that it has been received 
 */
void routerNodeBase::handleAckMessageTimeOut(routerMsg* ttmsg){
    ackMsg *aMsg = check_and_cast<ackMsg *>(ttmsg->getEncapsulatedPacket());
    int transactionId = aMsg->getTransactionId();
    auto iter = canceledTransactions.find(make_tuple(transactionId, 0, 0, 0, 0));
    if (iter != canceledTransactions.end()) {
        canceledTransactions.erase(iter);
    }
}

/* handleUpdateMessage - called when receive update message, increment back funds, see if we can
 *      process more jobs with new funds, delete update message
 */
void routerNodeBase::handleUpdateMessage(routerMsg* msg) {
    vector<tuple<int, double , routerMsg *, Id, simtime_t>> *q;
    int prevNode = msg->getRoute()[msg->getHopCount()-1];
    updateMsg *uMsg = check_and_cast<updateMsg *>(msg->getEncapsulatedPacket());
    PaymentChannel *prevChannel = &(nodeToPaymentChannel[prevNode]);
    
    // remove transaction from incoming_trans_units
    unordered_map<Id, double, hashId> *incomingTransUnits = &(prevChannel->incomingTransUnits);
    (*incomingTransUnits).erase(make_tuple(uMsg->getTransactionId(), uMsg->getHtlcIndex()));
    prevChannel->totalAmtIncomingInflight -= uMsg->getAmount();
    
    // increment the in flight funds back
    double newBalance = prevChannel->balance + uMsg->getAmount();
    setPaymentChannelBalanceByNode(prevNode, newBalance); 
    prevChannel->balanceEWMA = (1 -_ewmaFactor) * prevChannel->balanceEWMA 
        + (_ewmaFactor) * newBalance; 
    
    msg->decapsulate();
    delete uMsg;
    delete msg; //delete update message

    // see if we can send more jobs out
    q = &(prevChannel->queuedTransUnits);
    processTransUnits(prevNode, *q);
} 

/* handler that periodically computes the minimum balance on a payment channel 
 * this is then used accordingly to trigger rebalancing events */
void routerNodeBase::handleComputeMinAvailableBalanceMessage(routerMsg* ttmsg) {
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
 * that is responsible for equalizing the available balance across all of the
 * payment channels of a given router 
 */
void routerNodeBase::performRebalancing() {
    // compute avalable stash to redistribute
    double stash = 0.0;
    int numChannels = 0;
    for (auto it = nodeToPaymentChannel.begin(); it!= nodeToPaymentChannel.end(); it++){
        PaymentChannel *p = &(it->second);
        stash += p->balance;
        numChannels += 1;
    }

    // figure out how much to give each channel
    double targetBalancePerChannel = int(stash/numChannels); // target will always be lower as a result
    double totalToRemove = 0;
    map<int, double> pcsNeedingFunds; 
    for (auto it = nodeToPaymentChannel.begin(); it!= nodeToPaymentChannel.end(); it++){
        int id = it->first;
        PaymentChannel *p = &(it->second);
        double differential = p->balance - targetBalancePerChannel;

        if (differential < 0) {
            // add this to the list of payment channels to be addressed 
            // along with a particular addFundsEvent
            pcsNeedingFunds[id] =  -1 * differential;
            totalToRemove += -1 * differential;
        }
    }

    // make sure the amount given is appropriately removed from other channels
    for (auto it = nodeToPaymentChannel.begin(); it!= nodeToPaymentChannel.end(); it++){
        int id = it->first;
        PaymentChannel *p = &(it->second);
        double differential = min(totalToRemove, p->balance - targetBalancePerChannel);
        if (differential > 0) {
            // remove capacity immediately from these channel
            setPaymentChannelBalanceByNode(it->first, p->balance - differential);
            p->balanceEWMA -= differential;
            p->amtExplicitlyRebalanced -= differential;
            if (simTime() > _transStatStart && simTime() < _transStatEnd) {
                p->amtAdded += differential; 
                p->numRebalanceEvents += 1;
            }
            
            totalToRemove -= differential;
            if (p->balance < 0) 
                cout << "terrible: " << differential << " balance " << p->balance << "min available balance "
                    << p->minAvailBalance << endl;
            tuple<int, int> senderReceiverTuple = (id < myIndex()) ? make_tuple(id, myIndex()) :
                make_tuple(myIndex(), id);
            _capacities[senderReceiverTuple] -=  differential;
        }
        p->minAvailBalance = 1000000;
    }
    
    // generate and schedule add funds message to add these funds after some fixed time period
    if (pcsNeedingFunds.size() > 0) {
        addFunds(pcsNeedingFunds);
    }
}

/* handler to add the desired amount of funds to the given payment channels when an addFundsMessage
 * is received 
 */
void routerNodeBase::addFunds(map<int, double> pcsNeedingFunds) {
    for (auto it = pcsNeedingFunds.begin(); it!= pcsNeedingFunds.end(); it++) {
        int pcIdentifier = it->first;
        double fundsToAdd = it->second;
        PaymentChannel *p = &(nodeToPaymentChannel[pcIdentifier]);
        
        // add funds at this end
        setPaymentChannelBalanceByNode(pcIdentifier, p->balance+fundsToAdd);
        p->balanceEWMA += fundsToAdd;
        tuple<int, int> senderReceiverTuple = (pcIdentifier < myIndex()) ? make_tuple(pcIdentifier, myIndex()) :
            make_tuple(myIndex(), pcIdentifier);
        _capacities[senderReceiverTuple] +=  fundsToAdd;
        
        // track statistics
        if (simTime() > _transStatStart && simTime() < _transStatEnd) {
            p->numRebalanceEvents += 1;
            p->amtAdded += fundsToAdd;
        }
        p->amtExplicitlyRebalanced += fundsToAdd;

        // process as many new transUnits as you can for this payment channel
        processTransUnits(pcIdentifier, p->queuedTransUnits);
    }
}


/* emits all the default statistics across all the schemes
 * until the end of the simulation
 */
void routerNodeBase::handleStatMessage(routerMsg* ttmsg){
    if (simTime() > _simulationLength){
        delete ttmsg;
    }
    else {
        scheduleAt(simTime()+_statRate, ttmsg);
    }
    
    // per channel Stats
    for (auto it = nodeToPaymentChannel.begin(); it!= nodeToPaymentChannel.end(); it++){
        PaymentChannel *p = &(it->second);
        int id = it->first;
        if (simTime() > _transStatStart && simTime() < _transStatEnd)
            p->queueSizeSum += getTotalAmount(it->first);

        if (_signalsEnabled) {
            emit(p->amtInQueuePerChannelSignal, getTotalAmount(it->first));
            emit(p->balancePerChannelSignal, p->balance);
            emit(p->numInflightPerChannelSignal, getTotalAmountIncomingInflight(it->first) +
                    getTotalAmountOutgoingInflight(it->first));
            emit(p->queueDelayEWMASignal, p->queueDelayEWMA);

            tuple<int, int> senderReceiverTuple = (id < myIndex()) ? make_tuple(id, myIndex()) :
                make_tuple(myIndex(), id);
            emit(p->capacityPerChannelSignal, _capacities[senderReceiverTuple]);
            emit(p->explicitRebalancingAmtPerChannelSignal, p->amtExplicitlyRebalanced/_statRate);
            emit(p->implicitRebalancingAmtPerChannelSignal, p->amtImplicitlyRebalanced/_statRate);
            
            p->amtExplicitlyRebalanced = 0;
            p->amtImplicitlyRebalanced = 0;
            
            emit(p->timeInFlightPerChannelSignal, p->sumTimeInFlight/p->timeInFlightSamples);
            p->sumTimeInFlight = 0;
            p->timeInFlightSamples = 0;
        }
    }
}

/* handler that is responsible for removing all the state associated
 * with a cancelled transaction once its grace period has passed
 * this included removal from outgoing/incoming units and any
 * queues
 */
void routerNodeBase::handleClearStateMessage(routerMsg* ttmsg){
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
            if (nextNode != -1) {   
                vector<tuple<int, double, routerMsg*, Id, simtime_t>>* queuedTransUnits = 
                    &(nodeToPaymentChannel[nextNode].queuedTransUnits);

                auto iterQueue = find_if((*queuedTransUnits).begin(),
                  (*queuedTransUnits).end(),
                  [&transactionId](const tuple<int, double, routerMsg*, Id, simtime_t>& p)
                  { return (get<0>(get<3>(p)) == transactionId); });
                
                // delete all occurences of this transaction in the queue
                // especially if there are splits
                if (iterQueue != (*queuedTransUnits).end()){
                    routerMsg * rMsg = get<2>(*iterQueue);
                    auto tMsg = rMsg->getEncapsulatedPacket();
                    rMsg->decapsulate();
                    delete tMsg;
                    delete rMsg;
                    nodeToPaymentChannel[nextNode].totalAmtInQueue -= get<1>(*iterQueue);
                    iterQueue = (*queuedTransUnits).erase(iterQueue);
                }
                
                make_heap((*queuedTransUnits).begin(), (*queuedTransUnits).end(), 
                        _schedulingAlgorithm);
            }

            // remove from incoming TransUnits from the previous node
            if (prevNode != -1) {
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
        if (simTime() > (msgArrivalTime + _maxTravelTime + _maxOneHopDelay)) {
            if (nextNode != -1) {
                unordered_map<Id, double, hashId> *outgoingTransUnits = 
                    &(nodeToPaymentChannel[nextNode].outgoingTransUnits);
                
                auto iterOutgoing = find_if((*outgoingTransUnits).begin(),
                  (*outgoingTransUnits).end(),
                  [&transactionId](const pair<tuple<int, int >, double> & p)
                  { return get<0>(p.first) == transactionId; });
                
                if (iterOutgoing != (*outgoingTransUnits).end()){
                    double amount = iterOutgoing -> second;
                    iterOutgoing = (*outgoingTransUnits).erase(iterOutgoing);
              
                    PaymentChannel *nextChannel = &(nodeToPaymentChannel[nextNode]);
                    nextChannel->totalAmtOutgoingInflight -= amount;
                    double updatedBalance = nextChannel->balance + amount;
                    setPaymentChannelBalanceByNode(nextNode, updatedBalance);
                    nextChannel->balanceEWMA = (1 -_ewmaFactor) * nextChannel->balanceEWMA + 
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

/* helper method to set a particular payment channel's balance to the passed in amount 
 */ 
void routerNodeBase::setPaymentChannelBalanceByNode(int node, double amt) {
       nodeToPaymentChannel[node].balance = amt;
}

##################################################################
/home/kush/Desktop/spiderproj/spider_omnet/routerNodeCeler.cc

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

########################################################################
/home/kush/Desktop/spiderproj/spider_omnet/routerNodeDCTCP.cc

#include "routerNodeDCTCP.h"

Define_Module(routerNodeDCTCP);

/* handles forwarding of  transactions out of the queue
 * the way other schemes' routers do except that it marks the packet
 * if the queue is larger than the threshold, therfore mostly similar to the base code */ 
int routerNodeDCTCP::forwardTransactionMessage(routerMsg *msg, int dest, simtime_t arrivalTime) {
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
    return routerNodeBase::forwardTransactionMessage(msg, dest, arrivalTime);
}

/* handler for the statistic message triggered every x seconds to also
 * output DCTCP scheme stats in addition to the default
 */
void routerNodeDCTCP::handleStatMessage(routerMsg* ttmsg) {
    if (_signalsEnabled) {
        // per router payment channel stats
        for ( auto it = nodeToPaymentChannel.begin(); it!= nodeToPaymentChannel.end(); it++){
            int node = it->first; //key
            PaymentChannel* p = &(nodeToPaymentChannel[node]);
        
            // get latest queueing delay
            auto lastTransTimes =  p->serviceArrivalTimeStamps.back();
            double curQueueingDelay = get<1>(lastTransTimes).dbl() - get<2>(lastTransTimes).dbl();
            p->queueDelayEWMA = 0.6*curQueueingDelay + 0.4*p->queueDelayEWMA;
            
            emit(p->queueDelayEWMASignal, p->queueDelayEWMA);
        }
    }

    // call the base method to output rest of the stats
    routerNodeBase::handleStatMessage(ttmsg);
}

##############################################################################
/home/kush/Desktop/spiderproj/spider_omnet/routerNodeDCTCPBal.cc

#include "routerNodeDCTCPBal.h"

Define_Module(routerNodeDCTCPBal);

/* handles transactions the way other schemes' routers do except that it marks the packet
 * if the available balance is less than the threshold (compared to capacity of the 
 * payment channel) */ 
void routerNodeDCTCPBal::handleTransactionMessage(routerMsg *ttmsg) {
    //increment nValue
    int hopcount = ttmsg->getHopCount();
    transactionMsg *transMsg = check_and_cast<transactionMsg *>(ttmsg->getEncapsulatedPacket());
    ttmsg->decapsulate();

    //not a self-message, add to incoming_trans_units
    int nextNode = ttmsg->getRoute()[hopcount + 1];
    
    // check the queue size before marking
    PaymentChannel *neighbor = &(nodeToPaymentChannel[nextNode]);
    vector<tuple<int, double , routerMsg *, Id, simtime_t>> *q;
    q = &(neighbor->queuedTransUnits);

    if (neighbor->balance < _balEcnThreshold * neighbor->origTotalCapacity)
        transMsg->setIsMarked(true); 
    
    ttmsg->encapsulate(transMsg);
    routerNodeBase::handleTransactionMessage(ttmsg);
}

##############################################################################
/home/kush/Desktop/spiderproj/spider_omnet/routerNodeLndBaseline.cc

#include "routerNodeLndBaseline.h"
Define_Module(routerNodeLndBaseline);

/* generateAckMessage that encapsulates transaction message to use for reattempts 
 * Note: this is different from the routerNodeBase version that will delete the
 * passed in transaction message */
routerMsg *routerNodeLndBaseline::generateAckMessage(routerMsg* ttmsg, bool isSuccess) {
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

###################################################################
/home/kush/Desktop/spiderproj/spider_omnet/routerNodePriceScheme.cc

#include "routerNodePriceScheme.h"

Define_Module(routerNodePriceScheme);

/* additional initialization that has to be done for the price based scheme
 * in particular set price variables to zero, initialize more signals
 * and schedule the first price update and price trigger
 */
void routerNodePriceScheme::initialize() {
    routerNodeBase::initialize();

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

        signal = registerSignalPerChannel("fakeRebalanceQ", key);
        nodeToPaymentChannel[key].fakeRebalanceQSignal = signal;

        signal = registerSignalPerChannel("serviceRate", key);
        nodeToPaymentChannel[key].serviceRateSignal = signal;

        signal = registerSignalPerChannel("arrivalRate", key);
        nodeToPaymentChannel[key].arrivalRateSignal = signal;
        
        signal = registerSignalPerChannel("inflightOutgoing", key);
        nodeToPaymentChannel[key].inflightOutgoingSignal = signal;

        signal = registerSignalPerChannel("inflightIncoming", key);
        nodeToPaymentChannel[key].inflightIncomingSignal = signal;
    }

    // trigger the first set of triggers for price update 
    routerMsg *triggerPriceUpdateMsg = generateTriggerPriceUpdateMessage();
    scheduleAt(simTime() + _tUpdate, triggerPriceUpdateMsg );
}


/********* MESSAGE GENERATORS **************/
/* generate the trigger message to initiate price Updates periodically
 */
routerMsg *routerNodePriceScheme::generateTriggerPriceUpdateMessage(){
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
routerMsg * routerNodePriceScheme::generatePriceUpdateMessage(double nLocal, double serviceRate, 
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


/******* MESSAGE HANDLERS **************************/

/* overall controller for handling messages that dispatches the right function
 * based on message type in price Scheme
 */
void routerNodePriceScheme::handleMessage(cMessage *msg) {
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
             if (_loggingEnabled) cout<< "[ROUTER "<< myIndex() 
                 <<": RECEIVED TRIGGER_PRICE_UPDATE MSG] "<< ttmsg->getName() << endl;
             handleTriggerPriceUpdateMessage(ttmsg);
             if (_loggingEnabled) cout<< "[AFTER HANDLING:]  "<< endl;
             break;

        case PRICE_UPDATE_MSG:
             if (_loggingEnabled) cout<< "[ROUTER "<< myIndex() 
                 <<": RECEIVED PRICE_UPDATE MSG] "<< ttmsg->getName() << endl;
             handlePriceUpdateMessage(ttmsg);
             if (_loggingEnabled) cout<< "[AFTER HANDLING:]  "<< endl;
             break;

        case PRICE_QUERY_MSG:
             if (_loggingEnabled) cout<< "[ROUTER "<< myIndex() 
                 <<": RECEIVED PRICE_QUERY MSG] "<< ttmsg->getName() << endl;
             handlePriceQueryMessage(ttmsg);
             if (_loggingEnabled) cout<< "[AFTER HANDLING:]  "<< endl;
             break;

        default:
             routerNodeBase::handleMessage(msg);

    }
}

/* handler for the statistic message triggered every x seconds to also
 * output the price based scheme stats in addition to the default
 */
void routerNodePriceScheme::handleStatMessage(routerMsg* ttmsg) {
    if (_signalsEnabled) {
        // per router payment channel stats
        for ( auto it = nodeToPaymentChannel.begin(); it!= nodeToPaymentChannel.end(); it++){
            int node = it->first; //key
            PaymentChannel* p = &(nodeToPaymentChannel[node]);
            emit(p->nValueSignal, p->lastNValue);
            emit(p->fakeRebalanceQSignal, p->fakeRebalancingQueue);
            emit(p->inflightOutgoingSignal, getTotalAmountOutgoingInflight(it->first));
            emit(p->inflightIncomingSignal, getTotalAmountIncomingInflight(it->first));
            emit(p->serviceRateSignal, p->arrivalRate/p->serviceRate);
            emit(p->lambdaSignal, p->lambda);
            emit(p->muLocalSignal, p->muLocal);
        }
    }
    routerNodeBase::handleStatMessage(ttmsg);
}

/* main routine for handling a new transaction under the pricing scheme,
 * updates the nvalue and then calls the usual handler
 */
void routerNodePriceScheme::handleTransactionMessage(routerMsg* ttmsg){ 
    int hopcount = ttmsg->getHopCount();
    transactionMsg *transMsg = check_and_cast<transactionMsg *>(ttmsg->getEncapsulatedPacket());
    int nextNode = ttmsg->getRoute()[hopcount + 1];
    nodeToPaymentChannel[nextNode].nValue += transMsg->getAmount();
    routerNodeBase::handleTransactionMessage(ttmsg);
}


/* handler for the trigger message that regularly fires to indicate
 * that it is time to recompute prices for all payment channels 
 * and let your neighbors know about the latest rates of incoming 
 * transactions for every one of them and wait for them to send
 * you the same before recomputing prices
 */
void routerNodePriceScheme::handleTriggerPriceUpdateMessage(routerMsg* ttmsg) {
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

        // correction for really large service/arrival rate initially
        if (neighborChannel->serviceArrivalTimeStamps.size() < 0.3 * _serviceArrivalWindow)
            serviceTimeDiff = 1;
        if (neighborChannel->arrivalTimeStamps.size() < 0.3 * _serviceArrivalWindow)
            arrivalTimeDiff = 1;
        
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
}

/* handler that handles the receipt of a priceUpdateMessage from a neighboring 
 * node that causes this node to update its prices for this particular
 * payment channel
 * Nesterov and secondOrderOptimization are two means to speed up the convergence
 */
void routerNodePriceScheme::handlePriceUpdateMessage(routerMsg* ttmsg){
    // compute everything to do with the neighbor
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

    // collect all local stats
    double xLocal = neighborChannel->xLocal;
    double updateRateLocal = neighborChannel->updateRate;
    double nLocal = neighborChannel->lastNValue;
    double inflightLocal = min(getTotalAmountOutgoingInflight(sender) + 
        updateRateLocal* _avgDelay/1000.0, _capacities[senderReceiverTuple]);
    double qLocal = neighborChannel->lastQueueSize;
    double serviceRateLocal = neighborChannel->serviceRate;
    double arrivalRateLocal = neighborChannel->arrivalRate;
 
    // collect this payment channel specific details
    tuple<int, int> node1node2Pair = (myIndex() < sender) ? make_tuple(myIndex(), sender) : make_tuple(sender,
            myIndex());
    double cValue = _capacities[node1node2Pair]; 
    double oldLambda = nodeToPaymentChannel[sender].lambda;
    double oldMuLocal = nodeToPaymentChannel[sender].muLocal;
    double oldMuRemote = nodeToPaymentChannel[sender].muRemote;
    
    // compute the new delta to the lambda
    double newLambdaGrad;
    if (_useQueueEquation) {
        newLambdaGrad = inflightLocal*arrivalRateLocal/serviceRateLocal + 
            inflightRemote * arrivalRateRemote/serviceRateRemote + 
            2*_xi*min(qLocal, qRemote) - (_capacityFactor * cValue);
    } else {
        newLambdaGrad = inflightLocal*arrivalRateLocal/serviceRateLocal + 
            inflightRemote * arrivalRateRemote/serviceRateRemote - (_capacityFactor * cValue); 
    }
        
    // compute the new delta to mulocal
    double newMuLocalGrad;
    if (_useQueueEquation) {
        newMuLocalGrad = nLocal - nRemote + qLocal*_tUpdate/_routerQueueDrainTime - 
                        qRemote*_tUpdate/_routerQueueDrainTime;
    } else {
        newMuLocalGrad = nLocal - nRemote;
    }
    
    // use all parts to compute the new mu/lambda taking a step in its direction
    double newLambda = 0.0;
    double newMuLocal = 0.0;
    double newMuRemote = 0.0;
    double myKappa = _kappa /**20.0 /cValue*/;
    double myEta = _eta /** 20.0 / cValue*/;
    
    newLambda = oldLambda +  myEta*newLambdaGrad;
    newMuLocal = oldMuLocal + myKappa*newMuLocalGrad;
    newMuRemote = oldMuRemote - myKappa*newMuLocalGrad; 

    // make all price variable non-negative
    nodeToPaymentChannel[sender].lambda = max(newLambda, 0.0);
    nodeToPaymentChannel[sender].muLocal = max(newMuLocal, 0.0);
    nodeToPaymentChannel[sender].muRemote = max(newMuRemote, 0.0);

    //delete both messages
    ttmsg->decapsulate();
    delete puMsg;
    delete ttmsg;
}


/* handler for a price query message which adds their prices to the
 * probe and then sends it forward to the next node
 */
void routerNodePriceScheme::handlePriceQueryMessage(routerMsg* ttmsg){
    priceQueryMsg *pqMsg = check_and_cast<priceQueryMsg *>(ttmsg->getEncapsulatedPacket());
    bool isReversed = pqMsg->getIsReversed();
    
    if (!isReversed) { 
        int nextNode = ttmsg->getRoute()[ttmsg->getHopCount()+1];
        double zOld = pqMsg->getZValue();
        double lambda = nodeToPaymentChannel[nextNode].lambda;
        double muLocal = nodeToPaymentChannel[nextNode].muLocal;
        double muRemote = nodeToPaymentChannel[nextNode].muRemote;
        double zNew = zOld;

        if (ttmsg->getHopCount() < ttmsg->getRoute().size() - 2) {
            // ignore end host links
            zNew += (2 * lambda) + muLocal  - muRemote;
        }
        pqMsg->setZValue(zNew);
        forwardMessage(ttmsg);
    }
    else {
        forwardMessage(ttmsg);
    }
}

##################################################################
/home/kush/Desktop/spiderproj/spider_omnet/routerNodeWaterfilling.cc

#include "routerNodeWaterfilling.h"

Define_Module(routerNodeWaterfilling);

/* overall controller for handling messages that dispatches the right function
 * based on message type in waterfilling
 */
void routerNodeWaterfilling::handleMessage(cMessage *msg) {
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
             if (_loggingEnabled) cout<< "[ROUTER "<< myIndex() 
                 <<": RECEIVED PROBE MSG] "<< ttmsg->getName() << endl;
             handleProbeMessage(ttmsg);
             if (_loggingEnabled) cout<< "[AFTER HANDLING:]  "<< endl;
             break;
        default:
             routerNodeBase::handleMessage(msg);
    }
}

/* method to receive and forward probe messages to the next node */
void routerNodeWaterfilling::handleProbeMessage(routerMsg* ttmsg){
    probeMsg *pMsg = check_and_cast<probeMsg *>(ttmsg->getEncapsulatedPacket());
    bool isReversed = pMsg->getIsReversed();
    int nextDest = ttmsg->getRoute()[ttmsg->getHopCount()+1];
    forwardProbeMessage(ttmsg);
}

/* forwards probe messages 
 * after appending this channel's balances to the probe message on the
 * way back
 */
void routerNodeWaterfilling::forwardProbeMessage(routerMsg *msg){
    int prevDest = msg->getRoute()[msg->getHopCount() - 1];
    bool updateOnReverse = true;
    int nextDest = msg->getRoute()[msg->getHopCount()];
    
    probeMsg *pMsg = check_and_cast<probeMsg *>(msg->getEncapsulatedPacket());
    msg->setHopCount(msg->getHopCount()+1);

    if (pMsg->getIsReversed() == true && updateOnReverse == true){
        vector<double> *pathBalances = & ( pMsg->getPathBalances());
        (*pathBalances).push_back(nodeToPaymentChannel[prevDest].balance);
    } 
    else if (pMsg->getIsReversed() == false && updateOnReverse == false) {
        vector<double> *pathBalances = & ( pMsg->getPathBalances());
        (*pathBalances).push_back(nodeToPaymentChannel[nextDest].balance);
    }
    send(msg, nodeToPaymentChannel[nextDest].gate);
}

SUMMARY ################################################

# Developer Manual: Router Node Implementation in Spider Project

This manual provides a detailed and comprehensive understanding of the implementation of router nodes in the Spider project. It focuses on the `.cc` (implementation) files, which contain the core logic for transaction processing, routing algorithms, signal handling, and other functionalities. Developers reading this manual will gain an in-depth understanding of how these components work together to achieve the desired behavior.

---

## 1. **Overview of Router Node Implementation**

The `.cc` files define the behavior of router nodes, which are responsible for forwarding transactions, managing payment channels, rebalancing operations, and collecting statistics. These files implement the methods declared in the corresponding `.h` header files. The key responsibilities of router nodes include:

- **Transaction Forwarding**: Receiving, validating, and forwarding transactions along their paths.
- **Payment Channel Management**: Maintaining channel balances, handling rebalancing, and updating states based on incoming messages.
- **Routing Logic**: Implementing various routing algorithms (e.g., Celer, DCTCP, Price Scheme, etc.).
- **Signal Handling**: Collecting statistics and emitting signals for monitoring performance.
- **Error Handling**: Managing timeouts, retries, and failures during transaction forwarding.

---

## 2. **Key Components and Methods**

### 2.1 **Initialization Functions**
These functions set up the initial state of the router node, including payment channels, paths, and global parameters.

#### `initialize()`
- **Purpose**: Initializes global parameters, payment channels, and signals for statistics collection.
- **Details**:
  - Reads configuration parameters such as `_simulationLength`, `_statRate`, `_timeoutEnabled`, etc.
  - Sets up payment channels using `initializeMyChannels()`.
  - Registers signals for monitoring metrics like wallet balances, transit amounts, and queue delays.
  - Example:
    ```cpp
    void routerNodeBase::initialize() {
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
    void routerNodeDCTCP::initializePathInfo(vector<vector<int>> kShortestPaths, int destNode) {
        for (int pathIdx = 0; pathIdx < kShortestPaths.size(); pathIdx++) {
            PathInfo temp = {};
            temp.path = kShortestPaths[pathIdx];
            temp.window = _minDCTCPWindow;
            nodeToShortestPathsMap[destNode][pathIdx] = temp;
        }
    }
    ```

---

### 2.2 **Transaction Forwarding and Handling**

#### `forwardTransactionMessage()`
- **Purpose**: Forwards transaction messages along their paths while applying routing-specific logic.
- **Details**:
  - Validates the transaction and checks if it can be forwarded.
  - Updates payment channel balances and emits relevant signals.
  - Example:
    ```cpp
    int routerNodeDCTCP::forwardTransactionMessage(routerMsg *msg, int dest, simtime_t arrivalTime) {
        transactionMsg *transMsg = check_and_cast<transactionMsg *>(msg->getEncapsulatedPacket());
        int nextDest = msg->getRoute()[msg->getHopCount() + 1];
        PaymentChannel *neighbor = &(nodeToPaymentChannel[nextDest]);
        if (neighbor->balance <= 0 || transMsg->getAmount() > neighbor->balance) {
            return 0; // Cannot forward due to insufficient balance
        }
        updateChannelState(neighbor, transMsg->getAmount());
        forwardMessage(msg);
        return 1;
    }
    ```

#### `handleAckMessage()`
- **Purpose**: Processes acknowledgment messages for completed transactions.
- **Details**:
  - Updates payment channel balances and acknowledgment states.
  - Triggers update messages for subsequent nodes on the path.
  - Example:
    ```cpp
    void routerNodeBase::handleAckMessage(routerMsg *ttmsg) {
        ackMsg *aMsg = check_and_cast<ackMsg *>(ttmsg->getEncapsulatedPacket());
        int prevNode = ttmsg->getRoute()[ttmsg->getHopCount() - 1];
        updatePaymentChannelBalances(prevNode, aMsg->getAmount());
        triggerUpdateMessage(aMsg);
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
  - `handleStatMessage()`: Outputs DCTCP-specific statistics like queue delays and window sizes.

#### **Price Scheme**
- **Key Methods**:
  - `computeProjection()`: Adjusts rates to ensure feasibility.
  - `handleTriggerPriceUpdateMessage()`: Manages price update triggers.
  - `handlePriceUpdateMessage()`: Processes incoming price update messages from neighbors.

#### **Waterfilling**
- **Key Methods**:
  - `generateProbeMessage()`: Creates probe messages to measure path conditions.
  - `updatePathProbabilities()`: Adjusts probabilities based on measured balances.
  - `forwardProbeMessage()`: Forwards probe messages to assess path conditions.

---

### 2.4 **Signal Handling and Statistics Collection**

#### `registerSignalPerChannel()`
- **Purpose**: Registers signals for monitoring payment channel metrics.
- **Details**:
  - Creates unique signal names for each channel and registers them with the simulation environment.
  - Example:
    ```cpp
    simsignal_t routerNodeBase::registerSignalPerChannel(string signalStart, int id) {
        char signalName[64];
        sprintf(signalName, "%s(router %d)", signalStart.c_str(), id);
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
    void routerNodeDCTCP::handleStatMessage(routerMsg *ttmsg) {
        if (_signalsEnabled) {
            for (auto &p : nodeToPaymentChannel) {
                emit(p.second.queueDelayEWMA, p.second.queueDelayEWMA);
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
    bool routerNodeBase::handleTransactionMessageTimeOut(routerMsg *msg) {
        if (_timeoutEnabled) {
            // Remove transaction from queue and log failure
            return true;
        }
        return false;
    }
    ```

#### `handleAckMessageTimeOut()`
- **Purpose**: Processes acknowledgment timeouts for failed transactions.
- **Details**:
  - Updates payment channel states and triggers retries.
  - Example:
    ```cpp
    void routerNodeBase::handleAckMessageTimeOut(routerMsg *msg) {
        ackMsg *aMsg = check_and_cast<ackMsg *>(msg->getEncapsulatedPacket());
        retryTransaction(aMsg->getTransactionId());
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
    void routerNodeBase::performRebalancing() {
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
    void routerNodeBase::updatePaths(int destNode) {
        vector<vector<int>> newPaths = getKShortestRoutes(myIndex(), destNode, _maxPathsToConsider);
        initializePathInfo(newPaths, destNode);
    }
    ```

---

## 4. **Conclusion**

The `.cc` files for router nodes in the Spider project encapsulate the core logic for transaction forwarding, routing, and monitoring. By understanding the initialization, transaction handling, routing algorithms, and signal management implemented in these files, developers can effectively contribute to and maintain the project. This manual provides a solid foundation for navigating and extending the functionality of router nodes in the Spider project.