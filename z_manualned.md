/home/kush/Desktop/spiderproj/spider_omnet/hostNodeBase.ned

simple hostNodeBase
{
	parameters:
	    bool isHost = true;
	    
		string topologyFile = default("sample-topology.txt");
		string workloadFile = default("sample-workload.txt");
                string resultPrefix = default("");

		double simulationLength = default(30.0);
		double statRate = default(0.2);
		bool signalsEnabled = default(false);
		bool loggingEnabled = default(false);
                double splitSize = default(5.0);

		bool timeoutEnabled = default(true);
		double timeoutClearRate = default(0.5);

		bool celerEnabled = default(false);
		bool waterfillingEnabled = default(false);
		bool priceSchemeEnabled = default(false);
		bool smoothWaterfillingEnabled = default(false);
                bool landmarkRoutingEnabled = default(false);
                bool lndBaselineEnabled = default(false);
                bool dctcpEnabled = default(false);

                bool widestPathsEnabled = default(false);
                bool heuristicPathsEnabled = default(false);
                bool obliviousRoutingEnabled = default(false);
                bool kspYenEnabled = default(false);
                
                bool LIFOEnabled = default(false);
                bool FIFOEnabled = default(false);
                bool SPFEnabled = default(false);
                bool RREnabled = default(false);
                bool EDFEnabled = default(false);
                
                bool changingPathsEnabled = default(false);
                int maxPathsToConsider = default(100);
                double windowThresholdForChange = default(1);
                double pathMonitorRate = default(5);
               
                bool rebalancingEnabled = default(false);
                double queueDelayThreshold = default(3.0);
                double gamma = default(1.0);
                double gammaImbalanceQueueSize = default(5.0);
                double rebalancingRate = default(10.0);
                double rebalancingDelayForAddingFunds = default(0.1);
                double minBalanceComputeRate = default(0.1);

                bool windowEnabled = default(false);
		int numPathChoices = default(4);
                
                double transStatStart = default(3000);
                double transStatEnd = default(5000);

                double alpha = default(0.01);
                double kappa = default(0.01);
                double eta = default(0.01);
                double zeta = default(0.01);
                double rhoValue = default(0.05);
                double minRate = default(0.25);
                double updateQueryTime = default(0.8);
                double xi = default(1);
                int serviceArrivalWindow = default(100);
                double routerQueueDrainTime = default(1);
                
                bool splittingEnabled = default(false);
                double tau = default(10);
                double normalizer = default(100);

		
		@signal[completionTime](type="simtime_t"); 
		@statistic[completionTime](title="completionTime"; source="completionTime"; record=vector,stats; interpolationmode=none); 
		
		@signal[numCleared](type="long"); 
		@statistic[numCleared](title="numCleared"; source="numCleared"; record=vector,stats; interpolationmode=none); 
		
		@display("i=block/routing"); 

		@signal[implicitRebalancingAmtPerChannel*](type="double"); 
		@statisticTemplate[implicitRebalancingAmtPerChannelTemplate](record=vector, stats);  
		@signal[explicitRebalancingAmtPerChannel*](type="double"); 
		@statisticTemplate[explicitRebalancingAmtPerChannelTemplate](record=vector, stats);  
		
		@signal[numInQueuePerChannel*](type="unsigned long"); 
		@statisticTemplate[numInQueuePerChannelTemplate](record=vector,stats);   
                
                @signal[numInflightPerChannel*](type="unsigned long"); 
		@statisticTemplate[numInflightPerChannelTemplate](record=vector, stats);

		@signal[numProcessedPerChannel*](type="long");  
		@statisticTemplate[numProcessedPerChannelTemplate](record=vector, stats);  

 		@signal[probabilityPerDestPerPath*](type="double");  
		@statisticTemplate[probabilityPerDestPerPathTemplate](record=vector, stats);  

		@signal[numSentPerChannel*](type="long");  
		@statisticTemplate[numSentPerChannelTemplate](record=vector, stats);   

		@signal[rateCompletedPerDest*](type="long");  
		@statisticTemplate[rateCompletedPerDestTemplate](record=vector, stats);   
		
		@signal[numCompletedPerDest*](type="long");  
		@statisticTemplate[numCompletedPerDestTemplate](record=vector, stats);   

		@signal[numPendingPerDest*](type="long");  
		@statisticTemplate[numPendingPerDestTemplate](record=vector, stats); 

 		@signal[numWaitingPerDest*](type="long");  
		@statisticTemplate[numWaitingPerDestTemplate](record=vector, stats);  
	
		@signal[fracSuccessfulPerDest*](type="long");  
		@statisticTemplate[fracSuccessfulPerDestTemplate](record=vector, stats);   
		
		@signal[pathPerTransPerDest*](type="long");  
		@statisticTemplate[pathPerTransPerDestTemplate](record=vector, stats);   
		
		@signal[numTimedOutPerDest*](type="long");  
		@statisticTemplate[numTimedOutPerDestTemplate](record=vector, stats);

		@signal[demandEstimatePerDest*](type="double");  
		@statisticTemplate[demandEstimatePerDestTemplate](record=vector, stats);

		@signal[numTimedOutAtSenderPerDest*](type="long");  
		@statisticTemplate[numTimedOutAtSenderPerDestTemplate](record=vector, stats);
	
        	@signal[rateAttemptedPerDest*](type="long"); 
		@statisticTemplate[rateAttemptedPerDestTemplate](record=vector, stats);   
		
                @signal[rateArrivedPerDest*](type="long"); 
		@statisticTemplate[rateArrivedPerDestTemplate](record=vector, stats);  
	
		@signal[numArrivedPerDest*](type="long"); 
		@statisticTemplate[numArrivedPerDestTemplate](record=vector, stats);   

		@signal[balancePerChannel*](type="double"); 
		@statisticTemplate[balancePerChannelTemplate](record=vector, stats);   
		
		@signal[capacityPerChannel*](type="double"); 
		@statisticTemplate[capacityPerChannelTemplate](record=vector, stats);  

 		@signal[bankPerChannel*](type="double"); 
		@statisticTemplate[bankPerChannelTemplate](record=vector, stats); 
		
                @signal[timeInFlightPerChannel*](type="double"); 
		@statisticTemplate[timeInFlightPerChannelTemplate](record=vector, stats);   
		
                @signal[bottleneckPerDestPerPath*](type="long"); 
		@statisticTemplate[bottleneckPerDestPerPathTemplate](record=vector, stats);
		
		@signal[probeBackPerDestPerPath*](type="long"); 
		@statisticTemplate[probeBackPerDestPerPathTemplate](record=vector, stats);
		
		@signal[rateCompletedPerDestPerPath*](type="long"); 
		@statisticTemplate[rateCompletedPerDestPerPathTemplate](record=vector, stats);
		
		@signal[rateAttemptedPerDestPerPath*](type="long"); 
		@statisticTemplate[rateAttemptedPerDestPerPathTemplate](record=vector, stats);
		
		//has queue capacity specific signals
		@signal[numFailedPerDest*](type="long");  
		@statisticTemplate[numFailedPerDestTemplate](record=vector, stats);
		
		@signal[rateFailedPerDest*](type="long");  
		@statisticTemplate[rateFailedPerDestTemplate](record=vector, stats);
		
		//price scheme payment channel specific signals
		@signal[nValuePerChannel*](type="long"); 
		@statisticTemplate[nValuePerChannelTemplate](record=vector, stats);
		
		@signal[xLocalPerChannel*](type="double"); 
		@statisticTemplate[xLocalPerChannelTemplate](record=vector, stats);

		@signal[lambdaPerChannel*](type="double"); 
		@statisticTemplate[lambdaPerChannelTemplate](record=vector, stats);
		
		@signal[muLocalPerChannel*](type="double"); 
		@statisticTemplate[muLocalPerChannelTemplate](record=vector, stats);
		
		@signal[muRemotePerChannel*](type="double"); 
		@statisticTemplate[muRemotePerChannelTemplate](record=vector, stats);
		
		//price scheme path specific signals
		@signal[rateToSendTransPerDestPerPath*](type="double"); 
		@statisticTemplate[rateToSendTransPerDestPerPathTemplate](record=vector, stats);
		
		@signal[timeToNextSendPerDestPerPath*](type="simtime_t"); 
		@statisticTemplate[timeToNextSendPerDestPerPathTemplate](record=vector, stats);
		
		@signal[sumOfTransUnitsInFlightPerDestPerPath*](type="double"); 
		@statisticTemplate[sumOfTransUnitsInFlightPerDestPerPathTemplate](record=vector, stats);
		
		@signal[priceLastSeenPerDestPerPath*](type="double"); 
		@statisticTemplate[priceLastSeenPerDestPerPathTemplate](record=vector, stats);
		
		@signal[isSendTimerSetPerDestPerPath*](type="long"); 
		@statisticTemplate[isSendTimerSetPerDestPerPathTemplate](record=vector, stats);
		
		
		

	gates:
		input in[];
		output out[];
}


SUMMARY #########################################

# Developer Manual: `hostNodeBase` in Spider Project

This manual provides a comprehensive, developer-level understanding of the `hostNodeBase` module in the Spider project. The `hostNodeBase` serves as the foundational class for all host nodes, defining their behavior, parameters, and interactions within the network. This document explains each component in detail, enabling developers to understand and extend the functionality of this module.

---

## 1. **Overview**

The `hostNodeBase` is the base class for all host nodes in the Spider project. It encapsulates essential functionalities such as transaction generation, routing logic, payment channel management, signal handling, and statistics collection. This module is highly configurable, allowing developers to enable or disable specific features (e.g., Celer, Waterfilling, Price Scheme) via parameters.

---

## 2. **Parameters**

### 2.1 **General Parameters**
These parameters define the global configuration of the host node:

- **`isHost`**:  
  - Type: `bool`  
  - Default: `true`  
  - Description: Indicates whether the node is a host (`true`) or a router (`false`). Hosts are responsible for initiating transactions, while routers forward them.

- **`topologyFile`**:  
  - Type: `string`  
  - Default: `"sample-topology.txt"`  
  - Description: Specifies the file containing the network topology, including payment channels and capacities.

- **`workloadFile`**:  
  - Type: `string`  
  - Default: `"sample-workload.txt"`  
  - Description: Specifies the file containing the workload, which includes transaction details like sender, receiver, amount, and timestamp.

- **`resultPrefix`**:  
  - Type: `string`  
  - Default: `""`  
  - Description: Prefix for naming output files, enabling easier result categorization.

- **`simulationLength`**:  
  - Type: `double`  
  - Default: `30.0`  
  - Description: Duration of the simulation in seconds.

- **`statRate`**:  
  - Type: `double`  
  - Default: `0.2`  
  - Description: Interval (in seconds) at which statistics are collected and logged.

- **`signalsEnabled`**:  
  - Type: `bool`  
  - Default: `false`  
  - Description: Enables or disables signal emission for monitoring metrics like completion times, queue sizes, and balances.

- **`loggingEnabled`**:  
  - Type: `bool`  
  - Default: `false`  
  - Description: Enables or disables logging for debugging purposes.

- **`splitSize`**:  
  - Type: `double`  
  - Default: `5.0`  
  - Description: Maximum size of transaction splits when splitting large transactions into smaller ones.

---

### 2.2 **Feature-Specific Parameters**
These parameters control the activation and behavior of specific routing and transaction processing algorithms:

- **Timeout Handling**:
  - `timeoutEnabled`: Enables or disables timeout mechanisms for transactions.
  - `timeoutClearRate`: Rate at which timeouts are cleared.

- **Routing Algorithms**:
  - `celerEnabled`: Enables the Celer protocol.
  - `waterfillingEnabled`: Enables water-filling-based load balancing.
  - `priceSchemeEnabled`: Activates price-based routing.
  - `smoothWaterfillingEnabled`: Enables smooth water-filling with EWMA (Exponential Weighted Moving Average).
  - `landmarkRoutingEnabled`: Activates landmark routing.
  - `lndBaselineEnabled`: Enables the Lightning Network Daemon (LND) baseline routing.
  - `dctcpEnabled`: Activates DCTCP-based congestion control.

- **Path Selection**:
  - `widestPathsEnabled`: Prioritizes paths with the highest capacity.
  - `heuristicPathsEnabled`: Uses heuristic-based path selection.
  - `obliviousRoutingEnabled`: Ignores current network conditions during routing.
  - `kspYenEnabled`: Uses K-shortest paths generated by Yen's algorithm.

- **Scheduling Policies**:
  - `LIFOEnabled`, `FIFOEnabled`, `SPFEnabled`, `RREnabled`, `EDFEnabled`: Various scheduling policies for transaction queues.

- **Dynamic Path Changes**:
  - `changingPathsEnabled`: Allows dynamic path changes based on network conditions.
  - `maxPathsToConsider`: Maximum number of paths considered for routing.
  - `windowThresholdForChange`: Threshold for triggering path changes.
  - `pathMonitorRate`: Rate at which paths are monitored for performance.

---

### 2.3 **Rebalancing Parameters**
These parameters control rebalancing operations to maintain optimal channel states:

- **`rebalancingEnabled`**: Enables or disables rebalancing.
- **`queueDelayThreshold`**: Threshold for triggering rebalancing due to high queue delays.
- **`gamma`**: Weight factor for rebalancing decisions.
- **`gammaImbalanceQueueSize`**: Queue size threshold for imbalance detection.
- **`rebalancingRate`**: Rate at which rebalancing operations are performed.
- **`rebalancingDelayForAddingFunds`**: Delay before adding funds during rebalancing.
- **`minBalanceComputeRate`**: Minimum rate for computing balances during rebalancing.

---

### 2.4 **Advanced Parameters**
These parameters fine-tune specific behaviors:

- **Transaction Splitting**:
  - `splittingEnabled`: Enables splitting large transactions into smaller ones.
  - `tau`, `normalizer`: Parameters for smooth water-filling and transaction splitting.

- **Price Scheme**:
  - `alpha`, `kappa`, `eta`, `zeta`: Tuning parameters for price calculations.
  - `rhoValue`: Factor for demand estimation.
  - `minRate`: Minimum acceptable rate for transactions.
  - `updateQueryTime`: Interval for updating price queries.

- **Router-Specific Parameters**:
  - `routerQueueDrainTime`: Time required to drain queues in routers.

---

## 3. **Signals**

The `hostNodeBase` emits various signals for monitoring and collecting statistics. These signals are categorized by their purpose:

### 3.1 **Completion and Timing**
- `completionTime`: Records the time taken to complete transactions.
- `numCleared`: Tracks the number of transactions cleared.

### 3.2 **Queue and Channel Metrics**
- `implicitRebalancingAmtPerChannel*`: Amount of implicit rebalancing per channel.
- `explicitRebalancingAmtPerChannel*`: Amount of explicit rebalancing per channel.
- `numInQueuePerChannel*`: Number of transactions in the queue per channel.
- `numInflightPerChannel*`: Number of transactions in flight per channel.
- `numProcessedPerChannel*`: Number of transactions processed per channel.
- `numSentPerChannel*`: Number of transactions sent per channel.

### 3.3 **Destination-Specific Metrics**
- `rateCompletedPerDest*`: Rate of completed transactions per destination.
- `numCompletedPerDest*`: Number of completed transactions per destination.
- `numPendingPerDest*`: Number of pending transactions per destination.
- `fracSuccessfulPerDest*`: Fraction of successful transactions per destination.
- `demandEstimatePerDest*`: Estimated demand per destination.

### 3.4 **Path-Specific Metrics**
- `bottleneckPerDestPerPath*`: Bottleneck capacity per path.
- `probeBackPerDestPerPath*`: Probe messages received back per path.
- `rateCompletedPerDestPerPath*`: Rate of completed transactions per path.

---

## 4. **Gates**

The `hostNodeBase` defines two types of gates for communication:

- **`input in[]`**: Input gates for receiving messages from other nodes.
- **`output out[]`**: Output gates for sending messages to other nodes.

These gates enable the node to interact with its neighbors in the network.

---

## 5. **Key Functionalities**

### 5.1 **Initialization**
- **`initialize()`**: Sets up global parameters, payment channels, and signals. It also reads configuration files (`topologyFile` and `workloadFile`) to initialize the network state.

### 5.2 **Transaction Generation**
- **`generateNextTransaction()`**: Reads the next transaction from the workload file, creates a transaction message, and schedules it for transmission.

### 5.3 **Routing Logic**
- Implements various routing algorithms (e.g., Celer, DCTCP, Landmark Routing) based on enabled parameters.

### 5.4 **Payment Channel Management**
- Manages payment channel balances, handles rebalancing, and updates states based on incoming messages.

### 5.5 **Signal Handling**
- Collects statistics and emits signals for monitoring performance metrics.

---

## 6. **Conclusion**

The `hostNodeBase` module provides a robust foundation for implementing host nodes in the Spider project. Its modular design allows developers to enable or disable specific features, making it highly adaptable to different use cases. By understanding the parameters, signals, and functionalities described in this manual, developers can effectively contribute to and maintain the project.

####################################################################
/home/kush/Desktop/spiderproj/spider_omnet/hostNodeCeler.ned

simple hostNodeCeler
{
	parameters:
	    bool isHost = true;
	    
		string topologyFile = default("sample-topology.txt");
		string workloadFile = default("sample-workload.txt");

		double simulationLength = default(30.0);
		double statRate = default(0.2);
		bool signalsEnabled = default(false);
		bool loggingEnabled = default(true);
                double splitSize = default(5.0);

		bool timeoutEnabled = default(false);
		double timeoutClearRate = default(0.5);

		bool celerEnabled = default(true);
		bool waterfillingEnabled = default(false);
		bool priceSchemeEnabled = default(false);
		bool smoothWaterfillingEnabled = default(false);
                bool landmarkRoutingEnabled = default(false);
                bool lndBaselineEnabled = default(false);
                bool dctcpEnabled = default(false);

                bool widestPathsEnabled = default(false);
                bool heuristicPathsEnabled = default(false);
                bool obliviousRoutingEnabled = default(false);
                bool kspYenEnabled = default(false);
                
                bool LIFOEnabled = default(true);
                bool FIFOEnabled = default(false);
                bool SPFEnabled = default(false);
                bool RREnabled = default(false);
                bool EDFEnabled = default(false);
                
                bool changingPathsEnabled = default(false);
                int maxPathsToConsider = default(100);
                double windowThresholdForChange = default(1);
                double pathMonitorRate = default(5);
               
                bool rebalancingEnabled = default(false);
                double queueDelayThreshold = default(3.0);
                double gamma = default(1.0);
                double gammaImbalanceQueueSize = default(5.0);
                double rebalancingRate = default(10.0);
                double rebalancingDelayForAddingFunds = default(0.1);
                double minBalanceComputeRate = default(0.1);

                bool windowEnabled = default(false);
		int numPathChoices = default(4);
                
                double transStatStart = default(3000);
                double transStatEnd = default(5000);

                double alpha = default(0.01);
                double kappa = default(0.01);
                double eta = default(0.01);
                double zeta = default(0.01);
                double rhoValue = default(0.05);
                double minRate = default(0.25);
                double updateQueryTime = default(0.8);
                double xi = default(1);
                int serviceArrivalWindow = default(100);
                double routerQueueDrainTime = default(1);
                
                bool splittingEnabled = default(true);
                double tau = default(10);
                double normalizer = default(100);

		
		@signal[completionTime](type="simtime_t"); 
		@statistic[completionTime](title="completionTime"; source="completionTime"; record=vector,stats; interpolationmode=none); 
		
		@signal[numCleared](type="long"); 
		@statistic[numCleared](title="numCleared"; source="numCleared"; record=vector,stats; interpolationmode=none); 
		
		@display("i=block/routing"); 

		@signal[implicitRebalancingAmtPerChannel*](type="double"); 
		@statisticTemplate[implicitRebalancingAmtPerChannelTemplate](record=vector, stats);  
		@signal[explicitRebalancingAmtPerChannel*](type="double"); 
		@statisticTemplate[explicitRebalancingAmtPerChannelTemplate](record=vector, stats);  
		
		@signal[numInQueuePerChannel*](type="double"); 
		@statisticTemplate[numInQueuePerChannelTemplate](record=vector,stats);   
                
                @signal[numInflightPerChannel*](type="double"); 
		@statisticTemplate[numInflightPerChannelTemplate](record=vector, stats);

		@signal[numProcessedPerChannel*](type="long");  
		@statisticTemplate[numProcessedPerChannelTemplate](record=vector, stats);  

 		@signal[probabilityPerDestPerPath*](type="double");  
		@statisticTemplate[probabilityPerDestPerPathTemplate](record=vector, stats);  

		@signal[numSentPerChannel*](type="long");  
		@statisticTemplate[numSentPerChannelTemplate](record=vector, stats);   

		@signal[rateCompletedPerDest*](type="long");  
		@statisticTemplate[rateCompletedPerDestTemplate](record=vector, stats);   
		
		@signal[numCompletedPerDest*](type="long");  
		@statisticTemplate[numCompletedPerDestTemplate](record=vector, stats);   

		@signal[numPendingPerDest*](type="long");  
		@statisticTemplate[numPendingPerDestTemplate](record=vector, stats); 

 		@signal[numWaitingPerDest*](type="long");  
		@statisticTemplate[numWaitingPerDestTemplate](record=vector, stats);  
	
		@signal[fracSuccessfulPerDest*](type="double");  
		@statisticTemplate[fracSuccessfulPerDestTemplate](record=vector, stats);   
		
		@signal[pathPerTransPerDest*](type="long");  
		@statisticTemplate[pathPerTransPerDestTemplate](record=vector, stats);   
		
		@signal[numTimedOutPerDest*](type="long");  
		@statisticTemplate[numTimedOutPerDestTemplate](record=vector, stats);

		@signal[demandEstimatePerDest*](type="double");  
		@statisticTemplate[demandEstimatePerDestTemplate](record=vector, stats);

		@signal[numTimedOutAtSenderPerDest*](type="long");  
		@statisticTemplate[numTimedOutAtSenderPerDestTemplate](record=vector, stats);
	
        	@signal[rateAttemptedPerDest*](type="long"); 
		@statisticTemplate[rateAttemptedPerDestTemplate](record=vector, stats);   
		
                @signal[rateArrivedPerDest*](type="long"); 
		@statisticTemplate[rateArrivedPerDestTemplate](record=vector, stats);  
	
		@signal[numArrivedPerDest*](type="long"); 
		@statisticTemplate[numArrivedPerDestTemplate](record=vector, stats);   

		@signal[balancePerChannel*](type="double"); 
		@statisticTemplate[balancePerChannelTemplate](record=vector, stats);   
		
		@signal[capacityPerChannel*](type="double"); 
		@statisticTemplate[capacityPerChannelTemplate](record=vector, stats);  

 		@signal[bankPerChannel*](type="double"); 
		@statisticTemplate[bankPerChannelTemplate](record=vector, stats); 
		
                @signal[timeInFlightPerChannel*](type="double"); 
		@statisticTemplate[timeInFlightPerChannelTemplate](record=vector, stats);   
		
                @signal[bottleneckPerDestPerPath*](type="long"); 
		@statisticTemplate[bottleneckPerDestPerPathTemplate](record=vector, stats);
		
		@signal[probeBackPerDestPerPath*](type="long"); 
		@statisticTemplate[probeBackPerDestPerPathTemplate](record=vector, stats);
		
		@signal[rateCompletedPerDestPerPath*](type="long"); 
		@statisticTemplate[rateCompletedPerDestPerPathTemplate](record=vector, stats);
		
		@signal[rateAttemptedPerDestPerPath*](type="long"); 
		@statisticTemplate[rateAttemptedPerDestPerPathTemplate](record=vector, stats);
		
		//has queue capacity specific signals
		@signal[numFailedPerDest*](type="long");  
		@statisticTemplate[numFailedPerDestTemplate](record=vector, stats);
		
		@signal[rateFailedPerDest*](type="long");  
		@statisticTemplate[rateFailedPerDestTemplate](record=vector, stats);
		
		//price scheme payment channel specific signals
		@signal[nValuePerChannel*](type="long"); 
		@statisticTemplate[nValuePerChannelTemplate](record=vector, stats);
		
		@signal[xLocalPerChannel*](type="double"); 
		@statisticTemplate[xLocalPerChannelTemplate](record=vector, stats);

		@signal[lambdaPerChannel*](type="double"); 
		@statisticTemplate[lambdaPerChannelTemplate](record=vector, stats);
		
		@signal[muLocalPerChannel*](type="double"); 
		@statisticTemplate[muLocalPerChannelTemplate](record=vector, stats);
		
		@signal[muRemotePerChannel*](type="double"); 
		@statisticTemplate[muRemotePerChannelTemplate](record=vector, stats);
		
		//price scheme path specific signals
		@signal[rateToSendTransPerDestPerPath*](type="double"); 
		@statisticTemplate[rateToSendTransPerDestPerPathTemplate](record=vector, stats);
		
		@signal[timeToNextSendPerDestPerPath*](type="simtime_t"); 
		@statisticTemplate[timeToNextSendPerDestPerPathTemplate](record=vector, stats);
		
		@signal[sumOfTransUnitsInFlightPerDestPerPath*](type="double"); 
		@statisticTemplate[sumOfTransUnitsInFlightPerDestPerPathTemplate](record=vector, stats);
		
		@signal[priceLastSeenPerDestPerPath*](type="double"); 
		@statisticTemplate[priceLastSeenPerDestPerPathTemplate](record=vector, stats);
		
		@signal[isSendTimerSetPerDestPerPath*](type="long"); 
		@statisticTemplate[isSendTimerSetPerDestPerPathTemplate](record=vector, stats);
		
		
		//celer network signals
		@signal[cpiPerChannelPerDest*](type="double"); 
		@statisticTemplate[cpiPerChannelPerDestTemplate](record=vector, stats);
	
		@signal[destQueuePerDest*](type="double"); 
		@statisticTemplate[destQueuePerDestTemplate](record=vector, stats);
		
		@signal[queueTimedOutPerDest*](type="double"); 
		@statisticTemplate[queueTimedOutPerDestTemplate](record=vector, stats);
		

		@signal[kStarPerChannel*](type="long"); 
		@statisticTemplate[kStarPerChannelTemplate](record=vector, stats);	
		

	gates:
		input in[];
		output out[];
}

Summary #####################################################################

# Developer Manual: `hostNodeCeler` in Spider Project

This manual provides a comprehensive, developer-level understanding of the `hostNodeCeler` module in the Spider project. The `hostNodeCeler` is a specialized implementation of the `hostNodeBase` class that integrates the **Celer protocol** for efficient transaction routing and processing. This document explains each component in detail, enabling developers to understand and extend the functionality of this module.

---

## 1. **Overview**

The `hostNodeCeler` is designed to implement the **Celer protocol**, which optimizes transaction routing by leveraging cost-performance indices (CPI) and dynamic path selection. It extends the `hostNodeBase` class and introduces additional features specific to the Celer protocol, such as per-destination queues, CPI calculations, and k-star selection. This module is highly configurable, allowing developers to enable or disable specific features via parameters.

---

## 2. **Parameters**

### 2.1 **General Parameters**
These parameters define the global configuration of the host node:

- **`isHost`**:  
  - Type: `bool`  
  - Default: `true`  
  - Description: Indicates whether the node is a host (`true`) or a router (`false`). Hosts are responsible for initiating transactions, while routers forward them.

- **`topologyFile`**:  
  - Type: `string`  
  - Default: `"sample-topology.txt"`  
  - Description: Specifies the file containing the network topology, including payment channels and capacities.

- **`workloadFile`**:  
  - Type: `string`  
  - Default: `"sample-workload.txt"`  
  - Description: Specifies the file containing the workload, which includes transaction details like sender, receiver, amount, and timestamp.

- **`simulationLength`**:  
  - Type: `double`  
  - Default: `30.0`  
  - Description: Duration of the simulation in seconds.

