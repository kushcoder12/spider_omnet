import networkx as nx
import os

# create simple line graph
simple_line_graph = nx.Graph()
simple_line_graph.add_edge(0, 1)
simple_line_graph.add_edge(1, 2)
simple_line_graph.add_edge(3, 2)

# create hotnets topo
hotnets_topo_graph = nx.Graph()
hotnets_topo_graph.add_edge(3, 4)
hotnets_topo_graph.add_edge(2, 3)
hotnets_topo_graph.add_edge(2, 1)
hotnets_topo_graph.add_edge(3, 1)
hotnets_topo_graph.add_edge(0, 4)
hotnets_topo_graph.add_edge(0, 1)

# create simple deadlock
simple_deadlock_graph = nx.Graph()
simple_deadlock_graph.add_edge(0, 1)
simple_deadlock_graph.add_edge(1, 2)

# LIST OF STANDARD ROUTER GRAPHS
# two node graph
two_node_graph = nx.Graph()
two_node_graph.add_edge(0, 1)

# three node graph - triangle
three_node_graph = nx.Graph()
three_node_graph.add_edge(0, 1)
three_node_graph.add_edge(1, 2)
three_node_graph.add_edge(2, 0)

# four node graph - square
four_node_graph = nx.Graph()
four_node_graph.add_edge(0, 1)
four_node_graph.add_edge(1, 2)
four_node_graph.add_edge(2, 3)
four_node_graph.add_edge(3, 0)

# five node graph - pentagon
five_node_graph = nx.Graph()
five_node_graph.add_edge(0, 1)
five_node_graph.add_edge(1, 2)
five_node_graph.add_edge(2, 3)
five_node_graph.add_edge(3, 4)
five_node_graph.add_edge(4, 0)

# five node line
five_line_graph = nx.Graph()
five_line_graph.add_edge(0, 1)
five_line_graph.add_edge(1, 2)
five_line_graph.add_edge(2, 3)
five_line_graph.add_edge(3, 4)

# topology for dctcp
toy_dctcp_graph = nx.Graph()
toy_dctcp_graph.add_edge(0, 1)
toy_dctcp_graph.add_edge(0, 2)
toy_dctcp_graph.add_edge(0, 4)
toy_dctcp_graph.add_edge(0, 6)
toy_dctcp_graph.add_edge(0, 8)
toy_dctcp_graph.add_edge(0, 10)
toy_dctcp_graph.add_edge(1, 3)
toy_dctcp_graph.add_edge(1, 5)
toy_dctcp_graph.add_edge(1, 7)
toy_dctcp_graph.add_edge(1, 9)
toy_dctcp_graph.add_edge(1, 11)

# three node graph - triangle
dag_example_graph = nx.Graph()
dag_example_graph.add_edge(0, 1)
dag_example_graph.add_edge(1, 2)


# parallel graph
parallel_graph = nx.Graph()
parallel_graph.add_edge(0,2)
parallel_graph.add_edge(1,3)

# Filenames for Kaggle data
HOME = os.getenv('HOME')
OMNET = os.getenv('OMNET')
KAGGLE_PATH = HOME + '/' + OMNET + '/samples/spider_omnet/scripts/data/'
KAGGLE_AMT_DIST_FILENAME = KAGGLE_PATH + 'amt_dist.npy'
KAGGLE_AMT_MODIFIED_DIST_FILENAME = KAGGLE_PATH + 'amt_dist_cutoff.npy'
KAGGLE_TIME_DIST_FILENAME = KAGGLE_PATH + 'time_dist.npy'
PATH_PKL_DATA = "path_data/"
SAT_TO_EUR = 9158

# CONSTANTS
SEED = 23
SEED_LIST = [23, 4773, 76189, 99889, 1968, 2329]
SCALE_AMOUNT = 30
MEAN_RATE = 10
CIRCULATION_STD_DEV = 2
LARGE_BALANCE = 1000000000
REASONABLE_BALANCE = 15000 
REASONABLE_ROUTER_BALANCE = 1000
MIN_TXN_SIZE = 0.1
MAX_TXN_SIZE = 10
SMALLEST_UNIT=1
MEASUREMENT_INTERVAL = 200 # transStatEnd - start in experiments

EC2_INSTANCE_ADDRESS="ec2-34-224-216-215.compute-1.amazonaws.com"
PORT_NUMBER=8000

# json parameters for lnd testbed
ENDHOST_LND_ONE_WAY_CAPACITY = 1000000000
ROUTER_CAPACITY = 100
LND_FILE_PATH = HOME + "/" + OMNET + "/samples/spider_omnet/lnd_data/"
LOG_NORMAL_MEAN=-0.6152
LOG_NORMAL_SCALE=0.7310



''' OMNET SPECIFIC STUFF '''
# maximum number of paths to consider or plot data for
MAX_K = 4
NUM_LANDMARKS = MAX_K