- **`statRate`**:  
  - Type: `double`  
  - Default: `0.2`  
  - Description: Interval (in seconds) at which statistics are collected and logged.

- **`signalsEnabled`**:  
  - Type: `bool`  
  - Default: `false`  
  - Description: Enables or disables signal emission for monitoring metrics like completion times, queue sizes, and balances.

- **`loggingEnabled`**:  
  - Type: `bool`  
  - Default: `true`  
  - Description: Enables or disables logging for debugging purposes.

- **`splitSize`**:  
  - Type: `double`  
  - Default: `5.0`  
  - Description: Maximum size of transaction splits when splitting large transactions into smaller ones.

---

### 2.2 **Feature-Specific Parameters**
These parameters control the activation and behavior of specific routing and transaction processing algorithms:

- **Timeout Handling**:
  - `timeoutEnabled`: Enables or disables timeout mechanisms for transactions.
  - `timeoutClearRate`: Rate at which timeouts are cleared.

- **Routing Algorithms**:
  - `celerEnabled`: Activates the Celer protocol.
  - `waterfillingEnabled`: Enables water-filling-based load balancing.
  - `priceSchemeEnabled`: Activates price-based routing.
  - `smoothWaterfillingEnabled`: Enables smooth water-filling with EWMA (Exponential Weighted Moving Average).
  - `landmarkRoutingEnabled`: Activates landmark routing.
  - `lndBaselineEnabled`: Enables the Lightning Network Daemon (LND) baseline routing.
  - `dctcpEnabled`: Activates DCTCP-based congestion control.

- **Path Selection**:
  - `widestPathsEnabled`: Prioritizes paths with the highest capacity.
  - `heuristicPathsEnabled`: Uses heuristic-based path selection.
  - `obliviousRoutingEnabled`: Ignores current network conditions during routing.
  - `kspYenEnabled`: Uses K-shortest paths generated by Yen's algorithm.

- **Scheduling Policies**:
  - `LIFOEnabled`, `FIFOEnabled`, `SPFEnabled`, `RREnabled`, `EDFEnabled`: Various scheduling policies for transaction queues.

- **Dynamic Path Changes**:
  - `changingPathsEnabled`: Allows dynamic path changes based on network conditions.
  - `maxPathsToConsider`: Maximum number of paths considered for routing.
  - `windowThresholdForChange`: Threshold for triggering path changes.
  - `pathMonitorRate`: Rate at which paths are monitored for performance.

---

### 2.3 **Rebalancing Parameters**
These parameters control rebalancing operations to maintain optimal channel states:

- **`rebalancingEnabled`**: Enables or disables rebalancing.
- **`queueDelayThreshold`**: Threshold for triggering rebalancing due to high queue delays.
- **`gamma`**: Weight factor for rebalancing decisions.
- **`gammaImbalanceQueueSize`**: Queue size threshold for imbalance detection.
- **`rebalancingRate`**: Rate at which rebalancing operations are performed.
- **`rebalancingDelayForAddingFunds`**: Delay before adding funds during rebalancing.
- **`minBalanceComputeRate`**: Minimum rate for computing balances during rebalancing.

---

### 2.4 **Advanced Parameters**
These parameters fine-tune specific behaviors:

- **Transaction Splitting**:
  - `splittingEnabled`: Enables splitting large transactions into smaller ones.
  - `tau`, `normalizer`: Parameters for smooth water-filling and transaction splitting.

- **Price Scheme**:
  - `alpha`, `kappa`, `eta`, `zeta`: Tuning parameters for price calculations.
  - `rhoValue`: Factor for demand estimation.
  - `minRate`: Minimum acceptable rate for transactions.
  - `updateQueryTime`: Interval for updating price queries.

- **Router-Specific Parameters**:
  - `routerQueueDrainTime`: Time required to drain queues in routers.

---

## 3. **Signals**

The `hostNodeCeler` emits various signals for monitoring and collecting statistics. These signals are categorized by their purpose:

### 3.1 **Completion and Timing**
- `completionTime`: Records the time taken to complete transactions.
- `numCleared`: Tracks the number of transactions cleared.

### 3.2 **Queue and Channel Metrics**
- `implicitRebalancingAmtPerChannel*`: Amount of implicit rebalancing per channel.
- `explicitRebalancingAmtPerChannel*`: Amount of explicit rebalancing per channel.
- `numInQueuePerChannel*`: Number of transactions in the queue per channel.
- `numInflightPerChannel*`: Number of transactions in flight per channel.
- `numProcessedPerChannel*`: Number of transactions processed per channel.
- `numSentPerChannel*`: Number of transactions sent per channel.

### 3.3 **Destination-Specific Metrics**
- `rateCompletedPerDest*`: Rate of completed transactions per destination.
- `numCompletedPerDest*`: Number of completed transactions per destination.
- `numPendingPerDest*`: Number of pending transactions per destination.
- `fracSuccessfulPerDest*`: Fraction of successful transactions per destination.
- `demandEstimatePerDest*`: Estimated demand per destination.

### 3.4 **Path-Specific Metrics**
- `bottleneckPerDestPerPath*`: Bottleneck capacity per path.
- `probeBackPerDestPerPath*`: Probe messages received back per path.
- `rateCompletedPerDestPerPath*`: Rate of completed transactions per path.

---

## 4. **Gates**

The `hostNodeCeler` defines two types of gates for communication:

- **`input in[]`**: Input gates for receiving messages from other nodes.
- **`output out[]`**: Output gates for sending messages to other nodes.

These gates enable the node to interact with its neighbors in the network.

---

## 5. **Key Functionalities**

### 5.1 **Initialization**
- **`initialize()`**: Sets up global parameters, payment channels, and signals. It also reads configuration files (`topologyFile` and `workloadFile`) to initialize the network state.

### 5.2 **Transaction Generation**
- **`generateNextTransaction()`**: Reads the next transaction from the workload file, creates a transaction message, and schedules it for transmission.

### 5.3 **Routing Logic**
- Implements the **Celer protocol** for routing transactions:
  - **CPI Calculation**: Computes the Cost Performance Index (CPI) for selecting optimal paths.
  - **K-Star Selection**: Determines the best path index (`k*`) for forwarding transactions.
  - **Queue Management**: Manages per-destination queues for efficient transaction processing.

### 5.4 **Payment Channel Management**
- Manages payment channel balances, handles rebalancing, and updates states based on incoming messages.

### 5.5 **Signal Handling**
- Collects statistics and emits signals for monitoring performance metrics.

---

## 6. **Celer-Specific Features**

### 6.1 **CPI Calculation**
The **Cost Performance Index (CPI)** is calculated using the formula:
\[
\text{CPI} = Q_{ik} - Q_{jk} + \beta \cdot \text{channelImbalance}
\]
Where:
- \(Q_{ik}\): Queue length at the current node for destination \(k\).
- \(Q_{jk}\): Queue length at the neighbor node for destination \(k\).
- \(\beta\): Weight factor for channel imbalance.

### 6.2 **K-Star Selection**
The **k-star** is determined by selecting the path with the highest CPI for each destination. This ensures that transactions are forwarded along the most cost-effective routes.

### 6.3 **Queue Management**
Transactions are managed in per-destination queues. If a queue exceeds a certain threshold, rebalancing or splitting may be triggered to optimize performance.

---

## 7. **Conclusion**

The `hostNodeCeler` module provides a robust implementation of the Celer protocol for efficient transaction routing and processing. Its modular design allows developers to enable or disable specific features, making it highly adaptable to different use cases. By understanding the parameters, signals, and functionalities described in this manual, developers can effectively contribute to and maintain the project.

###########################################################################
/home/kush/Desktop/spiderproj/spider_omnet/hostNodeDCTCP.ned

simple hostNodeDCTCP
{
	parameters:
	    bool isHost = true;
	    
		string topologyFile = default("sample-topology.txt");
		string workloadFile = default("sample-workload.txt");
                string resultPrefix = default("");

		double simulationLength = default(30.0);
		double statRate = default(0.2);
		bool signalsEnabled = default(false);
		bool loggingEnabled = default(false);
                double splitSize = default(5.0);

		bool timeoutEnabled = default(true);
		double timeoutClearRate = default(0.5);

		bool celerEnabled = default(false);
		bool waterfillingEnabled = default(false);
		bool priceSchemeEnabled = default(false);
		bool smoothWaterfillingEnabled = default(false);
                bool windowEnabled = default(false);
                bool landmarkRoutingEnabled = default(false);
                bool lndBaselineEnabled = default(false);
                bool dctcpEnabled = default(false);
                bool DCTCPQEnabled = default(false);
                bool TCPEnabled = default(false);
                bool cubicEnabled = default(false);

	        bool widestPathsEnabled = default(false);
                bool heuristicPathsEnabled = default(false);
                bool obliviousRoutingEnabled = default(false);
                bool kspYenEnabled = default(false);
                
                bool LIFOEnabled = default(false);
                bool FIFOEnabled = default(false);
                bool SPFEnabled = default(false);
                bool RREnabled = default(false);
                bool EDFEnabled = default(false);

                bool changingPathsEnabled = default(false);
                int maxPathsToConsider = default(100);
                double windowThresholdForChange = default(1);
                double pathMonitorRate = default(5);
	        
                double transStatStart = default(3000);
                double transStatEnd = default(5000);
                int numPathChoices = default(4);

                double alpha = default(0.01);
                double kappa = default(0.01);
                double eta = default(0.01);
                double zeta = default(0.01);
                double rhoValue = default(0.05);
                double capacityFactor = default(1.0);

                double windowAlpha = default(0.2);
                double windowBeta = default(0.2);
                double queueThreshold = default(30);
                double queueDelayEcnThreshold = default(0.5);
                double balanceThreshold = default(0.01);
                double minDCTCPWindow = default(5.0);
                
                bool useQueueEquation = default(true);
                bool splittingEnabled = default(true);
                double minRate = default(0.25);
                double updateQueryTime = default(0.8);
                double xi = default(1);
                int serviceArrivalWindow = default(100);
                double routerQueueDrainTime = default(1);

                double tau = default(10);
                double normalizer = default(100);

                bool rebalancingEnabled = default(false);
                double queueDelayThreshold = default(3.0);
                double gamma = default(1.0);
                double gammaImbalanceQueueSize = default(5.0);
                double rebalancingRate = default(10.0);
                double rebalancingDelayForAddingFunds = default(0.1);
                double minBalanceComputeRate = default(0.1);
		
		@signal[completionTime](type="simtime_t"); 
		@statistic[completionTime](title="completionTime"; source="completionTime"; record=vector,stats; interpolationmode=none); 
		
		@signal[numCleared](type="long"); 
		@statistic[numCleared](title="numCleared"; source="numCleared"; record=vector,stats; interpolationmode=none); 
		
		@display("i=block/routing"); 

		@signal[numInQueuePerChannel*](type="double"); 
		@statisticTemplate[numInQueuePerChannelTemplate](record=vector,stats);   

                @signal[numInflightPerChannel*](type="unsigned long"); 
		@statisticTemplate[numInflightPerChannelTemplate](record=vector, stats);
		
                @signal[implicitRebalancingAmtPerChannel*](type="double"); 
		@statisticTemplate[implicitRebalancingAmtPerChannelTemplate](record=vector, stats);  
	
        	@signal[explicitRebalancingAmtPerChannel*](type="double"); 
		@statisticTemplate[explicitRebalancingAmtPerChannelTemplate](record=vector, stats);  
		
		@signal[numProcessedPerChannel*](type="long");  
		@statisticTemplate[numProcessedPerChannelTemplate](record=vector, stats);  

 		@signal[probabilityPerDestPerPath*](type="double");  
		@statisticTemplate[probabilityPerDestPerPathTemplate](record=vector, stats);  

		@signal[numSentPerChannel*](type="long");  
		@statisticTemplate[numSentPerChannelTemplate](record=vector, stats);   

		@signal[rateCompletedPerDest*](type="long");  
		@statisticTemplate[rateCompletedPerDestTemplate](record=vector, stats);   
		
		@signal[numCompletedPerDest*](type="long");  
		@statisticTemplate[numCompletedPerDestTemplate](record=vector, stats);   

		@signal[numPendingPerDest*](type="long");  
		@statisticTemplate[numPendingPerDestTemplate](record=vector, stats); 

 		@signal[numWaitingPerDest*](type="unsigned long");  
		@statisticTemplate[numWaitingPerDestTemplate](record=vector, stats);  
	
		@signal[fracSuccessfulPerDest*](type="double");  
		@statisticTemplate[fracSuccessfulPerDestTemplate](record=vector, stats);   
		
		@signal[pathPerTransPerDest*](type="long");  
		@statisticTemplate[pathPerTransPerDestTemplate](record=vector, stats);   
		
		@signal[numTimedOutPerDest*](type="long");  
		@statisticTemplate[numTimedOutPerDestTemplate](record=vector, stats);

		@signal[demandEstimatePerDest*](type="double");  
		@statisticTemplate[demandEstimatePerDestTemplate](record=vector, stats);

		@signal[numTimedOutAtSenderPerDest*](type="long");  
		@statisticTemplate[numTimedOutAtSenderPerDestTemplate](record=vector, stats);
	
        	@signal[rateAttemptedPerDest*](type="long"); 
		@statisticTemplate[rateAttemptedPerDestTemplate](record=vector, stats);   
		
                @signal[rateArrivedPerDest*](type="long"); 
		@statisticTemplate[rateArrivedPerDestTemplate](record=vector, stats);  
	
		@signal[numArrivedPerDest*](type="long"); 
		@statisticTemplate[numArrivedPerDestTemplate](record=vector, stats);   

		@signal[balancePerChannel*](type="double"); 
		@statisticTemplate[balancePerChannelTemplate](record=vector, stats);   
		
		@signal[capacityPerChannel*](type="double"); 
		@statisticTemplate[capacityPerChannelTemplate](record=vector, stats);   
 		
                @signal[bankPerChannel*](type="double"); 
		@statisticTemplate[bankPerChannelTemplate](record=vector, stats); 
		
		@signal[timeInFlightPerChannel*](type="double"); 
		@statisticTemplate[timeInFlightPerChannelTemplate](record=vector, stats);   
		
                @signal[bottleneckPerDestPerPath*](type="long"); 
		@statisticTemplate[bottleneckPerDestPerPathTemplate](record=vector, stats);
		
		@signal[probeBackPerDestPerPath*](type="long"); 
		@statisticTemplate[probeBackPerDestPerPathTemplate](record=vector, stats);
		
		@signal[rateCompletedPerDestPerPath*](type="long"); 
		@statisticTemplate[rateCompletedPerDestPerPathTemplate](record=vector, stats);
		
		@signal[rateAttemptedPerDestPerPath*](type="long"); 
		@statisticTemplate[rateAttemptedPerDestPerPathTemplate](record=vector, stats);
		
                @signal[rateOfAcksPerDestPerPath*](type="double"); 
		@statisticTemplate[rateOfAcksPerDestPerPathTemplate](record=vector, stats);
		
                @signal[fractionMarkedPerDestPerPath*](type="double"); 
		@statisticTemplate[fractionMarkedPerDestPerPathTemplate](record=vector, stats);
                
                //has queue capacity specific signals
		@signal[numFailedPerDest*](type="long");  
		@statisticTemplate[numFailedPerDestTemplate](record=vector, stats);
		
		@signal[rateFailedPerDest*](type="long");  
		@statisticTemplate[rateFailedPerDestTemplate](record=vector, stats);
				
		//price scheme path specific signals		
		@signal[sumOfTransUnitsInFlightPerDestPerPath*](type="double"); 
		@statisticTemplate[sumOfTransUnitsInFlightPerDestPerPathTemplate](record=vector, stats);
			
        	@signal[windowPerDestPerPath*](type="double"); 
		@statisticTemplate[windowPerDestPerPathTemplate](record=vector, stats);		

	gates:
		input in[];
		output out[];
}


SUMMARY ############################################################

# Developer Manual: `hostNodeDCTCP` in Spider Project

This manual provides a comprehensive, developer-level understanding of the `hostNodeDCTCP` module in the Spider project. The `hostNodeDCTCP` is a specialized implementation of the `hostNodeBase` class that integrates **Data Center TCP (DCTCP)** principles for efficient transaction routing and congestion control. This document explains each component in detail, enabling developers to understand and extend the functionality of this module.

---

## 1. **Overview**

The `hostNodeDCTCP` implements the **DCTCP protocol**, which optimizes transaction routing by leveraging congestion control mechanisms such as window size adjustments, queue thresholds, and marking packets when queues exceed specific thresholds. It extends the `hostNodeBase` class and introduces additional features specific to DCTCP, such as per-channel window management, monitoring paths, and handling acknowledgment messages.

---

## 2. **Parameters**

### 2.1 **General Parameters**
These parameters define the global configuration of the host node:

- **`isHost`**:  
  - Type: `bool`  
  - Default: `true`  
  - Description: Indicates whether the node is a host (`true`) or a router (`false`). Hosts are responsible for initiating transactions, while routers forward them.

- **`topologyFile`**:  
  - Type: `string`  
  - Default: `"sample-topology.txt"`  
  - Description: Specifies the file containing the network topology, including payment channels and capacities.

- **`workloadFile`**:  
  - Type: `string`  
  - Default: `"sample-workload.txt"`  
  - Description: Specifies the file containing the workload, which includes transaction details like sender, receiver, amount, and timestamp.

- **`simulationLength`**:  
  - Type: `double`  
  - Default: `30.0`  
  - Description: Duration of the simulation in seconds.

- **`statRate`**:  
  - Type: `double`  
  - Default: `0.2`  
  - Description: Interval (in seconds) at which statistics are collected and logged.

- **`signalsEnabled`**:  
  - Type: `bool`  
  - Default: `false`  
  - Description: Enables or disables signal emission for monitoring metrics like completion times, queue sizes, and balances.

- **`loggingEnabled`**:  
  - Type: `bool`  
  - Default: `false`  
  - Description: Enables or disables logging for debugging purposes.

- **`splitSize`**:  
  - Type: `double`  
  - Default: `5.0`  
  - Description: Maximum size of transaction splits when splitting large transactions into smaller ones.

---

### 2.2 **Feature-Specific Parameters**
These parameters control the activation and behavior of specific routing and transaction processing algorithms:

- **Timeout Handling**:
  - `timeoutEnabled`: Enables or disables timeout mechanisms for transactions.
  - `timeoutClearRate`: Rate at which timeouts are cleared.

- **Routing Algorithms**:
  - `celerEnabled`: Activates the Celer protocol.
  - `waterfillingEnabled`: Enables water-filling-based load balancing.
  - `priceSchemeEnabled`: Activates price-based routing.
  - `smoothWaterfillingEnabled`: Enables smooth water-filling with EWMA (Exponential Weighted Moving Average).
  - `landmarkRoutingEnabled`: Activates landmark routing.
  - `lndBaselineEnabled`: Enables the Lightning Network Daemon (LND) baseline routing.
  - `dctcpEnabled`: Activates DCTCP-based congestion control.

- **Path Selection**:
  - `widestPathsEnabled`: Prioritizes paths with the highest capacity.
  - `heuristicPathsEnabled`: Uses heuristic-based path selection.
  - `obliviousRoutingEnabled`: Ignores current network conditions during routing.
  - `kspYenEnabled`: Uses K-shortest paths generated by Yen's algorithm.

- **Scheduling Policies**:
  - `LIFOEnabled`, `FIFOEnabled`, `SPFEnabled`, `RREnabled`, `EDFEnabled`: Various scheduling policies for transaction queues.

- **Dynamic Path Changes**:
  - `changingPathsEnabled`: Allows dynamic path changes based on network conditions.
  - `maxPathsToConsider`: Maximum number of paths considered for routing.
  - `windowThresholdForChange`: Threshold for triggering path changes.
  - `pathMonitorRate`: Rate at which paths are monitored for performance.

---

### 2.3 **Rebalancing Parameters**
These parameters control rebalancing operations to maintain optimal channel states:

- **`rebalancingEnabled`**: Enables or disables rebalancing.
- **`queueDelayThreshold`**: Threshold for triggering rebalancing due to high queue delays.
- **`gamma`**: Weight factor for rebalancing decisions.
- **`gammaImbalanceQueueSize`**: Queue size threshold for imbalance detection.
- **`rebalancingRate`**: Rate at which rebalancing operations are performed.
- **`rebalancingDelayForAddingFunds`**: Delay before adding funds during rebalancing.
- **`minBalanceComputeRate`**: Minimum rate for computing balances during rebalancing.

---

### 2.4 **Advanced Parameters**
These parameters fine-tune specific behaviors:

- **Transaction Splitting**:
  - `splittingEnabled`: Enables splitting large transactions into smaller ones.
  - `tau`, `normalizer`: Parameters for smooth water-filling and transaction splitting.

- **Price Scheme**:
  - `alpha`, `kappa`, `eta`, `zeta`: Tuning parameters for price calculations.
  - `rhoValue`: Factor for demand estimation.
  - `minRate`: Minimum acceptable rate for transactions.
  - `updateQueryTime`: Interval for updating price queries.

- **Router-Specific Parameters**:
  - `routerQueueDrainTime`: Time required to drain queues in routers.

---

## 3. **Signals**

The `hostNodeDCTCP` emits various signals for monitoring and collecting statistics. These signals are categorized by their purpose:

### 3.1 **Completion and Timing**
- `completionTime`: Records the time taken to complete transactions.
- `numCleared`: Tracks the number of transactions cleared.

### 3.2 **Queue and Channel Metrics**
- `implicitRebalancingAmtPerChannel*`: Amount of implicit rebalancing per channel.
- `explicitRebalancingAmtPerChannel*`: Amount of explicit rebalancing per channel.
- `numInQueuePerChannel*`: Number of transactions in the queue per channel.
- `numInflightPerChannel*`: Number of transactions in flight per channel.
- `numProcessedPerChannel*`: Number of transactions processed per channel.
- `numSentPerChannel*`: Number of transactions sent per channel.

### 3.3 **Destination-Specific Metrics**
- `rateCompletedPerDest*`: Rate of completed transactions per destination.
- `numCompletedPerDest*`: Number of completed transactions per destination.
- `numPendingPerDest*`: Number of pending transactions per destination.
- `fracSuccessfulPerDest*`: Fraction of successful transactions per destination.
- `demandEstimatePerDest*`: Estimated demand per destination.

### 3.4 **Path-Specific Metrics**
- `bottleneckPerDestPerPath*`: Bottleneck capacity per path.
- `probeBackPerDestPerPath*`: Probe messages received back per path.
- `rateCompletedPerDestPerPath*`: Rate of completed transactions per path.

---

## 4. **Gates**

The `hostNodeDCTCP` defines two types of gates for communication:

- **`input in[]`**: Input gates for receiving messages from other nodes.
- **`output out[]`**: Output gates for sending messages to other nodes.

These gates enable the node to interact with its neighbors in the network.

---

## 5. **Key Functionalities**

### 5.1 **Initialization**
- **`initialize()`**: Sets up global parameters, payment channels, and signals. It also reads configuration files (`topologyFile` and `workloadFile`) to initialize the network state.

### 5.2 **Transaction Generation**
- **`generateNextTransaction()`**: Reads the next transaction from the workload file, creates a transaction message, and schedules it for transmission.

### 5.3 **Routing Logic**
- Implements the **DCTCP protocol** for routing transactions:
  - **Window Size Management**: Adjusts the window size based on network conditions.
  - **Queue Thresholds**: Marks packets when queues exceed specific thresholds.
  - **Path Monitoring**: Dynamically monitors paths and adjusts routing decisions.

### 5.4 **Payment Channel Management**
- Manages payment channel balances, handles rebalancing, and updates states based on incoming messages.

### 5.5 **Signal Handling**
- Collects statistics and emits signals for monitoring performance metrics.

---

## 6. **DCTCP-Specific Features**

### 6.1 **Window Size Management**
The **window size** is dynamically adjusted based on network conditions. The formula for adjusting the window size is:
\[
W = W \times (1 - \alpha) + \alpha \times \text{new\_window\_size}
\]
Where:
- \(W\): Current window size.
- \(\alpha\): Weight factor for smoothing.

### 6.2 **Queue Thresholds**
Packets are marked when the queue delay exceeds the threshold:
\[
\text{queueDelayEcnThreshold} = 0.5
\]

### 6.3 **Path Monitoring**
The **path monitoring** mechanism ensures that paths are dynamically updated based on performance metrics such as bottleneck capacity and queue delays.

---

## 7. **Conclusion**

The `hostNodeDCTCP` module provides a robust implementation of the DCTCP protocol for efficient transaction routing and congestion control. Its modular design allows developers to enable or disable specific features, making it highly adaptable to different use cases. By understanding the parameters, signals, and functionalities described in this manual, developers can effectively contribute to and maintain the project.

#################################################################
/home/kush/Desktop/spiderproj/spider_omnet/hostNodeLandmarkRouting.ned

simple hostNodeLandmarkRouting
{
	parameters:
	    bool isHost = true;
	    
		string topologyFile = default("sample-topology.txt");
		string workloadFile = default("sample-workload.txt");
                string resultPrefix = default("");

		double simulationLength = default(30.0);
		double statRate = default(0.2);
		bool signalsEnabled = default(false);
		bool loggingEnabled = default(false);
                double splitSize = default(5.0);

		bool timeoutEnabled = default(true);
		double timeoutClearRate = default(0.5);

		bool celerEnabled = default(false);
		bool waterfillingEnabled = default(false);
		bool priceSchemeEnabled = default(false);
		bool smoothWaterfillingEnabled = default(false);
                bool windowEnabled = default(false);
                bool landmarkRoutingEnabled = default(false);
                bool lndBaselineEnabled = default(false);
                bool dctcpEnabled = default(false);
		int numPathChoices = default(4);

                bool rebalancingEnabled = default(false);
                double queueDelayThreshold = default(3.0);
                double gamma = default(1.0);
                double gammaImbalanceQueueSize = default(5.0);

                double transStatStart = default(3000);
                double transStatEnd = default(5000);

                bool widestPathsEnabled = default(false);
                bool heuristicPathsEnabled = default(false);
                bool obliviousRoutingEnabled = default(false);
                bool kspYenEnabled = default(false);
                double rebalancingRate = default(10.0);
                double rebalancingDelayForAddingFunds = default(0.1);
                double minBalanceComputeRate = default(0.1);
                
                bool LIFOEnabled = default(false);
                bool FIFOEnabled = default(false);
                bool SPFEnabled = default(false);
                bool RREnabled = default(false);
                bool EDFEnabled = default(false);
                
                bool changingPathsEnabled = default(false);
                int maxPathsToConsider = default(100);
                double windowThresholdForChange = default(1);
                double pathMonitorRate = default(5);

                double alpha = default(0.01);
                double kappa = default(0.01);
                double eta = default(0.01);
                double zeta = default(0.01);
                double rhoValue = default(0.05);
                double minRate = default(0.25);
                double updateQueryTime = default(0.8);
                double xi = default(1);
                int serviceArrivalWindow = default(100);
                double routerQueueDrainTime = default(1);
                bool splittingEnabled = default(false);
                
                double tau = default(10);
                double normalizer = default(100);

		
		@signal[completionTime](type="long"); 
		@statistic[completionTime](title="completionTime"; source="completionTime"; record=vector,stats; interpolationmode=none); 
		
		@signal[numCleared](type="long"); 
		@statistic[numCleared](title="numCleared"; source="numCleared"; record=vector,stats; interpolationmode=none); 
		
		@display("i=block/routing"); 

		@signal[numInQueuePerChannel*](type="long"); // note an asterisk and the type of emitted values  
		@statisticTemplate[numInQueuePerChannelTemplate](record=vector,stats);   

                @signal[numInflightPerChannel*](type="unsigned long"); 
		@statisticTemplate[numInflightPerChannelTemplate](record=vector, stats);
		@signal[numProcessedPerChannel*](type="long");  
		@statisticTemplate[numProcessedPerChannelTemplate](record=vector, stats);  

 		@signal[probabilityPerDestPerPath*](type="double");  
		@statisticTemplate[probabilityPerDestPerPathTemplate](record=vector, stats);  

		@signal[numSentPerChannel*](type="long");  
		@statisticTemplate[numSentPerChannelTemplate](record=vector, stats);   

		@signal[rateCompletedPerDest*](type="long");  
		@statisticTemplate[rateCompletedPerDestTemplate](record=vector, stats);   
		
		@signal[numCompletedPerDest*](type="long");  
		@statisticTemplate[numCompletedPerDestTemplate](record=vector, stats);   

		@signal[numPendingPerDest*](type="long");  
		@statisticTemplate[numPendingPerDestTemplate](record=vector, stats); 

 		@signal[numWaitingPerDest*](type="long");  
		@statisticTemplate[numWaitingPerDestTemplate](record=vector, stats);  
	
		@signal[fracSuccessfulPerDest*](type="double");  
		@statisticTemplate[fracSuccessfulPerDestTemplate](record=vector, stats);   
		
		@signal[pathPerTransPerDest*](type="long");  
		@statisticTemplate[pathPerTransPerDestTemplate](record=vector, stats);   
		
		@signal[numTimedOutPerDest*](type="long");  
		@statisticTemplate[numTimedOutPerDestTemplate](record=vector, stats);

		@signal[demandEstimatePerDest*](type="double");  
		@statisticTemplate[demandEstimatePerDestTemplate](record=vector, stats);

		@signal[numTimedOutAtSenderPerDest*](type="long");  
		@statisticTemplate[numTimedOutAtSenderPerDestTemplate](record=vector, stats);
	
        	@signal[rateAttemptedPerDest*](type="long"); 
		@statisticTemplate[rateAttemptedPerDestTemplate](record=vector, stats);   
		
                @signal[rateArrivedPerDest*](type="long"); 
		@statisticTemplate[rateArrivedPerDestTemplate](record=vector, stats);  
	
		@signal[numArrivedPerDest*](type="long"); 
		@statisticTemplate[numArrivedPerDestTemplate](record=vector, stats);   

		@signal[balancePerChannel*](type="long"); 
		@statisticTemplate[balancePerChannelTemplate](record=vector, stats);   
		
		@signal[capacityPerChannel*](type="double"); 
		@statisticTemplate[capacityPerChannelTemplate](record=vector, stats);   
 		
                @signal[bankPerChannel*](type="double"); 
		@statisticTemplate[bankPerChannelTemplate](record=vector, stats); 
		
		@signal[timeInFlightPerChannel*](type="double"); 
		@statisticTemplate[timeInFlightPerChannelTemplate](record=vector, stats);   
		
		@signal[bottleneckPerDestPerPath*](type="long"); 
		@statisticTemplate[bottleneckPerDestPerPathTemplate](record=vector, stats);
		
		@signal[probeBackPerDestPerPath*](type="long"); 
		@statisticTemplate[probeBackPerDestPerPathTemplate](record=vector, stats);
		
		@signal[rateCompletedPerDestPerPath*](type="long"); 
		@statisticTemplate[rateCompletedPerDestPerPathTemplate](record=vector, stats);
		
		@signal[rateAttemptedPerDestPerPath*](type="long"); 
		@statisticTemplate[rateAttemptedPerDestPerPathTemplate](record=vector, stats);
		
		//has queue capacity specific signals
		@signal[numFailedPerDest*](type="long");  
		@statisticTemplate[numFailedPerDestTemplate](record=vector, stats);
		
		@signal[rateFailedPerDest*](type="long");  
		@statisticTemplate[rateFailedPerDestTemplate](record=vector, stats);
		
		//price scheme payment channel specific signals
		@signal[nValuePerChannel*](type="long"); 
		@statisticTemplate[nValuePerChannelTemplate](record=vector, stats);
		
		@signal[xLocalPerChannel*](type="double"); 
		@statisticTemplate[xLocalPerChannelTemplate](record=vector, stats);

		@signal[lambdaPerChannel*](type="double"); 
		@statisticTemplate[lambdaPerChannelTemplate](record=vector, stats);
		
		@signal[muLocalPerChannel*](type="double"); 
		@statisticTemplate[muLocalPerChannelTemplate](record=vector, stats);
		
		@signal[muRemotePerChannel*](type="double"); 
		@statisticTemplate[muRemotePerChannelTemplate](record=vector, stats);
		
		@signal[implicitRebalancingAmtPerChannel*](type="double"); 
		@statisticTemplate[implicitRebalancingAmtPerChannelTemplate](record=vector, stats);  
		@signal[explicitRebalancingAmtPerChannel*](type="double"); 
		@statisticTemplate[explicitRebalancingAmtPerChannelTemplate](record=vector, stats);  
		
                //price scheme path specific signals
		@signal[rateToSendTransPerDestPerPath*](type="double"); 
		@statisticTemplate[rateToSendTransPerDestPerPathTemplate](record=vector, stats);
		
		@signal[timeToNextSendPerDestPerPath*](type="simtime_t"); 
		@statisticTemplate[timeToNextSendPerDestPerPathTemplate](record=vector, stats);
		
		@signal[sumOfTransUnitsInFlightPerDestPerPath*](type="double"); 
		@statisticTemplate[sumOfTransUnitsInFlightPerDestPerPathTemplate](record=vector, stats);
		
		@signal[priceLastSeenPerDestPerPath*](type="double"); 
		@statisticTemplate[priceLastSeenPerDestPerPathTemplate](record=vector, stats);
		
		@signal[isSendTimerSetPerDestPerPath*](type="long"); 
		@statisticTemplate[isSendTimerSetPerDestPerPathTemplate](record=vector, stats);
		
		
		

	gates:
		input in[];
		output out[];
}


SUMMARY #######################################################################

# Developer Manual: `hostNodeLandmarkRouting` in Spider Project

This manual provides a comprehensive, developer-level understanding of the `hostNodeLandmarkRouting` module in the Spider project. The `hostNodeLandmarkRouting` is a specialized implementation of the `hostNodeBase` class that integrates **landmark routing** principles for efficient transaction routing and network optimization. This document explains each component in detail, enabling developers to understand and extend the functionality of this module.

---

## 1. **Overview**

The `hostNodeLandmarkRouting` implements **landmark routing**, which optimizes transaction routing by leveraging a set of predefined landmarks (intermediate nodes) to split paths into two segments: from the source to the landmark and from the landmark to the destination. This approach reduces the complexity of path computation and improves scalability in large networks. It extends the `hostNodeBase` class and introduces additional features specific to landmark routing, such as probe message handling, bottleneck estimation, and random splitting.

---

## 2. **Parameters**

### 2.1 **General Parameters**
These parameters define the global configuration of the host node:

- **`isHost`**:  
  - Type: `bool`  
  - Default: `true`  
  - Description: Indicates whether the node is a host (`true`) or a router (`false`). Hosts are responsible for initiating transactions, while routers forward them.

- **`topologyFile`**:  
  - Type: `string`  
  - Default: `"sample-topology.txt"`  
  - Description: Specifies the file containing the network topology, including payment channels and capacities.

- **`workloadFile`**:  
  - Type: `string`  
  - Default: `"sample-workload.txt"`  
  - Description: Specifies the file containing the workload, which includes transaction details like sender, receiver, amount, and timestamp.

- **`simulationLength`**:  
  - Type: `double`  
  - Default: `30.0`  
  - Description: Duration of the simulation in seconds.

- **`statRate`**:  
  - Type: `double`  
  - Default: `0.2`  
  - Description: Interval (in seconds) at which statistics are collected and logged.

- **`signalsEnabled`**:  
  - Type: `bool`  
  - Default: `false`  
  - Description: Enables or disables signal emission for monitoring metrics like completion times, queue sizes, and balances.

- **`loggingEnabled`**:  
  - Type: `bool`  
  - Default: `false`  
  - Description: Enables or disables logging for debugging purposes.

- **`splitSize`**:  
  - Type: `double`  
  - Default: `5.0`  
  - Description: Maximum size of transaction splits when splitting large transactions into smaller ones.

---

### 2.2 **Feature-Specific Parameters**
These parameters control the activation and behavior of specific routing and transaction processing algorithms:

- **Timeout Handling**:
  - `timeoutEnabled`: Enables or disables timeout mechanisms for transactions.
  - `timeoutClearRate`: Rate at which timeouts are cleared.

- **Routing Algorithms**:
  - `celerEnabled`: Activates the Celer protocol.
  - `waterfillingEnabled`: Enables water-filling-based load balancing.
  - `priceSchemeEnabled`: Activates price-based routing.
  - `smoothWaterfillingEnabled`: Enables smooth water-filling with EWMA (Exponential Weighted Moving Average).
  - `landmarkRoutingEnabled`: Activates landmark routing.
  - `lndBaselineEnabled`: Enables the Lightning Network Daemon (LND) baseline routing.
  - `dctcpEnabled`: Activates DCTCP-based congestion control.

- **Path Selection**:
  - `widestPathsEnabled`: Prioritizes paths with the highest capacity.
  - `heuristicPathsEnabled`: Uses heuristic-based path selection.
  - `obliviousRoutingEnabled`: Ignores current network conditions during routing.
  - `kspYenEnabled`: Uses K-shortest paths generated by Yen's algorithm.

- **Scheduling Policies**:
  - `LIFOEnabled`, `FIFOEnabled`, `SPFEnabled`, `RREnabled`, `EDFEnabled`: Various scheduling policies for transaction queues.

- **Dynamic Path Changes**:
  - `changingPathsEnabled`: Allows dynamic path changes based on network conditions.
  - `maxPathsToConsider`: Maximum number of paths considered for routing.
  - `windowThresholdForChange`: Threshold for triggering path changes.
  - `pathMonitorRate`: Rate at which paths are monitored for performance.

---

### 2.3 **Rebalancing Parameters**
These parameters control rebalancing operations to maintain optimal channel states:

- **`rebalancingEnabled`**: Enables or disables rebalancing.
- **`queueDelayThreshold`**: Threshold for triggering rebalancing due to high queue delays.
- **`gamma`**: Weight factor for rebalancing decisions.
- **`gammaImbalanceQueueSize`**: Queue size threshold for imbalance detection.
- **`rebalancingRate`**: Rate at which rebalancing operations are performed.
- **`rebalancingDelayForAddingFunds`**: Delay before adding funds during rebalancing.
- **`minBalanceComputeRate`**: Minimum rate for computing balances during rebalancing.

---

### 2.4 **Advanced Parameters**
These parameters fine-tune specific behaviors:

- **Transaction Splitting**:
  - `splittingEnabled`: Enables splitting large transactions into smaller ones.
  - `tau`, `normalizer`: Parameters for smooth water-filling and transaction splitting.

- **Price Scheme**:
  - `alpha`, `kappa`, `eta`, `zeta`: Tuning parameters for price calculations.
  - `rhoValue`: Factor for demand estimation.
  - `minRate`: Minimum acceptable rate for transactions.
  - `updateQueryTime`: Interval for updating price queries.

- **Router-Specific Parameters**:
  - `routerQueueDrainTime`: Time required to drain queues in routers.

---

## 3. **Signals**

The `hostNodeLandmarkRouting` emits various signals for monitoring and collecting statistics. These signals are categorized by their purpose:

### 3.1 **Completion and Timing**
- `completionTime`: Records the time taken to complete transactions.
- `numCleared`: Tracks the number of transactions cleared.

### 3.2 **Queue and Channel Metrics**
- `implicitRebalancingAmtPerChannel*`: Amount of implicit rebalancing per channel.
- `explicitRebalancingAmtPerChannel*`: Amount of explicit rebalancing per channel.
- `numInQueuePerChannel*`: Number of transactions in the queue per channel.
- `numInflightPerChannel*`: Number of transactions in flight per channel.
- `numProcessedPerChannel*`: Number of transactions processed per channel.
- `numSentPerChannel*`: Number of transactions sent per channel.

### 3.3 **Destination-Specific Metrics**
- `rateCompletedPerDest*`: Rate of completed transactions per destination.
- `numCompletedPerDest*`: Number of completed transactions per destination.
- `numPendingPerDest*`: Number of pending transactions per destination.
- `fracSuccessfulPerDest*`: Fraction of successful transactions per destination.
- `demandEstimatePerDest*`: Estimated demand per destination.

### 3.4 **Path-Specific Metrics**
- `bottleneckPerDestPerPath*`: Bottleneck capacity per path.
- `probeBackPerDestPerPath*`: Probe messages received back per path.
- `rateCompletedPerDestPerPath*`: Rate of completed transactions per path.

---

## 4. **Gates**

The `hostNodeLandmarkRouting` defines two types of gates for communication:

- **`input in[]`**: Input gates for receiving messages from other nodes.
- **`output out[]`**: Output gates for sending messages to other nodes.

These gates enable the node to interact with its neighbors in the network.

---

## 5. **Key Functionalities**

### 5.1 **Initialization**
- **`initialize()`**: Sets up global parameters, payment channels, and signals. It also reads configuration files (`topologyFile` and `workloadFile`) to initialize the network state.

### 5.2 **Transaction Generation**
- **`generateNextTransaction()`**: Reads the next transaction from the workload file, creates a transaction message, and schedules it for transmission.

### 5.3 **Routing Logic**
- Implements the **landmark routing protocol** for routing transactions:
  - **Probe Messages**: Sends probe messages to measure path conditions.
  - **Bottleneck Estimation**: Estimates bottlenecks along paths using probe responses.
  - **Random Splitting**: Divides transactions across multiple paths based on bottleneck capacities.

### 5.4 **Payment Channel Management**
- Manages payment channel balances, handles rebalancing, and updates states based on incoming messages.

### 5.5 **Signal Handling**
- Collects statistics and emits signals for monitoring performance metrics.

---

## 6. **Landmark Routing-Specific Features**

### 6.1 **Probe Message Handling**
- **`generateProbeMessage()`**: Creates a probe message to measure path conditions.
- **`forwardProbeMessage()`**: Forwards probe messages along the path and collects balance information.
- **`handleProbeMessage()`**: Processes probe responses to update path probabilities.

### 6.2 **Random Splitting**
- **`randomSplit()`**: Divides transactions across multiple paths based on bottleneck capacities. This ensures load balancing and minimizes congestion.

### 6.3 **Path Initialization**
- **`initializePathInfoLandmarkRouting()`**: Initializes path-specific data structures for landmark routing, including bottlenecks and probabilities.

### 6.4 **Timeout Handling**
- **`handleTimeOutMessage()`**: Handles timeouts for transactions that fail to complete. Clears state and retries if necessary.

---

## 7. **Conclusion**

The `hostNodeLandmarkRouting` module provides a robust implementation of the landmark routing protocol for efficient transaction routing and network optimization. Its modular design allows developers to enable or disable specific features, making it highly adaptable to different use cases. By understanding the parameters, signals, and functionalities described in this manual, developers can effectively contribute to and maintain the project.


##############################################################################
/home/kush/Desktop/spiderproj/spider_omnet/hostNodeLndBaseline.ned

simple hostNodeLndBaseline
{
	parameters:
	    bool isHost = true;
	    
		string topologyFile = default("sample-topology.txt");
		string workloadFile = default("sample-workload.txt");
                string resultPrefix = default("");

		double simulationLength = default(30.0);
		double statRate = default(0.2);
		bool signalsEnabled = default(false);
		bool loggingEnabled = default(false);
                double splitSize = default(5.0);

		bool timeoutEnabled = default(false);
		double timeoutClearRate = default(0.5);

		bool celerEnabled = default(false);
		bool waterfillingEnabled = default(false);
		bool priceSchemeEnabled = default(false);
		bool smoothWaterfillingEnabled = default(false);
                bool windowEnabled = default(false);
                bool landmarkRoutingEnabled = default(false);
                bool lndBaselineEnabled = default(false);
                bool dctcpEnabled = default(false);
		int numPathChoices = default(4);

                bool widestPathsEnabled = default(false);
                bool heuristicPathsEnabled = default(false);
                bool obliviousRoutingEnabled = default(false);
                bool kspYenEnabled = default(false);
                
                bool LIFOEnabled = default(false);
                bool FIFOEnabled = default(false);
                bool SPFEnabled = default(false);
                bool RREnabled = default(false);
                bool EDFEnabled = default(false);
                
                bool changingPathsEnabled = default(false);
                int maxPathsToConsider = default(100);
                double windowThresholdForChange = default(1);
                double pathMonitorRate = default(5);

                bool rebalancingEnabled = default(false);
                double queueDelayThreshold = default(3.0);
                double gamma = default(1.0);
                double gammaImbalanceQueueSize = default(5.0);
                double rebalancingRate = default(10.0);
                double rebalancingDelayForAddingFunds = default(0.1);
                double minBalanceComputeRate = default(0.1);

                double alpha = default(0.01);
                double kappa = default(0.01);
                double eta = default(0.01);
                double zeta = default(0.01);
                double rhoValue = default(0.05);
                double minRate = default(0.25);
                double updateQueryTime = default(0.8);
                bool splittingEnabled = default(false);
                double xi = default(1);
                int serviceArrivalWindow = default(100);
                double routerQueueDrainTime = default(1);

                double transStatStart = default(3000);
                double transStatEnd = default(5000);
                
                double tau = default(10);
                double normalizer = default(100);

		
		@signal[completionTime](type="long"); 
		@statistic[completionTime](title="completionTime"; source="completionTime"; record=vector,stats; interpolationmode=none); 
		
		@signal[numCleared](type="long"); 
		@statistic[numCleared](title="numCleared"; source="numCleared"; record=vector,stats; interpolationmode=none); 
		
		@display("i=block/routing"); 

		@signal[numInQueuePerChannel*](type="long"); // note an asterisk and the type of emitted values  
                @signal[bankPerChannel*](type="double"); 
		@statisticTemplate[bankPerChannelTemplate](record=vector, stats); 
		@statisticTemplate[numInQueuePerChannelTemplate](record=vector,stats);   

		@signal[numProcessedPerChannel*](type="long");  
		@statisticTemplate[numProcessedPerChannelTemplate](record=vector, stats);  

 		@signal[probabilityPerDestPerPath*](type="double");  
		@statisticTemplate[probabilityPerDestPerPathTemplate](record=vector, stats);  

		@signal[numSentPerChannel*](type="long");  
		@statisticTemplate[numSentPerChannelTemplate](record=vector, stats);   

		@signal[rateCompletedPerDest*](type="long");  
		@statisticTemplate[rateCompletedPerDestTemplate](record=vector, stats);   
		
		@signal[numCompletedPerDest*](type="long");  
		@statisticTemplate[numCompletedPerDestTemplate](record=vector, stats);   

		@signal[numPendingPerDest*](type="long");  
		@statisticTemplate[numPendingPerDestTemplate](record=vector, stats); 

 		@signal[numWaitingPerDest*](type="long");  
		@statisticTemplate[numWaitingPerDestTemplate](record=vector, stats);  
	
		@signal[fracSuccessfulPerDest*](type="double");  
		@statisticTemplate[fracSuccessfulPerDestTemplate](record=vector, stats);   
		
		@signal[pathPerTransPerDest*](type="long");  
		@statisticTemplate[pathPerTransPerDestTemplate](record=vector, stats);   
		
		@signal[numTimedOutPerDest*](type="long");  
		@statisticTemplate[numTimedOutPerDestTemplate](record=vector, stats);

		@signal[demandEstimatePerDest*](type="double");  
		@statisticTemplate[demandEstimatePerDestTemplate](record=vector, stats);

		@signal[numTimedOutAtSenderPerDest*](type="long");  
		@statisticTemplate[numTimedOutAtSenderPerDestTemplate](record=vector, stats);
	
        	@signal[rateAttemptedPerDest*](type="long"); 
		@statisticTemplate[rateAttemptedPerDestTemplate](record=vector, stats);   
		
                @signal[rateArrivedPerDest*](type="long"); 
		@statisticTemplate[rateArrivedPerDestTemplate](record=vector, stats);  
	
		@signal[numArrivedPerDest*](type="long"); 
		@statisticTemplate[numArrivedPerDestTemplate](record=vector, stats);   

		@signal[balancePerChannel*](type="long"); 
		@statisticTemplate[balancePerChannelTemplate](record=vector, stats);   
		
		@signal[capacityPerChannel*](type="double"); 
		@statisticTemplate[capacityPerChannelTemplate](record=vector, stats);   
		
		@signal[implicitRebalancingAmtPerChannel*](type="double"); 
		@statisticTemplate[implicitRebalancingAmtPerChannelTemplate](record=vector, stats);  
		@signal[explicitRebalancingAmtPerChannel*](type="double"); 
		@statisticTemplate[explicitRebalancingAmtPerChannelTemplate](record=vector, stats);  
		
		@signal[timeInFlightPerChannel*](type="double"); 
		@statisticTemplate[timeInFlightPerChannelTemplate](record=vector, stats);   
		
		@signal[bottleneckPerDestPerPath*](type="long"); 
		@statisticTemplate[bottleneckPerDestPerPathTemplate](record=vector, stats);
		
                @signal[numInflightPerChannel*](type="unsigned long"); 
		@statisticTemplate[numInflightPerChannelTemplate](record=vector, stats);
		
                @signal[probeBackPerDestPerPath*](type="long"); 
		@statisticTemplate[probeBackPerDestPerPathTemplate](record=vector, stats);
		
		@signal[rateCompletedPerDestPerPath*](type="long"); 
		@statisticTemplate[rateCompletedPerDestPerPathTemplate](record=vector, stats);
		
		@signal[rateAttemptedPerDestPerPath*](type="long"); 
		@statisticTemplate[rateAttemptedPerDestPerPathTemplate](record=vector, stats);
		
		//has queue capacity specific signals
		@signal[numFailedPerDest*](type="long");  
		@statisticTemplate[numFailedPerDestTemplate](record=vector, stats);
		
		@signal[rateFailedPerDest*](type="long");  
		@statisticTemplate[rateFailedPerDestTemplate](record=vector, stats);
		
		//price scheme payment channel specific signals
		@signal[nValuePerChannel*](type="long"); 
		@statisticTemplate[nValuePerChannelTemplate](record=vector, stats);
		
		@signal[xLocalPerChannel*](type="double"); 
		@statisticTemplate[xLocalPerChannelTemplate](record=vector, stats);

		@signal[lambdaPerChannel*](type="double"); 
		@statisticTemplate[lambdaPerChannelTemplate](record=vector, stats);
		
		@signal[muLocalPerChannel*](type="double"); 
		@statisticTemplate[muLocalPerChannelTemplate](record=vector, stats);
		
		@signal[muRemotePerChannel*](type="double"); 
		@statisticTemplate[muRemotePerChannelTemplate](record=vector, stats);
		
		//price scheme path specific signals
		@signal[rateToSendTransPerDestPerPath*](type="double"); 
		@statisticTemplate[rateToSendTransPerDestPerPathTemplate](record=vector, stats);
		
		@signal[timeToNextSendPerDestPerPath*](type="simtime_t"); 
		@statisticTemplate[timeToNextSendPerDestPerPathTemplate](record=vector, stats);
		
		@signal[sumOfTransUnitsInFlightPerDestPerPath*](type="double"); 
		@statisticTemplate[sumOfTransUnitsInFlightPerDestPerPathTemplate](record=vector, stats);
		
		@signal[priceLastSeenPerDestPerPath*](type="double"); 
		@statisticTemplate[priceLastSeenPerDestPerPathTemplate](record=vector, stats);
		
		@signal[isSendTimerSetPerDestPerPath*](type="long"); 
		@statisticTemplate[isSendTimerSetPerDestPerPathTemplate](record=vector, stats);
		
		@signal[numPathsPerTransPerDest*](type="long");  
		@statisticTemplate[numPathsPerTransPerDestTemplate](record=vector, stats);   
	
        gates:
		input in[];
		output out[];
}


SUMMARY #################################################################

# Developer Manual: `hostNodeLndBaseline` in Spider Project

This manual provides a comprehensive, developer-level understanding of the `hostNodeLndBaseline` module in the Spider project. The `hostNodeLndBaseline` is a specialized implementation of the `hostNodeBase` class that integrates **Lightning Network Daemon (LND) Baseline Routing** principles for efficient transaction routing and retry mechanisms. This document explains each component in detail, enabling developers to understand and extend the functionality of this module.

---

## 1. **Overview**

The `hostNodeLndBaseline` implements **LND Baseline Routing**, which optimizes transaction routing by leveraging retry mechanisms and path pruning strategies. It extends the `hostNodeBase` class and introduces additional features specific to LND baseline routing, such as:

- **Path Pruning**: Temporarily disabling edges that fail during transactions.
- **Retry Mechanisms**: Retrying failed transactions multiple times with different paths.
- **Tail Retry Logging**: Recording successful and failed retries for performance analysis.

This module is designed to handle large-scale networks where dynamic path adjustments are critical for maintaining high throughput and reliability.

---

## 2. **Parameters**

### 2.1 **General Parameters**
These parameters define the global configuration of the host node:

- **`isHost`**:  
  - Type: `bool`  
  - Default: `true`  
  - Description: Indicates whether the node is a host (`true`) or a router (`false`). Hosts initiate transactions, while routers forward them.

- **`topologyFile`**:  
  - Type: `string`  
  - Default: `"sample-topology.txt"`  
  - Description: Specifies the file containing the network topology, including payment channels and capacities.

- **`workloadFile`**:  
  - Type: `string`  
  - Default: `"sample-workload.txt"`  
  - Description: Specifies the file containing the workload, which includes transaction details like sender, receiver, amount, and timestamp.

- **`simulationLength`**:  
  - Type: `double`  
  - Default: `30.0`  
  - Description: Duration of the simulation in seconds.

- **`statRate`**:  
  - Type: `double`  
  - Default: `0.2`  
  - Description: Interval (in seconds) at which statistics are collected and logged.

- **`signalsEnabled`**:  
  - Type: `bool`  
  - Default: `false`  
  - Description: Enables or disables signal emission for monitoring metrics like completion times, queue sizes, and balances.

- **`loggingEnabled`**:  
  - Type: `bool`  
  - Default: `false`  
  - Description: Enables or disables logging for debugging purposes.

- **`splitSize`**:  
  - Type: `double`  
  - Default: `5.0`  
  - Description: Maximum size of transaction splits when splitting large transactions into smaller ones.

---

### 2.2 **Feature-Specific Parameters**
These parameters control the activation and behavior of specific routing and transaction processing algorithms:

- **Timeout Handling**:
  - `timeoutEnabled`: Enables or disables timeout mechanisms for transactions.
  - `timeoutClearRate`: Rate at which timeouts are cleared.

- **Routing Algorithms**:
  - `celerEnabled`: Activates the Celer protocol.
  - `waterfillingEnabled`: Enables water-filling-based load balancing.
  - `priceSchemeEnabled`: Activates price-based routing.
  - `smoothWaterfillingEnabled`: Enables smooth water-filling with EWMA (Exponential Weighted Moving Average).
  - `landmarkRoutingEnabled`: Activates landmark routing.
  - `lndBaselineEnabled`: Enables the Lightning Network Daemon (LND) baseline routing.
  - `dctcpEnabled`: Activates DCTCP-based congestion control.

- **Path Selection**:
  - `widestPathsEnabled`: Prioritizes paths with the highest capacity.
  - `heuristicPathsEnabled`: Uses heuristic-based path selection.
  - `obliviousRoutingEnabled`: Ignores current network conditions during routing.
  - `kspYenEnabled`: Uses K-shortest paths generated by Yen's algorithm.

- **Scheduling Policies**:
  - `LIFOEnabled`, `FIFOEnabled`, `SPFEnabled`, `RREnabled`, `EDFEnabled`: Various scheduling policies for transaction queues.

- **Dynamic Path Changes**:
  - `changingPathsEnabled`: Allows dynamic path changes based on network conditions.
  - `maxPathsToConsider`: Maximum number of paths considered for routing.
  - `windowThresholdForChange`: Threshold for triggering path changes.
  - `pathMonitorRate`: Rate at which paths are monitored for performance.