# List of recognized and parsable omnet signals
RECOGNIZED_OMNET_SIGNALS = ["numInQueuePerChannel",\
        "numSentPerChannel", "balancePerChannel", "numProcessedPerChannel",\
        "numAttemptedPerDest_Total", "numCompletedPerDest_Total", "numTimedOutPerDest_Total",\
        "fracSuccessfulPerDest_Total",\
        "bottleneckPerDestPerPath", "rateCompletedPerDestPerPath", "rateAttemptedPerDestPerPath"]


# map of what field to parse for what plot
INTERESTING_SIGNALS = dict()
INTERESTING_SIGNALS["completion_rate_cdfs"] = ["rateCompletedPerDest",\
        "rateArrivedPerDest"]
INTERESTING_SIGNALS["rateCompleted"] = ["rateCompletedPerDest_Total"] 
INTERESTING_SIGNALS["numCompleted"] = ["numCompletedPerDest_Total"] 
INTERESTING_SIGNALS["numArrived"] = ["numArrivedPerDest_Total"] 
INTERESTING_SIGNALS["rateArrived"] = ["rateArrivedPerDest_Total"]
INTERESTING_SIGNALS["rateToSendTrans"] = ["rateToSendTransPerDestPerPath"]
INTERESTING_SIGNALS["rateSent"] = ["rateSentPerDestPerPath"]
INTERESTING_SIGNALS["window"] = ["windowPerDestPerPath"]
INTERESTING_SIGNALS["sumOfTransUnitsInFlight"] = ["sumOfTransUnitsInFlightPerDestPerPath"]
INTERESTING_SIGNALS["priceLastSeen"] = ["priceLastSeenPerDestPerPath"]
INTERESTING_SIGNALS["fractionMarked"] = ["fractionMarkedPerDestPerPath"]
INTERESTING_SIGNALS["measuredRTT"] = ["measuredRTTPerDestPerPath"]
INTERESTING_SIGNALS["smoothedFractionMarked"] = ["smoothedFractionMarkedPerDestPerPath"]
INTERESTING_SIGNALS["rateOfAcks"] = ["rateOfAcksPerDestPerPath"]

# DO NOT CHANGE THIS: PAINFULLY HARDCODED TO NOT INTERFERE WITH numTimedOutAtSender
INTERESTING_SIGNALS["numTimedOutPerDest"] = ["numTimedOutPerDest"]

per_dest_list = []
for signal in ["numWaiting", "probability", "bottleneck", "pathPerTrans", \
        "fracSuccessful", "demandEstimate", "destQueue", "queueTimedOut"]:
    INTERESTING_SIGNALS[signal] = signal + "PerDest"
    per_dest_list.append(signal + "PerDest")
per_dest_list.extend(["rateCompletedPerDest_Total", "rateArrivedPerDest_Total", \
        "numCompletedPerDest_Total", "numArrivedPerDest_Total", \
        "rateToSendTransPerDestPerPath", "rateSentPerDestPerPath", "rateOfAcksPerDestPerPath", \
        "fractionMarkedPerDestPerPath", "sumOfTransUnitsInFlightPerDestPerPath", "windowPerDestPerPath", \
        "priceLastSeenPerDestPerPath", "numTimedOutPerDest", "smoothedFractionMarkedPerDestPerPath",\
        "measuredRTTPerDestPerPath"])

per_channel_list = []
for signal in ["balance", "numInQueue", "lambda", "muLocal", "xLocal", "nValue", "serviceRate", "arrivalRate",
        "inflightOutgoing", "inflightIncoming", 'queueDelayEWMA', 'fakeRebalanceQ', "capacity", "bank", "kStar", \
        "numSent", "muRemote", "numInflight", "timeInFlight", "explicitRebalancingAmt", "implicitRebalancingAmt"]:
    INTERESTING_SIGNALS[signal] = signal + "PerChannel"
    per_channel_list.append(signal + "PerChannel")

INTERESTING_SIGNALS["per_src_dest_plot"] = per_dest_list
INTERESTING_SIGNALS["per_channel_plot"] = per_channel_list

# added for celer_network
per_channel_dest_list = []
for signal in ["cpi"]:
    INTERESTING_SIGNALS[signal] = signal + "PerChannelPerDest"
    per_channel_dest_list.append(signal + "PerChannelPerDest")

INTERESTING_SIGNALS["per_channel_dest_plot"] = per_channel_dest_list




## ggplot related constants
PLOT_DIR = "data/"
GGPLOT_DATA_DIR = "ggplot_data/"
SUMMARY_DIR = "figures/"
RESULT_DIR = HOME + "/" + OMNET + "/samples/spider_omnet/benchmarks/circulations/results/"

# define scheme codes for ggplot
SCHEME_CODE = { "priceSchemeWindow": "PS",\
        "lndBaseline": "LND",\
        "landmarkRouting": "LR",\
        "shortestPath": "SP",\
        "waterfilling": "WF",\
        "DCTCP": "DCTCP",\
        "DCTCPRate": "DCTCPRate", \
        "DCTCPQ": "DCTCP_qdelay", \
        "DCTCPBal": "DCTCPBal",\
        "celer": "celer"}