---

### 2.3 **Rebalancing Parameters**
These parameters control rebalancing operations to maintain optimal channel states:

- **`rebalancingEnabled`**: Enables or disables rebalancing.
- **`queueDelayThreshold`**: Threshold for triggering rebalancing due to high queue delays.
- **`gamma`**: Weight factor for rebalancing decisions.
- **`gammaImbalanceQueueSize`**: Queue size threshold for imbalance detection.
- **`rebalancingRate`**: Rate at which rebalancing operations are performed.
- **`rebalancingDelayForAddingFunds`**: Delay before adding funds during rebalancing.
- **`minBalanceComputeRate`**: Minimum rate for computing balances during rebalancing.

---

### 2.4 **Advanced Parameters**
These parameters fine-tune specific behaviors:

- **Transaction Splitting**:
  - `splittingEnabled`: Enables splitting large transactions into smaller ones.
  - `tau`, `normalizer`: Parameters for smooth water-filling and transaction splitting.

- **Price Scheme**:
  - `alpha`, `kappa`, `eta`, `zeta`: Tuning parameters for price calculations.
  - `rhoValue`: Factor for demand estimation.
  - `minRate`: Minimum acceptable rate for transactions.
  - `updateQueryTime`: Interval for updating price queries.

- **Router-Specific Parameters**:
  - `routerQueueDrainTime`: Time required to drain queues in routers.

---

## 3. **Signals**

The `hostNodeLndBaseline` emits various signals for monitoring and collecting statistics. These signals are categorized by their purpose:

### 3.1 **Completion and Timing**
- `completionTime`: Records the time taken to complete transactions.
- `numCleared`: Tracks the number of transactions cleared.

### 3.2 **Queue and Channel Metrics**
- `implicitRebalancingAmtPerChannel*`: Amount of implicit rebalancing per channel.
- `explicitRebalancingAmtPerChannel*`: Amount of explicit rebalancing per channel.
- `numInQueuePerChannel*`: Number of transactions in the queue per channel.
- `numInflightPerChannel*`: Number of transactions in flight per channel.
- `numProcessedPerChannel*`: Number of transactions processed per channel.
- `numSentPerChannel*`: Number of transactions sent per channel.

### 3.3 **Destination-Specific Metrics**
- `rateCompletedPerDest*`: Rate of completed transactions per destination.
- `numCompletedPerDest*`: Number of completed transactions per destination.
- `numPendingPerDest*`: Number of pending transactions per destination.
- `fracSuccessfulPerDest*`: Fraction of successful transactions per destination.
- `demandEstimatePerDest*`: Estimated demand per destination.

### 3.4 **Path-Specific Metrics**
- `bottleneckPerDestPerPath*`: Bottleneck capacity per path.
- `probeBackPerDestPerPath*`: Probe messages received back per path.
- `rateCompletedPerDestPerPath*`: Rate of completed transactions per path.

---

## 4. **Gates**

The `hostNodeLndBaseline` defines two types of gates for communication:

- **`input in[]`**: Input gates for receiving messages from other nodes.
- **`output out[]`**: Output gates for sending messages to other nodes.

These gates enable the node to interact with its neighbors in the network.

---

## 5. **Key Functionalities**

### 5.1 **Initialization**
- **`initialize()`**: Sets up global parameters, payment channels, and signals. It also reads configuration files (`topologyFile` and `workloadFile`) to initialize the network state.

### 5.2 **Transaction Generation**
- **`generateNextTransaction()`**: Reads the next transaction from the workload file, creates a transaction message, and schedules it for transmission.

### 5.3 **Routing Logic**
- Implements the **LND Baseline Routing** protocol for routing transactions:
  - **Path Pruning**: Temporarily disables edges that fail during transactions.
  - **Retry Mechanisms**: Retries failed transactions multiple times with different paths.
  - **BFS-Based Path Selection**: Uses Breadth-First Search (BFS) to find alternative paths after pruning.

### 5.4 **Payment Channel Management**
- Manages payment channel balances, handles rebalancing, and updates states based on incoming messages.

### 5.5 **Signal Handling**
- Collects statistics and emits signals for monitoring performance metrics.

---

## 6. **LND Baseline-Specific Features**

### 6.1 **Path Pruning**
- **`pruneEdge()`**: Temporarily removes an edge from the active channel list if it fails during a transaction. The edge is restored after a specified period (`_restorePeriod`).

### 6.2 **Retry Mechanisms**
- **`recordTailRetries()`**: Logs the number of retries for successful and failed transactions within a specified time window (`_transStatStart` to `_transStatEnd`).
- **`_numAttemptsLndBaseline`**: Defines the maximum number of retries allowed for a transaction.

### 6.3 **BFS-Based Path Selection**
- **`generateNextPath()`**: Uses BFS to generate the next available path after pruning failed edges.

### 6.4 **Tail Retry Logging**
- **`succRetriesList`**: Logs the number of successful retries.
- **`failRetriesList`**: Logs the number of failed retries.

---

## 7. **Conclusion**

The `hostNodeLndBaseline` module provides a robust implementation of the LND Baseline Routing protocol for efficient transaction routing and retry mechanisms. Its modular design allows developers to enable or disable specific features, making it highly adaptable to different use cases. By understanding the parameters, signals, and functionalities described in this manual, developers can effectively contribute to and maintain the project.

##############################################################

/home/kush/Desktop/spiderproj/spider_omnet/hostNodePriceScheme.ned

simple hostNodePriceScheme
{
	parameters:
	    bool isHost = true;
	    
		string topologyFile = default("sample-topology.txt");
		string workloadFile = default("sample-workload.txt");
                string resultPrefix = default("");

		double simulationLength = default(30.0);
		double statRate = default(0.2);
		bool signalsEnabled = default(false);
		bool loggingEnabled = default(false);
                double splitSize = default(5.0);

		bool timeoutEnabled = default(true);
		double timeoutClearRate = default(0.5);

		bool celerEnabled = default(false);
		bool waterfillingEnabled = default(false);
		bool priceSchemeEnabled = default(false);
		bool smoothWaterfillingEnabled = default(false);
                bool windowEnabled = default(false);
                bool landmarkRoutingEnabled = default(false);
                bool lndBaselineEnabled = default(false);
                bool dctcpEnabled = default(false);

	        bool widestPathsEnabled = default(false);
                bool heuristicPathsEnabled = default(false);
                bool obliviousRoutingEnabled = default(false);
                bool kspYenEnabled = default(false);
                
                bool LIFOEnabled = default(false);
                bool FIFOEnabled = default(false);
                bool SPFEnabled = default(false);
                bool RREnabled = default(false);
                bool EDFEnabled = default(false);
                
                bool changingPathsEnabled = default(false);
                int maxPathsToConsider = default(100);
                double windowThresholdForChange = default(1);
                double pathMonitorRate = default(5);
	        
                double transStatStart = default(3000);
                double transStatEnd = default(5000);
                int numPathChoices = default(4);

                double alpha = default(0.01);
                double kappa = default(0.01);
                double eta = default(0.01);
                double zeta = default(0.01);
                double rhoValue = default(0.05);
                double capacityFactor = default(1.0);
                
                bool useQueueEquation = default(true);
                bool splittingEnabled = default(true);
                double minRate = default(0.25);
                double updateQueryTime = default(0.8);
                double xi = default(1);
                int serviceArrivalWindow = default(100);
                double routerQueueDrainTime = default(1);

                double tau = default(10);
                double normalizer = default(100);

                bool rebalancingEnabled = default(false);
                double queueDelayThreshold = default(3.0);
                double gamma = default(1.0);
                double gammaImbalanceQueueSize = default(5.0);
                double rebalancingRate = default(10.0);
                double rebalancingDelayForAddingFunds = default(0.1);
                double minBalanceComputeRate = default(0.1);
		
		@signal[completionTime](type="simtime_t"); 
		@statistic[completionTime](title="completionTime"; source="completionTime"; record=vector,stats; interpolationmode=none); 
		
		@signal[numCleared](type="long"); 
		@statistic[numCleared](title="numCleared"; source="numCleared"; record=vector,stats; interpolationmode=none); 
		
		@display("i=block/routing"); 

		@signal[numInQueuePerChannel*](type="unsigned long"); 
		@statisticTemplate[numInQueuePerChannelTemplate](record=vector,stats);   

		@signal[numProcessedPerChannel*](type="long");  
		@statisticTemplate[numProcessedPerChannelTemplate](record=vector, stats);  

 		@signal[probabilityPerDestPerPath*](type="double");  
		@statisticTemplate[probabilityPerDestPerPathTemplate](record=vector, stats);  

		@signal[numSentPerChannel*](type="long");  
		@statisticTemplate[numSentPerChannelTemplate](record=vector, stats);   

		@signal[rateCompletedPerDest*](type="long");  
		@statisticTemplate[rateCompletedPerDestTemplate](record=vector, stats);   
		
		@signal[numCompletedPerDest*](type="long");  
		@statisticTemplate[numCompletedPerDestTemplate](record=vector, stats);   

		@signal[numPendingPerDest*](type="long");  
		@statisticTemplate[numPendingPerDestTemplate](record=vector, stats); 

 		@signal[numWaitingPerDest*](type="unsigned long");  
		@statisticTemplate[numWaitingPerDestTemplate](record=vector, stats);  
	
		@signal[fracSuccessfulPerDest*](type="double");  
		@statisticTemplate[fracSuccessfulPerDestTemplate](record=vector, stats);   
		
		@signal[pathPerTransPerDest*](type="long");  
		@statisticTemplate[pathPerTransPerDestTemplate](record=vector, stats);   
		
		@signal[numTimedOutPerDest*](type="long");  
		@statisticTemplate[numTimedOutPerDestTemplate](record=vector, stats);

		@signal[demandEstimatePerDest*](type="double");  
		@statisticTemplate[demandEstimatePerDestTemplate](record=vector, stats);

		@signal[numTimedOutAtSenderPerDest*](type="long");  
		@statisticTemplate[numTimedOutAtSenderPerDestTemplate](record=vector, stats);
	
        	@signal[rateAttemptedPerDest*](type="long"); 
		@statisticTemplate[rateAttemptedPerDestTemplate](record=vector, stats);   
		
                @signal[rateArrivedPerDest*](type="long"); 
		@statisticTemplate[rateArrivedPerDestTemplate](record=vector, stats);  
	
		@signal[numArrivedPerDest*](type="long"); 
		@statisticTemplate[numArrivedPerDestTemplate](record=vector, stats);   

		@signal[balancePerChannel*](type="double"); 
		@statisticTemplate[balancePerChannelTemplate](record=vector, stats);   
		
		@signal[capacityPerChannel*](type="double"); 
		@statisticTemplate[capacityPerChannelTemplate](record=vector, stats);   
 		
                @signal[bankPerChannel*](type="double"); 
		@statisticTemplate[bankPerChannelTemplate](record=vector, stats); 
		
		@signal[timeInFlightPerChannel*](type="double"); 
		@statisticTemplate[timeInFlightPerChannelTemplate](record=vector, stats);   
		
		@signal[bottleneckPerDestPerPath*](type="long"); 
		@statisticTemplate[bottleneckPerDestPerPathTemplate](record=vector, stats);
		
		@signal[probeBackPerDestPerPath*](type="long"); 
		@statisticTemplate[probeBackPerDestPerPathTemplate](record=vector, stats);
		
		@signal[rateCompletedPerDestPerPath*](type="long"); 
		@statisticTemplate[rateCompletedPerDestPerPathTemplate](record=vector, stats);
		
		@signal[rateAttemptedPerDestPerPath*](type="long"); 
		@statisticTemplate[rateAttemptedPerDestPerPathTemplate](record=vector, stats);
		
		//has queue capacity specific signals
		@signal[numFailedPerDest*](type="long");  
		@statisticTemplate[numFailedPerDestTemplate](record=vector, stats);
		
		@signal[rateFailedPerDest*](type="long");  
		@statisticTemplate[rateFailedPerDestTemplate](record=vector, stats);
		
		//price scheme payment channel specific signals
		@signal[nValuePerChannel*](type="double"); 
		@statisticTemplate[nValuePerChannelTemplate](record=vector, stats);
		
		@signal[xLocalPerChannel*](type="double"); 
		@statisticTemplate[xLocalPerChannelTemplate](record=vector, stats);

		@signal[lambdaPerChannel*](type="double"); 
		@statisticTemplate[lambdaPerChannelTemplate](record=vector, stats);
		
		@signal[muLocalPerChannel*](type="double"); 
		@statisticTemplate[muLocalPerChannelTemplate](record=vector, stats);
		
		@signal[muRemotePerChannel*](type="double"); 
		@statisticTemplate[muRemotePerChannelTemplate](record=vector, stats);
		
		@signal[implicitRebalancingAmtPerChannel*](type="double"); 
		@statisticTemplate[implicitRebalancingAmtPerChannelTemplate](record=vector, stats);  
		@signal[explicitRebalancingAmtPerChannel*](type="double"); 
		@statisticTemplate[explicitRebalancingAmtPerChannelTemplate](record=vector, stats);  
		
		//price scheme path specific signals
		@signal[rateToSendTransPerDestPerPath*](type="double"); 
		@statisticTemplate[rateToSendTransPerDestPerPathTemplate](record=vector, stats);

                @signal[rateSentPerDestPerPath*](type="double"); 
		@statisticTemplate[rateSentPerDestPerPathTemplate](record=vector, stats);	
		
                @signal[rateOfAcksPerDestPerPath*](type="double"); 
		@statisticTemplate[rateOfAcksPerDestPerPathTemplate](record=vector, stats);
	
        	@signal[timeToNextSendPerDestPerPath*](type="simtime_t"); 
		@statisticTemplate[timeToNextSendPerDestPerPathTemplate](record=vector, stats);
		
		@signal[sumOfTransUnitsInFlightPerDestPerPath*](type="double"); 
		@statisticTemplate[sumOfTransUnitsInFlightPerDestPerPathTemplate](record=vector, stats);
			
        	@signal[windowPerDestPerPath*](type="double"); 
		@statisticTemplate[windowPerDestPerPathTemplate](record=vector, stats);

		@signal[priceLastSeenPerDestPerPath*](type="double"); 
		@statisticTemplate[priceLastSeenPerDestPerPathTemplate](record=vector, stats);
		
		@signal[isSendTimerSetPerDestPerPath*](type="bool"); 
		@statisticTemplate[isSendTimerSetPerDestPerPathTemplate](record=vector, stats);
		
		
		

	gates:
		input in[];
		output out[];
}


SUMMARY #############################################################

# Developer Manual for `hostNodePriceScheme`

## Table of Contents
1. **Introduction**
   - Purpose of the Manual
   - Overview of the Code
2. **Components of the Code**
   - Parameters
   - Signals and Statistics
   - Gates
3. **Detailed Explanation of Each Section**
   - Parameters (`isHost`, `topologyFile`, etc.)
   - Signals and Statistics (e.g., `completionTime`, `numInQueuePerChannel`)
   - Gates (`in[]`, `out[]`)
4. **Simulation Workflow**
5. **Customization and Extensions**
   - Modifying Parameters
   - Adding New Signals/Statistics
   - Extending Functionality
6. **Best Practices**
7. **Troubleshooting**

---

## 1. Introduction

### Purpose of the Manual
This developer manual provides a comprehensive guide to understanding, customizing, and extending the `hostNodePriceScheme` module. It explains the structure, functionality, and advanced features of the module, enabling developers to effectively integrate it into their simulations.

### Overview of the Code
The `hostNodePriceScheme` module represents a host node designed for network simulations that involve price-based schemes for resource allocation and traffic management. It supports configurable parameters, signals/statistics for monitoring, and input/output gates for communication with other nodes. This module is particularly suited for scenarios involving payment channel networks, traditional IP networks, or custom routing protocols.

---

## 2. Components of the Code

The code is structured into three main sections:
1. **Parameters**: Configurable variables that control the behavior of the host.
2. **Signals and Statistics**: Metrics for monitoring the host's performance and state.
3. **Gates**: Input and output ports for communication.

---

## 3. Detailed Explanation of Each Section

### Parameters
```cpp
parameters:
    bool isHost = true;
    string topologyFile = default("sample-topology.txt");
    string workloadFile = default("sample-workload.txt");
    string resultPrefix = default("");
    double simulationLength = default(30.0);
    double statRate = default(0.2);
    bool signalsEnabled = default(false);
    bool loggingEnabled = default(false);
    double splitSize = default(5.0);
    bool timeoutEnabled = default(true);
    double timeoutClearRate = default(0.5);
    bool celerEnabled = default(false);
    bool waterfillingEnabled = default(false);
    bool priceSchemeEnabled = default(false);
    bool smoothWaterfillingEnabled = default(false);
    bool windowEnabled = default(false);
    bool landmarkRoutingEnabled = default(false);
    bool lndBaselineEnabled = default(false);
    bool dctcpEnabled = default(false);
    bool widestPathsEnabled = default(false);
    bool heuristicPathsEnabled = default(false);
    bool obliviousRoutingEnabled = default(false);
    bool kspYenEnabled = default(false);
    bool LIFOEnabled = default(false);
    bool FIFOEnabled = default(false);
    bool SPFEnabled = default(false);
    bool RREnabled = default(false);
    bool EDFEnabled = default(false);
    bool changingPathsEnabled = default(false);
    int maxPathsToConsider = default(100);
    double windowThresholdForChange = default(1);
    double pathMonitorRate = default(5);
    double transStatStart = default(3000);
    double transStatEnd = default(5000);
    int numPathChoices = default(4);
    double alpha = default(0.01);
    double kappa = default(0.01);
    double eta = default(0.01);
    double zeta = default(0.01);
    double rhoValue = default(0.05);
    double capacityFactor = default(1.0);
    bool useQueueEquation = default(true);
    bool splittingEnabled = default(true);
    double minRate = default(0.25);
    double updateQueryTime = default(0.8);
    double xi = default(1);
    int serviceArrivalWindow = default(100);
    double routerQueueDrainTime = default(1);
    double tau = default(10);
    double normalizer = default(100);
    bool rebalancingEnabled = default(false);
    double queueDelayThreshold = default(3.0);
    double gamma = default(1.0);
    double gammaImbalanceQueueSize = default(5.0);
    double rebalancingRate = default(10.0);
    double rebalancingDelayForAddingFunds = default(0.1);
    double minBalanceComputeRate = default(0.1);
```
- **Purpose**: These parameters define the behavior and configuration of the host.
- **Details**:
  - **Basic Configuration**:
    - `isHost`: Indicates whether this node behaves as a host. Default is `true`.
    - `topologyFile`: Path to the file defining the network topology. Default is `"sample-topology.txt"`.
    - `workloadFile`: Path to the file defining the workload or traffic patterns. Default is `"sample-workload.txt"`.
    - `resultPrefix`: Prefix for saving results (e.g., logs or statistics).
    - `simulationLength`: Duration of the simulation in seconds. Default is `30.0`.
    - `statRate`: Rate at which statistics are recorded. Default is `0.2`.
    - `signalsEnabled`: Enables or disables signal emission. Default is `false`.
    - `loggingEnabled`: Enables or disables logging. Default is `false`.
    - `splitSize`: Size of splits for transactions. Default is `5.0`.
  - **Routing and Resource Allocation**:
    - `waterfillingEnabled`: Enables or disables water-filling-based resource allocation. Default is `false`.
    - `priceSchemeEnabled`: Enables or disables price-based schemes. Default is `false`.
    - `smoothWaterfillingEnabled`: Enables smoother water-filling algorithms. Default is `false`.
    - `landmarkRoutingEnabled`: Enables landmark-based routing. Default is `false`.
    - `lndBaselineEnabled`: Enables LND baseline routing. Default is `false`.
    - `dctcpEnabled`: Enables DCTCP-based congestion control. Default is `false`.
  - **Rebalancing and Queuing**:
    - `rebalancingEnabled`: Enables or disables rebalancing mechanisms. Default is `false`.
    - `queueDelayThreshold`: Threshold for queue delay rebalancing. Default is `3.0`.
    - `gamma`, `gammaImbalanceQueueSize`: Parameters for rebalancing logic.
    - `rebalancingRate`: Rate at which rebalancing occurs. Default is `10.0`.
    - `timeoutEnabled`: Enables or disables timeout mechanisms. Default is `true`.
  - **Advanced Routing and Scheduling**:
    - `widestPathsEnabled`: Enables widest-path routing. Default is `false`.
    - `heuristicPathsEnabled`: Enables heuristic-based path selection. Default is `false`.
    - `kspYenEnabled`: Enables K-shortest paths using Yen's algorithm. Default is `false`.
    - `LIFOEnabled`, `FIFOEnabled`, `SPFEnabled`, `RREnabled`, `EDFEnabled`: Enable different scheduling policies.
    - `changingPathsEnabled`: Enables dynamic path changes during simulation. Default is `false`.

### Signals and Statistics
```cpp
@signal[completionTime](type="simtime_t"); 
@statistic[completionTime](title="completionTime"; source="completionTime"; record=vector,stats; interpolationmode=none); 
...
```
- **Purpose**: Signals and statistics are used to monitor and analyze the host's performance and internal state.
- **Details**:
  - **Signals**: Represent real-time data emitted by the host during simulation.
    - Example: `completionTime` tracks the time taken to complete tasks.
  - **Statistics**: Aggregate data from signals for analysis.
    - Example: `@statistic[completionTime]` records the `completionTime` signal as both a vector and summary statistics (e.g., mean, max).
  - **Key Metrics**:
    - **Transaction Metrics**:
      - `numCleared`: Number of transactions cleared.
      - `numCompletedPerDest*`: Number of completed transactions per destination.
      - `numPendingPerDest*`: Number of pending transactions per destination.
      - `numTimedOutPerDest*`: Number of timed-out transactions per destination.
    - **Queue Metrics**:
      - `numInQueuePerChannel*`: Number of packets in the queue per channel.
      - `numWaitingPerDest*`: Number of waiting transactions per destination.
    - **Resource Metrics**:
      - `balancePerChannel*`: Balance (e.g., available capacity) per channel.
      - `capacityPerChannel*`: Total capacity of each channel.
      - `timeInFlightPerChannel*`: Time packets spend in flight per channel.
    - **Routing Metrics**:
      - `rateCompletedPerDestPerPath*`: Rate of completed transactions per destination and path.
      - `rateAttemptedPerDestPerPath*`: Rate of attempted transactions per destination and path.
    - **Price Scheme Metrics**:
      - `nValuePerChannel*`: Value associated with pricing schemes.
      - `xLocalPerChannel*`, `muLocalPerChannel*`, `muRemotePerChannel*`: Local and remote metrics for pricing algorithms.
    - **Rebalancing Metrics**:
      - `implicitRebalancingAmtPerChannel*`: Amount of implicit rebalancing performed per channel.
      - `explicitRebalancingAmtPerChannel*`: Amount of explicit rebalancing performed per channel.

### Gates
```cpp
gates:
    input in[];
    output out[];
```
- **Purpose**: Gates define the communication interfaces of the host.
- **Details**:
  - `in[]`: Array of input gates for receiving packets from other nodes.
  - `out[]`: Array of output gates for sending packets to other nodes.

---

## 4. Simulation Workflow

1. **Initialization**:
   - The host reads configuration files (`topologyFile` and `workloadFile`) to initialize its state.
   - Parameters like `timeoutEnabled` and `priceSchemeEnabled` determine the host's operational mode.

2. **Packet Processing**:
   - Incoming packets are received via the `in[]` gates and queued for processing.
   - The host applies routing policies (e.g., price-based schemes) to allocate resources (e.g., bandwidth) among channels.
   - Packets are forwarded to the appropriate `out[]` gate based on the routing logic.

3. **Monitoring**:
   - Signals are emitted during packet processing to track metrics like queue length, processing rate, and channel balance.
   - Statistics aggregate these signals for post-simulation analysis.

4. **Completion**:
   - The simulation ends when all packets are processed, or a predefined condition is met.
   - Final statistics are recorded for evaluation.

---

## 5. Customization and Extensions

### Modifying Parameters
- To change the default behavior, update the `parameters` section:
  ```cpp
  parameters:
      bool isHost = false;  // Treat this node as a router
      string topologyFile = "custom-topology.txt";  // Use a custom topology file
      bool priceSchemeEnabled = true;  // Enable price scheme
  ```

### Adding New Signals/Statistics
- To monitor additional metrics, define new signals and statistics:
  ```cpp
  @signal[newMetric](type="double");
  @statistic[newMetric](title="New Metric"; source="newMetric"; record=vector,stats);
  ```
- Emit the signal in the host's logic:
  ```cpp
  emit(newMetric, value);
  ```

### Extending Functionality
- Add new methods or algorithms to the host's logic (e.g., custom rebalancing strategies).
- Modify the topology and workload files to simulate different scenarios.

---

## 6. Best Practices

1. **Use Descriptive File Names**:
   - Name topology and workload files clearly to reflect their content.

2. **Leverage Default Parameters**:
   - Use default values for parameters unless specific changes are required.

3. **Monitor Key Metrics**:
   - Focus on critical signals like `balancePerChannel*`, `serviceRatePerChannel*`, and `queueDelayEWMAPerChannel*` to identify bottlenecks.

4. **Test Incrementally**:
   - Test the host with small topologies and workloads before scaling up.

5. **Document Changes**:
   - Add comments to explain any customizations or extensions made to the code.

---

## 7. Troubleshooting

### Common Issues
1. **Undefined Files**:
   - Ensure that `topologyFile` and `workloadFile` exist and are correctly formatted.

2. **Signal Errors**:
   - Verify that all signals are properly defined and emitted in the host's logic.

3. **Performance Issues**:
   - If the simulation runs slowly, reduce the number of nodes or simplify the topology.

### Debugging Tips
- Use logging or debugging tools to trace packet flow and identify bottlenecks.
- Check the simulation logs for errors or warnings related to undefined parameters or invalid configurations.

---

By following this manual, developers can effectively understand, customize, and extend the `hostNodePriceScheme` module to suit their needs.

#####################################################################

/home/kush/Desktop/spiderproj/spider_omnet/hostNodePropFairPriceScheme.ned

simple hostNodePropFairPriceScheme
{
	parameters:
	    bool isHost = true;
	    
		string topologyFile = default("sample-topology.txt");
		string workloadFile = default("sample-workload.txt");
                string resultPrefix = default("");

		double simulationLength = default(30.0);
		double statRate = default(0.2);
		bool signalsEnabled = default(false);
		bool loggingEnabled = default(false);
                double splitSize = default(5.0);

		bool timeoutEnabled = default(true);
		double timeoutClearRate = default(0.5);

		bool celerEnabled = default(false);
		bool waterfillingEnabled = default(false);
		bool priceSchemeEnabled = default(false);
		bool smoothWaterfillingEnabled = default(false);
                bool windowEnabled = default(false);
                bool landmarkRoutingEnabled = default(false);
                bool lndBaselineEnabled = default(false);
                bool dctcpEnabled = default(false);

	        bool widestPathsEnabled = default(false);
                bool heuristicPathsEnabled = default(false);
                bool obliviousRoutingEnabled = default(false);
                bool kspYenEnabled = default(false);
                
                bool LIFOEnabled = default(false);
                bool FIFOEnabled = default(false);
                bool SPFEnabled = default(false);
                bool RREnabled = default(false);
                bool EDFEnabled = default(false);
                
                bool changingPathsEnabled = default(false);
                int maxPathsToConsider = default(100);
                double windowThresholdForChange = default(1);
                double pathMonitorRate = default(5);
	        
                double transStatStart = default(3000);
                double transStatEnd = default(5000);
                int numPathChoices = default(4);

                double alpha = default(0.01);
                double kappa = default(0.01);
                double eta = default(0.01);
                double zeta = default(0.01);
                double rhoValue = default(0.05);
                double capacityFactor = default(1.0);
                
                double windowAlpha = default(0.2);
                double windowBeta = default(0.2);
                double queueThreshold = default(30);
                double balanceThreshold = default(0.01);
                double minDCTCPWindow = default(5.0);
                double rateDecreaseFrequency = default(5.0);

                bool useQueueEquation = default(true);
                bool splittingEnabled = default(true);
                double minRate = default(0.25);
                double updateQueryTime = default(0.8);
                double xi = default(1);
                int serviceArrivalWindow = default(100);
                double routerQueueDrainTime = default(1);

                double tau = default(10);
                double normalizer = default(100);

                bool rebalancingEnabled = default(false);
                double queueDelayThreshold = default(3.0);
                double gamma = default(1.0);
                double gammaImbalanceQueueSize = default(5.0);
                double rebalancingRate = default(10.0);
                double rebalancingDelayForAddingFunds = default(0.1);
                double minBalanceComputeRate = default(0.1);
		
		@signal[completionTime](type="simtime_t"); 
		@statistic[completionTime](title="completionTime"; source="completionTime"; record=vector,stats; interpolationmode=none); 
		
		@signal[numCleared](type="long"); 
		@statistic[numCleared](title="numCleared"; source="numCleared"; record=vector,stats; interpolationmode=none); 
		
		@display("i=block/routing"); 

		@signal[numInQueuePerChannel*](type="double"); 
		@statisticTemplate[numInQueuePerChannelTemplate](record=vector,stats);   

		@signal[numProcessedPerChannel*](type="long");  
		@statisticTemplate[numProcessedPerChannelTemplate](record=vector, stats);  

 		@signal[probabilityPerDestPerPath*](type="double");  
		@statisticTemplate[probabilityPerDestPerPathTemplate](record=vector, stats);  

		@signal[numSentPerChannel*](type="long");  
		@statisticTemplate[numSentPerChannelTemplate](record=vector, stats);   

		@signal[rateCompletedPerDest*](type="long");  
		@statisticTemplate[rateCompletedPerDestTemplate](record=vector, stats);   
		
		@signal[numCompletedPerDest*](type="long");  
		@statisticTemplate[numCompletedPerDestTemplate](record=vector, stats);   

		@signal[numPendingPerDest*](type="long");  
		@statisticTemplate[numPendingPerDestTemplate](record=vector, stats); 

 		@signal[numWaitingPerDest*](type="unsigned long");  
		@statisticTemplate[numWaitingPerDestTemplate](record=vector, stats);  
	
		@signal[fracSuccessfulPerDest*](type="double");  
		@statisticTemplate[fracSuccessfulPerDestTemplate](record=vector, stats);   
		
		@signal[pathPerTransPerDest*](type="long");  
		@statisticTemplate[pathPerTransPerDestTemplate](record=vector, stats);   
		
		@signal[numTimedOutPerDest*](type="long");  
		@statisticTemplate[numTimedOutPerDestTemplate](record=vector, stats);

		@signal[demandEstimatePerDest*](type="double");  
		@statisticTemplate[demandEstimatePerDestTemplate](record=vector, stats);

		@signal[numTimedOutAtSenderPerDest*](type="long");  
		@statisticTemplate[numTimedOutAtSenderPerDestTemplate](record=vector, stats);
	
        	@signal[rateAttemptedPerDest*](type="long"); 
		@statisticTemplate[rateAttemptedPerDestTemplate](record=vector, stats);   
		
                @signal[rateArrivedPerDest*](type="long"); 
		@statisticTemplate[rateArrivedPerDestTemplate](record=vector, stats);  
	
		@signal[numArrivedPerDest*](type="long"); 
		@statisticTemplate[numArrivedPerDestTemplate](record=vector, stats);   

		@signal[balancePerChannel*](type="double"); 
		@statisticTemplate[balancePerChannelTemplate](record=vector, stats);   
		
		@signal[capacityPerChannel*](type="double"); 
		@statisticTemplate[capacityPerChannelTemplate](record=vector, stats);   
 		
                @signal[bankPerChannel*](type="double"); 
		@statisticTemplate[bankPerChannelTemplate](record=vector, stats); 
		
		@signal[timeInFlightPerChannel*](type="double"); 
		@statisticTemplate[timeInFlightPerChannelTemplate](record=vector, stats);   
		
		@signal[bottleneckPerDestPerPath*](type="long"); 
		@statisticTemplate[bottleneckPerDestPerPathTemplate](record=vector, stats);
		
		@signal[probeBackPerDestPerPath*](type="long"); 
		@statisticTemplate[probeBackPerDestPerPathTemplate](record=vector, stats);
		
		@signal[rateCompletedPerDestPerPath*](type="long"); 
		@statisticTemplate[rateCompletedPerDestPerPathTemplate](record=vector, stats);
		
		@signal[rateAttemptedPerDestPerPath*](type="long"); 
		@statisticTemplate[rateAttemptedPerDestPerPathTemplate](record=vector, stats);
		
		//has queue capacity specific signals
		@signal[numFailedPerDest*](type="long");  
		@statisticTemplate[numFailedPerDestTemplate](record=vector, stats);
		
		@signal[rateFailedPerDest*](type="long");  
		@statisticTemplate[rateFailedPerDestTemplate](record=vector, stats);
		
		//price scheme payment channel specific signals
		@signal[nValuePerChannel*](type="double"); 
		@statisticTemplate[nValuePerChannelTemplate](record=vector, stats);
		
		@signal[xLocalPerChannel*](type="double"); 
		@statisticTemplate[xLocalPerChannelTemplate](record=vector, stats);

		@signal[lambdaPerChannel*](type="double"); 
		@statisticTemplate[lambdaPerChannelTemplate](record=vector, stats);
		
		@signal[muLocalPerChannel*](type="double"); 
		@statisticTemplate[muLocalPerChannelTemplate](record=vector, stats);
		
		@signal[muRemotePerChannel*](type="double"); 
		@statisticTemplate[muRemotePerChannelTemplate](record=vector, stats);
		
		@signal[implicitRebalancingAmtPerChannel*](type="double"); 
		@statisticTemplate[implicitRebalancingAmtPerChannelTemplate](record=vector, stats);  
		@signal[explicitRebalancingAmtPerChannel*](type="double"); 
		@statisticTemplate[explicitRebalancingAmtPerChannelTemplate](record=vector, stats);  
		
		//price scheme path specific signals
		@signal[rateToSendTransPerDestPerPath*](type="double"); 
		@statisticTemplate[rateToSendTransPerDestPerPathTemplate](record=vector, stats);

                @signal[rateSentPerDestPerPath*](type="double"); 
		@statisticTemplate[rateSentPerDestPerPathTemplate](record=vector, stats);	
		
                @signal[rateOfAcksPerDestPerPath*](type="double"); 
		@statisticTemplate[rateOfAcksPerDestPerPathTemplate](record=vector, stats);
	
                @signal[fractionMarkedPerDestPerPath*](type="double"); 
		@statisticTemplate[fractionMarkedPerDestPerPathTemplate](record=vector, stats);
                
                @signal[smoothedFractionMarkedPerDestPerPath*](type="double"); 
		@statisticTemplate[smoothedFractionMarkedPerDestPerPathTemplate](record=vector, stats);
        	
                @signal[measuredRTTPerDestPerPath*](type="double"); 
		@statisticTemplate[measuredRTTPerDestPerPathTemplate](record=vector, stats);
               
                 @signal[timeToNextSendPerDestPerPath*](type="simtime_t"); 
		@statisticTemplate[timeToNextSendPerDestPerPathTemplate](record=vector, stats);
		
		@signal[sumOfTransUnitsInFlightPerDestPerPath*](type="double"); 
		@statisticTemplate[sumOfTransUnitsInFlightPerDestPerPathTemplate](record=vector, stats);
			
        	@signal[windowPerDestPerPath*](type="double"); 
		@statisticTemplate[windowPerDestPerPathTemplate](record=vector, stats);

		@signal[priceLastSeenPerDestPerPath*](type="double"); 
		@statisticTemplate[priceLastSeenPerDestPerPathTemplate](record=vector, stats);
		
		@signal[isSendTimerSetPerDestPerPath*](type="bool"); 
		@statisticTemplate[isSendTimerSetPerDestPerPathTemplate](record=vector, stats);
		
		
		

	gates:
		input in[];
		output out[];
}


summary ####################################################################

# Developer Manual for `hostNodePropFairPriceScheme`

## Table of Contents
1. **Introduction**
   - Purpose of the Manual
   - Overview of the Code
2. **Components of the Code**
   - Parameters
   - Signals and Statistics
   - Gates
3. **Detailed Explanation of Each Section**
   - Parameters (`isHost`, `topologyFile`, etc.)
   - Signals and Statistics (e.g., `completionTime`, `numInQueuePerChannel`)
   - Gates (`in[]`, `out[]`)
4. **Simulation Workflow**
5. **Customization and Extensions**
   - Modifying Parameters
   - Adding New Signals/Statistics
   - Extending Functionality
6. **Best Practices**
7. **Troubleshooting**

---

## 1. Introduction

### Purpose of the Manual
This developer manual provides a comprehensive guide to understanding, customizing, and extending the `hostNodePropFairPriceScheme` module. It explains the structure, functionality, and advanced features of the module, enabling developers to effectively integrate it into their simulations.

### Overview of the Code
The `hostNodePropFairPriceScheme` module represents a host node designed for network simulations that involve proportional fairness and price-based schemes for resource allocation and traffic management. It supports configurable parameters, signals/statistics for monitoring, and input/output gates for communication with other nodes. This module is particularly suited for scenarios involving payment channel networks, traditional IP networks, or custom routing protocols.

---

## 2. Components of the Code

The code is structured into three main sections:
1. **Parameters**: Configurable variables that control the behavior of the host.
2. **Signals and Statistics**: Metrics for monitoring the host's performance and state.
3. **Gates**: Input and output ports for communication.

---

## 3. Detailed Explanation of Each Section

### Parameters
```cpp
parameters:
    bool isHost = true;
    string topologyFile = default("sample-topology.txt");
    string workloadFile = default("sample-workload.txt");
    string resultPrefix = default("");
    double simulationLength = default(30.0);
    double statRate = default(0.2);
    bool signalsEnabled = default(false);
    bool loggingEnabled = default(false);
    double splitSize = default(5.0);
    bool timeoutEnabled = default(true);
    double timeoutClearRate = default(0.5);
    bool celerEnabled = default(false);
    bool waterfillingEnabled = default(false);
    bool priceSchemeEnabled = default(false);
    bool smoothWaterfillingEnabled = default(false);
    bool windowEnabled = default(false);
    bool landmarkRoutingEnabled = default(false);
    bool lndBaselineEnabled = default(false);
    bool dctcpEnabled = default(false);
    bool widestPathsEnabled = default(false);
    bool heuristicPathsEnabled = default(false);
    bool obliviousRoutingEnabled = default(false);
    bool kspYenEnabled = default(false);
    bool LIFOEnabled = default(false);
    bool FIFOEnabled = default(false);
    bool SPFEnabled = default(false);
    bool RREnabled = default(false);
    bool EDFEnabled = default(false);
    bool changingPathsEnabled = default(false);
    int maxPathsToConsider = default(100);
    double windowThresholdForChange = default(1);
    double pathMonitorRate = default(5);
    double transStatStart = default(3000);
    double transStatEnd = default(5000);
    int numPathChoices = default(4);
    double alpha = default(0.01);
    double kappa = default(0.01);
    double eta = default(0.01);
    double zeta = default(0.01);
    double rhoValue = default(0.05);
    double capacityFactor = default(1.0);
    double windowAlpha = default(0.2);
    double windowBeta = default(0.2);
    double queueThreshold = default(30);
    double balanceThreshold = default(0.01);
    double minDCTCPWindow = default(5.0);
    double rateDecreaseFrequency = default(5.0);
    bool useQueueEquation = default(true);
    bool splittingEnabled = default(true);
    double minRate = default(0.25);
    double updateQueryTime = default(0.8);
    double xi = default(1);
    int serviceArrivalWindow = default(100);
    double routerQueueDrainTime = default(1);
    double tau = default(10);
    double normalizer = default(100);
    bool rebalancingEnabled = default(false);
    double queueDelayThreshold = default(3.0);
    double gamma = default(1.0);
    double gammaImbalanceQueueSize = default(5.0);
    double rebalancingRate = default(10.0);
    double rebalancingDelayForAddingFunds = default(0.1);
    double minBalanceComputeRate = default(0.1);
```
- **Purpose**: These parameters define the behavior and configuration of the host.
- **Details**:
  - **Basic Configuration**:
    - `isHost`: Indicates whether this node behaves as a host. Default is `true`.
    - `topologyFile`: Path to the file defining the network topology. Default is `"sample-topology.txt"`.
    - `workloadFile`: Path to the file defining the workload or traffic patterns. Default is `"sample-workload.txt"`.
    - `resultPrefix`: Prefix for saving results (e.g., logs or statistics).
    - `simulationLength`: Duration of the simulation in seconds. Default is `30.0`.
    - `statRate`: Rate at which statistics are recorded. Default is `0.2`.
    - `signalsEnabled`: Enables or disables signal emission. Default is `false`.
    - `loggingEnabled`: Enables or disables logging. Default is `false`.
    - `splitSize`: Size of splits for transactions. Default is `5.0`.
  - **Routing and Resource Allocation**:
    - `waterfillingEnabled`: Enables or disables water-filling-based resource allocation. Default is `false`.
    - `priceSchemeEnabled`: Enables or disables price-based schemes. Default is `false`.
    - `smoothWaterfillingEnabled`: Enables smoother water-filling algorithms. Default is `false`.
    - `landmarkRoutingEnabled`: Enables landmark-based routing. Default is `false`.
    - `lndBaselineEnabled`: Enables LND baseline routing. Default is `false`.
    - `dctcpEnabled`: Enables DCTCP-based congestion control. Default is `false`.
  - **Rebalancing and Queuing**:
    - `rebalancingEnabled`: Enables or disables rebalancing mechanisms. Default is `false`.
    - `queueDelayThreshold`: Threshold for queue delay rebalancing. Default is `3.0`.
    - `gamma`, `gammaImbalanceQueueSize`: Parameters for rebalancing logic.
    - `rebalancingRate`: Rate at which rebalancing occurs. Default is `10.0`.
    - `timeoutEnabled`: Enables or disables timeout mechanisms. Default is `true`.
  - **Advanced Routing and Scheduling**:
    - `widestPathsEnabled`: Enables widest-path routing. Default is `false`.
    - `heuristicPathsEnabled`: Enables heuristic-based path selection. Default is `false`.
    - `kspYenEnabled`: Enables K-shortest paths using Yen's algorithm. Default is `false`.
    - `LIFOEnabled`, `FIFOEnabled`, `SPFEnabled`, `RREnabled`, `EDFEnabled`: Enable different scheduling policies.
    - `changingPathsEnabled`: Enables dynamic path changes during simulation. Default is `false`.

### Signals and Statistics
```cpp
@signal[completionTime](type="simtime_t"); 
@statistic[completionTime](title="completionTime"; source="completionTime"; record=vector,stats; interpolationmode=none); 
...
```
- **Purpose**: Signals and statistics are used to monitor and analyze the host's performance and internal state.
- **Details**:
  - **Signals**: Represent real-time data emitted by the host during simulation.
    - Example: `completionTime` tracks the time taken to complete tasks.
  - **Statistics**: Aggregate data from signals for analysis.
    - Example: `@statistic[completionTime]` records the `completionTime` signal as both a vector and summary statistics (e.g., mean, max).
  - **Key Metrics**:
    - **Transaction Metrics**:
      - `numCleared`: Number of transactions cleared.
      - `numCompletedPerDest*`: Number of completed transactions per destination.
      - `numPendingPerDest*`: Number of pending transactions per destination.
      - `numTimedOutPerDest*`: Number of timed-out transactions per destination.
    - **Queue Metrics**:
      - `numInQueuePerChannel*`: Number of packets in the queue per channel.
      - `numWaitingPerDest*`: Number of waiting transactions per destination.
    - **Resource Metrics**:
      - `balancePerChannel*`: Balance (e.g., available capacity) per channel.
      - `capacityPerChannel*`: Total capacity of each channel.
      - `timeInFlightPerChannel*`: Time packets spend in flight per channel.
    - **Routing Metrics**:
      - `rateCompletedPerDestPerPath*`: Rate of completed transactions per destination and path.
      - `rateAttemptedPerDestPerPath*`: Rate of attempted transactions per destination and path.
    - **Price Scheme Metrics**:
      - `nValuePerChannel*`: Value associated with pricing schemes.
      - `xLocalPerChannel*`, `muLocalPerChannel*`, `muRemotePerChannel*`: Local and remote metrics for pricing algorithms.
    - **Rebalancing Metrics**:
      - `implicitRebalancingAmtPerChannel*`: Amount of implicit rebalancing performed per channel.
      - `explicitRebalancingAmtPerChannel*`: Amount of explicit rebalancing performed per channel.

### Gates
```cpp
gates:
    input in[];
    output out[];
```
- **Purpose**: Gates define the communication interfaces of the host.
- **Details**:
  - `in[]`: Array of input gates for receiving packets from other nodes.
  - `out[]`: Array of output gates for sending packets to other nodes.

---

## 4. Simulation Workflow

1. **Initialization**:
   - The host reads configuration files (`topologyFile` and `workloadFile`) to initialize its state.
   - Parameters like `timeoutEnabled` and `priceSchemeEnabled` determine the host's operational mode.

2. **Packet Processing**:
   - Incoming packets are received via the `in[]` gates and queued for processing.
   - The host applies routing policies (e.g., proportional fairness or price schemes) to allocate resources (e.g., bandwidth) among channels.
   - Packets are forwarded to the appropriate `out[]` gate based on the routing logic.

3. **Monitoring**:
   - Signals are emitted during packet processing to track metrics like queue length, processing rate, and channel balance.
   - Statistics aggregate these signals for post-simulation analysis.

4. **Completion**:
   - The simulation ends when all packets are processed, or a predefined condition is met.
   - Final statistics are recorded for evaluation.

---

## 5. Customization and Extensions

### Modifying Parameters
- To change the default behavior, update the `parameters` section:
  ```cpp
  parameters:
      bool isHost = false;  // Treat this node as a router
      string topologyFile = "custom-topology.txt";  // Use a custom topology file
      bool priceSchemeEnabled = true;  // Enable price scheme
  ```

### Adding New Signals/Statistics
- To monitor additional metrics, define new signals and statistics:
  ```cpp
  @signal[newMetric](type="double");
  @statistic[newMetric](title="New Metric"; source="newMetric"; record=vector,stats);
  ```
- Emit the signal in the host's logic:
  ```cpp
  emit(newMetric, value);
  ```

### Extending Functionality
- Add new methods or algorithms to the host's logic (e.g., custom rebalancing strategies).
- Modify the topology and workload files to simulate different scenarios.

---

## 6. Best Practices

1. **Use Descriptive File Names**:
   - Name topology and workload files clearly to reflect their content.

2. **Leverage Default Parameters**:
   - Use default values for parameters unless specific changes are required.

3. **Monitor Key Metrics**:
   - Focus on critical signals like `balancePerChannel*`, `serviceRatePerChannel*`, and `queueDelayEWMAPerChannel*` to identify bottlenecks.

4. **Test Incrementally**:
   - Test the host with small topologies and workloads before scaling up.

5. **Document Changes**:
   - Add comments to explain any customizations or extensions made to the code.

---

## 7. Troubleshooting

### Common Issues
1. **Undefined Files**:
   - Ensure that `topologyFile` and `workloadFile` exist and are correctly formatted.

2. **Signal Errors**:
   - Verify that all signals are properly defined and emitted in the host's logic.

3. **Performance Issues**:
   - If the simulation runs slowly, reduce the number of nodes or simplify the topology.

### Debugging Tips
- Use logging or debugging tools to trace packet flow and identify bottlenecks.
- Check the simulation logs for errors or warnings related to undefined parameters or invalid configurations.

---

By following this manual, developers can effectively understand, customize, and extend the `hostNodePropFairPriceScheme` module to suit their needs.


###########################################################################

/home/kush/Desktop/spiderproj/spider_omnet/hostNodeWaterfilling.ned

simple hostNodeWaterfilling
{
	parameters:
	    bool isHost = true;
	    
		string topologyFile = default("sample-topology.txt");
		string workloadFile = default("sample-workload.txt");
                string resultPrefix = default("");

		double simulationLength = default(30.0);
		double statRate = default(0.2);
		bool signalsEnabled = default(false);
		bool loggingEnabled = default(false);
                double splitSize = default(5.0);

		bool timeoutEnabled = default(true);
		double timeoutClearRate = default(0.5);

		bool celerEnabled = default(false);
		bool waterfillingEnabled = default(false);
		bool priceSchemeEnabled = default(false);
		bool smoothWaterfillingEnabled = default(false);
                bool landmarkRoutingEnabled = default(false);
                bool lndBaselineEnabled = default(false);
                bool splittingEnabled = default(true);
                bool dctcpEnabled = default(false);

                bool rebalancingEnabled = default(false);
                double queueDelayThreshold = default(3.0);
                double gamma = default(1.0);
                double gammaImbalanceQueueSize = default(5.0);
                double rebalancingRate = default(10.0);
                double rebalancingDelayForAddingFunds = default(0.1);
                double minBalanceComputeRate = default(0.1);
                
                double transStatStart = default(3000);
                double transStatEnd = default(5000);
               
                bool widestPathsEnabled = default(false);
                bool heuristicPathsEnabled = default(false);
                bool obliviousRoutingEnabled = default(false);
                bool kspYenEnabled = default(false);
                
                bool LIFOEnabled = default(false);
                bool FIFOEnabled = default(false);
                bool SPFEnabled = default(false);
                bool RREnabled = default(false);
                bool EDFEnabled = default(false);
                
                bool changingPathsEnabled = default(false);
                int maxPathsToConsider = default(100);
                double windowThresholdForChange = default(1);
                double pathMonitorRate = default(5);
                
                bool windowEnabled = default(false);
		int numPathChoices = default(4);

                double alpha = default(0.01);
                double kappa = default(0.01);
                double eta = default(0.01);
                double zeta = default(0.01);
                double rhoValue = default(0.05);
                double minRate = default(0.25);
                double updateQueryTime = default(0.8);
                double xi = default(1);
                int serviceArrivalWindow = default(100);
                double routerQueueDrainTime = default(1);

                double tau = default(10);
                double normalizer = default(100);

		
		@signal[completionTime](type="simtime_t"); 
		@statistic[completionTime](title="completionTime"; source="completionTime"; record=vector,stats; interpolationmode=none); 
		
		@signal[numCleared](type="long"); 
		@statistic[numCleared](title="numCleared"; source="numCleared"; record=vector,stats; interpolationmode=none); 
		
		@display("i=block/routing"); 

		@signal[numInQueuePerChannel*](type="double"); 
		@statisticTemplate[numInQueuePerChannelTemplate](record=vector,stats);   

                @signal[numInflightPerChannel*](type="unsigned long"); 
		@statisticTemplate[numInflightPerChannelTemplate](record=vector, stats);
		
                @signal[numProcessedPerChannel*](type="long");  
		@statisticTemplate[numProcessedPerChannelTemplate](record=vector, stats);  

 		@signal[probabilityPerDestPerPath*](type="double");  
		@statisticTemplate[probabilityPerDestPerPathTemplate](record=vector, stats);  

		@signal[numSentPerChannel*](type="long");  
		@statisticTemplate[numSentPerChannelTemplate](record=vector, stats);   
                
                @signal[bankPerChannel*](type="double"); 
		@statisticTemplate[bankPerChannelTemplate](record=vector, stats); 
		
                @signal[rateCompletedPerDest*](type="long");  
		@statisticTemplate[rateCompletedPerDestTemplate](record=vector, stats);   
		
		@signal[numCompletedPerDest*](type="long");  
		@statisticTemplate[numCompletedPerDestTemplate](record=vector, stats);   

		@signal[numPendingPerDest*](type="long");  
		@statisticTemplate[numPendingPerDestTemplate](record=vector, stats); 

 		@signal[numWaitingPerDest*](type="long");  
		@statisticTemplate[numWaitingPerDestTemplate](record=vector, stats);  
	
		@signal[fracSuccessfulPerDest*](type="double");  
		@statisticTemplate[fracSuccessfulPerDestTemplate](record=vector, stats);   
		
		@signal[pathPerTransPerDest*](type="long");  
		@statisticTemplate[pathPerTransPerDestTemplate](record=vector, stats);   
		
		@signal[numTimedOutPerDest*](type="long");  
		@statisticTemplate[numTimedOutPerDestTemplate](record=vector, stats);

		@signal[demandEstimatePerDest*](type="double");  
		@statisticTemplate[demandEstimatePerDestTemplate](record=vector, stats);

		@signal[numTimedOutAtSenderPerDest*](type="long");  
		@statisticTemplate[numTimedOutAtSenderPerDestTemplate](record=vector, stats);
	
        	@signal[rateAttemptedPerDest*](type="long"); 
		@statisticTemplate[rateAttemptedPerDestTemplate](record=vector, stats);   
		
                @signal[rateArrivedPerDest*](type="long"); 
		@statisticTemplate[rateArrivedPerDestTemplate](record=vector, stats);  
	
		@signal[numArrivedPerDest*](type="long"); 
		@statisticTemplate[numArrivedPerDestTemplate](record=vector, stats);   

		@signal[balancePerChannel*](type="double"); 
		@statisticTemplate[balancePerChannelTemplate](record=vector, stats);   
		
		@signal[capacityPerChannel*](type="double"); 
		@statisticTemplate[capacityPerChannelTemplate](record=vector, stats);   
		
		@signal[timeInFlightPerChannel*](type="double"); 
		@statisticTemplate[timeInFlightPerChannelTemplate](record=vector, stats);   
		
		@signal[bottleneckPerDestPerPath*](type="long"); 
		@statisticTemplate[bottleneckPerDestPerPathTemplate](record=vector, stats);
		
		@signal[probeBackPerDestPerPath*](type="long"); 
		@statisticTemplate[probeBackPerDestPerPathTemplate](record=vector, stats);
		
		@signal[rateCompletedPerDestPerPath*](type="long"); 
		@statisticTemplate[rateCompletedPerDestPerPathTemplate](record=vector, stats);
		
		@signal[rateAttemptedPerDestPerPath*](type="long"); 
		@statisticTemplate[rateAttemptedPerDestPerPathTemplate](record=vector, stats);
		
		//has queue capacity specific signals
		@signal[numFailedPerDest*](type="long");  
		@statisticTemplate[numFailedPerDestTemplate](record=vector, stats);
		
		@signal[rateFailedPerDest*](type="long");  
		@statisticTemplate[rateFailedPerDestTemplate](record=vector, stats);
		
		//price scheme payment channel specific signals
		@signal[nValuePerChannel*](type="long"); 
		@statisticTemplate[nValuePerChannelTemplate](record=vector, stats);
		
		@signal[xLocalPerChannel*](type="double"); 
		@statisticTemplate[xLocalPerChannelTemplate](record=vector, stats);

		@signal[lambdaPerChannel*](type="double"); 
		@statisticTemplate[lambdaPerChannelTemplate](record=vector, stats);
		
		@signal[muLocalPerChannel*](type="double"); 
		@statisticTemplate[muLocalPerChannelTemplate](record=vector, stats);
		
		@signal[muRemotePerChannel*](type="double"); 
		@statisticTemplate[muRemotePerChannelTemplate](record=vector, stats);
		
                @signal[implicitRebalancingAmtPerChannel*](type="double"); 
		@statisticTemplate[implicitRebalancingAmtPerChannelTemplate](record=vector, stats);  
		@signal[explicitRebalancingAmtPerChannel*](type="double"); 
		@statisticTemplate[explicitRebalancingAmtPerChannelTemplate](record=vector, stats);  
		
		//price scheme path specific signals
		@signal[rateToSendTransPerDestPerPath*](type="double"); 
		@statisticTemplate[rateToSendTransPerDestPerPathTemplate](record=vector, stats);
		
		@signal[timeToNextSendPerDestPerPath*](type="simtime_t"); 
		@statisticTemplate[timeToNextSendPerDestPerPathTemplate](record=vector, stats);
		
		@signal[sumOfTransUnitsInFlightPerDestPerPath*](type="double"); 
		@statisticTemplate[sumOfTransUnitsInFlightPerDestPerPathTemplate](record=vector, stats);
		
		@signal[priceLastSeenPerDestPerPath*](type="double"); 
		@statisticTemplate[priceLastSeenPerDestPerPathTemplate](record=vector, stats);
		
		@signal[isSendTimerSetPerDestPerPath*](type="long"); 
		@statisticTemplate[isSendTimerSetPerDestPerPathTemplate](record=vector, stats);
		
		@signal[sumOfTransUnitsInFlightPerDestPerPath*](type="double"); 
		@statisticTemplate[sumOfTransUnitsInFlightPerDestPerPathTemplate](record=vector, stats);
			
        	@signal[windowPerDestPerPath*](type="double"); 
		@statisticTemplate[windowPerDestPerPathTemplate](record=vector, stats);		
		

	gates:
		input in[];
		output out[];
}