# define actual dag percent mapping for ggplot
PERCENT_MAPPING = { '0' : 0,\
        '20': 5, \
        '45' : 20, \
        '65' : 40 }

# Wallet Configuration for Payment Channel Network (PCN)

# Initial wallet balance for all nodes
INITIAL_WALLET_BALANCE = 10000  # Default starting balance per node

class ChannelInfo:
    """Stores information about a channel's stake and current state"""
    def __init__(self, staked_amount):
        self.staked_amount = staked_amount
        self.available_balance = staked_amount  # Track actual available balance

class WalletSystem:
    """Manages node wallets in PCN with pooled liquidity across channels"""
    def __init__(self, initial_balance=INITIAL_WALLET_BALANCE):
        self.initial_balance = initial_balance
        self.wallet_balances = {}      # Node -> total balance mapping
        self.channel_info = {}         # (node1, node2) -> ChannelInfo
        self.node_channels = {}        # Node -> set of channels
        self.total_staked = {}         # Node -> total staked amount

    def _get_channel_key(self, node1, node2):
        """Create consistent channel key regardless of node order"""
        return (min(node1, node2), max(node1, node2))

    def initialize_wallets(self, graph):
        """Initialize wallet system based on network topology"""
        self.wallet_balances.clear()
        self.channel_info.clear()
        self.node_channels.clear()
        self.total_staked.clear()
        
        # First pass: Initialize basic structures
        for node in graph.nodes:
            self.wallet_balances[node] = self.initial_balance
            self.node_channels[node] = set()
            self.total_staked[node] = 0
        
        # Second pass: Process channels and stakes
        for node1, node2 in graph.edges:
            channel_key = self._get_channel_key(node1, node2)
            staked = graph[node1][node2].get("staked_amount", 0)
            
            self.channel_info[channel_key] = ChannelInfo(staked)
            self.node_channels[node1].add(channel_key)
            self.node_channels[node2].add(channel_key)
            
            # Update total staked amounts
            self.total_staked[node1] += staked
            self.total_staked[node2] += staked
        
        # Third pass: Ensure sufficient wallet balances
        for node in graph.nodes:
            if self.total_staked[node] > self.wallet_balances[node]:
                self.wallet_balances[node] = self.total_staked[node]

    def can_process_transaction(self, from_node, amount):
        """Check if node has enough liquidity for transaction"""
        if from_node not in self.wallet_balances:
            return False
        return amount <= self.wallet_balances[from_node]

    def process_transaction(self, from_node, to_node, amount):
        """Process a transaction between nodes using pooled liquidity"""
        if amount <= 0:
            return False
            
        # Handle credit operation
        if from_node is None and to_node is not None:
            if to_node not in self.wallet_balances:
                return False
            self.wallet_balances[to_node] += amount
            return True
            
        # Handle debit operation
        if to_node is None and from_node is not None:
            if not self.can_process_transaction(from_node, amount):
                return False
            self.wallet_balances[from_node] -= amount
            return True
            
        # Handle transfer between nodes
        if not self.can_process_transaction(from_node, amount):
            return False
        self.wallet_balances[from_node] -= amount
        self.wallet_balances[to_node] += amount
        return True

    def get_wallet_balance(self, node):
        """Get current wallet balance for a node"""
        return self.wallet_balances.get(node, 0.0)

    def get_available_liquidity(self, node):
        """Get available liquidity (wallet balance minus total staked)"""
        if node not in self.wallet_balances:
            return 0.0
        return self.wallet_balances[node] - self.total_staked[node]

    def print_wallet_status(self, node):
        """Print detailed wallet status for a node"""
        if node not in self.wallet_balances:
            print(f"Node {node}: Not found in network")
            return
            
        print(f"\nWallet Status for Node {node}:")
        print(f"Total Balance: {self.wallet_balances[node]}")
        print(f"Total Staked: {self.total_staked[node]}")
        print(f"Available Liquidity: {self.get_available_liquidity(node)}")
        print("\nChannel Details:")
        for channel in self.node_channels.get(node, set()):
            info = self.channel_info[channel]
            print(f"Channel {channel}: Staked={info.staked_amount}")

# Create global wallet system instance
wallet_system = WalletSystem(INITIAL_WALLET_BALANCE)

# Convenience functions for backward compatibility
def initialize_wallets(graph):
    wallet_system.initialize_wallets(graph)

def credit_wallet(node, amount):
    return wallet_system.process_transaction(None, node, amount)

def debit_wallet(node, amount):
    return wallet_system.process_transaction(node, None, amount)

def get_wallet_balance(node):
    return wallet_system.get_wallet_balance(node)

def print_wallet_balances():
    for node in wallet_system.wallet_balances:
        wallet_system.print_wallet_status(node)