SUMMARY #######################################################

# Developer Manual for `hostNodeWaterfilling`

## Table of Contents
1. **Introduction**
   - Purpose of the Manual
   - Overview of the Code
2. **Components of the Code**
   - Parameters
   - Signals and Statistics
   - Gates
3. **Detailed Explanation of Each Section**
   - Parameters (`isHost`, `topologyFile`, etc.)
   - Signals and Statistics (e.g., `completionTime`, `numInQueuePerChannel`)
   - Gates (`in[]`, `out[]`)
4. **Simulation Workflow**
5. **Customization and Extensions**
   - Modifying Parameters
   - Adding New Signals/Statistics
   - Extending Functionality
6. **Best Practices**
7. **Troubleshooting**

---

## 1. Introduction

### Purpose of the Manual
This developer manual provides a comprehensive guide to understanding, customizing, and extending the `hostNodeWaterfilling` module. It explains the structure, functionality, and advanced features of the module, enabling developers to effectively integrate it into their simulations.

### Overview of the Code
The `hostNodeWaterfilling` module represents a host node designed for network simulations that involve water-filling-based resource allocation and traffic management. It supports configurable parameters, signals/statistics for monitoring, and input/output gates for communication with other nodes. This module is particularly suited for scenarios involving payment channel networks, traditional IP networks, or custom routing protocols.

---

## 2. Components of the Code

The code is structured into three main sections:
1. **Parameters**: Configurable variables that control the behavior of the host.
2. **Signals and Statistics**: Metrics for monitoring the host's performance and state.
3. **Gates**: Input and output ports for communication.

---

## 3. Detailed Explanation of Each Section

### Parameters
```cpp
parameters:
    bool isHost = true;
    string topologyFile = default("sample-topology.txt");
    string workloadFile = default("sample-workload.txt");
    string resultPrefix = default("");
    double simulationLength = default(30.0);
    double statRate = default(0.2);
    bool signalsEnabled = default(false);
    bool loggingEnabled = default(false);
    double splitSize = default(5.0);
    bool timeoutEnabled = default(true);
    double timeoutClearRate = default(0.5);
    bool celerEnabled = default(false);
    bool waterfillingEnabled = default(false);
    bool priceSchemeEnabled = default(false);
    bool smoothWaterfillingEnabled = default(false);
    bool landmarkRoutingEnabled = default(false);
    bool lndBaselineEnabled = default(false);
    bool splittingEnabled = default(true);
    bool dctcpEnabled = default(false);
    bool rebalancingEnabled = default(false);
    double queueDelayThreshold = default(3.0);
    double gamma = default(1.0);
    double gammaImbalanceQueueSize = default(5.0);
    double rebalancingRate = default(10.0);
    double rebalancingDelayForAddingFunds = default(0.1);
    double minBalanceComputeRate = default(0.1);
    double transStatStart = default(3000);
    double transStatEnd = default(5000);
    bool widestPathsEnabled = default(false);
    bool heuristicPathsEnabled = default(false);
    bool obliviousRoutingEnabled = default(false);
    bool kspYenEnabled = default(false);
    bool LIFOEnabled = default(false);
    bool FIFOEnabled = default(false);
    bool SPFEnabled = default(false);
    bool RREnabled = default(false);
    bool EDFEnabled = default(false);
    bool changingPathsEnabled = default(false);
    int maxPathsToConsider = default(100);
    double windowThresholdForChange = default(1);
    double pathMonitorRate = default(5);
    bool windowEnabled = default(false);
    int numPathChoices = default(4);
    double alpha = default(0.01);
    double kappa = default(0.01);
    double eta = default(0.01);
    double zeta = default(0.01);
    double rhoValue = default(0.05);
    double minRate = default(0.25);
    double updateQueryTime = default(0.8);
    double xi = default(1);
    int serviceArrivalWindow = default(100);
    double routerQueueDrainTime = default(1);
    double tau = default(10);
    double normalizer = default(100);
```
- **Purpose**: These parameters define the behavior and configuration of the host.
- **Details**:
  - **Basic Configuration**:
    - `isHost`: Indicates whether this node behaves as a host. Default is `true`.
    - `topologyFile`: Path to the file defining the network topology. Default is `"sample-topology.txt"`.
    - `workloadFile`: Path to the file defining the workload or traffic patterns. Default is `"sample-workload.txt"`.
    - `resultPrefix`: Prefix for saving results (e.g., logs or statistics).
    - `simulationLength`: Duration of the simulation in seconds. Default is `30.0`.
    - `statRate`: Rate at which statistics are recorded. Default is `0.2`.
    - `signalsEnabled`: Enables or disables signal emission. Default is `false`.
    - `loggingEnabled`: Enables or disables logging. Default is `false`.
  - **Routing and Resource Allocation**:
    - `splitSize`: Size of splits for transactions. Default is `5.0`.
    - `waterfillingEnabled`: Enables or disables water-filling-based resource allocation. Default is `false`.
    - `priceSchemeEnabled`: Enables or disables price-based schemes. Default is `false`.
    - `smoothWaterfillingEnabled`: Enables smoother water-filling algorithms. Default is `false`.
    - `landmarkRoutingEnabled`: Enables landmark-based routing. Default is `false`.
    - `lndBaselineEnabled`: Enables LND baseline routing. Default is `false`.
    - `dctcpEnabled`: Enables DCTCP-based congestion control. Default is `false`.
  - **Rebalancing and Queuing**:
    - `rebalancingEnabled`: Enables or disables rebalancing mechanisms. Default is `false`.
    - `queueDelayThreshold`: Threshold for queue delay rebalancing. Default is `3.0`.
    - `gamma`, `gammaImbalanceQueueSize`: Parameters for rebalancing logic.
    - `rebalancingRate`: Rate at which rebalancing occurs. Default is `10.0`.
    - `timeoutEnabled`: Enables or disables timeout mechanisms. Default is `true`.
  - **Advanced Routing and Scheduling**:
    - `widestPathsEnabled`: Enables widest-path routing. Default is `false`.
    - `heuristicPathsEnabled`: Enables heuristic-based path selection. Default is `false`.
    - `kspYenEnabled`: Enables K-shortest paths using Yen's algorithm. Default is `false`.
    - `LIFOEnabled`, `FIFOEnabled`, `SPFEnabled`, `RREnabled`, `EDFEnabled`: Enable different scheduling policies.
    - `changingPathsEnabled`: Enables dynamic path changes during simulation. Default is `false`.

### Signals and Statistics
```cpp
@signal[completionTime](type="simtime_t"); 
@statistic[completionTime](title="completionTime"; source="completionTime"; record=vector,stats; interpolationmode=none); 
...
```
- **Purpose**: Signals and statistics are used to monitor and analyze the host's performance and internal state.
- **Details**:
  - **Signals**: Represent real-time data emitted by the host during simulation.
    - Example: `completionTime` tracks the time taken to complete tasks.
  - **Statistics**: Aggregate data from signals for analysis.
    - Example: `@statistic[completionTime]` records the `completionTime` signal as both a vector and summary statistics (e.g., mean, max).
  - **Key Metrics**:
    - **Transaction Metrics**:
      - `numCleared`: Number of transactions cleared.
      - `numCompletedPerDest*`: Number of completed transactions per destination.
      - `numPendingPerDest*`: Number of pending transactions per destination.
      - `numTimedOutPerDest*`: Number of timed-out transactions per destination.
    - **Queue Metrics**:
      - `numInQueuePerChannel*`: Number of packets in the queue per channel.
      - `numInflightPerChannel*`: Number of inflight packets per channel.
    - **Resource Metrics**:
      - `balancePerChannel*`: Balance (e.g., available capacity) per channel.
      - `capacityPerChannel*`: Total capacity of each channel.
      - `timeInFlightPerChannel*`: Time packets spend in flight per channel.
    - **Routing Metrics**:
      - `rateCompletedPerDestPerPath*`: Rate of completed transactions per destination and path.
      - `rateAttemptedPerDestPerPath*`: Rate of attempted transactions per destination and path.
    - **Price Scheme Metrics**:
      - `nValuePerChannel*`: Value associated with pricing schemes.
      - `xLocalPerChannel*`, `muLocalPerChannel*`, `muRemotePerChannel*`: Local and remote metrics for pricing algorithms.
    - **Rebalancing Metrics**:
      - `implicitRebalancingAmtPerChannel*`: Amount of implicit rebalancing performed per channel.
      - `explicitRebalancingAmtPerChannel*`: Amount of explicit rebalancing performed per channel.

### Gates
```cpp
gates:
    input in[];
    output out[];
```
- **Purpose**: Gates define the communication interfaces of the host.
- **Details**:
  - `in[]`: Array of input gates for receiving packets from other nodes.
  - `out[]`: Array of output gates for sending packets to other nodes.

---

## 4. Simulation Workflow

1. **Initialization**:
   - The host reads configuration files (`topologyFile` and `workloadFile`) to initialize its state.
   - Parameters like `timeoutEnabled` and `waterfillingEnabled` determine the host's operational mode.

2. **Packet Processing**:
   - Incoming packets are received via the `in[]` gates and queued for processing.
   - The host applies routing policies (e.g., water-filling or price schemes) to allocate resources (e.g., bandwidth) among channels.
   - Packets are forwarded to the appropriate `out[]` gate based on the routing logic.

3. **Monitoring**:
   - Signals are emitted during packet processing to track metrics like queue length, processing rate, and channel balance.
   - Statistics aggregate these signals for post-simulation analysis.

4. **Completion**:
   - The simulation ends when all packets are processed, or a predefined condition is met.
   - Final statistics are recorded for evaluation.

---

## 5. Customization and Extensions

### Modifying Parameters
- To change the default behavior, update the `parameters` section:
  ```cpp
  parameters:
      bool isHost = false;  // Treat this node as a router
      string topologyFile = "custom-topology.txt";  // Use a custom topology file
      bool waterfillingEnabled = true;  // Enable water-filling
  ```

### Adding New Signals/Statistics
- To monitor additional metrics, define new signals and statistics:
  ```cpp
  @signal[newMetric](type="double");
  @statistic[newMetric](title="New Metric"; source="newMetric"; record=vector,stats);
  ```
- Emit the signal in the host's logic:
  ```cpp
  emit(newMetric, value);
  ```

### Extending Functionality
- Add new methods or algorithms to the host's logic (e.g., custom rebalancing strategies).
- Modify the topology and workload files to simulate different scenarios.

---

## 6. Best Practices

1. **Use Descriptive File Names**:
   - Name topology and workload files clearly to reflect their content.

2. **Leverage Default Parameters**:
   - Use default values for parameters unless specific changes are required.

3. **Monitor Key Metrics**:
   - Focus on critical signals like `balancePerChannel*`, `serviceRatePerChannel*`, and `queueDelayEWMAPerChannel*` to identify bottlenecks.

4. **Test Incrementally**:
   - Test the host with small topologies and workloads before scaling up.

5. **Document Changes**:
   - Add comments to explain any customizations or extensions made to the code.

---

## 7. Troubleshooting

### Common Issues
1. **Undefined Files**:
   - Ensure that `topologyFile` and `workloadFile` exist and are correctly formatted.

2. **Signal Errors**:
   - Verify that all signals are properly defined and emitted in the host's logic.

3. **Performance Issues**:
   - If the simulation runs slowly, reduce the number of nodes or simplify the topology.

### Debugging Tips
- Use logging or debugging tools to trace packet flow and identify bottlenecks.
- Check the simulation logs for errors or warnings related to undefined parameters or invalid configurations.

---

By following this manual, developers can effectively understand, customize, and extend the `hostNodeWaterfilling` module to suit their needs.

################################################################

/home/kush/Desktop/spiderproj/spider_omnet/routerNodeBase.ned

simple routerNodeBase
{
	parameters:
	    bool isHost = false;
		string topologyFile = default("sample-topology.txt");
		string workloadFile = default("sample-workload.txt");
		bool timeoutEnabled = default(true);
		bool waterfillingEnabled = default(true);
		
		@signal[completionTime](type="long"); 
		@statistic[completionTime](title="completionTime"; source="completionTime"; record=vector,stats; interpolationmode=none); 

		@display("i=block/routing"); 

		@signal[numInQueuePerChannel*](type="unsigned long"); 
		@statisticTemplate[numInQueuePerChannelTemplate](record=vector,stats);   

		@signal[numProcessedPerChannel*](type="long");  
		@statisticTemplate[numProcessedPerChannelTemplate](record=vector, stats);   

		@signal[numSentPerChannel*](type="long");  
		@statisticTemplate[numSentPerChannelTemplate](record=vector, stats);   
		
		@signal[balancePerChannel*](type="double"); 
		@statisticTemplate[balancePerChannelTemplate](record=vector, stats);   
		
		@signal[capacityPerChannel*](type="double"); 
		@statisticTemplate[capacityPerChannelTemplate](record=vector, stats);   
		
		@signal[timeInFlightPerChannel*](type="double"); 
		@statisticTemplate[timeInFlightPerChannelTemplate](record=vector, stats);   
		
		//price scheme payment channel specific signals
		@signal[nValuePerChannel*](type="double"); 
		@statisticTemplate[nValuePerChannelTemplate](record=vector, stats);
		
		@signal[fakeRebalanceQPerChannel*](type="double"); 
		@statisticTemplate[fakeRebalanceQPerChannelTemplate](record=vector, stats);
                
		@signal[queueDelayEWMAPerChannel*](type="double"); 
		@statisticTemplate[queueDelayEWMAPerChannelTemplate](record=vector, stats);
                
                @signal[serviceRatePerChannel*](type="double"); 
		@statisticTemplate[serviceRatePerChannelTemplate](record=vector, stats);
				
                @signal[arrivalRatePerChannel*](type="double"); 
		@statisticTemplate[arrivalRatePerChannelTemplate](record=vector, stats);
                
                @signal[inflightOutgoingPerChannel*](type="long"); 
		@statisticTemplate[inflightOutgoingPerChannelTemplate](record=vector, stats);

                @signal[inflightIncomingPerChannel*](type="long"); 
		@statisticTemplate[inflightIncomingPerChannelTemplate](record=vector, stats);
                
                @signal[xLocalPerChannel*](type="double"); 
		@statisticTemplate[xLocalPerChannelTemplate](record=vector, stats);

		@signal[lambdaPerChannel*](type="double"); 
		@statisticTemplate[lambdaPerChannelTemplate](record=vector, stats);
		
		@signal[muLocalPerChannel*](type="double"); 
		@statisticTemplate[muLocalPerChannelTemplate](record=vector, stats);
		
		@signal[muRemotePerChannel*](type="double"); 
		@statisticTemplate[muRemotePerChannelTemplate](record=vector, stats);
    		
                @signal[numInflightPerChannel*](type="unsigned long"); 
		@statisticTemplate[numInflightPerChannelTemplate](record=vector, stats);

		@signal[implicitRebalancingAmtPerChannel*](type="double"); 
		@statisticTemplate[implicitRebalancingAmtPerChannelTemplate](record=vector, stats);  
		@signal[explicitRebalancingAmtPerChannel*](type="double"); 
		@statisticTemplate[explicitRebalancingAmtPerChannelTemplate](record=vector, stats);  

	gates:
		input in[];
		output out[];
}


SUMMARY #################################################################

# Developer Manual for `routerNodeBase`

## Table of Contents
1. **Introduction**
   - Purpose of the Manual
   - Overview of the Code
2. **Components of the Code**
   - Parameters
   - Signals and Statistics
   - Gates
3. **Detailed Explanation of Each Section**
   - Parameters (`isHost`, `topologyFile`, etc.)
   - Signals and Statistics (e.g., `completionTime`, `numInQueuePerChannel`)
   - Gates (`in[]`, `out[]`)
4. **Simulation Workflow**
5. **Customization and Extensions**
   - Modifying Parameters
   - Adding New Signals/Statistics
   - Extending Functionality
6. **Best Practices**
7. **Troubleshooting**

---

## 1. Introduction

### Purpose of the Manual
This developer manual provides a comprehensive guide to understanding, customizing, and extending the `routerNodeBase` module. It explains the structure, functionality, and advanced features of the module, enabling developers to effectively integrate it into their simulations.

### Overview of the Code
The `routerNodeBase` module represents a generic router designed for network simulations. It supports configurable parameters, signals/statistics for monitoring, and input/output gates for communication with other nodes. This module serves as a foundational component for simulating various routing strategies, including payment channel networks, traditional IP networks, or custom routing protocols.

---

## 2. Components of the Code

The code is structured into three main sections:
1. **Parameters**: Configurable variables that control the behavior of the router.
2. **Signals and Statistics**: Metrics for monitoring the router's performance and state.
3. **Gates**: Input and output ports for communication.

---

## 3. Detailed Explanation of Each Section

### Parameters
```cpp
parameters:
    bool isHost = false;
    string topologyFile = default("sample-topology.txt");
    string workloadFile = default("sample-workload.txt");
    bool timeoutEnabled = default(true);
    bool waterfillingEnabled = default(true);
```
- **Purpose**: These parameters define the behavior and configuration of the router.
- **Details**:
  - `isHost`: Indicates whether this node behaves as a host. Default is `false`.
  - `topologyFile`: Path to the file defining the network topology. Default is `"sample-topology.txt"`.
  - `workloadFile`: Path to the file defining the workload or traffic patterns. Default is `"sample-workload.txt"`.
  - `timeoutEnabled`: Enables or disables timeout mechanisms. Default is `true`.
  - `waterfillingEnabled`: Enables or disables water-filling-based resource allocation. Default is `true`.

### Signals and Statistics
```cpp
@signal[completionTime](type="long"); 
@statistic[completionTime](title="completionTime"; source="completionTime"; record=vector,stats; interpolationmode=none); 
...
```
- **Purpose**: Signals and statistics are used to monitor and analyze the router's performance and internal state.
- **Details**:
  - **Signals**: Represent real-time data emitted by the router during simulation.
    - Example: `completionTime` tracks the time taken to complete tasks.
  - **Statistics**: Aggregate data from signals for analysis.
    - Example: `@statistic[completionTime]` records the `completionTime` signal as both a vector and summary statistics (e.g., mean, max).
  - **Key Metrics**:
    - **Queue Metrics**:
      - `numInQueuePerChannel*`: Number of packets in the queue per channel.
      - `queueDelayEWMAPerChannel*`: Exponentially Weighted Moving Average (EWMA) of queue delays per channel.
    - **Processing Metrics**:
      - `numProcessedPerChannel*`: Number of packets processed per channel.
      - `numSentPerChannel*`: Number of packets sent per channel.
    - **Resource Metrics**:
      - `balancePerChannel*`: Balance (e.g., available capacity) per channel.
      - `capacityPerChannel*`: Total capacity of each channel.
      - `timeInFlightPerChannel*`: Time packets spend in flight per channel.
    - **Rate Metrics**:
      - `arrivalRatePerChannel*`: Rate of incoming packets per channel.
      - `serviceRatePerChannel*`: Rate of outgoing packets per channel.
    - **Rebalancing Metrics**:
      - `implicitRebalancingAmtPerChannel*`: Amount of implicit rebalancing performed per channel.
      - `explicitRebalancingAmtPerChannel*`: Amount of explicit rebalancing performed per channel.
    - **Advanced Metrics**:
      - `xLocalPerChannel*`, `muLocalPerChannel*`, `muRemotePerChannel*`: Local and remote metrics for advanced algorithms.
      - `lambdaPerChannel*`: Lambda values for rate control or optimization algorithms.

### Gates
```cpp
gates:
    input in[];
    output out[];
```
- **Purpose**: Gates define the communication interfaces of the router.
- **Details**:
  - `in[]`: Array of input gates for receiving packets from other nodes.
  - `out[]`: Array of output gates for sending packets to other nodes.

---

## 4. Simulation Workflow

1. **Initialization**:
   - The router reads configuration files (`topologyFile` and `workloadFile`) to initialize its state.
   - Parameters like `timeoutEnabled` and `waterfillingEnabled` determine the router's operational mode.

2. **Packet Processing**:
   - Incoming packets are received via the `in[]` gates and queued for processing.
   - The router applies routing policies (e.g., water-filling or basic queuing) to allocate resources (e.g., bandwidth) among channels.
   - Packets are forwarded to the appropriate `out[]` gate based on the routing logic.

3. **Monitoring**:
   - Signals are emitted during packet processing to track metrics like queue length, processing rate, and channel balance.
   - Statistics aggregate these signals for post-simulation analysis.

4. **Completion**:
   - The simulation ends when all packets are processed, or a predefined condition is met.
   - Final statistics are recorded for evaluation.

---

## 5. Customization and Extensions

### Modifying Parameters
- To change the default behavior, update the `parameters` section:
  ```cpp
  parameters:
      bool isHost = true;  // Treat this node as a host
      string topologyFile = "custom-topology.txt";  // Use a custom topology file
      bool waterfillingEnabled = false;  // Disable water-filling
  ```

### Adding New Signals/Statistics
- To monitor additional metrics, define new signals and statistics:
  ```cpp
  @signal[newMetric](type="double");
  @statistic[newMetric](title="New Metric"; source="newMetric"; record=vector,stats);
  ```
- Emit the signal in the router's logic:
  ```cpp
  emit(newMetric, value);
  ```

### Extending Functionality
- Add new methods or algorithms to the router's logic (e.g., custom routing strategies).
- Modify the topology and workload files to simulate different scenarios.

---

## 6. Best Practices

1. **Use Descriptive File Names**:
   - Name topology and workload files clearly to reflect their content.

2. **Leverage Default Parameters**:
   - Use default values for parameters unless specific changes are required.

3. **Monitor Key Metrics**:
   - Focus on critical signals like `balancePerChannel*`, `serviceRatePerChannel*`, and `queueDelayEWMAPerChannel*` to identify bottlenecks.

4. **Test Incrementally**:
   - Test the router with small topologies and workloads before scaling up.

5. **Document Changes**:
   - Add comments to explain any customizations or extensions made to the code.

---

## 7. Troubleshooting

### Common Issues
1. **Undefined Files**:
   - Ensure that `topologyFile` and `workloadFile` exist and are correctly formatted.

2. **Signal Errors**:
   - Verify that all signals are properly defined and emitted in the router's logic.

3. **Performance Issues**:
   - If the simulation runs slowly, reduce the number of nodes or simplify the topology.

### Debugging Tips
- Use logging or debugging tools to trace packet flow and identify bottlenecks.
- Check the simulation logs for errors or warnings related to undefined parameters or invalid configurations.

---

By following this manual, developers can effectively understand, customize, and extend the `routerNodeBase` module to suit their needs.

#######################################################################

/home/kush/Desktop/spiderproj/spider_omnet/routerNodeCeler.ned

simple routerNodeCeler
{
	parameters:
	    bool isHost = false;
		string topologyFile = default("sample-topology.txt");
		string workloadFile = default("sample-workload.txt");
		bool timeoutEnabled = default(true);
		bool waterfillingEnabled = default(false);
		
		@signal[completionTime](type="long"); 
		@statistic[completionTime](title="completionTime"; source="completionTime"; record=vector,stats; interpolationmode=none); 

		@display("i=block/routing"); 

		@signal[numInQueuePerChannel*](type="double"); 
		@statisticTemplate[numInQueuePerChannelTemplate](record=vector,stats);   

		@signal[numProcessedPerChannel*](type="long");  
		@statisticTemplate[numProcessedPerChannelTemplate](record=vector, stats);   

		@signal[numSentPerChannel*](type="long");  
		@statisticTemplate[numSentPerChannelTemplate](record=vector, stats);   
		
		@signal[balancePerChannel*](type="double"); 
		@statisticTemplate[balancePerChannelTemplate](record=vector, stats);   
		
		@signal[capacityPerChannel*](type="double"); 
		@statisticTemplate[capacityPerChannelTemplate](record=vector, stats);   
		
		@signal[timeInFlightPerChannel*](type="double"); 
		@statisticTemplate[timeInFlightPerChannelTemplate](record=vector, stats);   
		
		//price scheme payment channel specific signals
		@signal[nValuePerChannel*](type="double"); 
		@statisticTemplate[nValuePerChannelTemplate](record=vector, stats);
		
		@signal[fakeRebalanceQPerChannel*](type="double"); 
		@statisticTemplate[fakeRebalanceQPerChannelTemplate](record=vector, stats);
                
		@signal[queueDelayEWMAPerChannel*](type="double"); 
		@statisticTemplate[queueDelayEWMAPerChannelTemplate](record=vector, stats);
                
                @signal[serviceRatePerChannel*](type="double"); 
		@statisticTemplate[serviceRatePerChannelTemplate](record=vector, stats);
				
                @signal[arrivalRatePerChannel*](type="double"); 
		@statisticTemplate[arrivalRatePerChannelTemplate](record=vector, stats);
                
                @signal[inflightOutgoingPerChannel*](type="long"); 
		@statisticTemplate[inflightOutgoingPerChannelTemplate](record=vector, stats);

                @signal[inflightIncomingPerChannel*](type="long"); 
		@statisticTemplate[inflightIncomingPerChannelTemplate](record=vector, stats);
                
                @signal[xLocalPerChannel*](type="double"); 
		@statisticTemplate[xLocalPerChannelTemplate](record=vector, stats);

		@signal[lambdaPerChannel*](type="double"); 
		@statisticTemplate[lambdaPerChannelTemplate](record=vector, stats);
		
		@signal[muLocalPerChannel*](type="double"); 
		@statisticTemplate[muLocalPerChannelTemplate](record=vector, stats);
		
		@signal[muRemotePerChannel*](type="double"); 
		@statisticTemplate[muRemotePerChannelTemplate](record=vector, stats);
    		
                @signal[numInflightPerChannel*](type="double"); 
		@statisticTemplate[numInflightPerChannelTemplate](record=vector, stats);

		@signal[implicitRebalancingAmtPerChannel*](type="double"); 
		@statisticTemplate[implicitRebalancingAmtPerChannelTemplate](record=vector, stats);  
		@signal[explicitRebalancingAmtPerChannel*](type="double"); 
		@statisticTemplate[explicitRebalancingAmtPerChannelTemplate](record=vector, stats);  
		
		//celer network signals
		@signal[cpiPerChannelPerDest*](type="double"); 
		@statisticTemplate[cpiPerChannelPerDestTemplate](record=vector, stats);

		@signal[destQueuePerDest*](type="double"); 
		@statisticTemplate[destQueuePerDestTemplate](record=vector, stats);
		
		@signal[queueTimedOutPerDest*](type="double"); 
		@statisticTemplate[queueTimedOutPerDestTemplate](record=vector, stats);

		@signal[kStarPerChannel*](type="long"); 
		@statisticTemplate[kStarPerChannelTemplate](record=vector, stats);


	gates:
		input in[];
		output out[];
}


SUMMARY #########################################################

# Developer Manual for `routerNodeCeler`

## Table of Contents
1. **Introduction**
   - Purpose of the Manual
   - Overview of the Code
2. **Components of the Code**
   - Parameters
   - Signals and Statistics
   - Gates
3. **Detailed Explanation of Each Section**
   - Parameters (`isHost`, `topologyFile`, etc.)
   - Signals and Statistics (e.g., `completionTime`, `numInQueuePerChannel`)
   - Gates (`in[]`, `out[]`)
4. **Simulation Workflow**
5. **Customization and Extensions**
   - Modifying Parameters
   - Adding New Signals/Statistics
   - Extending Functionality
6. **Best Practices**
7. **Troubleshooting**

---

## 1. Introduction

### Purpose of the Manual
This developer manual provides a comprehensive guide to understanding, customizing, and extending the `routerNodeCeler` module. It explains the structure, functionality, and advanced features of the module, enabling developers to effectively integrate it into their simulations.

### Overview of the Code
The `routerNodeCeler` module represents a router that implements the Celer Network protocol for efficient payment channel management and routing. It supports configurable parameters, signals/statistics for monitoring, and input/output gates for communication with other nodes. This module is particularly designed for scenarios involving payment channel networks, where rebalancing, congestion control, and optimal routing are critical.

---

## 2. Components of the Code

The code is structured into three main sections:
1. **Parameters**: Configurable variables that control the behavior of the router.
2. **Signals and Statistics**: Metrics for monitoring the router's performance and state.
3. **Gates**: Input and output ports for communication.

---

## 3. Detailed Explanation of Each Section

### Parameters
```cpp
parameters:
    bool isHost = false;
    string topologyFile = default("sample-topology.txt");
    string workloadFile = default("sample-workload.txt");
    bool timeoutEnabled = default(true);
    bool waterfillingEnabled = default(false);
```
- **Purpose**: These parameters define the behavior and configuration of the router.
- **Details**:
  - `isHost`: Indicates whether this node behaves as a host. Default is `false`.
  - `topologyFile`: Path to the file defining the network topology. Default is `"sample-topology.txt"`.
  - `workloadFile`: Path to the file defining the workload or traffic patterns. Default is `"sample-workload.txt"`.
  - `timeoutEnabled`: Enables or disables timeout mechanisms. Default is `true`.
  - `waterfillingEnabled`: Enables or disables water-filling-based resource allocation. Default is `false`.

### Signals and Statistics
```cpp
@signal[completionTime](type="long"); 
@statistic[completionTime](title="completionTime"; source="completionTime"; record=vector,stats; interpolationmode=none); 
...
```
- **Purpose**: Signals and statistics are used to monitor and analyze the router's performance and internal state.
- **Details**:
  - **Signals**: Represent real-time data emitted by the router during simulation.
    - Example: `completionTime` tracks the time taken to complete tasks.
  - **Statistics**: Aggregate data from signals for analysis.
    - Example: `@statistic[completionTime]` records the `completionTime` signal as both a vector and summary statistics (e.g., mean, max).
  - **Key Metrics**:
    - **General Metrics**:
      - `numInQueuePerChannel*`: Number of packets in the queue per channel.
      - `numProcessedPerChannel*`: Number of packets processed per channel.
      - `balancePerChannel*`: Balance (e.g., available capacity) per channel.
      - `arrivalRatePerChannel*`: Rate of incoming packets per channel.
      - `serviceRatePerChannel*`: Rate of outgoing packets per channel.
    - **Rebalancing Metrics**:
      - `implicitRebalancingAmtPerChannel*`: Amount of implicit rebalancing performed per channel.
      - `explicitRebalancingAmtPerChannel*`: Amount of explicit rebalancing performed per channel.
    - **Celer-Specific Metrics**:
      - `cpiPerChannelPerDest*`: Cost-per-increment (CPI) metric for each channel and destination.
      - `destQueuePerDest*`: Queue length for each destination.
      - `queueTimedOutPerDest*`: Number of timed-out queues per destination.
      - `kStarPerChannel*`: Optimal value of `k` (a parameter used in Celer's algorithms) per channel.

### Gates
```cpp
gates:
    input in[];
    output out[];
```
- **Purpose**: Gates define the communication interfaces of the router.
- **Details**:
  - `in[]`: Array of input gates for receiving packets from other nodes.
  - `out[]`: Array of output gates for sending packets to other nodes.

---

## 4. Simulation Workflow

1. **Initialization**:
   - The router reads configuration files (`topologyFile` and `workloadFile`) to initialize its state.
   - Parameters like `timeoutEnabled` and `waterfillingEnabled` determine the router's operational mode.

2. **Packet Processing**:
   - Incoming packets are received via the `in[]` gates and queued for processing.
   - The router applies Celer Network algorithms to allocate resources (e.g., bandwidth) among channels.
   - Packets are forwarded to the appropriate `out[]` gate based on the routing logic.

3. **Monitoring**:
   - Signals are emitted during packet processing to track metrics like queue length, processing rate, and channel balance.
   - Statistics aggregate these signals for post-simulation analysis.

4. **Completion**:
   - The simulation ends when all packets are processed, or a predefined condition is met.
   - Final statistics are recorded for evaluation.

---

## 5. Customization and Extensions

### Modifying Parameters
- To change the default behavior, update the `parameters` section:
  ```cpp
  parameters:
      bool isHost = true;  // Treat this node as a host
      string topologyFile = "custom-topology.txt";  // Use a custom topology file
      bool waterfillingEnabled = true;  // Enable water-filling
  ```

### Adding New Signals/Statistics
- To monitor additional metrics, define new signals and statistics:
  ```cpp
  @signal[newMetric](type="double");
  @statistic[newMetric](title="New Metric"; source="newMetric"; record=vector,stats);
  ```
- Emit the signal in the router's logic:
  ```cpp
  emit(newMetric, value);
  ```

### Extending Functionality
- Add new methods or algorithms to the router's logic (e.g., custom rebalancing strategies).
- Modify the topology and workload files to simulate different scenarios.

---

## 6. Best Practices

1. **Use Descriptive File Names**:
   - Name topology and workload files clearly to reflect their content.

2. **Leverage Default Parameters**:
   - Use default values for parameters unless specific changes are required.

3. **Monitor Key Metrics**:
   - Focus on critical signals like `balancePerChannel*`, `destQueuePerDest*`, and `cpiPerChannelPerDest*` to identify bottlenecks.

4. **Test Incrementally**:
   - Test the router with small topologies and workloads before scaling up.

5. **Document Changes**:
   - Add comments to explain any customizations or extensions made to the code.

---

## 7. Troubleshooting

### Common Issues
1. **Undefined Files**:
   - Ensure that `topologyFile` and `workloadFile` exist and are correctly formatted.

2. **Signal Errors**:
   - Verify that all signals are properly defined and emitted in the router's logic.

3. **Performance Issues**:
   - If the simulation runs slowly, reduce the number of nodes or simplify the topology.

### Debugging Tips
- Use logging or debugging tools to trace packet flow and identify bottlenecks.
- Check the simulation logs for errors or warnings related to undefined parameters or invalid configurations.

---

By following this manual, developers can effectively understand, customize, and extend the `routerNodeCeler` module to suit their needs.

################################################################

/home/kush/Desktop/spiderproj/spider_omnet/routerNodeDCTCPBal.ned

simple routerNodeDCTCPBal
{
	parameters:
	    bool isHost = false;
		string topologyFile = default("sample-topology.txt");
		string workloadFile = default("sample-workload.txt");
		bool timeoutEnabled = default(true);
		bool waterfillingEnabled = default(true);
		
		@signal[completionTime](type="long"); 
		@statistic[completionTime](title="completionTime"; source="completionTime"; record=vector,stats; interpolationmode=none); 

		@display("i=block/routing"); 

		@signal[numInQueuePerChannel*](type="double"); 
		@statisticTemplate[numInQueuePerChannelTemplate](record=vector,stats);   

		@signal[numProcessedPerChannel*](type="long");  
		@statisticTemplate[numProcessedPerChannelTemplate](record=vector, stats);   

		@signal[numSentPerChannel*](type="long");  
		@statisticTemplate[numSentPerChannelTemplate](record=vector, stats);   
		
		@signal[balancePerChannel*](type="double"); 
		@statisticTemplate[balancePerChannelTemplate](record=vector, stats);   
		
		@signal[timeInFlightPerChannel*](type="double"); 
		@statisticTemplate[timeInFlightPerChannelTemplate](record=vector, stats);   
		
		//price scheme payment channel specific signals
		@signal[nValuePerChannel*](type="double"); 
		@statisticTemplate[nValuePerChannelTemplate](record=vector, stats);
		
		@signal[fakeRebalanceQPerChannel*](type="double"); 
		@statisticTemplate[fakeRebalanceQPerChannelTemplate](record=vector, stats);
                
		@signal[queueDelayEWMAPerChannel*](type="double"); 
		@statisticTemplate[queueDelayEWMAPerChannelTemplate](record=vector, stats);
                
                @signal[serviceRatePerChannel*](type="double"); 
		@statisticTemplate[serviceRatePerChannelTemplate](record=vector, stats);
				
                @signal[arrivalRatePerChannel*](type="double"); 
		@statisticTemplate[arrivalRatePerChannelTemplate](record=vector, stats);
                
                @signal[inflightOutgoingPerChannel*](type="long"); 
		@statisticTemplate[inflightOutgoingPerChannelTemplate](record=vector, stats);

                @signal[inflightIncomingPerChannel*](type="long"); 
		@statisticTemplate[inflightIncomingPerChannelTemplate](record=vector, stats);
                
                @signal[xLocalPerChannel*](type="double"); 
		@statisticTemplate[xLocalPerChannelTemplate](record=vector, stats);

		@signal[lambdaPerChannel*](type="double"); 
		@statisticTemplate[lambdaPerChannelTemplate](record=vector, stats);
		
		@signal[muLocalPerChannel*](type="double"); 
		@statisticTemplate[muLocalPerChannelTemplate](record=vector, stats);
		
		@signal[muRemotePerChannel*](type="double"); 
		@statisticTemplate[muRemotePerChannelTemplate](record=vector, stats);
    		
                @signal[numInflightPerChannel*](type="double"); 
		@statisticTemplate[numInflightPerChannelTemplate](record=vector, stats);

		@signal[implicitRebalancingAmtPerChannel*](type="double"); 
		@statisticTemplate[implicitRebalancingAmtPerChannelTemplate](record=vector, stats);  
		@signal[explicitRebalancingAmtPerChannel*](type="double"); 
		@statisticTemplate[explicitRebalancingAmtPerChannelTemplate](record=vector, stats);  


	gates:
		input in[];
		output out[];
}




SUMMARY #################################################################

# Developer Manual for `routerNodeDCTCPBal`

## Table of Contents
1. **Introduction**
   - Purpose of the Manual
   - Overview of the Code
2. **Components of the Code**
   - Parameters
   - Signals and Statistics
   - Gates
3. **Detailed Explanation of Each Section**
   - Parameters (`isHost`, `topologyFile`, etc.)
   - Signals and Statistics (e.g., `completionTime`, `numInQueuePerChannel`)
   - Gates (`in[]`, `out[]`)
4. **Simulation Workflow**
5. **Customization and Extensions**
   - Modifying Parameters
   - Adding New Signals/Statistics
   - Extending Functionality
6. **Best Practices**
7. **Troubleshooting**

---

## 1. Introduction

### Purpose of the Manual
This developer manual provides a comprehensive guide to understanding, customizing, and extending the `routerNodeDCTCPBal` module. It explains the structure, functionality, and advanced features of the module, enabling developers to effectively integrate it into their simulations.

### Overview of the Code
The `routerNodeDCTCPBal` module represents a router that uses Data Center TCP (DCTCP) balancing techniques for resource allocation and traffic management. It supports configurable parameters, signals/statistics for monitoring, and input/output gates for communication with other nodes. This module is particularly designed for scenarios where efficient load balancing and congestion control are critical.

---

## 2. Components of the Code

The code is structured into three main sections:
1. **Parameters**: Configurable variables that control the behavior of the router.
2. **Signals and Statistics**: Metrics for monitoring the router's performance and state.
3. **Gates**: Input and output ports for communication.

---

## 3. Detailed Explanation of Each Section

### Parameters
```cpp
parameters:
    bool isHost = false;
    string topologyFile = default("sample-topology.txt");
    string workloadFile = default("sample-workload.txt");
    bool timeoutEnabled = default(true);
    bool waterfillingEnabled = default(true);
```
- **Purpose**: These parameters define the behavior and configuration of the router.
- **Details**:
  - `isHost`: Indicates whether this node behaves as a host. Default is `false`.
  - `topologyFile`: Path to the file defining the network topology. Default is `"sample-topology.txt"`.
  - `workloadFile`: Path to the file defining the workload or traffic patterns. Default is `"sample-workload.txt"`.
  - `timeoutEnabled`: Enables or disables timeout mechanisms. Default is `true`.
  - `waterfillingEnabled`: Enables or disables water-filling-based resource allocation. Default is `true`.

### Signals and Statistics
```cpp
@signal[completionTime](type="long"); 
@statistic[completionTime](title="completionTime"; source="completionTime"; record=vector,stats; interpolationmode=none); 
...
```
- **Purpose**: Signals and statistics are used to monitor and analyze the router's performance and internal state.
- **Details**:
  - **Signals**: Represent real-time data emitted by the router during simulation.
    - Example: `completionTime` tracks the time taken to complete tasks.
  - **Statistics**: Aggregate data from signals for analysis.
    - Example: `@statistic[completionTime]` records the `completionTime` signal as both a vector and summary statistics (e.g., mean, max).
  - **Key Metrics**:
    - `numInQueuePerChannel*`: Number of packets in the queue per channel.
    - `numProcessedPerChannel*`: Number of packets processed per channel.
    - `balancePerChannel*`: Balance (e.g., available capacity) per channel.
    - `arrivalRatePerChannel*`: Rate of incoming packets per channel.
    - `serviceRatePerChannel*`: Rate of outgoing packets per channel.
    - `xLocalPerChannel*`, `muLocalPerChannel*`, `muRemotePerChannel*`: Local and remote metrics for DCTCP balancing algorithms.
    - `explicitRebalancingAmtPerChannel*`: Amount of explicit rebalancing performed per channel.

### Gates
```cpp
gates:
    input in[];
    output out[];
```
- **Purpose**: Gates define the communication interfaces of the router.
- **Details**:
  - `in[]`: Array of input gates for receiving packets from other nodes.
  - `out[]`: Array of output gates for sending packets to other nodes.

---

## 4. Simulation Workflow

1. **Initialization**:
   - The router reads configuration files (`topologyFile` and `workloadFile`) to initialize its state.
   - Parameters like `timeoutEnabled` and `waterfillingEnabled` determine the router's operational mode.

2. **Packet Processing**:
   - Incoming packets are received via the `in[]` gates and queued for processing.
   - The router applies DCTCP balancing techniques to allocate resources (e.g., bandwidth) among channels.
   - Packets are forwarded to the appropriate `out[]` gate based on the routing logic.

3. **Monitoring**:
   - Signals are emitted during packet processing to track metrics like queue length, processing rate, and channel balance.
   - Statistics aggregate these signals for post-simulation analysis.

4. **Completion**:
   - The simulation ends when all packets are processed, or a predefined condition is met.
   - Final statistics are recorded for evaluation.

---

## 5. Customization and Extensions

### Modifying Parameters
- To change the default behavior, update the `parameters` section:
  ```cpp
  parameters:
      bool isHost = true;  // Treat this node as a host
      string topologyFile = "custom-topology.txt";  // Use a custom topology file
      bool waterfillingEnabled = false;  // Disable water-filling
  ```

### Adding New Signals/Statistics
- To monitor additional metrics, define new signals and statistics:
  ```cpp
  @signal[newMetric](type="double");
  @statistic[newMetric](title="New Metric"; source="newMetric"; record=vector,stats);
  ```
- Emit the signal in the router's logic:
  ```cpp
  emit(newMetric, value);
  ```

### Extending Functionality
- Add new methods or algorithms to the router's logic (e.g., custom balancing strategies).
- Modify the topology and workload files to simulate different scenarios.

---

## 6. Best Practices

1. **Use Descriptive File Names**:
   - Name topology and workload files clearly to reflect their content.

2. **Leverage Default Parameters**:
   - Use default values for parameters unless specific changes are required.

3. **Monitor Key Metrics**:
   - Focus on critical signals like `balancePerChannel*` and `serviceRatePerChannel*` to identify bottlenecks.

4. **Test Incrementally**:
   - Test the router with small topologies and workloads before scaling up.

5. **Document Changes**:
   - Add comments to explain any customizations or extensions made to the code.

---

## 7. Troubleshooting

### Common Issues
1. **Undefined Files**:
   - Ensure that `topologyFile` and `workloadFile` exist and are correctly formatted.

2. **Signal Errors**:
   - Verify that all signals are properly defined and emitted in the router's logic.

3. **Performance Issues**:
   - If the simulation runs slowly, reduce the number of nodes or simplify the topology.

### Debugging Tips
- Use logging or debugging tools to trace packet flow and identify bottlenecks.
- Check the simulation logs for errors or warnings related to undefined parameters or invalid configurations.

---

By following this manual, developers can effectively understand, customize, and extend the `routerNodeDCTCPBal` module to suit their needs.

########################################################################

/home/kush/Desktop/spiderproj/spider_omnet/routerNodeLndBaseline.ned

simple routerNodeLndBaseline
{
	parameters:
	    bool isHost = false;
		string topologyFile = default("sample-topology.txt");
		string workloadFile = default("sample-workload.txt");
		bool timeoutEnabled = default(true);
		bool waterfillingEnabled = default(true);
		
		@signal[completionTime](type="long"); 
		@statistic[completionTime](title="completionTime"; source="completionTime"; record=vector,stats; interpolationmode=none); 

		@display("i=block/routing"); 

		@signal[numInQueuePerChannel*](type="unsigned long"); 
		@statisticTemplate[numInQueuePerChannelTemplate](record=vector,stats);   

		@signal[numProcessedPerChannel*](type="long");  
		@statisticTemplate[numProcessedPerChannelTemplate](record=vector, stats);   

		@signal[numSentPerChannel*](type="long");  
		@statisticTemplate[numSentPerChannelTemplate](record=vector, stats);   
		
		@signal[balancePerChannel*](type="double"); 
		@statisticTemplate[balancePerChannelTemplate](record=vector, stats);   
		
		@signal[capacityPerChannel*](type="double"); 
		@statisticTemplate[capacityPerChannelTemplate](record=vector, stats);   
		
		@signal[timeInFlightPerChannel*](type="double"); 
		@statisticTemplate[timeInFlightPerChannelTemplate](record=vector, stats);   
		
		//price scheme payment channel specific signals
		@signal[nValuePerChannel*](type="double"); 
		@statisticTemplate[nValuePerChannelTemplate](record=vector, stats);
		
		@signal[fakeRebalanceQPerChannel*](type="double"); 
		@statisticTemplate[fakeRebalanceQPerChannelTemplate](record=vector, stats);
                
		@signal[queueDelayEWMAPerChannel*](type="double"); 
		@statisticTemplate[queueDelayEWMAPerChannelTemplate](record=vector, stats);
                
                @signal[serviceRatePerChannel*](type="double"); 
		@statisticTemplate[serviceRatePerChannelTemplate](record=vector, stats);
				
                @signal[arrivalRatePerChannel*](type="double"); 
		@statisticTemplate[arrivalRatePerChannelTemplate](record=vector, stats);
                
                @signal[inflightOutgoingPerChannel*](type="long"); 
		@statisticTemplate[inflightOutgoingPerChannelTemplate](record=vector, stats);

                @signal[inflightIncomingPerChannel*](type="long"); 
		@statisticTemplate[inflightIncomingPerChannelTemplate](record=vector, stats);
                
                @signal[xLocalPerChannel*](type="double"); 
		@statisticTemplate[xLocalPerChannelTemplate](record=vector, stats);

		@signal[lambdaPerChannel*](type="double"); 
		@statisticTemplate[lambdaPerChannelTemplate](record=vector, stats);
		
		@signal[muLocalPerChannel*](type="double"); 
		@statisticTemplate[muLocalPerChannelTemplate](record=vector, stats);
		
		@signal[muRemotePerChannel*](type="double"); 
		@statisticTemplate[muRemotePerChannelTemplate](record=vector, stats);
    		
                @signal[numInflightPerChannel*](type="unsigned long"); 
		@statisticTemplate[numInflightPerChannelTemplate](record=vector, stats);
		
                @signal[implicitRebalancingAmtPerChannel*](type="double"); 
		@statisticTemplate[implicitRebalancingAmtPerChannelTemplate](record=vector, stats);  
		@signal[explicitRebalancingAmtPerChannel*](type="double"); 
		@statisticTemplate[explicitRebalancingAmtPerChannelTemplate](record=vector, stats);  


	gates:
		input in[];
		output out[];
}


SUMMARY ################################################################

# Developer Manual for `routerNodeLndBaseline`

## Table of Contents
1. **Introduction**
   - Purpose of the Manual
   - Overview of the Code
2. **Components of the Code**
   - Parameters
   - Signals and Statistics
   - Gates
3. **Detailed Explanation of Each Section**
   - Parameters (`isHost`, `topologyFile`, etc.)
   - Signals and Statistics (e.g., `completionTime`, `numInQueuePerChannel`)
   - Gates (`in[]`, `out[]`)
4. **Simulation Workflow**
5. **Customization and Extensions**
   - Modifying Parameters
   - Adding New Signals/Statistics
   - Extending Functionality
6. **Best Practices**
7. **Troubleshooting**

---

## 1. Introduction

### Purpose of the Manual
This developer manual provides a comprehensive guide to understanding, customizing, and extending the `routerNodeLndBaseline` module. It explains the structure, functionality, and advanced features of the module, enabling developers to effectively integrate it into their simulations.

### Overview of the Code
The `routerNodeLndBaseline` module represents a router that mimics the behavior of the Lightning Network Daemon (LND) baseline routing mechanism. It supports configurable parameters, signals/statistics for monitoring, and input/output gates for communication with other nodes. This module is particularly designed for simulating payment channel networks and analyzing routing performance under LND's default policies.

---

## 2. Components of the Code

The code is structured into three main sections:
1. **Parameters**: Configurable variables that control the behavior of the router.
2. **Signals and Statistics**: Metrics for monitoring the router's performance and state.
3. **Gates**: Input and output ports for communication.

---

## 3. Detailed Explanation of Each Section

### Parameters
```cpp
parameters:
    bool isHost = false;
    string topologyFile = default("sample-topology.txt");
    string workloadFile = default("sample-workload.txt");
    bool timeoutEnabled = default(true);
    bool waterfillingEnabled = default(true);
```
- **Purpose**: These parameters define the behavior and configuration of the router.
- **Details**:
  - `isHost`: Indicates whether this node behaves as a host. Default is `false`.
  - `topologyFile`: Path to the file defining the network topology. Default is `"sample-topology.txt"`.
  - `workloadFile`: Path to the file defining the workload or traffic patterns. Default is `"sample-workload.txt"`.
  - `timeoutEnabled`: Enables or disables timeout mechanisms. Default is `true`.
  - `waterfillingEnabled`: Enables or disables water-filling-based resource allocation. Default is `true`.

### Signals and Statistics
```cpp
@signal[completionTime](type="long"); 
@statistic[completionTime](title="completionTime"; source="completionTime"; record=vector,stats; interpolationmode=none); 
...
```
- **Purpose**: Signals and statistics are used to monitor and analyze the router's performance and internal state.
- **Details**:
  - **Signals**: Represent real-time data emitted by the router during simulation.
    - Example: `completionTime` tracks the time taken to complete tasks.
  - **Statistics**: Aggregate data from signals for analysis.
    - Example: `@statistic[completionTime]` records the `completionTime` signal as both a vector and summary statistics (e.g., mean, max).
  - **Key Metrics**:
    - `numInQueuePerChannel*`: Number of packets in the queue per channel.
    - `numProcessedPerChannel*`: Number of packets processed per channel.
    - `balancePerChannel*`: Balance (e.g., available capacity) per channel.
    - `arrivalRatePerChannel*`: Rate of incoming packets per channel.
    - `serviceRatePerChannel*`: Rate of outgoing packets per channel.
    - `xLocalPerChannel*`, `muLocalPerChannel*`, `muRemotePerChannel*`: Local and remote metrics for routing algorithms.
    - `explicitRebalancingAmtPerChannel*`: Amount of explicit rebalancing performed per channel.

### Gates
```cpp
gates:
    input in[];
    output out[];
```
- **Purpose**: Gates define the communication interfaces of the router.
- **Details**:
  - `in[]`: Array of input gates for receiving packets from other nodes.
  - `out[]`: Array of output gates for sending packets to other nodes.

---

## 4. Simulation Workflow

1. **Initialization**:
   - The router reads configuration files (`topologyFile` and `workloadFile`) to initialize its state.
   - Parameters like `timeoutEnabled` and `waterfillingEnabled` determine the router's operational mode.

2. **Packet Processing**:
   - Incoming packets are received via the `in[]` gates and queued for processing.
   - The router applies LND baseline routing policies to allocate resources (e.g., bandwidth) among channels.
   - Packets are forwarded to the appropriate `out[]` gate based on the routing logic.

3. **Monitoring**:
   - Signals are emitted during packet processing to track metrics like queue length, processing rate, and channel balance.
   - Statistics aggregate these signals for post-simulation analysis.

4. **Completion**:
   - The simulation ends when all packets are processed, or a predefined condition is met.
   - Final statistics are recorded for evaluation.

---

## 5. Customization and Extensions

### Modifying Parameters
- To change the default behavior, update the `parameters` section:
  ```cpp
  parameters:
      bool isHost = true;  // Treat this node as a host
      string topologyFile = "custom-topology.txt";  // Use a custom topology file
      bool waterfillingEnabled = false;  // Disable water-filling
  ```

### Adding New Signals/Statistics
- To monitor additional metrics, define new signals and statistics:
  ```cpp
  @signal[newMetric](type="double");
  @statistic[newMetric](title="New Metric"; source="newMetric"; record=vector,stats);
  ```
- Emit the signal in the router's logic:
  ```cpp
  emit(newMetric, value);
  ```

### Extending Functionality
- Add new methods or algorithms to the router's logic (e.g., custom routing strategies).
- Modify the topology and workload files to simulate different scenarios.

---

## 6. Best Practices

1. **Use Descriptive File Names**:
   - Name topology and workload files clearly to reflect their content.

2. **Leverage Default Parameters**:
   - Use default values for parameters unless specific changes are required.

3. **Monitor Key Metrics**:
   - Focus on critical signals like `balancePerChannel*` and `serviceRatePerChannel*` to identify bottlenecks.

4. **Test Incrementally**:
   - Test the router with small topologies and workloads before scaling up.

5. **Document Changes**:
   - Add comments to explain any customizations or extensions made to the code.

---

## 7. Troubleshooting

### Common Issues
1. **Undefined Files**:
   - Ensure that `topologyFile` and `workloadFile` exist and are correctly formatted.

2. **Signal Errors**:
   - Verify that all signals are properly defined and emitted in the router's logic.

3. **Performance Issues**:
   - If the simulation runs slowly, reduce the number of nodes or simplify the topology.

### Debugging Tips
- Use logging or debugging tools to trace packet flow and identify bottlenecks.
- Check the simulation logs for errors or warnings related to undefined parameters or invalid configurations.

---

By following this manual, developers can effectively understand, customize, and extend the `routerNodeLndBaseline` module to suit their needs.

#######################################################################

/home/kush/Desktop/spiderproj/spider_omnet/routerNodePriceScheme.ned

simple routerNodePriceScheme
{
	parameters:
	    bool isHost = false;
		string topologyFile = default("sample-topology.txt");
		string workloadFile = default("sample-workload.txt");
		bool timeoutEnabled = default(true);
		bool waterfillingEnabled = default(true);
		
		@signal[completionTime](type="long"); 
		@statistic[completionTime](title="completionTime"; source="completionTime"; record=vector,stats; interpolationmode=none); 

		@display("i=block/routing"); 

		@signal[numInQueuePerChannel*](type="unsigned long"); 
		@statisticTemplate[numInQueuePerChannelTemplate](record=vector,stats);   

		@signal[numProcessedPerChannel*](type="long");  
		@statisticTemplate[numProcessedPerChannelTemplate](record=vector, stats);   

		@signal[numSentPerChannel*](type="long");  
		@statisticTemplate[numSentPerChannelTemplate](record=vector, stats);   
		
		@signal[balancePerChannel*](type="double"); 
		@statisticTemplate[balancePerChannelTemplate](record=vector, stats);   
		
		@signal[capacityPerChannel*](type="double"); 
		@statisticTemplate[capacityPerChannelTemplate](record=vector, stats);   
		
		@signal[timeInFlightPerChannel*](type="double"); 
		@statisticTemplate[timeInFlightPerChannelTemplate](record=vector, stats);   
		
		//price scheme payment channel specific signals
		@signal[nValuePerChannel*](type="double"); 
		@statisticTemplate[nValuePerChannelTemplate](record=vector, stats);
		
		@signal[fakeRebalanceQPerChannel*](type="double"); 
		@statisticTemplate[fakeRebalanceQPerChannelTemplate](record=vector, stats);
                
		@signal[queueDelayEWMAPerChannel*](type="double"); 
		@statisticTemplate[queueDelayEWMAPerChannelTemplate](record=vector, stats);
                
                @signal[serviceRatePerChannel*](type="double"); 
		@statisticTemplate[serviceRatePerChannelTemplate](record=vector, stats);
				
                @signal[arrivalRatePerChannel*](type="double"); 
		@statisticTemplate[arrivalRatePerChannelTemplate](record=vector, stats);
                
                @signal[inflightOutgoingPerChannel*](type="long"); 
		@statisticTemplate[inflightOutgoingPerChannelTemplate](record=vector, stats);

                @signal[inflightIncomingPerChannel*](type="long"); 
		@statisticTemplate[inflightIncomingPerChannelTemplate](record=vector, stats);
                
                @signal[xLocalPerChannel*](type="double"); 
		@statisticTemplate[xLocalPerChannelTemplate](record=vector, stats);

		@signal[lambdaPerChannel*](type="double"); 
		@statisticTemplate[lambdaPerChannelTemplate](record=vector, stats);
		
		@signal[muLocalPerChannel*](type="double"); 
		@statisticTemplate[muLocalPerChannelTemplate](record=vector, stats);
		
		@signal[muRemotePerChannel*](type="double"); 
		@statisticTemplate[muRemotePerChannelTemplate](record=vector, stats);
    		
                @signal[numInflightPerChannel*](type="unsigned long"); 
		@statisticTemplate[numInflightPerChannelTemplate](record=vector, stats);

		@signal[implicitRebalancingAmtPerChannel*](type="double"); 
		@statisticTemplate[implicitRebalancingAmtPerChannelTemplate](record=vector, stats);  
		@signal[explicitRebalancingAmtPerChannel*](type="double"); 
		@statisticTemplate[explicitRebalancingAmtPerChannelTemplate](record=vector, stats);  

	gates:
		input in[];
		output out[];
}


SUMMARY ################################################################

# Developer Manual for `routerNodePriceScheme`

## Table of Contents
1. **Introduction**
   - Purpose of the Manual
   - Overview of the Code
2. **Components of the Code**
   - Parameters
   - Signals and Statistics
   - Gates
3. **Detailed Explanation of Each Section**
   - Parameters (`isHost`, `topologyFile`, etc.)
   - Signals and Statistics (e.g., `completionTime`, `numInQueuePerChannel`)
   - Gates (`in[]`, `out[]`)
4. **Simulation Workflow**
5. **Customization and Extensions**
   - Modifying Parameters
   - Adding New Signals/Statistics
   - Extending Functionality
6. **Best Practices**
7. **Troubleshooting**

---

## 1. Introduction

### Purpose of the Manual
This developer manual provides a comprehensive guide to understanding, customizing, and extending the `routerNodePriceScheme` module. It explains the structure, functionality, and advanced features of the module, enabling developers to effectively integrate it into their simulations.

### Overview of the Code
The `routerNodePriceScheme` module represents a router that uses price-based schemes for resource allocation and traffic management. It supports configurable parameters, signals/statistics for monitoring, and input/output gates for communication with other nodes. This module is particularly designed for scenarios where pricing mechanisms are used to manage payment channels or network resources.

---

## 2. Components of the Code

The code is structured into three main sections:
1. **Parameters**: Configurable variables that control the behavior of the router.
2. **Signals and Statistics**: Metrics for monitoring the router's performance and state.
3. **Gates**: Input and output ports for communication.

---

## 3. Detailed Explanation of Each Section

### Parameters
```cpp
parameters:
    bool isHost = false;
    string topologyFile = default("sample-topology.txt");
    string workloadFile = default("sample-workload.txt");
    bool timeoutEnabled = default(true);
    bool waterfillingEnabled = default(true);
```
- **Purpose**: These parameters define the behavior and configuration of the router.
- **Details**:
  - `isHost`: Indicates whether this node behaves as a host. Default is `false`.
  - `topologyFile`: Path to the file defining the network topology. Default is `"sample-topology.txt"`.
  - `workloadFile`: Path to the file defining the workload or traffic patterns. Default is `"sample-workload.txt"`.
  - `timeoutEnabled`: Enables or disables timeout mechanisms. Default is `true`.
  - `waterfillingEnabled`: Enables or disables water-filling-based resource allocation. Default is `true`.

### Signals and Statistics
```cpp
@signal[completionTime](type="long"); 
@statistic[completionTime](title="completionTime"; source="completionTime"; record=vector,stats; interpolationmode=none); 
...
```
- **Purpose**: Signals and statistics are used to monitor and analyze the router's performance and internal state.
- **Details**:
  - **Signals**: Represent real-time data emitted by the router during simulation.
    - Example: `completionTime` tracks the time taken to complete tasks.
  - **Statistics**: Aggregate data from signals for analysis.
    - Example: `@statistic[completionTime]` records the `completionTime` signal as both a vector and summary statistics (e.g., mean, max).
  - **Key Metrics**:
    - `numInQueuePerChannel*`: Number of packets in the queue per channel.
    - `numProcessedPerChannel*`: Number of packets processed per channel.
    - `balancePerChannel*`: Balance (e.g., available capacity) per channel.
    - `arrivalRatePerChannel*`: Rate of incoming packets per channel.
    - `serviceRatePerChannel*`: Rate of outgoing packets per channel.
    - `xLocalPerChannel*`, `muLocalPerChannel*`, `muRemotePerChannel*`: Local and remote metrics for pricing algorithms.
    - `explicitRebalancingAmtPerChannel*`: Amount of explicit rebalancing performed per channel.

### Gates
```cpp
gates:
    input in[];
    output out[];
```
- **Purpose**: Gates define the communication interfaces of the router.
- **Details**:
  - `in[]`: Array of input gates for receiving packets from other nodes.
  - `out[]`: Array of output gates for sending packets to other nodes.

---

## 4. Simulation Workflow

1. **Initialization**:
   - The router reads configuration files (`topologyFile` and `workloadFile`) to initialize its state.
   - Parameters like `timeoutEnabled` and `waterfillingEnabled` determine the router's operational mode.

2. **Packet Processing**:
   - Incoming packets are received via the `in[]` gates and queued for processing.
   - The router applies price-based schemes to allocate resources (e.g., bandwidth) among channels.
   - Packets are forwarded to the appropriate `out[]` gate based on the routing logic.

3. **Monitoring**:
   - Signals are emitted during packet processing to track metrics like queue length, processing rate, and channel balance.
   - Statistics aggregate these signals for post-simulation analysis.

4. **Completion**:
   - The simulation ends when all packets are processed, or a predefined condition is met.
   - Final statistics are recorded for evaluation.

---

## 5. Customization and Extensions

### Modifying Parameters
- To change the default behavior, update the `parameters` section:
  ```cpp
  parameters:
      bool isHost = true;  // Treat this node as a host
      string topologyFile = "custom-topology.txt";  // Use a custom topology file
      bool waterfillingEnabled = false;  // Disable water-filling
  ```

### Adding New Signals/Statistics
- To monitor additional metrics, define new signals and statistics:
  ```cpp
  @signal[newMetric](type="double");
  @statistic[newMetric](title="New Metric"; source="newMetric"; record=vector,stats);
  ```
- Emit the signal in the router's logic:
  ```cpp
  emit(newMetric, value);
  ```

### Extending Functionality
- Add new methods or algorithms to the router's logic (e.g., custom pricing strategies).
- Modify the topology and workload files to simulate different scenarios.

---

## 6. Best Practices

1. **Use Descriptive File Names**:
   - Name topology and workload files clearly to reflect their content.

2. **Leverage Default Parameters**:
   - Use default values for parameters unless specific changes are required.

3. **Monitor Key Metrics**:
   - Focus on critical signals like `balancePerChannel*` and `serviceRatePerChannel*` to identify bottlenecks.

4. **Test Incrementally**:
   - Test the router with small topologies and workloads before scaling up.

5. **Document Changes**:
   - Add comments to explain any customizations or extensions made to the code.

---

## 7. Troubleshooting

### Common Issues
1. **Undefined Files**:
   - Ensure that `topologyFile` and `workloadFile` exist and are correctly formatted.

2. **Signal Errors**:
   - Verify that all signals are properly defined and emitted in the router's logic.

3. **Performance Issues**:
   - If the simulation runs slowly, reduce the number of nodes or simplify the topology.

### Debugging Tips
- Use logging or debugging tools to trace packet flow and identify bottlenecks.
- Check the simulation logs for errors or warnings related to undefined parameters or invalid configurations.

---

By following this manual, developers can effectively understand, customize, and extend the `routerNodePriceScheme` module to suit their needs.

#################################################################3

/home/kush/Desktop/spiderproj/spider_omnet/routerNodeWaterfilling.ned

simple routerNodeWaterfilling
{
	parameters:
	    bool isHost = false;
		string topologyFile = default("sample-topology.txt");
		string workloadFile = default("sample-workload.txt");
		bool timeoutEnabled = default(true);
		bool waterfillingEnabled = default(true);
		
		@signal[completionTime](type="long"); 
		@statistic[completionTime](title="completionTime"; source="completionTime"; record=vector,stats; interpolationmode=none); 

		@display("i=block/routing"); 

		@signal[numInQueuePerChannel*](type="double"); 
		@statisticTemplate[numInQueuePerChannelTemplate](record=vector,stats);   

		@signal[numProcessedPerChannel*](type="long");  
		@statisticTemplate[numProcessedPerChannelTemplate](record=vector, stats);   

		@signal[numSentPerChannel*](type="long");  
		@statisticTemplate[numSentPerChannelTemplate](record=vector, stats);   
		
		@signal[balancePerChannel*](type="double"); 
		@statisticTemplate[balancePerChannelTemplate](record=vector, stats);   
		
		@signal[capacityPerChannel*](type="double"); 
		@statisticTemplate[capacityPerChannelTemplate](record=vector, stats);   
		
		@signal[timeInFlightPerChannel*](type="double"); 
		@statisticTemplate[timeInFlightPerChannelTemplate](record=vector, stats);   
		
		//price scheme payment channel specific signals
		@signal[nValuePerChannel*](type="double"); 
		@statisticTemplate[nValuePerChannelTemplate](record=vector, stats);
		
		@signal[fakeRebalanceQPerChannel*](type="double"); 
		@statisticTemplate[fakeRebalanceQPerChannelTemplate](record=vector, stats);
                
		@signal[queueDelayEWMAPerChannel*](type="double"); 
		@statisticTemplate[queueDelayEWMAPerChannelTemplate](record=vector, stats);
                
                @signal[serviceRatePerChannel*](type="double"); 
		@statisticTemplate[serviceRatePerChannelTemplate](record=vector, stats);
				
                @signal[arrivalRatePerChannel*](type="double"); 
		@statisticTemplate[arrivalRatePerChannelTemplate](record=vector, stats);
                
                @signal[inflightOutgoingPerChannel*](type="long"); 
		@statisticTemplate[inflightOutgoingPerChannelTemplate](record=vector, stats);

                @signal[inflightIncomingPerChannel*](type="long"); 
		@statisticTemplate[inflightIncomingPerChannelTemplate](record=vector, stats);
                
                @signal[xLocalPerChannel*](type="double"); 
		@statisticTemplate[xLocalPerChannelTemplate](record=vector, stats);

		@signal[lambdaPerChannel*](type="double"); 
		@statisticTemplate[lambdaPerChannelTemplate](record=vector, stats);
		
		@signal[muLocalPerChannel*](type="double"); 
		@statisticTemplate[muLocalPerChannelTemplate](record=vector, stats);
		
		@signal[muRemotePerChannel*](type="double"); 
		@statisticTemplate[muRemotePerChannelTemplate](record=vector, stats);
    		
                @signal[numInflightPerChannel*](type="double"); 
		@statisticTemplate[numInflightPerChannelTemplate](record=vector, stats);

		@signal[implicitRebalancingAmtPerChannel*](type="double"); 
		@statisticTemplate[implicitRebalancingAmtPerChannelTemplate](record=vector, stats);  
		@signal[explicitRebalancingAmtPerChannel*](type="double"); 
		@statisticTemplate[explicitRebalancingAmtPerChannelTemplate](record=vector, stats);  

	gates:
		input in[];
		output out[];
}


SUMMARY ######################################################################

# Developer Manual for `routerNodeWaterfilling`

## Table of Contents
1. **Introduction**
   - Purpose of the Manual
   - Overview of the Code
2. **Components of the Code**
   - Parameters
   - Signals and Statistics
   - Gates
3. **Detailed Explanation of Each Section**
   - Parameters (`isHost`, `topologyFile`, etc.)
   - Signals and Statistics (e.g., `completionTime`, `numInQueuePerChannel`)
   - Gates (`in[]`, `out[]`)
4. **Simulation Workflow**
5. **Customization and Extensions**
   - Modifying Parameters
   - Adding New Signals/Statistics
   - Extending Functionality
6. **Best Practices**
7. **Troubleshooting**

---

## 1. Introduction

### Purpose of the Manual
This developer manual provides a comprehensive guide to understanding, customizing, and extending the `routerNodeWaterfilling` module. It explains the structure, functionality, and advanced features of the module, enabling developers to effectively integrate it into their simulations.

### Overview of the Code
The `routerNodeWaterfilling` module represents a router that uses water-filling-based algorithms for resource allocation and traffic management. It supports configurable parameters, signals/statistics for monitoring, and input/output gates for communication with other nodes.

---

## 2. Components of the Code

The code is structured into three main sections:
1. **Parameters**: Configurable variables that control the behavior of the router.
2. **Signals and Statistics**: Metrics for monitoring the router's performance and state.
3. **Gates**: Input and output ports for communication.

---

## 3. Detailed Explanation of Each Section

### Parameters
```cpp
parameters:
    bool isHost = false;
    string topologyFile = default("sample-topology.txt");
    string workloadFile = default("sample-workload.txt");
    bool timeoutEnabled = default(true);
    bool waterfillingEnabled = default(true);
```
- **Purpose**: These parameters define the behavior and configuration of the router.
- **Details**:
  - `isHost`: Indicates whether this node behaves as a host. Default is `false`.
  - `topologyFile`: Path to the file defining the network topology. Default is `"sample-topology.txt"`.
  - `workloadFile`: Path to the file defining the workload or traffic patterns. Default is `"sample-workload.txt"`.
  - `timeoutEnabled`: Enables or disables timeout mechanisms. Default is `true`.
  - `waterfillingEnabled`: Enables or disables water-filling-based resource allocation. Default is `true`.

### Signals and Statistics
```cpp
@signal[completionTime](type="long"); 
@statistic[completionTime](title="completionTime"; source="completionTime"; record=vector,stats; interpolationmode=none); 
...
```
- **Purpose**: Signals and statistics are used to monitor and analyze the router's performance and internal state.
- **Details**:
  - **Signals**: Represent real-time data emitted by the router during simulation.
    - Example: `completionTime` tracks the time taken to complete tasks.
  - **Statistics**: Aggregate data from signals for analysis.
    - Example: `@statistic[completionTime]` records the `completionTime` signal as both a vector and summary statistics (e.g., mean, max).
  - **Key Metrics**:
    - `numInQueuePerChannel*`: Number of packets in the queue per channel.
    - `numProcessedPerChannel*`: Number of packets processed per channel.
    - `balancePerChannel*`: Balance (e.g., available capacity) per channel.
    - `arrivalRatePerChannel*`: Rate of incoming packets per channel.
    - `serviceRatePerChannel*`: Rate of outgoing packets per channel.
    - `xLocalPerChannel*`, `muLocalPerChannel*`, `muRemotePerChannel*`: Local and remote metrics for water-filling algorithms.
    - `explicitRebalancingAmtPerChannel*`: Amount of explicit rebalancing performed per channel.

### Gates
```cpp
gates:
    input in[];
    output out[];
```
- **Purpose**: Gates define the communication interfaces of the router.
- **Details**:
  - `in[]`: Array of input gates for receiving packets from other nodes.
  - `out[]`: Array of output gates for sending packets to other nodes.

---

## 4. Simulation Workflow

1. **Initialization**:
   - The router reads configuration files (`topologyFile` and `workloadFile`) to initialize its state.
   - Parameters like `timeoutEnabled` and `waterfillingEnabled` determine the router's operational mode.

2. **Packet Processing**:
   - Incoming packets are received via the `in[]` gates and queued for processing.
   - The router applies water-filling algorithms to allocate resources (e.g., bandwidth) among channels.
   - Packets are forwarded to the appropriate `out[]` gate based on the routing logic.

3. **Monitoring**:
   - Signals are emitted during packet processing to track metrics like queue length, processing rate, and channel balance.
   - Statistics aggregate these signals for post-simulation analysis.

4. **Completion**:
   - The simulation ends when all packets are processed, or a predefined condition is met.
   - Final statistics are recorded for evaluation.

---

## 5. Customization and Extensions

### Modifying Parameters
- To change the default behavior, update the `parameters` section:
  ```cpp
  parameters:
      bool isHost = true;  // Treat this node as a host
      string topologyFile = "custom-topology.txt";  // Use a custom topology file
      bool waterfillingEnabled = false;  // Disable water-filling
  ```

### Adding New Signals/Statistics
- To monitor additional metrics, define new signals and statistics:
  ```cpp
  @signal[newMetric](type="double");
  @statistic[newMetric](title="New Metric"; source="newMetric"; record=vector,stats);
  ```
- Emit the signal in the router's logic:
  ```cpp
  emit(newMetric, value);
  ```

### Extending Functionality
- Add new methods or algorithms to the router's logic (e.g., custom rebalancing strategies).
- Modify the topology and workload files to simulate different scenarios.

---

## 6. Best Practices

1. **Use Descriptive File Names**:
   - Name topology and workload files clearly to reflect their content.

2. **Leverage Default Parameters**:
   - Use default values for parameters unless specific changes are required.

3. **Monitor Key Metrics**:
   - Focus on critical signals like `balancePerChannel*` and `serviceRatePerChannel*` to identify bottlenecks.

4. **Test Incrementally**:
   - Test the router with small topologies and workloads before scaling up.

5. **Document Changes**:
   - Add comments to explain any customizations or extensions made to the code.

---

## 7. Troubleshooting

### Common Issues
1. **Undefined Files**:
   - Ensure that `topologyFile` and `workloadFile` exist and are correctly formatted.

2. **Signal Errors**:
   - Verify that all signals are properly defined and emitted in the router's logic.

3. **Performance Issues**:
   - If the simulation runs slowly, reduce the number of nodes or simplify the topology.

### Debugging Tips
- Use logging or debugging tools to trace packet flow and identify bottlenecks.
- Check the simulation logs for errors or warnings related to undefined parameters or invalid configurations.

---

By following this manual, developers can effectively understand, customize, and extend the `routerNodeWaterfilling` module to suit their needs.

###################################################################################

/home/kush/Desktop/spiderproj/spider_omnet/simpleNet.ned

import routerNodeCeler;
import hostNodeCeler;

network simpleNet
{
        parameters:
                double linkDelay @unit("s") = default(100us);
                double linkDataRate @unit("Gbps") = default(1Gbps);
        submodules:
                host[2]: hostNodeCeler {}
                router[2]: routerNodeCeler {}
connections:
    			host[0].out++ --> {delay = 300.0ms; } --> router[0].in++;
    			host[0].in++ <-- {delay = 300.0ms; } <-- router[0].out++;
                router[0].out++ --> {delay = 300.0ms; } --> router[1].in++;
                router[0].in++ <-- {delay = 300.0ms; } <-- router[1].out++;
                host[1].out++ --> {delay = 300.0ms; } --> router[1].in++;
    			host[1].in++ <-- {delay = 300.0ms; } <-- router[1].out++;
}                                                                    

SUMMARY #######################################################

# Developer Manual for `simpleNet` Network Simulation

## Table of Contents
1. **Introduction**
   - Purpose of the Manual
   - Overview of the Code
2. **Components of the Code**
   - Imports
   - Parameters
   - Submodules
   - Connections
3. **Detailed Explanation of Each Section**
   - Imports (`routerNodeCeler`, `hostNodeCeler`)
   - Parameters (`linkDelay`, `linkDataRate`)
   - Submodules (`host`, `router`)
   - Connections (Bidirectional Links)
4. **Simulation Workflow**
5. **Customization and Extensions**
   - Modifying Parameters
   - Adding More Nodes
   - Changing Connection Topology
6. **Best Practices**
7. **Troubleshooting**

---

## 1. Introduction

### Purpose of the Manual
This developer manual is designed to provide a comprehensive understanding of the `simpleNet` network simulation code. It explains the structure, functionality, and customization options for the code, enabling developers to effectively modify and extend it for their specific use cases.

### Overview of the Code
The `simpleNet` network is a simple simulation model that uses two types of nodes: `hostNodeCeler` and `routerNodeCeler`. The network consists of two hosts connected via two routers, forming a basic topology. Each connection has a specified delay, and the network parameters include link delay and data rate.

---

## 2. Components of the Code

The code is structured into four main sections:
1. **Imports**: Defines the modules used in the simulation.
2. **Parameters**: Specifies global variables that control the behavior of the network.
3. **Submodules**: Declares the nodes in the network.
4. **Connections**: Defines how the nodes are interconnected.

---

## 3. Detailed Explanation of Each Section

### Imports
```cpp
import routerNodeCeler;
import hostNodeCeler;
```
- **Purpose**: These lines import the definitions of the `routerNodeCeler` and `hostNodeCeler` modules. These modules represent the behavior of routers and hosts in the network.
- **Details**:
  - `routerNodeCeler`: A module representing a router with input/output ports for forwarding packets.
  - `hostNodeCeler`: A module representing a host with input/output ports for sending and receiving packets.

### Parameters
```cpp
parameters:
    double linkDelay @unit("s") = default(100us);
    double linkDataRate @unit("Gbps") = default(1Gbps);
```
- **Purpose**: These parameters define the characteristics of the links between nodes.
- **Details**:
  - `linkDelay`: The propagation delay of the links, measured in seconds (`s`). The default value is `100 microseconds`.
  - `linkDataRate`: The data transmission rate of the links, measured in gigabits per second (`Gbps`). The default value is `1 Gbps`.

### Submodules
```cpp
submodules:
    host[2]: hostNodeCeler {}
    router[2]: routerNodeCeler {}
```
- **Purpose**: This section declares the nodes in the network.
- **Details**:
  - `host[2]`: An array of two `hostNodeCeler` modules, representing two hosts in the network.
  - `router[2]`: An array of two `routerNodeCeler` modules, representing two routers in the network.

### Connections
```cpp
connections:
    host[0].out++ --> {delay = 300.0ms; } --> router[0].in++;
    host[0].in++ <-- {delay = 300.0ms; } <-- router[0].out++;
    router[0].out++ --> {delay = 300.0ms; } --> router[1].in++;
    router[0].in++ <-- {delay = 300.0ms; } <-- router[1].out++;
    host[1].out++ --> {delay = 300.0ms; } --> router[1].in++;
    host[1].in++ <-- {delay = 300.0ms; } <-- router[1].out++;
```
- **Purpose**: This section defines the connections between the nodes, specifying the direction of communication and the delay for each link.
- **Details**:
  - Each connection is bidirectional, with separate lines for forward (`-->`) and reverse (`<--`) communication.
  - The delay for each link is explicitly set to `300 milliseconds` (`300.0ms`).
  - Example:
    - `host[0].out++ --> {delay = 300.0ms; } --> router[0].in++`: Host 0 sends packets to Router 0 with a delay of 300 ms.
    - `host[0].in++ <-- {delay = 300.0ms; } <-- router[0].out++`: Router 0 sends packets back to Host 0 with a delay of 300 ms.

---

## 4. Simulation Workflow

1. **Initialization**:
   - The simulation initializes the `host` and `router` nodes based on the `hostNodeCeler` and `routerNodeCeler` definitions.
   - Global parameters like `linkDelay` and `linkDataRate` are applied to all connections unless overridden locally.

2. **Packet Transmission**:
   - Hosts send packets to routers, which forward them to other routers or hosts.
   - Each link introduces a delay of `300 ms` for both forward and reverse communication.

3. **Simulation Execution**:
   - The simulation runs according to the defined topology and parameters, simulating packet flow and delays.

---

## 5. Customization and Extensions

### Modifying Parameters
- To change the default link delay or data rate, update the `parameters` section:
  ```cpp
  parameters:
      double linkDelay @unit("s") = default(200us);  // New default delay
      double linkDataRate @unit("Gbps") = default(10Gbps);  // New data rate
  ```

### Adding More Nodes
- To add more hosts or routers, expand the arrays in the `submodules` section:
  ```cpp
  submodules:
      host[3]: hostNodeCeler {}  // Add a third host
      router[3]: routerNodeCeler {}  // Add a third router
  ```
- Update the `connections` section to include new links.

### Changing Connection Topology
- Modify the `connections` section to create different topologies, such as star, ring, or mesh networks. For example:
  ```cpp
  connections:
      host[0].out++ --> {delay = 200.0ms; } --> router[0].in++;
      router[0].out++ --> {delay = 200.0ms; } --> router[1].in++;
      router[1].out++ --> {delay = 200.0ms; } --> host[1].in++;
  ```

---

## 6. Best Practices

1. **Use Meaningful Names**:
   - Name nodes and connections descriptively to improve readability and maintainability.

2. **Document Changes**:
   - Add comments to explain any customizations or extensions made to the code.

3. **Test Incrementally**:
   - Test the simulation after each modification to ensure correctness.

4. **Leverage Default Parameters**:
   - Use default values for parameters unless specific changes are required.

---

## 7. Troubleshooting

### Common Issues
1. **Undefined Modules**:
   - Ensure that `routerNodeCeler` and `hostNodeCeler` are correctly imported and available in the project directory.

2. **Connection Errors**:
   - Verify that all connections are properly defined with matching input/output ports.

3. **Performance Issues**:
   - If the simulation runs slowly, reduce the number of nodes or adjust the link delay and data rate.

### Debugging Tips
- Use logging or debugging tools to trace packet flow and identify bottlenecks.
- Check the simulation logs for errors or warnings related to undefined modules or invalid connections.

---

By following this manual, developers can effectively understand, customize, and extend the `simpleNet` network simulation to suit their needs.

##############################################################