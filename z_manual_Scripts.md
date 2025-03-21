# File Path: \\wsl.localhost\Ubuntu-20.04\home\ad\omnetpp-5.6\samples\spider_omnet-Copy\exp-run-scripts\pent_best_yet.sh


#!/bin/bash
PATH_NAME="/home/ubuntu/omnetpp-5.4.1/samples/spider_omnet/benchmarks/circulations/"
GRAPH_PATH="/home/ubuntu/omnetpp-5.4.1/samples/spider_omnet/scripts/figures/"

num_nodes=("2" "2" "3" "4" "5" "5" "5" "0" "0" "10" "20" "50" "60" "80" "100" "200" "400" "600" "800" "1000" \
    "10" "20" "50" "60" "80" "100" "200" "400" "600" "800" "1000" "40" "10" "20" "30" "40" "0")

balance=100

prefix=("two_node_imbalance" "two_node_capacity" "three_node" "four_node" "five_node_hardcoded" \
    "hotnets" "five_line" "lnd_dec4_2018" "lnd_dec4_2018lessScale" \
    "sw_10_routers" "sw_20_routers" "sw_50_routers" "sw_60_routers" "sw_80_routers"  \
    "sw_100_routers" "sw_200_routers" "sw_400_routers" "sw_600_routers" \
    "sw_800_routers" "sw_1000_routers"\
    "sf_10_routers" "sf_20_routers" \
    "sf_50_routers" "sf_60_routers" "sf_80_routers"  \
    "sf_100_routers" "sf_200_routers" "sf_400_routers" "sf_600_routers" \
    "sf_800_routers" "sf_1000_routers" "tree_40_routers" "random_10_routers" "random_20_routers"\
    "random_30_routers" "sw_sparse_40_routers" "lnd_gaussian" "lnd_uniform")


demand_scale=("35") # "60" "90")
routing_scheme=$1
pathChoice=$2
echo $routing_scheme
random_init_bal=false
random_capacity=false

widestPathsEnabled=false
obliviousRoutingEnabled=false
kspYenEnabled=false

#general parameters that do not affect config names
simulationLength=8000
statCollectionRate=1
timeoutClearRate=1
timeoutEnabled=true
signalsEnabled=true
loggingEnabled=false
transStatStart=0
transStatEnd=8000

# scheme specific parameters
eta=0.07
alpha=0.1
kappa=0.07
updateQueryTime=1.5
minPriceRate=0.25
zeta=0.01
rho=0.04
tau=10
normalizer=100
xi=5
routerQueueDrainTime=5
serviceArrivalWindow=300

cp hostNodeBase.ned ${PATH_NAME}
cp hostNodeWaterfilling.ned ${PATH_NAME}
cp hostNodeLandmarkRouting.ned ${PATH_NAME}
cp hostNodePriceScheme.ned ${PATH_NAME}
cp hostNodeLndBaseline.ned ${PATH_NAME}
cp routerNode.ned ${PATH_NAME}

arraylength=${#prefix[@]}
PYTHON="/usr/bin/python"
mkdir -p ${PATH_NAME}

if [ -z "$pathChoice" ]; then
    pathChoice="shortest"
fi

echo $pathChoice




# TODO: find the indices in prefix of the topologies you want to run on and then specify them in array
# adjust experiment time as needed
#array=( 0 1 4 5 8 19 32)
array=( 4 ) #10 11 13 22 24)
for i in "${array[@]}" 
do
    network="${prefix[i]}_circ_net"
    topofile="${PATH_NAME}${prefix[i]}_topo.txt"

    # identify graph type for topology
    if [ ${prefix[i]:0:2} == "sw" ]; then
        graph_type="small_world"
    elif [ ${prefix[i]:0:2} == "sf" ]; then
        graph_type="scale_free"
    elif [ ${prefix[i]:0:4} == "tree" ]; then
        graph_type="tree"
    elif [ ${prefix[i]:0:3} == "lnd" ]; then
        graph_type=${prefix[i]}
    elif [ ${prefix[i]} == "hotnets" ]; then
        graph_type="hotnets_topo"
    elif [ ${prefix[i]:0:6} == "random" ]; then
        graph_type="random"
    else
        graph_type="simple_topologies"
    fi
    
    # set delay amount
    if [[ (${prefix[i]:0:3} == "two") || (${prefix[i]:0:5} == "three") ]]; then
        delay="1150"
    else
        delay="30"
    fi
    
    # STEP 1: create topology
    $PYTHON scripts/create_topo_ned_file.py $graph_type\
            --network-name ${PATH_NAME}$network\
            --topo-filename $topofile\
            --num-nodes ${num_nodes[i]}\
            --balance-per-channel $balance\
            --separate-end-hosts \
            --delay-per-channel $delay\
            --randomize-start-bal $random_init_bal\
            --random-channel-capacity $random_capacity 


    # create workload files and run different demand levels
    for scale in "${demand_scale[@]}"
    do

        # generate the graph first to ned file
        workloadname="${prefix[i]}_circ_demand${scale}"
        workload="${PATH_NAME}$workloadname"
        inifile="${PATH_NAME}${workloadname}_default.ini"
        payment_graph_topo="custom"
        
        # figure out payment graph/workload topology
        if [ ${prefix[i]:0:9} == "five_line" ]; then
            payment_graph_topo="simple_line"
        elif [ ${prefix[i]:0:5} == "three" ]; then
            payment_graph_topo="simple_line"
        elif [ ${prefix[i]:0:4} == "five" ]; then
            payment_graph_topo="hardcoded_circ"
        elif [ ${prefix[i]:0:7} == "hotnets" ]; then
            payment_graph_topo="hotnets_topo"
        fi

        echo $network
        echo $topofile
        echo $inifile
        echo $graph_type

        # STEP 2: create transactions corresponding to this experiment run
        $PYTHON scripts/create_workload.py $workload poisson \
                --graph-topo $payment_graph_topo \
                --payment-graph-dag-percentage 0\
                --topo-filename $topofile\
                --experiment-time $simulationLength \
                --balance-per-channel $balance\
                --generate-json-also \
                --timeout-value 5 \
                --scale-amount $scale \
                --txn-size-mean 1


        # STEP 3: run the experiment
        # routing schemes where number of path choices doesn't matter
        if [ ${routing_scheme} ==  "shortestPath" ]; then 
          output_file=outputs/${prefix[i]}_circ_${routing_scheme}_demand${scale}0_${pathChoice}
          inifile=${PATH_NAME}${prefix[i]}_circ_${routing_scheme}_demand${scale}_${pathChoice}.ini

          # create the ini file with specified parameters
          python scripts/create_ini_file.py \
                  --network-name ${network}\
                  --topo-filename ${topofile}\
                  --workload-filename ${workload}_workload.txt\
                  --ini-filename $inifile\
                  --signals-enabled $signalsEnabled\
                  --logging-enabled $loggingEnabled\
                  --simulation-length $((simulationLength + 10))\
                  --stat-collection-rate $statCollectionRate\
                  --timeout-clear-rate $timeoutClearRate\
                  --timeout-enabled $timeoutEnabled\
                  --routing-scheme ${routing_scheme}\
                  --demand-scale ${scale}\
                  --transStatStart $transStatStart\
                  --transStatEnd $transStatEnd\
                  --path-choice $pathChoice


          # run the omnetexecutable with the right parameters
          ./spiderNet -u Cmdenv -f $inifile -c ${network}_${routing_scheme}_demand${scale}_${pathChoice} -n ${PATH_NAME}\
                > ${output_file}.txt & 
        
      else
          pids=""
          # if you add more choices for the number of paths you might run out of cores/memory
          for numPathChoices in 4
          do
            output_file=outputs/${prefix[i]}_circ_${routing_scheme}_demand${scale}0_${pathChoice}_${numPathChoices}
            inifile=${PATH_NAME}${prefix[i]}_circ_${routing_scheme}_demand${scale}_${pathChoice}_${numPathChoices}.ini

            if [[ $routing_scheme =~ .*Window.* ]]; then
                windowEnabled=true
            else 
                windowEnabled=false
            fi


            echo "Creating ini file"
            # create the ini file with specified parameters
            python scripts/create_ini_file.py \
                    --network-name ${network}\
                    --topo-filename ${topofile}\
                    --workload-filename ${workload}_workload.txt\
                    --ini-filename ${inifile}\
                    --signals-enabled $signalsEnabled\
                    --logging-enabled $loggingEnabled\
                    --simulation-length $((simulationLength + 10))\
                    --stat-collection-rate $statCollectionRate\
                    --timeout-clear-rate $timeoutClearRate\
                    --timeout-enabled $timeoutEnabled\
                    --routing-scheme ${routing_scheme}\
                    --num-path-choices ${numPathChoices}\
                    --zeta $zeta\
                    --alpha $alpha\
                    --eta $eta\
                    --kappa $kappa\
                    --rho $rho\
                    --update-query-time $updateQueryTime\
                    --min-rate $minPriceRate\
                    --tau $tau\
                    --normalizer $normalizer \
                    --window-enabled $windowEnabled\
                    --demand-scale $scale\
                    --xi $xi\
                    --router-queue-drain-time $routerQueueDrainTime\
                    --service-arrival-window $serviceArrivalWindow\
                    --transStatStart $transStatStart\
                    --transStatEnd $transStatEnd\
                    --path-choice $pathChoice

            # run the omnetexecutable with the right parameters
            # in the background
            ./spiderNet -u Cmdenv -f ${inifile}\
                -c ${network}_${routing_scheme}_demand${scale}_${pathChoice}_${numPathChoices} -n ${PATH_NAME}\
                > ${output_file}.txt &
            pids+=($!)
            done
        fi
        wait # for all algorithms to complete for this demand

        # STEP 4: plot everything for this demand
        # TODO: add plotting script
        echo "Plotting"
        payment_graph_type='circ' 
        if [ "$timeoutEnabled" = true ] ; then timeout="timeouts"; else timeout="no_timeouts"; fi
        if [ "$random_init_bal" = true ] ; then suffix="randomInitBal_"; else suffix=""; fi
        if [ "$random_capacity" = true ]; then suffix="${suffix}randomCapacity_"; fi
        echo $suffix
        graph_op_prefix=${GRAPH_PATH}${timeout}/${prefix[i]}_delay${delay}_demand${scale}0_${suffix}
        vec_file_prefix=${PATH_NAME}results/${prefix[i]}_${payment_graph_type}_net_
        
        #routing schemes where number of path choices doesn't matter
        if [ ${routing_scheme} ==  "shortestPath" ]; then 
            vec_file_path=${vec_file_prefix}${routing_scheme}_demand${scale}_${pathChoice}-#0.vec
            sca_file_path=${vec_file_prefix}${routing_scheme}_demand${scale}_${pathChoice}-#0.sca


            python scripts/generate_analysis_plots_for_single_run.py \
              --detail $signalsEnabled \
              --vec_file ${vec_file_path} \
              --sca_file ${sca_file_path} \
              --save ${graph_op_prefix}${routing_scheme}_${pathChoice} \
              --balance \
              --queue_info --timeouts --frac_completed \
              --inflight --timeouts_sender \
              --waiting --bottlenecks
        

        #routing schemes where number of path choices matter
        else
          for numPathChoices in 4
            do
                vec_file_path=${vec_file_prefix}${routing_scheme}_demand${scale}_${pathChoice}_${numPathChoices}-#0.vec
                sca_file_path=${vec_file_prefix}${routing_scheme}_demand${scale}_${pathChoice}_${numPathChoices}-#0.sca


                python scripts/generate_analysis_plots_for_single_run.py \
                  --detail $signalsEnabled \
                  --vec_file ${vec_file_path} \
                  --sca_file ${sca_file_path} \
                  --save ${graph_op_prefix}${routing_scheme}_${pathChoice}_${numPathChoices} \
                  --balance \
                  --queue_info --timeouts --frac_completed \
                  --frac_completed_window \
                  --inflight --timeouts_sender \
                  --waiting --bottlenecks --probabilities \
                  --mu_local --lambda --n_local --service_arrival_ratio --inflight_outgoing \
                  --inflight_incoming --rate_to_send --price --mu_remote --demand \
                  --rate_sent --amt_inflight_per_path
              done
          fi

        # STEP 5: cleanup        
        #rm ${PATH_NAME}${prefix[i]}_circ*_demand${scale}.ini
        #rm ${workload}_workload.txt
        #rm ${workload}.json
    done
    #rm $topofile
done


# File Path:\\wsl.localhost\Ubuntu-20.04\home\ad\omnetpp-5.6\samples\spider_omnet-Copy\exp-run-scripts\run_celer_comp.sh

#!/bin/bash
PATH_NAME="/home/ubuntu/omnetpp-5.4.1/samples/spider_omnet/benchmarks/circulations/"
GRAPH_PATH="/home/ubuntu/omnetpp-5.4.1/samples/spider_omnet/scripts/figures/"

# generally run DCTCPQ with LIFO and celer with FIFO
routing_scheme=$1
capacity_type=$2
pathChoice=$3
schedulingAlgorithm=$4
echo $routing_scheme
random_init_bal=false
random_capacity=false
lnd_capacity=false

widestPathsEnabled=false
obliviousRoutingEnabled=false
kspYenEnabled=false

#general parameters that do not affect config names
simulationLength=610
statCollectionRate=10
timeoutClearRate=1
timeoutEnabled=true
signalsEnabled=true
loggingEnabled=false
transStatStart=400
transStatEnd=600
mtu=1.0

# scheme specific parameters
eta=0.025
alpha=0.2
kappa=0.025
updateQueryTime=1.5
minPriceRate=0.25
zeta=0.01
rho=0.04
tau=10
normalizer=100
xi=1
routerQueueDrainTime=5
serviceArrivalWindow=300

#DCTCP parameters
windowBeta=0.1
windowAlpha=10
queueThreshold=160
queueDelayThreshold=300
balanceThreshold=0.1
minDCTCPWindow=1
rateDecreaseFrequency=3.0

for suffix in "Base" "Waterfilling" "LndBaseline" "PriceScheme" "DCTCP" "Celer"
do
    cp hostNode${suffix}.ned ${PATH_NAME}
    cp routerNode${suffix}.ned ${PATH_NAME}
done
cp hostNodeLandmarkRouting.ned ${PATH_NAME}
cp routerNodeDCTCPBal.ned ${PATH_NAME}

PYTHON="/usr/bin/python"
mkdir -p ${PATH_NAME}

if [ -z "$pathChoice" ]; then
    pathChoice="shortest"
fi

if [ -z "$schedulingAlgorithm" ]; then
    schedulingAlgorithm="LIFO"
fi

prefix="sf_10_routers"
workload_prefix="sf_10_routers"
echo $pathChoice

# celer experiment list for 10 nodes
balance_scale=("200" "400" "800" "1600" "3200" "6400") 
for num in {0..4}
do
    echo "doing run $num"
    for balance in "${balance_scale[@]}"
    do
        network="${prefix}_circ_net"
        topofile="${PATH_NAME}${prefix}_topo${balance}.txt"
        graph_type="scale_free"
        delay="30"
        scale="3"
        
        # generate the graph first to ned file
        workloadname="${workload_prefix}_circ${num}_demand${scale}"
        workload="${PATH_NAME}$workloadname"
        inifile="${PATH_NAME}${workloadname}_default.ini"
        payment_graph_topo="custom"
        

        echo $network
        echo $topofile
        echo $inifile
        echo $graph_type

        # STEP 3: run the experiment
        # routing schemes where number of path choices doesn't matter
        if [ ${routing_scheme} ==  "shortestPath" ]; then 
          output_file=outputs/${prefix}_${balance}_circ${num}_${routing_scheme}_demand${scale}0_${pathChoice}
          inifile=${PATH_NAME}${prefix}_${balance}_circ${num}_${routing_scheme}_demand${scale}_${pathChoice}.ini

          # create the ini file with specified parameters
          python scripts/create_ini_file.py \
                  --network-name ${network}\
                  --topo-filename ${topofile}\
                  --workload-filename ${workload}_workload.txt\
                  --ini-filename $inifile\
                  --signals-enabled $signalsEnabled\
                  --logging-enabled $loggingEnabled\
                  --simulation-length $simulationLength\
                  --stat-collection-rate $statCollectionRate\
                  --timeout-clear-rate $timeoutClearRate\
                  --timeout-enabled $timeoutEnabled\
                  --routing-scheme ${routing_scheme}\
                  --demand-scale ${scale}\
                  --transStatStart $transStatStart\
                  --transStatEnd $transStatEnd\
                  --path-choice $pathChoice\
                  --balance $balance\
                  --circ-num $num


          # run the omnetexecutable with the right parameters
          ./spiderNet -u Cmdenv -f $inifile -c \
          ${network}_${balance}_${routing_scheme}_circ${num}_demand${scale}_${pathChoice} -n ${PATH_NAME}\
                > ${output_file}.txt & 
        
      else
          pids=""
          # if you add more choices for the number of paths you might run out of cores/memory
          for numPathChoices in 4
          do
            output_file=outputs/${prefix}_${balance}_circ${num}_${routing_scheme}_demand${scale}0_${pathChoice}_${numPathChoices}_${schedulingAlgorithm}
            inifile=${PATH_NAME}${prefix}_${balance}_circ${num}_${routing_scheme}_demand${scale}_${pathChoice}_${numPathChoices}_${schedulingAlgorithm}.ini

            if [[ $routing_scheme =~ .*Window.* ]]; then
                windowEnabled=true
            else 
                windowEnabled=false
            fi


            echo "Creating ini file"
            # create the ini file with specified parameters
            python scripts/create_ini_file.py \
                    --network-name ${network}\
                    --topo-filename ${topofile}\
                    --workload-filename ${workload}_workload.txt\
                    --ini-filename ${inifile}\
                    --signals-enabled $signalsEnabled\
                    --logging-enabled $loggingEnabled\
                    --simulation-length $simulationLength\
                    --stat-collection-rate $statCollectionRate\
                    --timeout-clear-rate $timeoutClearRate\
                    --timeout-enabled $timeoutEnabled\
                    --routing-scheme ${routing_scheme}\
                    --num-path-choices ${numPathChoices}\
                    --zeta $zeta\
                    --alpha $alpha\
                    --eta $eta\
                    --kappa $kappa\
                    --rho $rho\
                    --update-query-time $updateQueryTime\
                    --min-rate $minPriceRate\
                    --tau $tau\
                    --normalizer $normalizer \
                    --window-enabled $windowEnabled\
                    --demand-scale $scale\
                    --xi $xi\
                    --router-queue-drain-time $routerQueueDrainTime\
                    --service-arrival-window $serviceArrivalWindow\
                    --transStatStart $transStatStart\
                    --transStatEnd $transStatEnd\
                    --path-choice $pathChoice\
                    --scheduling-algorithm $schedulingAlgorithm\
                    --balance $balance\
                    --circ-num $num \
                    --window-alpha $windowAlpha \
                    --window-beta $windowBeta \
                    --queue-threshold $queueThreshold \
                    --queue-delay-threshold $queueDelayThreshold \
                    --balance-ecn-threshold $balanceThreshold \
                    --mtu $mtu\
                    --min-dctcp-window $minDCTCPWindow\
                    --rate-decrease-frequency $rateDecreaseFrequency

            # run the omnetexecutable with the right parameters
            # in the background
            ./spiderNet -u Cmdenv -f ${inifile} -c\
            ${network}_${balance}_${routing_scheme}_circ${num}_demand${scale}_${pathChoice}_${numPathChoices}_${schedulingAlgorithm}  -n ${PATH_NAME}\
                > ${output_file}.txt &
             pids+=($!)
            done
        fi
        wait # for all algorithms to complete for this demand

        # STEP 4: plot everything for this demand
        # TODO: add plotting script
        echo "Plotting"
        payment_graph_type='circ' 
        if [ "$timeoutEnabled" = true ] ; then timeout="timeouts"; else timeout="no_timeouts"; fi
        if [ "$random_init_bal" = true ] ; then suffix="randomInitBal_"; else suffix=""; fi
        echo $suffix
        graph_op_prefix=${GRAPH_PATH}${timeout}/${prefix}${balance}_circ${num}_delay${delay}_demand${scale}0_${suffix}
        vec_file_prefix=${PATH_NAME}results/${prefix}_${payment_graph_type}_net_${balance}_
        
        #routing schemes where number of path choices doesn't matter
        if [ ${routing_scheme} ==  "shortestPath" ]; then 
            vec_file_path=${vec_file_prefix}${routing_scheme}_circ${num}_demand${scale}_${pathChoice}-#0.vec
            sca_file_path=${vec_file_prefix}${routing_scheme}_circ${num}_demand${scale}_${pathChoice}-#0.sca


            python scripts/generate_analysis_plots_for_single_run.py \
              --detail $signalsEnabled \
              --vec_file ${vec_file_path} \
              --sca_file ${sca_file_path} \
              --save ${graph_op_prefix}${routing_scheme}_${pathChoice} \
              --balance \
              --queue_info --timeouts --frac_completed \
              --inflight --timeouts_sender \
              --waiting --bottlenecks --time_inflight --queue_delay 
        

        #routing schemes where number of path choices matter
        else
          for numPathChoices in 4
            do
                vec_file_path=${vec_file_prefix}${routing_scheme}_circ${num}_demand${scale}_${pathChoice}_${numPathChoices}_${schedulingAlgorithm}-#0.vec
                sca_file_path=${vec_file_prefix}${routing_scheme}_circ${num}_demand${scale}_${pathChoice}_${numPathChoices}_${schedulingAlgorithm}-#0.sca


                python scripts/generate_analysis_plots_for_single_run.py \
                  --detail $signalsEnabled \
                  --vec_file ${vec_file_path} \
                  --sca_file ${sca_file_path} \
                  --save ${graph_op_prefix}${routing_scheme}_${pathChoice}_${numPathChoices}_${schedulingAlgorithm}  \
                  --balance \
                  --queue_info --timeouts --frac_completed \
                  --frac_completed_window \
                  --inflight --timeouts_sender --time_inflight \
                  --waiting --bottlenecks --probabilities \
                  --mu_local --lambda --n_local --service_arrival_ratio --inflight_outgoing \
                  --inflight_incoming --rate_to_send --price --mu_remote --demand \
                  --rate_sent --amt_inflight_per_path --rate_acked --fraction_marked --queue_delay \
                  --cpi --perDestQueue --kStar
              done
          fi

        # STEP 5: cleanup        
        #rm ${PATH_NAME}${prefix}_circ*_demand${scale}.ini
        #rm ${workload}_workload.txt
        #rm ${workload}.json
    done
    #rm $topofile
done


# File Path: \\wsl.localhost\Ubuntu-20.04\home\ad\omnetpp-5.6\samples\spider_omnet-Copy\exp-run-scripts\run_circulations.sh

#!/bin/bash
PATH_NAME="benchmarks/circulations/"

prefix=( "hotnets" ) #five_node_hardcoded" )
#"two_node_imbalance" "two_node_capacity" ) #"sw_sparse_40_routers") # "sw_40_routers" "sf_40_routers")
    #"sw_400_routers" "sf_400_routers")
    #"sw_1000_routers" "sf_1000_routers")

arraylength=${#prefix[@]}

#general parameters that do not affect config names
simulationLength=1000
statCollectionRate=25
timeoutClearRate=1
timeoutEnabled=true
signalsEnabled=true
loggingEnabled=false
#path_choices_dep_list=( "priceSchemeWindow" "waterfilling" "smoothWaterfilling")
#path_choices_indep_list=( "shortestPath" )
path_choices_dep_list=( "priceSchemeWindow")
path_choices_indep_list=(  )
scale=10

eta=0.025
alpha=0.05
kappa=0.025
updateQueryTime=1
minPriceRate=0.25
zeta=0.01
rho=0

tau=10
normalizer=100

cp hostNodeBase.ned ${PATH_NAME}
cp hostNodeWaterfilling.ned ${PATH_NAME}
cp hostNodeLandmarkRouting.ned ${PATH_NAME}
cp hostNodePriceScheme.ned ${PATH_NAME}
cp routerNode.ned ${PATH_NAME}

for (( i=0; i<${arraylength}; i++));
do 
    workloadname="${prefix[i]}_circ_demand${scale}"
    topofile="${PATH_NAME}${prefix[i]}_topo.txt"
    workload="${PATH_NAME}$workloadname"
    network=${prefix[i]}_circ_net

    #routing schemes where number of path choices doesn't matter
    for routing_scheme in "${path_choices_indep_list[@]}" 
    do
      output_file=outputs/${prefix[i]}_circ_${routing_scheme}_demand${scale}0
      inifile=${PATH_NAME}${prefix[i]}_circ_${routing_scheme}_demand${scale}.ini

      # create the ini file with specified parameters
      python scripts/create_ini_file.py \
              --network-name ${network}\
              --topo-filename ${topofile}\
              --workload-filename ${workload}_workload.txt\
              --ini-filename $inifile\
              --signals-enabled $signalsEnabled\
              --logging-enabled $loggingEnabled\
              --simulation-length $simulationLength\
              --stat-collection-rate $statCollectionRate\
              --timeout-clear-rate $timeoutClearRate\
              --timeout-enabled $timeoutEnabled\
              --routing-scheme ${routing_scheme}\
              --demand-scale ${scale}


      # run the omnetexecutable with the right parameters
      ./spiderNet -u Cmdenv -f $inifile -c ${network}_${routing_scheme}_demand${scale}  -n ${PATH_NAME}\
            > ${output_file}.txt & 
    done

  #routing schemes where number of path choices matter
    for routing_scheme in  "${path_choices_dep_list[@]}" 
    do
      pids=""
      # if you add more choices for the number of paths you might run out of cores/memory
      for numPathChoices in 4
      do
        output_file=outputs/${prefix[i]}_circ_${routing_scheme}_demand${scale}0
        inifile=${PATH_NAME}${prefix[i]}_circ_${routing_scheme}_demand${scale}0.ini

        if [[ $routing_scheme =~ .*Window.* ]]; then
            windowEnabled=true
        else 
            windowEnabled=false
        fi


        # create the ini file with specified parameters
        python scripts/create_ini_file.py \
                --network-name ${network}\
                --topo-filename ${topofile}\
                --workload-filename ${workload}_workload.txt\
                --ini-filename ${inifile}\
                --signals-enabled $signalsEnabled\
                --logging-enabled $loggingEnabled\
                --simulation-length $simulationLength\
                --stat-collection-rate $statCollectionRate\
                --timeout-clear-rate $timeoutClearRate\
                --timeout-enabled $timeoutEnabled\
                --routing-scheme ${routing_scheme}\
                --num-path-choices ${numPathChoices}\
                --zeta $zeta\
                --alpha $alpha\
                --eta $eta\
                --kappa $kappa\
                --rho $rho\
                --update-query-time $updateQueryTime\
                --min-rate $minPriceRate\
                --tau $tau\
                --normalizer $normalizer \
                --window-enabled $windowEnabled \
                --demand-scale ${scale}
                --xi $xi\
                --routerQueueDrainTime $routerQueueDrainTime\
                --serviceArrivalWindow $serviceArrivalWindow

        # run the omnetexecutable with the right parameters
        # in the background
        ./spiderNet -u Cmdenv -f ${inifile}\
            -c ${network}_${routing_scheme}_demand${scale}_${numPathChoices} -n ${PATH_NAME}\
            > ${output_file}.txt &
        pids+=($!)
      done 
    done
done
wait




# File Path: \\wsl.localhost\Ubuntu-20.04\home\ad\omnetpp-5.6\samples\spider_omnet-Copy\exp-run-scripts\run_dag_Temp.sh

#!/bin/bash
PATH_PREFIX="/home/ubuntu/omnetpp-5.4.1/samples/spider_omnet/benchmarks/"
GRAPH_PATH="/home/ubuntu/omnetpp-5.4.1/samples/spider_omnet/scripts/figures/"

routing_scheme=$1
echo $routing_scheme
random_init_bal=false
random_capacity=false
pathChoice=$2

if [ -z "$pathChoice" ]; then
    pathChoice="shortest"
fi

#general parameters that do not affect config names
simulationLength=2010
statCollectionRate=10
timeoutClearRate=1
timeoutEnabled=true
signalsEnabled=true
loggingEnabled=false
transStatStart=1800
transStatEnd=2000
mtu=1.0
echo $transStatStart
echo $transStatEnd
echo $signalsEnabled

# scheme specific parameters
eta=0.025
alpha=0.2
kappa=0.025
updateQueryTime=1.5
minPriceRate=0.25
zeta=0.01
rho=0.04
tau=10
normalizer=100
xi=1
routerQueueDrainTime=5
serviceArrivalWindow=300

#DCTCP parameters
windowBeta=0.1
windowAlpha=10
queueThreshold=160
queueDelayThreshold=300
balanceThreshold=0.1
minDCTCPWindow=1
rateDecreaseFrequency=3.0


PYTHON="/usr/bin/python"
mkdir -p ${PATH_PREFIX}

dag_percent=("45" "65")
balance=800
scale=20 # "60" "90")

# TODO: find the indices in prefix of the topologies you want to run on and then specify them in array
# adjust experiment time as needed
#array=( 0 1 4 5 8 19 32)
prefix="sw_50_routers"
for num in {0..4}
do
    echo "doing run $num"
    # create workload files and run different demand levels
    for dag_amt in "${dag_percent[@]}"
    do
        # generate the graph first to ned file
        PATH_NAME="${PATH_PREFIX}dag${dag_amt}/"
        mkdir -p ${PATH_NAME}

        for suffix in "Base" "Waterfilling" "LndBaseline" "PriceScheme"
        do
            cp hostNode${suffix}.ned ${PATH_NAME}
            cp routerNode${suffix}.ned ${PATH_NAME}
        done
        cp hostNodeLandmarkRouting.ned ${PATH_NAME}
        
        network="${prefix}_dag${dag_amt}_net"
        topofile="${PATH_NAME}${prefix}_topo${balance}.txt"
        graph_type="lnd"
        delay="30"

        workloadname="${prefix}_demand${scale}_dag${dag_amt}_num${num}"
        workload="${PATH_NAME}$workloadname"
        inifile="${PATH_NAME}${workloadname}_default.ini"
        payment_graph_topo="custom"
        
        echo $network
        echo $topofile
        echo $inifile
        echo $graph_type
       
        # STEP 3: run the experiment
        # routing schemes where number of path choices doesn't matter
        if [ ${routing_scheme} ==  "shortestPath" ]; then 
          output_file=outputs/${prefix}_dag${dag_amt}_${balance}_dag${num}_${routing_scheme}_demand${scale}0
          inifile=${PATH_NAME}${prefix}_dag${dag_amt}_${balance}_dag${num}_${routing_scheme}_demand${scale}.ini

          # create the ini file with specified parameters
          python scripts/create_ini_file.py \
                  --network-name ${network}\
                  --topo-filename ${topofile}\
                  --workload-filename ${workload}_workload.txt\
                  --ini-filename $inifile\
                  --signals-enabled $signalsEnabled\
                  --logging-enabled $loggingEnabled\
                  --simulation-length $simulationLength\
                  --stat-collection-rate $statCollectionRate\
                  --timeout-clear-rate $timeoutClearRate\
                  --timeout-enabled $timeoutEnabled\
                  --routing-scheme ${routing_scheme}\
                  --demand-scale ${scale}\
                  --transStatStart $transStatStart\
                  --transStatEnd $transStatEnd\
                  --path-choice $pathChoice \
                  --balance $balance\
                  --dag-num $num \


          # run the omnetexecutable with the right parameters
          ./spiderNet -u Cmdenv -f $inifile -c\
          ${network}_${balance}_${routing_scheme}_dag${num}_demand${scale}_shortest -n ${PATH_NAME}\
                > ${output_file}.txt & 
        

        #routing schemes where number of path choices matter
        else
          pids=""
          # if you add more choices for the number of paths you might run out of cores/memory
          for numPathChoices in 4
          do
            output_file=outputs/${prefix}_dag${dag_amt}_${balance}_dag${num}_${routing_scheme}_demand${scale}0_${pathChoice}            
            inifile=${PATH_NAME}${prefix}_dag${dag_amt}_${balance}_dag${num}_${routing_scheme}_demand${scale}_${pathChoice}
            configname=${network}_${balance}_${routing_scheme}_dag${num}_demand${scale}_${pathChoice}_${numPathChoices}

            if [[ $routing_scheme =~ .*Window.* ]]; then
                windowEnabled=true
            else 
                windowEnabled=false
            fi

            if [ ${routing_scheme} ==  "DCTCPQ" ]; then 
                output_file=${output_file} #_qd${queueDelayThreshold}
                inifile=${inifile} #_qd${queueDelayThreshold}
                configname=${configname} #_qd${queueDelayThreshold} 
            fi

            echo "Creating ini file"
            # create the ini file with specified parameters
            python scripts/create_ini_file.py \
                    --network-name ${network}\
                    --topo-filename ${topofile}\
                    --workload-filename ${workload}_workload.txt\
                    --ini-filename ${inifile}.ini\
                    --signals-enabled $signalsEnabled\
                    --logging-enabled $loggingEnabled\
                    --simulation-length $simulationLength\
                    --stat-collection-rate $statCollectionRate\
                    --timeout-clear-rate $timeoutClearRate\
                    --timeout-enabled $timeoutEnabled\
                    --routing-scheme ${routing_scheme}\
                    --num-path-choices ${numPathChoices}\
                    --zeta $zeta\
                    --alpha $alpha\
                    --eta $eta\
                    --kappa $kappa\
                    --rho $rho\
                    --update-query-time $updateQueryTime\
                    --min-rate $minPriceRate\
                    --tau $tau\
                    --normalizer $normalizer \
                    --window-enabled $windowEnabled\
                    --demand-scale $scale\
                    --xi $xi\
                    --router-queue-drain-time $routerQueueDrainTime\
                    --service-arrival-window $serviceArrivalWindow\
                    --transStatStart $transStatStart\
                    --transStatEnd $transStatEnd\
                    --path-choice $pathChoice \
                    --balance $balance\
                    --dag-num $num \
                    --window-alpha $windowAlpha \
                    --window-beta $windowBeta \
                    --queue-threshold $queueThreshold \
                    --queue-delay-threshold $queueDelayThreshold \
                    --balance-ecn-threshold $balanceThreshold \
                    --mtu $mtu\
                    --min-dctcp-window $minDCTCPWindow\
                    --rate-decrease-frequency $rateDecreaseFrequency



            # run the omnetexecutable with the right parameters
            # in the background
            ./spiderNet -u Cmdenv -f ${inifile}.ini -c ${configname} -n ${PATH_NAME}\
                > ${output_file}.txt &
            pids+=($!)
         done
        fi 
        wait # for all algorithms to complete for this demand

        # STEP 4: plot everything for this demand
        # TODO: add plotting script
        echo "Plotting"
        payment_graph_type="dag${dag_amt}" 
        if [ "$timeoutEnabled" = true ] ; then timeout="timeouts"; else timeout="no_timeouts"; fi
        if [ "$random_init_bal" = true ] ; then suffix="randomInitBal_"; else suffix=""; fi
        if [ "$random_capacity" = true ]; then suffix="${suffix}randomCapacity_"; fi
        echo $suffix
        graph_op_prefix=${GRAPH_PATH}${timeout}/${prefix}_dag${dag_amt}_${balance}_num${num}_delay${delay}_demand${scale}0_${suffix}
        vec_file_prefix=${PATH_NAME}results/${prefix}_dag${dag_amt}_net_${balance}_
        
        #routing schemes where number of path choices doesn't matter
        if [ ${routing_scheme} ==  "shortestPath" ]; then 
            vec_file_path=${vec_file_prefix}${routing_scheme}_dag${num}demand${scale}_${pathChoice}-#0.vec
            sca_file_path=${vec_file_prefix}${routing_scheme}_dag${num}_demand${scale}_${pathChoice}-#0.sca


            python scripts/generate_analysis_plots_for_single_run.py \
              --detail $signalsEnabled \
              --vec_file ${vec_file_path} \
              --sca_file ${sca_file_path} \
              --save ${graph_op_prefix}${routing_scheme}_${pathChoice} \
              --balance \
              --queue_info --timeouts --frac_completed \
              --inflight --timeouts_sender \
              --waiting --bottlenecks --time_inflight --queue_delay

        #routing schemes where number of path choices matter
        else
          for numPathChoices in 4
            do
                vec_file_path=${PATH_NAME}results/${configname}-#0.vec
                sca_file_path=${PATH_NAME}results/${configname}-#0.sca
                graph_name=${graph_op_prefix}${routing_scheme}_${pathChoice}_${numPathChoices}

                if [ ${routing_scheme} ==  "DCTCPQ" ]; then 
                    graph_name=${graph_name} #_qd${queueDelayThreshold}
                fi

                python scripts/generate_analysis_plots_for_single_run.py \
                  --detail $signalsEnabled \
                  --vec_file ${vec_file_path} \
                  --sca_file ${sca_file_path} \
                  --save ${graph_name}\
                  --balance \
                  --queue_info --timeouts --frac_completed \
                  --frac_completed_window \
                  --inflight --timeouts_sender --time_inflight \
                  --waiting --bottlenecks --probabilities \
                  --mu_local --lambda --n_local --service_arrival_ratio --inflight_outgoing \
                  --inflight_incoming --rate_to_send --price --mu_remote --demand \
                  --rate_sent --amt_inflight_per_path --rate_acked --fraction_marked --queue_delay
            done
        fi

        # STEP 5: cleanup        
        #rm ${PATH_NAME}${prefix[i]}_circ*_demand${scale}.ini
        #rm ${workload}_workload.txt
        #rm ${workload}.json
    done
    #rm $topofile
done



# # File Path: \\wsl.localhost\Ubuntu-20.04\home\ad\omnetpp-5.6\samples\spider_omnet-Copy\exp-run-scripts\run_dag.sh

#!/bin/bash
PATH_PREFIX="/home/ubuntu/omnetpp-5.4.1/samples/spider_omnet/benchmarks/"
GRAPH_PATH="/home/ubuntu/omnetpp-5.4.1/samples/spider_omnet/scripts/figures/"

routing_scheme=$1
echo $routing_scheme
random_init_bal=false
random_capacity=false
pathChoice=$2
prefix=$3 
workload_prefix=$4 
schedulingAlgorithm="LIFO"

if [ -z "$pathChoice" ]; then
    pathChoice="shortest"
fi

#general parameters that do not affect config names
simulationLength=1010
statCollectionRate=10
timeoutClearRate=1
timeoutEnabled=true
signalsEnabled=true
loggingEnabled=false
transStatStart=800
transStatEnd=1000
mtu=1.0
echo $transStatStart
echo $transStatEnd
echo $signalsEnabled

# scheme specific parameters
eta=0.025
alpha=0.2
kappa=0.025
updateQueryTime=1.5
minPriceRate=0.25
zeta=0.01
rho=0.04
tau=10
normalizer=100
xi=1
routerQueueDrainTime=5
serviceArrivalWindow=300

#DCTCP parameters
windowBeta=0.1
windowAlpha=10
queueThreshold=160
queueDelayThreshold=300
balanceThreshold=0.1
minDCTCPWindow=1
rateDecreaseFrequency=3.0

# dynamic paths
changingPathsEnabled=true
pathMonitorRate=10


PYTHON="/usr/bin/python"
mkdir -p ${PATH_PREFIX}

dag_percent=("20" "45" "65")
balance=40
scale=3 # "60" "90")

# TODO: find the indices in prefix of the topologies you want to run on and then specify them in array
# adjust experiment time as needed
for num in {0..4}
do
    echo "doing run $num"
    # create workload files and run different demand levels
    for dag_amt in "${dag_percent[@]}"
    do
        # generate the graph first to ned file
        PATH_NAME="${PATH_PREFIX}dag${dag_amt}/"
        mkdir -p ${PATH_NAME}

        for suffix in "Base" "Waterfilling" "LndBaseline" "PriceScheme"
        do
            cp hostNode${suffix}.ned ${PATH_NAME}
            cp routerNode${suffix}.ned ${PATH_NAME}
        done
        cp hostNodeLandmarkRouting.ned ${PATH_NAME}
        
        network="${prefix}_dag${dag_amt}_net"
        topofile="${PATH_NAME}${prefix}_topo${balance}.txt"
        graph_type="lnd"
        delay="30"

        workloadname="${workload_prefix}_demand${scale}_dag${dag_amt}_num${num}"
        workload="${PATH_NAME}$workloadname"
        inifile="${PATH_NAME}${workloadname}_default.ini"
        payment_graph_topo="custom"
        
        echo $network
        echo $topofile
        echo $inifile
        echo $graph_type
       
        # STEP 3: run the experiment
        # routing schemes where number of path choices doesn't matter
        if [ ${routing_scheme} ==  "shortestPath" ]; then 
          output_file=outputs/${prefix}_dag${dag_amt}_${balance}_dag${num}_${routing_scheme}_demand${scale}0_${pathChoice}_${schedulingAlgorithm} 
          inifile=${PATH_NAME}${prefix}_dag${dag_amt}_${balance}_dag${num}_${routing_scheme}_demand${scale}_${pathChoice}_${schedulingAlgorithm}.ini

          # create the ini file with specified parameters
          python scripts/create_ini_file.py \
                  --network-name ${network}\
                  --topo-filename ${topofile}\
                  --workload-filename ${workload}_workload.txt\
                  --ini-filename $inifile\
                  --signals-enabled $signalsEnabled\
                  --logging-enabled $loggingEnabled\
                  --simulation-length $simulationLength\
                  --stat-collection-rate $statCollectionRate\
                  --timeout-clear-rate $timeoutClearRate\
                  --timeout-enabled $timeoutEnabled\
                  --routing-scheme ${routing_scheme}\
                  --demand-scale ${scale}\
                  --transStatStart $transStatStart\
                  --transStatEnd $transStatEnd\
                  --path-choice $pathChoice \
                  --balance $balance\
                  --dag-num $num \


          # run the omnetexecutable with the right parameters
          ./spiderNet -u Cmdenv -f $inifile -c\
          ${network}_${balance}_${routing_scheme}_dag${num}_demand${scale}_${pathChoice}_${schedulingAlgorithm}\
          -n ${PATH_NAME}\
                > ${output_file}.txt & 
        

        #routing schemes where number of path choices matter
        else
          pids=""
          # if you add more choices for the number of paths you might run out of cores/memory
          for numPathChoices in 4
          do
            output_file=outputs/${prefix}_dag${dag_amt}_${balance}_dag${num}_${routing_scheme}_demand${scale}0_${pathChoice}_${schedulingAlgorithm}           
            inifile=${PATH_NAME}${prefix}_dag${dag_amt}_${balance}_dag${num}_${routing_scheme}_demand${scale}_${pathChoice}_${schedulingAlgorithm}
            configname=${network}_${balance}_${routing_scheme}_dag${num}_demand${scale}_${pathChoice}_${numPathChoices}_${schedulingAlgorithm}

            if [[ $routing_scheme =~ .*Window.* ]]; then
                windowEnabled=true
            else 
                windowEnabled=false
            fi

            if [ ${routing_scheme} ==  "DCTCPQ" ]; then 
                output_file=${output_file}_qd${queueDelayThreshold}
                inifile=${inifile}_qd${queueDelayThreshold}
                configname=${configname}_qd${queueDelayThreshold} 
            fi

            echo "Creating ini file"
            # create the ini file with specified parameters
            python scripts/create_ini_file.py \
                    --network-name ${network}\
                    --topo-filename ${topofile}\
                    --workload-filename ${workload}_workload.txt\
                    --ini-filename ${inifile}.ini\
                    --signals-enabled $signalsEnabled\
                    --logging-enabled $loggingEnabled\
                    --simulation-length $simulationLength\
                    --stat-collection-rate $statCollectionRate\
                    --timeout-clear-rate $timeoutClearRate\
                    --timeout-enabled $timeoutEnabled\
                    --routing-scheme ${routing_scheme}\
                    --num-path-choices ${numPathChoices}\
                    --zeta $zeta\
                    --alpha $alpha\
                    --eta $eta\
                    --kappa $kappa\
                    --rho $rho\
                    --update-query-time $updateQueryTime\
                    --min-rate $minPriceRate\
                    --tau $tau\
                    --normalizer $normalizer \
                    --window-enabled $windowEnabled\
                    --demand-scale $scale\
                    --xi $xi\
                    --router-queue-drain-time $routerQueueDrainTime\
                    --service-arrival-window $serviceArrivalWindow\
                    --transStatStart $transStatStart\
                    --transStatEnd $transStatEnd\
                    --path-choice $pathChoice \
                    --scheduling-algorithm $schedulingAlgorithm\
                    --balance $balance\
                    --dag-num $num \
                    --window-alpha $windowAlpha \
                    --window-beta $windowBeta \
                    --queue-threshold $queueThreshold \
                    --queue-delay-threshold $queueDelayThreshold \
                    --balance-ecn-threshold $balanceThreshold \
                    --mtu $mtu\
                    --min-dctcp-window $minDCTCPWindow\
                    --rate-decrease-frequency $rateDecreaseFrequency 



            # run the omnetexecutable with the right parameters
            # in the background
            ./spiderNet -u Cmdenv -f ${inifile}.ini -c ${configname} -n ${PATH_NAME}\
               > ${output_file}.txt &
           pids+=($!)
         done
        fi 
        wait # for all algorithms to complete for this demand

        # STEP 4: plot everything for this demand
        # TODO: add plotting script
        echo "Plotting"
        payment_graph_type="dag${dag_amt}" 
        if [ "$timeoutEnabled" = true ] ; then timeout="timeouts"; else timeout="no_timeouts"; fi
        if [ "$random_init_bal" = true ] ; then suffix="randomInitBal_"; else suffix=""; fi
        if [ "$random_capacity" = true ]; then suffix="${suffix}randomCapacity_"; fi
        echo $suffix
        graph_op_prefix=${GRAPH_PATH}${timeout}/${prefix}_dag${dag_amt}_${balance}_num${num}_delay${delay}_demand${scale}0_${suffix}
        vec_file_prefix=${PATH_NAME}results/${prefix}_dag${dag_amt}_net_${balance}_
        
        #routing schemes where number of path choices doesn't matter
        if [ ${routing_scheme} ==  "shortestPath" ]; then 
            vec_file_path=${vec_file_prefix}${routing_scheme}_dag${num}_demand${scale}_${pathChoice}_${schedulingAlgorithm}-#0.vec
            sca_file_path=${vec_file_prefix}${routing_scheme}_dag${num}_demand${scale}_${pathChoice}_${schedulingAlgorithm}-#0.sca


            python scripts/generate_analysis_plots_for_single_run.py \
              --detail $signalsEnabled \
              --vec_file ${vec_file_path} \
              --sca_file ${sca_file_path} \
              --save ${graph_op_prefix}${routing_scheme}_${pathChoice} \
              --balance \
              --queue_info --timeouts --frac_completed \
              --inflight --timeouts_sender \
              --waiting --bottlenecks --time_inflight --queue_delay

        #routing schemes where number of path choices matter
        else
          for numPathChoices in 4
            do
                vec_file_path=${PATH_NAME}results/${configname}-#0.vec
                sca_file_path=${PATH_NAME}results/${configname}-#0.sca
                graph_name=${graph_op_prefix}${routing_scheme}_${pathChoice}_${numPathChoices}_${schedulingAlgorithm}

                if [ ${routing_scheme} ==  "DCTCPQ" ]; then 
                    graph_name=${graph_name}_qd${queueDelayThreshold}
                fi

                python scripts/generate_analysis_plots_for_single_run.py \
                  --detail $signalsEnabled \
                  --vec_file ${vec_file_path} \
                  --sca_file ${sca_file_path} \
                  --save ${graph_name}\
                  --balance \
                  --queue_info --timeouts --frac_completed \
                  --frac_completed_window \
                  --inflight --timeouts_sender --time_inflight \
                  --waiting --bottlenecks --probabilities \
                  --mu_local --lambda --n_local --service_arrival_ratio --inflight_outgoing \
                  --inflight_incoming --rate_to_send --price --mu_remote --demand \
                  --rate_sent --amt_inflight_per_path --rate_acked --fraction_marked --queue_delay
            done
        fi

        # STEP 5: cleanup        
        #rm ${PATH_NAME}${prefix[i]}_circ*_demand${scale}.ini
        #rm ${workload}_workload.txt
        #rm ${workload}.json
    done
    #rm $topofile
done





# File Path: \\wsl.localhost\Ubuntu-20.04\home\ad\omnetpp-5.6\samples\spider_omnet-Copy\exp-run-scripts\run_exp.sh

#!/bin/bash
PATH_NAME="$HOME/$OMNET/samples/spider_omnet/benchmarks/circulations/"
GRAPH_PATH="$HOME/$OMNET/samples/spider_omnet/scripts/figures/"

num_nodes=("2" "2" "3" "4" "5" "5" "5" "0" "0" "10" "20" "50" "60" "80" "100" "200" "400" "600" "800" "1000" \
    "10" "20" "50" "60" "80" "100" "200" "400" "600" "800" "1000" "40" "10" "20" "30" "40" "0" "0" "11" "4")

# balance=100

prefix=("two_node_imbalance" "two_node_capacity" "three_node" "four_node" "five_node_hardcoded" \
    "hotnets" "five_line" "lnd_dec4_2018" "lnd_dec4_2018lessScale" \
    "sw_10_routers" "sw_20_routers" "sw_50_routers" "sw_60_routers" "sw_80_routers"  \
    "sw_100_routers" "sw_200_routers" "sw_400_routers" "sw_600_routers" \
    "sw_800_routers" "sw_1000_routers"\
    "sf_10_routers" "sf_20_routers" \
    "sf_50_routers" "sf_60_routers" "sf_80_routers"  \
    "sf_100_routers" "sf_200_routers" "sf_400_routers" "sf_600_routers" \
    "sf_800_routers" "sf_1000_routers" "tree_40_routers" "random_10_routers" "random_20_routers"\
    "random_30_routers" "sw_sparse_40_routers" "lnd_gaussian" "lnd_uniform" "toy_dctcp" "parallel_graph")


demand_scale=("10" "60" "90")
routing_scheme=$1
pathChoice=$2
schedulingAlgorithm=$3
echo $routing_scheme
random_init_bal=false
random_capacity=false

widestPathsEnabled=false
obliviousRoutingEnabled=false
kspYenEnabled=false

#general parameters that do not affect config names
simulationLength=1000
statCollectionRate=5
timeoutClearRate=1
timeoutEnabled=true
signalsEnabled=true
loggingEnabled=false
transStatStart=0
transStatEnd=2000
mtu=1.0

# scheme specific parameters
eta=0.025
alpha=0.2
kappa=0.025
updateQueryTime=1.5
minPriceRate=0.25
zeta=0.01
rho=2
tau=10
normalizer=100
xi=1
routerQueueDrainTime=5
serviceArrivalWindow=300

#DCTCP parameters
windowBeta=0.1
windowAlpha=0.5
queueThreshold=20
queueDelayThreshold=300
balanceThreshold=0.01
minDCTCPWindow=1
rateDecreaseFrequency=3.0

for suffix in "Base" "Waterfilling" "LndBaseline" "PriceScheme" "DCTCP" "Celer"
do
    cp hostNode${suffix}.ned ${PATH_NAME}
    cp routerNode${suffix}.ned ${PATH_NAME}
done
cp hostNodeLandmarkRouting.ned ${PATH_NAME}
cp routerNodeDCTCPBal.ned ${PATH_NAME}
cp hostNodePropFairPriceScheme.ned ${PATH_NAME}


arraylength=${#prefix[@]}
PYTHON="python3"
mkdir -p ${PATH_NAME}

if [ -z "$pathChoice" ]; then
    pathChoice="shortest"
fi

if [ -z "$schedulingAlgorithm" ]; then
    schedulingAlgorithm="LIFO"
fi

echo $pathChoice
echo $schedulingAlgorithm

echo "${#num_nodes[@]}"


# TODO: find the indices in prefix of the topologies you want to run on and then specify them in array
# adjust experiment time as needed
#array=( 0 1 4 5 8 19 32)
array=( 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40)
for i in "${array[@]}"
do
    for balance in 100 200 300 600 900 #2000 9000 
    do
        echo ${prefix[i]}
        network="${prefix[i]}_circ_net"
        topofile="${PATH_NAME}${prefix[i]}_topo${balance}.txt"

        # identify graph type for topology
        if [ ${prefix[i]:0:2} == "sw" ]; then
            graph_type="small_world"
        elif [ ${prefix[i]:0:2} == "sf" ]; then
            graph_type="scale_free"
        elif [ ${prefix[i]:0:4} == "tree" ]; then
            graph_type="tree"
        elif [ ${prefix[i]:0:3} == "lnd" ]; then
            graph_type=${prefix[i]}
        elif [ ${prefix[i]} == "hotnets" ]; then
            graph_type="hotnets_topo"
        elif [ ${prefix[i]} == "toy_dctcp" ]; then
            graph_type="toy_dctcp"
        elif [ ${prefix[i]:0:8} == "parallel" ]; then
            graph_type="parallel_graph"
        elif [ ${prefix[i]:0:6} == "random" ]; then
            graph_type="random"
        else
            graph_type="simple_topologies"
        fi
        
        # set delay amount
        if [[ (${prefix[i]:0:3} == "two") || (${prefix[i]:0:5} == "three") ]]; then
            delay="120"
        else
            delay="30"
        fi
        
        # STEP 1: create topology
        $PYTHON scripts/create_topo_ned_file.py $graph_type\
                --network-name ${PATH_NAME}$network\
                --topo-filename $topofile\
                --num-nodes ${num_nodes[i]}\
                --balance-per-channel $balance\
                --separate-end-hosts \
                --delay-per-channel $delay\
                --randomize-start-bal $random_init_bal\
                --random-channel-capacity $random_capacity 


        # create workload files and run different demand levels
        for scale in "${demand_scale[@]}"
        do

            # generate the graph first to ned file
            workloadname="${prefix[i]}_circ_demand${scale}"
            workload="${PATH_NAME}$workloadname"
            inifile="${PATH_NAME}${workloadname}_default.ini"
            payment_graph_topo="custom"
            
            # figure out payment graph/workload topology
            if [ ${prefix[i]:0:9} == "five_line" ]; then
                payment_graph_topo="simple_line"
            elif [ ${prefix[i]:0:5} == "three" ]; then
                payment_graph_topo="simple_line"
            elif [ ${prefix[i]:0:4} == "five" ]; then
                payment_graph_topo="hardcoded_circ"
            elif [ ${prefix[i]:0:3} == "toy" ]; then
                payment_graph_topo="toy_dctcp"
            elif [ ${prefix[i]:0:7} == "hotnets" ]; then
                payment_graph_topo="hotnets_topo"
            elif [ ${prefix[i]:0:8} == "parallel" ]; then
                payment_graph_topo="parallel_graph"
            fi

            echo $network
            echo $topofile
            echo $inifile
            echo $graph_type

            # STEP 1: create transactions corresponding to this experiment run
            $PYTHON scripts/create_workload.py $workload poisson \
                    --graph-topo $payment_graph_topo \
                    --payment-graph-dag-percentage 0\
                    --topo-filename $topofile\
                    --experiment-time $simulationLength \
                    --balance-list $balance\
                    --generate-json-also \
                    --timeout-value 5 \
                    --scale-amount $scale \
                    --run-num 0 

            # STEP 3: run the experiment
            # routing schemes where number of path choices doesn't matter
            if [ ${routing_scheme} ==  "shortestPath" ]; then 
              output_file=outputs/${prefix[i]}_${balance}_circ_${routing_scheme}_demand${scale}0_${pathChoice}
              inifile=${PATH_NAME}${prefix[i]}_${balance}_circ_${routing_scheme}_demand${scale}_${pathChoice}.ini

              # create the ini file with specified parameters
              $PYTHON scripts/create_ini_file.py \
                      --network-name ${network}\
                      --topo-filename ${topofile}\
                      --workload-filename ${workload}_workload.txt\
                      --ini-filename $inifile\
                      --signals-enabled $signalsEnabled\
                      --logging-enabled $loggingEnabled\
                      --simulation-length $simulationLength\
                      --stat-collection-rate $statCollectionRate\
                      --timeout-clear-rate $timeoutClearRate\
                      --timeout-enabled $timeoutEnabled\
                      --routing-scheme ${routing_scheme}\
                      --demand-scale ${scale}\
                      --transStatStart $transStatStart\
                      --transStatEnd $transStatEnd\
                      --path-choice $pathChoice\
                      --balance $balance


              # run the omnetexecutable with the right parameters
              ./spiderNet -u Cmdenv -f $inifile -c ${network}_${balance}_${routing_scheme}_demand${scale}_${pathChoice} -n ${PATH_NAME}\
                    > ${output_file}.txt & 
            
          else
              pids=""
              # if you add more choices for the number of paths you might run out of cores/memory
              for numPathChoices in 4
              do
                output_file=outputs/${prefix[i]}_${balance}_circ_${routing_scheme}_demand${scale}0_${pathChoice}_${numPathChoices}
                inifile=${PATH_NAME}${prefix[i]}_${balance}_circ_${routing_scheme}_demand${scale}_
		inifile=${inifile}${pathChoice}_${numPathChoices}.ini

                if [[ $routing_scheme =~ .*Window.* ]]; then
                    windowEnabled=true
                else 
                    windowEnabled=false
                fi


                echo "Creating ini file"
                # create the ini file with specified parameters
                $PYTHON scripts/create_ini_file.py \
                        --network-name ${network}\
                        --topo-filename ${topofile}\
                        --workload-filename ${workload}_workload.txt\
                        --ini-filename ${inifile}\
                        --signals-enabled $signalsEnabled\
                        --logging-enabled $loggingEnabled\
                        --simulation-length $simulationLength\
                        --stat-collection-rate $statCollectionRate\
                        --timeout-clear-rate $timeoutClearRate\
                        --timeout-enabled $timeoutEnabled\
                        --routing-scheme ${routing_scheme}\
                        --num-path-choices ${numPathChoices}\
                        --zeta $zeta\
                        --alpha $alpha\
                        --eta $eta\
                        --kappa $kappa\
                        --rho $rho\
                        --update-query-time $updateQueryTime\
                        --min-rate $minPriceRate\
                        --tau $tau\
                        --normalizer $normalizer \
                        --window-enabled $windowEnabled\
                        --demand-scale $scale\
                        --xi $xi\
                        --router-queue-drain-time $routerQueueDrainTime\
                        --service-arrival-window $serviceArrivalWindow\
                        --transStatStart $transStatStart\
                        --transStatEnd $transStatEnd\
                        --path-choice $pathChoice\
                        --balance $balance \
                        --window-alpha $windowAlpha \
                        --window-beta $windowBeta \
                        --queue-threshold $queueThreshold \
                        --scheduling-algorithm $schedulingAlgorithm\
                        --queue-delay-threshold $queueDelayThreshold \
                        --balance-ecn-threshold $balanceThreshold \
                        --mtu $mtu\
                        --min-dctcp-window $minDCTCPWindow \
                        --rate-decrease-frequency $rateDecreaseFrequency


                # run the omnetexecutable with the right parameters
                # in the background
                ./spiderNet -u Cmdenv -f ${inifile}\
                    -c ${network}_${balance}_${routing_scheme}_demand${scale}_${pathChoice}_${numPathChoices}_${schedulingAlgorithm}  -n ${PATH_NAME}\
                    > ${output_file}.txt &
                pids+=($!)
                done
            fi
            wait # for all algorithms to complete for this demand

            # STEP 4: plot everything for this demand
            # TODO: add plotting script
            echo "Plotting"
            payment_graph_type='circ' 
            if [ "$timeoutEnabled" = true ] ; then timeout=""; else timeout="no_timeouts"; fi
            if [ "$random_init_bal" = true ] ; then suffix="randomInitBal_"; else suffix=""; fi
            if [ "$random_capacity" = true ]; then suffix="${suffix}randomCapacity_"; fi
            echo $suffix
            graph_op_prefix=${GRAPH_PATH}${timeout}/${prefix[i]}${balance}_delay${delay}_demand${scale}0_${suffix}
            vec_file_prefix=${PATH_NAME}results/${prefix[i]}_${payment_graph_type}_net_${balance}_
            
            #routing schemes where number of path choices doesn't matter
            if [ ${routing_scheme} ==  "shortestPath" ]; then 
                vec_file_path=${vec_file_prefix}${routing_scheme}_demand${scale}_${pathChoice}-#0.vec
                sca_file_path=${vec_file_prefix}${routing_scheme}_demand${scale}_${pathChoice}-#0.sca


                $PYTHON scripts/generate_analysis_plots_for_single_run.py \
                  --detail $signalsEnabled \
                  --vec_file ${vec_file_path} \
                  --sca_file ${sca_file_path} \
                  --save ${graph_op_prefix}${routing_scheme}_${pathChoice} \
                  --balance \
                  --queue_info --timeouts --frac_completed \
                  --inflight --timeouts_sender \
                  --waiting --bottlenecks --time_inflight
            

            #routing schemes where number of path choices matter
            else
              for numPathChoices in 4
                do
                    vec_file_path=${vec_file_prefix}${routing_scheme}_demand${scale}_${pathChoice}_${numPathChoices}_${schedulingAlgorithm}-#0.vec
                    sca_file_path=${vec_file_prefix}${routing_scheme}_demand${scale}_${pathChoice}_${numPathChoices}_${schedulingAlgorithm}-#0.sca


                    $PYTHON scripts/generate_analysis_plots_for_single_run.py \
                      --detail $signalsEnabled \
                      --vec_file ${vec_file_path} \
                      --sca_file ${sca_file_path} \
                      --save ${graph_op_prefix}${routing_scheme}_${pathChoice}_${numPathChoices} \
                      --balance \
                      --queue_info --timeouts --frac_completed \
                      --frac_completed_window \
                      --inflight --timeouts_sender --time_inflight \
                      --waiting --bottlenecks --probabilities \
                      --mu_local --lambda --n_local --service_arrival_ratio --inflight_outgoing \
                      --inflight_incoming --rate_to_send --price --mu_remote --demand \
                      --rate_sent --amt_inflight_per_path --rate_acked --fraction_marked --queue_delay \
                      --cpi --perDestQueue --kStar
                  done
              fi

            # STEP 5: cleanup        
            #rm ${PATH_NAME}${prefix[i]}_circ*_demand${scale}.ini
            #rm ${workload}_workload.txt
            #rm ${workload}.json
        done
        #rm $topofile
    done
done



# FIle \\wsl.localhost\Ubuntu-20.04\home\ad\omnetpp-5.6\samples\spider_omnet-Copy\scripts\collect_lnd_ping_times.py

import json
import subprocess
import pingparsing as pp
import statistics as stat
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import argparse
from config import *


parser = argparse.ArgumentParser(description='Collect ping times to lnd nodes')
parser.add_argument('--rerun-ping', action="store_true", help="whether to rerun the pings")
args = parser.parse_args()

# get the list of ip addresses to ping
def parse_json_data():
    with open(LND_FILE_PATH + 'lnd_july15_2019.json') as f:
        nodes = (json.load(f))["nodes"]

    ping_addresses = []
    for n in nodes:
        for a in n["addresses"]:
            if "onion" not in a["addr"]:
                ping_addresses.append(a["addr"])

    return ping_addresses




# ping a specific node at the given ip address and return stats for it
def ping_node(ip_addr):
    ping_parser = pp.PingParsing()
    transmitter = pp.PingTransmitter()
    transmitter.destination = ip_addr
    transmitter.count = 10
    result = transmitter.ping()
    try: 
        return ping_parser.parse(result.stdout).as_dict()
    except: 
        print(result)
        return None



# ping all nodes in the list and aggregate stats over all of them
# all stats are in milliseconds
def ping_nodes(addr_list):
    overall_rtts = []
    f = open(LND_FILE_PATH + "ping_times_data", "w+")
    for a in addr_list:
        ip_address = a.split(":")[0]
        result = ping_node(ip_address)
        rtt_avg = result["rtt_avg"] if result is not None else None

        if rtt_avg is not None:
            overall_rtts.append(rtt_avg)
            if (rtt_avg) < 10:
                print(ip_address, rtt_avg)
            f.write(str(rtt_avg) + "\n")
    f.close()

    print("Mean", stat.mean(overall_rtts))
    print("Median", stat.median(overall_rtts))
    print("Variance", stat.variance(overall_rtts))

    return overall_rtts


# parse rtts from text file
def parse_rtts_from_file():
    rtts = []
    with open(LND_FILE_PATH + "ping_times_data") as f:
        for line in f.readlines():
            if "None" not in f and " " not in f:
                rtts.append(float(line))
    return rtts


# visualize the data
def visualize_rtts(rtts):
    f = plt.figure()
    plt.hist(rtts, 100, density=True, histtype='step', cumulative=True)
    f.savefig("lnd_ping_rtt_spread.pdf")
    print("Mean", stat.mean(rtts))
    print("Median", stat.median(rtts))
    print("Variance", stat.variance(rtts))
    print("Max", max(rtts))
    print("Min", min(rtts))



address_list = parse_json_data()
print(len(address_list))
all_rtts = ping_nodes(address_list) if args.rerun_ping else parse_rtts_from_file()
visualize_rtts(all_rtts)


# File Path: \\wsl.localhost\Ubuntu-20.04\home\ad\omnetpp-5.6\samples\spider_omnet-Copy\scripts\config.py

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



# File Path: \\wsl.localhost\Ubuntu-20.04\home\ad\omnetpp-5.6\samples\spider_omnet-Copy\scripts\convert_pkl_paths_to_format.py
import pickle
from config import *
import collections

OP_FILE_PATH="../benchmarks/circulations/"

topo_filelist = ['sw_50_routers_lndCap_topo4000', 'sf_50_routers_lndCap_topo4000', 'lnd_july15_2019_topo40'] 
file_prefix = ['sw_50_routers_lndCap_topo4000', 'sf_50_routers_lndCap_topo4000', 'lnd_july15_2019_topo40'] 
path_type_list = ['ksp_yen', 'oblivious', 'ksp_edge_disjoint', 'heuristic', 'kwp_edge_disjoint']
op_suffix_list = ['_kspYenPaths', '_obliviousPaths', '_edgeDisjointPaths', '_heuristicPaths', '_widestPaths']


for topo, op_prefix in zip(topo_filelist, file_prefix):
    for path_type, op_suffix in zip(path_type_list, op_suffix_list):
        num_paths = []
        filename = topo + "_" + path_type
        pkl_file = open(PATH_PKL_DATA + filename + ".pkl", 'rb')
        paths = pickle.load(pkl_file)
        pkl_file.close()

        output_filename = OP_FILE_PATH + op_prefix + ".txt" + op_suffix
        output_file = open(output_filename, "w+")

        for (sender, receiver) in list(paths.keys()):
            output_file.write("pair " + str(sender) + " " + str(receiver) + "\n")
            num_paths.append(len(paths[(sender, receiver)]))
            for path in paths[(sender, receiver)]:
                path_str = [str(p) for p in path]
                output_file.write(" ".join(path_str) + "\n")

        ctr = collections.Counter(num_paths)
        print(filename, ctr)
        output_file.close()


# File Path: \\wsl.localhost\Ubuntu-20.04\home\ad\omnetpp-5.6\samples\spider_omnet-Copy\scripts\create_topo_ned_file.py

import sys
import textwrap
import argparse
import networkx as nx
from config import *
import re
import os
import math
import random
import numpy as np

def parse_node_name(node_name, max_router, max_host):
    try:
        val = int(node_name[:-1])
        if(node_name[-1] == 'r'):
            if(val > max_router):
                max_router = val
            return ("router[" + str(val) + "]", max_router, max_host)
        if(node_name[-1] == 'e'):
            if(val > max_host):
                max_host = val
            return ("host[" + str(val) + "]", max_router, max_host)
        return -1
    except:
        return -1



# take the topology file in a specific format and write it to a ned file
def write_ned_file(topo_filename, output_filename, network_name, routing_alg):
    # topo_filename must be a text file where each line contains the ids of two neighbouring nodes that 
    # have a payment channel between them, relative delays in each direction,  initial balance on each 
    # end (see sample-topology.txt)
    # each line is of form:
    # [node1] [node2] [1->2 delay] [2->1 delay] [balance @ 1] [balance @ 2]
    topo_file = open(topo_filename).readlines() 
    outfile = open(output_filename, "w")

    # metadata used for forwarding table
    neighbor_interfaces = dict()
    node_interface_count = dict()
    node_used_interface = dict()
    linklist = list()
    max_val = -1 #used to find number of nodes, assume nodes start at 0 and number consecutively
    max_router = -1
    max_host = -1
    line_num = 0
    for line in topo_file:
        line_num += 1

        # landmark line
        if line_num == 1:
            continue

        if line == "\n":
            continue

        n1 = parse_node_name(line.split()[0], max_router, max_host)
        if(n1 == -1):
            print("Bad line " + line)
            continue
        max_router = n1[1]
        max_host = n1[2]

        n2 = parse_node_name(line.split()[1], max_router, max_host)
        if(n2 == -1):
            print("Bad line " + line)
            continue
        max_router = n2[1]
        max_host = n2[2]

        n3 = float(line.split()[2]) # delay going from n1 to n2
        n4 = float(line.split()[3]) # delay going from n2 to n1

        linklist.append((n1[0], n2[0], n3, n4))

    max_router = max_router + 1
    max_host = max_host + 1

    # generic routerNode and hostNode definition that every network will have
    print(routing_alg)
    if (routing_alg == 'shortestPath'):
        host_node_type = 'hostNodeBase'
        router_node_type = 'routerNodeBase'
    else:
        if routing_alg == 'DCTCPBal' or routing_alg == 'DCTCPQ' or routing_alg == 'TCP' or routing_alg == 'TCPCubic':
            host_node_type = 'hostNodeDCTCP'
        elif routing_alg == 'DCTCPRate':
            host_node_type = 'hostNodePropFairPriceScheme'
        else:
            host_node_type = 'hostNode' + routing_alg[0].upper() + routing_alg[1:]
        
        if routing_alg == 'landmarkRouting':
            router_node_type = 'routerNodeWaterfilling'
        elif routing_alg == 'DCTCPRate' or routing_alg == 'DCTCPQ' or routing_alg == 'TCP' or routing_alg == 'TCPCubic':
            router_node_type = 'routerNodeDCTCP'
        else:
            router_node_type = 'routerNode' + routing_alg[0].upper() + routing_alg[1:]
        print(router_node_type)

    outfile.write("import " + router_node_type + ";\n")
    outfile.write("import " + host_node_type + ";\n\n")

    outfile.write("network " + network_name + "_" + routing_alg + "\n")
    outfile.write("{\n")

    # This script (meant for a simpler datacenter topology) just assigns the same link delay to all links.
    # You need to change this such that the parameter values are instead assigned on a per node basis and 
    # are read from an additional 'delay' column and 'channel balance' columns in the text file.
    outfile.write('\tparameters:\n\t\tdouble linkDelay @unit("s") = default(100us);\n')
    outfile.write('\t\tdouble linkDataRate @unit("Gbps") = default(1Gbps);\n')
    outfile.write('\tsubmodules:\n')
    outfile.write('\t\thost['+str(max_host)+']: ' + host_node_type + ' {} \n')
    outfile.write('\t\trouter['+str(max_router)+']: ' + router_node_type + ' {} \n')
    outfile.write('\tconnections: \n')

    for link in linklist:
        a = link[0]
        b = link[1]
        abDelay = link[2]
        baDelay = link[3]

        outfile.write('\t\t' + a + '.out++ --> {delay = ' + str(abDelay) +'ms; }')
        outfile.write(' --> ' + b + '.in++;  \n')
        outfile.write('\t\t' + a + '.in++ <-- {delay = ' + str(baDelay) +'ms; }')
        outfile.write(' <-- ' + b + '.out++;  \n')
    outfile.write('}\n')




# generate either a small world or scale free graph
def generate_graph(size, graph_type):
    if graph_type == 'random':
        G = nx.dense_gnm_random_graph(size, size * 5,seed=SEED)
    elif graph_type == 'small_world':
        G = nx.watts_strogatz_graph(size, 8, 0.25, seed=SEED)
    elif graph_type == 'small_world_sparse':
        G = nx.watts_strogatz_graph(size, size/8, 0.25, seed=SEED)
    elif graph_type == 'scale_free':
        # regular expts
        G = nx.barabasi_albert_graph(size, 8, seed=SEED) 

        # implementation, celer expts - 10 node graph
        # G = nx.barabasi_albert_graph(size, 5, seed=12)
    elif graph_type == 'scale_free_sparse':
        G = nx.barabasi_albert_graph(size, size/8, seed=SEED)
    elif graph_type == 'tree':
        G = nx.random_tree(size, seed=SEED)

    # remove self loops and parallel edges
    G.remove_edges_from(G.selfloop_edges())
    G = nx.Graph(G)

    print('Generated a ', graph_type, ' graph')
    print('number of nodes: ', G.number_of_nodes())
    print('Number of Edges: ', G.number_of_edges())
    print('Number of connected components: ', nx.number_connected_components(G))
    return G




# print the output in the desired format for write_ned_file to read
# generate extra end host nodes if need be
# make the first line list of landmarks for this topology
def print_topology_in_format(G, balance_per_channel, delay_per_channel, output_filename, separate_end_hosts,\
        randomize_init_bal=False, random_channel_capacity=False, lnd_capacity=False, is_lnd=False, rebalancing_enabled=False):
    f1 = open(output_filename, "w+")
    end_host_delay = delay_per_channel

    offset = G.number_of_nodes()
    if (separate_end_hosts == False):
        offset = 0

    nodes_sorted_by_degree = sorted(G.degree, key=lambda x: x[1], reverse=True)

    # generate landmarks based on degree
    i = 0
    landmarks, current_list = [], []
    max_degree = -1
    while len(landmarks) < NUM_LANDMARKS and i < len(nodes_sorted_by_degree):
        num_remaining = NUM_LANDMARKS - len(landmarks)
        if nodes_sorted_by_degree[i][1] == max_degree:
            current_list.append(nodes_sorted_by_degree[i][0])
        else:
            spaced_indices = np.round(np.linspace(0, len(current_list)-1, \
                    min(num_remaining, len(current_list)))).astype(int)
            if max_degree != -1:
                landmarks.extend([current_list[x] for x in spaced_indices])
            current_list = [nodes_sorted_by_degree[i][0]]
            max_degree = nodes_sorted_by_degree[i][1]
        i += 1
    if len(landmarks) < NUM_LANDMARKS:
        spaced_indices = np.round(np.linspace(0, len(current_list)-1, \
                    min(num_remaining, len(current_list)))).astype(int)
        landmarks.extend([current_list[x] for x in spaced_indices])

     
    # make the first line the landmarks and make them all router nodes
    for l in landmarks[:NUM_LANDMARKS]:
        f1.write(str(l) + "r ")
    f1.write("\n")

    total_budget = balance_per_channel * len(G.edges())
    weights = {e: min(G.degree(e[0]), G.degree(e[1])) for e in G.edges()}
    sum_weights = sum(weights.values())
    capacity_dict = dict()

    # get lnd capacity data
    lnd_capacities_graph = nx.read_edgelist(LND_FILE_PATH + 'lnd_july15_2019_reducedsize' + '.edgelist')
    lnd_capacities = list(nx.get_edge_attributes(lnd_capacities_graph, 'capacity').values()) 

    # write rest of topology
    real_rtts = np.loadtxt(LND_FILE_PATH + "ping_times_data")
    for e in G.edges():

        f1.write(str(e[0]) + "r " + str(e[1]) +  "r ")
        
        if not random_channel_capacity and is_lnd and "uniform" not in output_filename:
            delay_per_channel = np.random.choice(real_rtts) / 2.0
            f1.write(str(delay_per_channel) + " " + str(delay_per_channel) + " ")
        else:
            f1.write(str(delay_per_channel) + " " + str(delay_per_channel) + " ")
        
        if random_channel_capacity:
            balance_for_this_channel = -1
            while balance_for_this_channel < 2:
                balance_for_this_channel = round(np.random.normal(balance_per_channel, \
                        0.75 * balance_per_channel))
        
        elif lnd_capacity:
            balance_for_this_channel = -1
            while balance_for_this_channel < 40:
                balance_for_this_channel = round(np.random.choice(lnd_capacities) * \
                    (balance_per_channel / np.mean(lnd_capacities)))
        
        elif is_lnd and "uniform" not in output_filename:
            if "lessScale" in output_filename:
                balance_for_this_channel = int(round(G[e[0]][e[1]]['capacity'] *10 * balance_per_channel))
            else:
                balance_for_this_channel = int(round(G[e[0]][e[1]]['capacity'] * balance_per_channel))
        
        else:
            balance_for_this_channel = balance_per_channel

        capacity_dict[e] = balance_for_this_channel

        if randomize_init_bal:
            one_end_bal = random.randint(1, balance_for_this_channel)
            other_end_bal = balance_for_this_channel - one_end_bal
            f1.write(str(one_end_bal) + " " + str(other_end_bal) + "\n")
        else:
            f1.write(str(round(balance_for_this_channel/2)) + " " + \
                    str(round(balance_for_this_channel/2)) + "\n")

    # generate extra end host nodes
    if separate_end_hosts : 
        for n in G.nodes():
            f1.write(str(n) + "e " + str(n) + "r ")
            f1.write(str(end_host_delay) + " " + str(end_host_delay) + " ")
            if rebalancing_enabled:
                f1.write(str(REASONABLE_BALANCE) + " " + str(REASONABLE_ROUTER_BALANCE) + "\n")
            else:
                f1.write(str(LARGE_BALANCE/2) + " " + str(LARGE_BALANCE/2) + "\n")

        if args.graph_type == "parallel_graph":
            for (e,r) in zip([1,3], [0, 2]):
                f1.write(str(e) + "e " + str(r) + "r ")
                f1.write(str(end_host_delay) + " " + str(end_host_delay) + " ")
                f1.write(str(LARGE_BALANCE/2) + " " + str(LARGE_BALANCE/2) + "\n")
    f1.close()

    nx.set_edge_attributes(G, capacity_dict, 'capacity')

# parse arguments
parser = argparse.ArgumentParser(description="Create arbitrary topologies to run the omnet simulator on")
parser.add_argument('--num-nodes', type=int, dest='num_nodes', help='number of nodes in the graph', default=20)
parser.add_argument('--delay-per-channel', type=int, dest='delay_per_channel', \
        help='delay between nodes (ms)', default=30)
parser.add_argument('graph_type', choices=['small_world', 'scale_free', 'hotnets_topo', 'simple_line', 'toy_dctcp', \
        'simple_deadlock', 'simple_topologies', 'parallel_graph', 'dag_example', 'lnd_dec4_2018','lnd_dec4_2018lessScale', \
        'lnd_dec4_2018_randomCap', 'lnd_dec4_2018_modified', 'lnd_uniform', 'tree', 'random', \
        'lnd_july15_2019'], \
        help='type of graph (Small world or scale free or custom topology list)', default='small_world')
parser.add_argument('--balance-per-channel', type=int, dest='balance_per_channel', default=100)
parser.add_argument('--topo-filename', dest='topo_filename', type=str, \
        help='name of intermediate output file', default="topo.txt")
parser.add_argument('--network-name', type=str, dest='network_name', \
        help='name of the output ned filename', default='simpleNet')
parser.add_argument('--separate-end-hosts', action='store_true', \
        help='do you need separate end hosts that only send transactions')
parser.add_argument('--randomize-start-bal', type=str, dest='randomize_start_bal', \
        help='Do not start from pergect balance, but rather randomize it', default='False')
parser.add_argument('--random-channel-capacity', type=str, dest='random_channel_capacity', \
        help='Give channels a random balance between bal/2 and bal', default='False')
parser.add_argument('--lnd-channel-capacity', type=str, dest='lnd_capacity', \
        help='Give channels a random balance sampled from lnd', default='False')
parser.add_argument('--rebalancing-enabled', type=str, dest="rebalancing_enabled",\
        help="should the end host router channel be reasonably sized", default="false")
routing_alg_list = ['shortestPath', 'priceScheme', 'waterfilling', 'landmarkRouting', 'lndBaseline', \
        'DCTCP', 'DCTCPBal', 'DCTCPRate', 'DCTCPQ', 'TCP', 'TCPCubic', 'celer']

args = parser.parse_args()
np.random.seed(SEED)
random.seed(SEED)

# generate graph and print topology and ned file
if args.num_nodes <= 5 and args.graph_type == 'simple_topologies':
    if args.num_nodes == 2:
        G = two_node_graph
    elif args.num_nodes == 3:
        G = three_node_graph
    elif args.num_nodes == 4:
        G = four_node_graph
    elif 'line' in args.network_name:
        G = five_line_graph
    else:
        G = five_node_graph
elif args.graph_type in ['small_world', 'scale_free', 'tree', 'random']:
    if "sparse" in args.topo_filename:
        args.graph_type = args.graph_type + "_sparse"
    G = generate_graph(args.num_nodes, args.graph_type)
elif args.graph_type == 'toy_dctcp':
    G = toy_dctcp_graph
elif args.graph_type == 'dag_example':
    print("generating dag example")
    G = dag_example_graph
elif args.graph_type == 'parallel_graph':
    G = parallel_graph
elif args.graph_type == 'hotnets_topo':
    G = hotnets_topo_graph
elif args.graph_type == 'simple_deadlock':
    G = simple_deadlock_graph
    args.separate_end_hosts = False
elif args.graph_type.startswith('lnd_'):
    G = nx.read_edgelist(LND_FILE_PATH + 'lnd_july15_2019_reducedsize' + '.edgelist')
else:
    G = simple_line_graph
    args.separate_end_hosts = False

args.randomize_start_bal = args.randomize_start_bal == 'true'
args.random_channel_capacity = args.random_channel_capacity == 'true'
args.lnd_capacity = args.lnd_capacity == 'true'

print_topology_in_format(G, args.balance_per_channel, args.delay_per_channel, args.topo_filename, \
        args.separate_end_hosts, args.randomize_start_bal, args.random_channel_capacity,\
        args.lnd_capacity, args.graph_type.startswith('lnd_'), args.rebalancing_enabled == "true")
network_base = os.path.basename(args.network_name)

for routing_alg in routing_alg_list:
    write_ned_file(args.topo_filename, args.network_name + '_' + routing_alg + '.ned', \
            network_base, routing_alg)


# File Path: \\wsl.localhost\Ubuntu-20.04\home\ad\omnetpp-5.6\samples\spider_omnet-Copy\scripts\create_workload.py


import sys
import textwrap
import argparse
import numpy as np
import networkx as nx
import random
import json
from config import *
from max_circulation import *
import pickle
from functools import reduce

# generates the start and end nodes for a fixed set of topologies - hotnets/line/simple graph
def generate_workload_standard(filename, payment_graph_topo, workload_type, total_time, \
        log_normal, kaggle_size, txn_size_mean, timeout_value, generate_json_also, circ_frac, std_workload=True):
    # by default ASSUMES NO END HOSTS

    dag_frac = round(1 - circ_frac,3)
    demand_dict_dag = dict()
    demand_dict_circ = dict()

    # define start and end nodes and amounts
    # edge a->b in payment graph appears in index i as start_nodes[i]=a, and end_nodes[i]=b
    if payment_graph_topo == 'hotnets_topo':
        if circ_frac == 1:
            start_nodes = [0, 1, 2, 2, 3, 3, 4]
            end_nodes = [1, 3, 1, 4, 2, 0, 2]
            amt_relative = [1, 2, 1, 1, 1, 1, 1]
            amt_absolute = [SCALE_AMOUNT * x for x in amt_relative]
        else:
            start_nodes = [0,3,0,1,2,3,2,4]
            end_nodes = [4,0,1,3,1,2,4,2]
            amt_relative = [1,2,1,2,1,2,2,1]
            amt_absolute = [SCALE_AMOUNT * x for x in amt_relative]
        graph = hotnets_topo_graph

    elif payment_graph_topo == 'toy_dctcp':
        start_nodes = [2, 4, 6, 8, 10, 3, 5, 7, 9, 11]
        end_nodes = [3, 5, 7, 9, 11, 2, 4, 6, 8, 10]
        amt_relative = [1] * 10
        print("here generating topo")
        amt_absolute = [SCALE_AMOUNT * MEAN_RATE * x for x in amt_relative]
        graph = toy_dctcp_graph

    elif payment_graph_topo == 'simple_deadlock':
        start_nodes = [1,0,2]
        end_nodes = [2,2,0]
        amt_relative = [2,1,2]
        amt_absolute = [SCALE_AMOUNT * x for x in amt_relative]
        graph = simple_deadlock_graph

    elif payment_graph_topo == 'dag_example':
        start_nodes = [0, 2, 1]
        end_nodes = [2, 1, 2]
        amt_relative = [10, 5, 5]
        amt_absolute = [SCALE_AMOUNT * x for x in amt_relative]
        graph = dag_example_graph

    elif payment_graph_topo == 'parallel_graph':
        start_nodes = [0, 2, 1, 3]
        end_nodes = [2, 0, 3, 1]
        amt_relative = [1, 1, 1, 1]
        amt_absolute = [SCALE_AMOUNT * x for x in amt_relative]
        graph = parallel_graph


    elif payment_graph_topo == 'simple_line':
        if "five" in filename:
            num_nodes = 5
            graph = five_line_graph
        else:
            num_nodes = 3
            graph = simple_line_graph
        print(num_nodes)

        start_nodes = [0, num_nodes - 1]
        end_nodes = [num_nodes - 1, 0]
        amt_relative = [MEAN_RATE] * 2
        '''start_nodes = [0, 2, 0, 1] 
        end_nodes = [2, 0, 1, 0]
        amt_relative = [MEAN_RATE, MEAN_RATE, 2*MEAN_RATE, 2*MEAN_RATE]'''
        amt_absolute = [SCALE_AMOUNT * x for x in amt_relative]
        
        demand_dict_circ[0,num_nodes - 1] = MEAN_RATE
        demand_dict_circ[num_nodes - 1, 0] = MEAN_RATE
        demand_dict_dag[0, num_nodes - 1] = MEAN_RATE


    elif payment_graph_topo == 'hardcoded_circ':
        start_nodes = [0, 1, 2, 3, 4]
        end_nodes = [1, 2, 3, 4, 0]
        amt_relative = [MEAN_RATE] * 5
        amt_absolute = [SCALE_AMOUNT * x for x in amt_relative]
        graph = five_node_graph

    # generate circulation instead if you need a circulation
    if not std_workload:
        start_nodes, end_nodes, amt_relative = [], [], []
        num_nodes = graph.number_of_nodes()
    	
        """ generate circulation and dag demand """


        if circ_frac > 0:
    	    demand_dict_circ = circ_demand(list(graph), mean=MEAN_RATE, \
                    std_dev=CIRCULATION_STD_DEV)
                
        if dag_frac > 0:
            demand_dict_dag = dag_demand(list(graph), mean=MEAN_RATE, \
                std_dev=CIRCULATION_STD_DEV)
                    
    demand_dict = { key: circ_frac * demand_dict_circ.get(key, 0) + 
            dag_frac * demand_dict_dag.get(key, 0) \
            for key in set(demand_dict_circ) | set(demand_dict_dag) }

    print(demand_dict)


    for i, j in list(demand_dict.keys()):
        start_nodes.append(i)
        end_nodes.append(j)
        amt_relative.append(demand_dict[i, j])
    if payment_graph_topo != 'hardcoded_circ':
        amt_absolute = [SCALE_AMOUNT * MEAN_RATE * x for x in amt_relative]

    print(amt_absolute)


    if generate_json_also:
        generate_json_files(filename + '.json', graph, graph, start_nodes, end_nodes, amt_absolute)

    write_txns_to_file(filename + '_workload.txt', start_nodes, end_nodes, amt_absolute,\
            workload_type, total_time, log_normal, kaggle_size, txn_size_mean, timeout_value)




# write the given set of txns denotes by start_node -> end_node with absolute_amts as passed in
# to a separate workload file
# workload file of form
# [amount] [timeSent] [sender] [receiver] [priorityClass] [timeout_value]
# write to file - assume no priority for now
# transaction sizes are either constant or exponentially distributed around their mean
def write_txns_to_file(filename, start_nodes, end_nodes, amt_absolute,\
        workload_type, total_time, log_normal, kaggle_size, txn_size_mean, timeout_value, mode="w", start_time=0):
    outfile = open(filename, mode)
    if "newseed" in filename:
        print("newseed")
        np.random.seed(12493)

    if distribution == 'uniform':
        # constant transaction size generated at uniform intervals
        for k in range(len(start_nodes)):
            cur_time = 0
            '''if (start_nodes[k] == 1 or end_nodes[k] == 1):
                cur_time = 300'''

            while cur_time < total_time:
                rate = amt_absolute[k]
                if log_normal:
                    txn_size = MIN_TXN_SIZE/10
                    while (txn_size < MIN_TXN_SIZE or txn_size > MAX_TXN_SIZE):
                        txn_power = np.random.normal(loc=LOG_NORMAL_MEAN, scale=LOG_NORMAL_SCALE)
                        txn_size = round(10 ** txn_power, 1) 
                else:
                    txn_size = txn_size_mean

                outfile.write(str(txn_size) + " " + str(cur_time + start_time) + " " + str(start_nodes[k]) + \
                        " " + str(end_nodes[k]) + " 0 " + str(timeout_value) + "\n")
                cur_time += (1.0 / rate)

    elif distribution == 'poisson':
        if kaggle_size:
            print("generating from kaggle for size")
            amt_dist = np.load(KAGGLE_AMT_DIST_FILENAME)
            num_amts = amt_dist.item().get('p').size

        # constant transaction size to be sent in a poisson fashion
        for k in range(len(start_nodes)):
            current_time = 0.0
            rate = amt_absolute[k]*1.0
            beta = (1.0) / (1.0 * rate)

            # if the rate is higher, given pair will have more transactions in a single second
            while current_time < total_time:

                if log_normal:
                    txn_size = MIN_TXN_SIZE/10
                    while (txn_size < MIN_TXN_SIZE or txn_size > MAX_TXN_SIZE):
                        txn_power = np.random.normal(loc=LOG_NORMAL_SCALE, scale=LOG_NORMAL_SCALE)
                        txn_size = round(10 ** txn_power, 1) 
                elif kaggle_size:
                    # draw an index according to the amount pmf
                    txn_idx = np.random.choice(num_amts, 1, \
                                           p=amt_dist.item().get('p'))[0]
                    # map the index to a tx amount
                    txn_size = int(round(amt_dist.item().get('bins')[txn_idx], 1))
                else:                
                    txn_size = txn_size_mean

                outfile.write(str(txn_size) + " " + str(current_time + start_time) + " " + str(start_nodes[k]) \
                        + " " + str(end_nodes[k]) + " 0 " + str(timeout_value) + "\n")

                time_incr = np.random.exponential(beta)
                current_time = current_time + time_incr 

    elif distribution == 'kaggle':
        # load the data
        amt_dist = np.load(KAGGLE_AMT_DIST_FILENAME)
        time_dist = np.load(KAGGLE_TIME_DIST_FILENAME)
        num_amts = amt_dist.item().get('p').size
        num_times = time_dist.item().get('p').size
        # transaction sizes drawn from kaggle data, as is inter-transaction time
        for k in range(len(start_nodes)):
            current_time = 0.0
            while current_time < total_time:
                # draw an index according to the amount pmf
                txn_idx = np.random.choice(num_amts, 1, \
                                           p=amt_dist.item().get('p'))[0]
                # map the index to a tx amount
                txn_size = amt_dist.item().get('bins')[txn_idx]
                outfile.write(str(txn_size) + " " + str(current_time + start_time) + " " + str(start_nodes[k]) + \
                        " " + str(end_nodes[k]) + " 0 " + str(timeout_value) + "\n")
                # draw an index according to the time pmf
                time_idx = np.random.choice(num_times, 1, \
                                            p=time_dist.item().get('p'))[0]
                # map index to an inter-tx time
                time_incr = time_dist.item().get('bins')[time_idx]
                current_time = current_time + time_incr

    outfile.close()
    np.random.seed(SEED_LIST[args.run_num])




# generates the json file necessary for the distributed testbed to be used to test
# the lnd implementation
def generate_json_files(filename, graph, inside_graph, start_nodes, end_nodes, amt_absolute):
    for balance in balance_list: 
        json_string = {}
        # create btcd connections
        # routers connected to each other and end hosts connected to respective router
        btcd_connections = []
        for i in range(graph.number_of_nodes() - 1):
            connection = {"src": str(i) + "r", "dst" : str(i + 1) + "r"}
            btcd_connections.append(connection)
            connection = {"src": str(i) + "e", "dst" : str(i) + "r"}
            btcd_connections.append(connection)
        connection = {"src": str(graph.number_of_nodes() - 1) + "e", "dst" : str(graph.number_of_nodes() - 1) + "r"}
        btcd_connections.append(connection)
        json_string["btcd_connections"] = btcd_connections

        # miner node
        json_string["miner"] = "0r"


        # create nodesi for end hosts and router nodes and assign them distinct ips
        nodes = []
        for n in graph.nodes():
            node = {"name": str(n) + "r", "ip" : "10.0.1." + str(100 + n)}
            nodes.append(node)
            node = {"name": str(n) + "e", "ip" : "10.0.2." + str(100 + n)}
            nodes.append(node)
        json_string["nodes"] = nodes

        # creates all the lnd channels
        edges = []
        for (u,v) in graph.edges():
            if  u == v:
                cap = ENDHOST_LND_ONE_WAY_CAPACITY
                node_type = "e"
            else:
                cap = balance * 400 / 2
                node_type = "r"

            if u <= v: 
                edge = {"src": str(u) + "r", "dst": str(v) + node_type, "capacity" : cap}
                edges.append(edge)

        json_string["lnd_channels"] = edges

        # creates the string for the demands
        demands = []
        for s, e, a in zip(start_nodes, end_nodes, amt_absolute):
            demand_entry = {"src": str(s) + "e", "dst": str(e) + "e",\
                            "rate": a}
            demands.append(demand_entry)

        json_string["demands"] = demands

        with open(filename + '_' + str(balance) + '.json', 'w') as outfile:
                json.dump(json_string, outfile, indent=8)




# generate workload for arbitrary topology by uniformly sampling
# the set of nodes for sender-receiver pairs
# size of transaction is determined when writing to the file to
# either be exponentially distributed or constant size
def generate_workload_for_provided_topology(filename, inside_graph, whole_graph, end_host_map, \
        workload_type, total_time, \
        log_normal, kaggle_size, txn_size_mean, timeout_value, generate_json_also, circ_frac):
    num_nodes = inside_graph.number_of_nodes()
    start_nodes, end_nodes, amt_relative = [], [], []
    

    """ generate circulation and dag demand """
    circ_frac = round(circ_frac, 3)
    dag_frac = round(1 - circ_frac, 3)
    demand_dict_dag = dict()
    demand_dict_circ = dict()

    if circ_frac > 0:
        demand_dict_circ = circ_demand(list(inside_graph), mean=MEAN_RATE, \
            std_dev=CIRCULATION_STD_DEV)
    if dag_frac > 0: 
        demand_dict_dag = dag_demand(list(inside_graph), mean=MEAN_RATE, \
            std_dev=CIRCULATION_STD_DEV, skew_param=dag_frac*10, gen_method="src_skew")
    
    circ_total = reduce(lambda x, value: x + value, iter(demand_dict_circ.values()), 0)
    dag_total = reduce(lambda x, value: x + value, iter(demand_dict_dag.values()), 0)

    if "weird" not in filename or dag_frac == 0.20 or dag_frac == 0.45:
        demand_dict = { key: circ_frac * demand_dict_circ.get(key, 0) + dag_frac * demand_dict_dag.get(key, 0) \
            for key in set(demand_dict_circ) | set(demand_dict_dag) } 
    else:
        # just add dag and don't weigh
        demand_dict = { key: demand_dict_circ.get(key, 0) + dag_frac * demand_dict_dag.get(key, 0) \
            for key in set(demand_dict_circ) | set(demand_dict_dag) } 

    total = reduce(lambda x, value: x + value, iter(demand_dict.values()), 0)
    
    print("Circulation", circ_total)
    print("Dag", dag_total)
    print("total", total)
    print(circ_frac)
    print(dag_frac)


    '''
    pkl_op = open(filename + '_demand.pkl', 'wb')
    pickle.dump(demand_dict, pkl_op)
    pkl_op.close()
    '''
    

    if "two_node_imbalance" in filename:
        demand_dict = dict()
        demand_dict[0, 1] = MEAN_RATE
        demand_dict[1, 0] = 5 * MEAN_RATE
        print(demand_dict)
    elif "two_node_capacity" in filename:
        demand_dict = dict()
        demand_dict[0, 1] = 2 * MEAN_RATE
        demand_dict[1, 0] = 5 * MEAN_RATE
        print(demand_dict)

    if "three_node" in filename:
        demand_dict = dict()
        demand_dict[0, 2] = MEAN_RATE
        demand_dict[1, 2] = MEAN_RATE
        demand_dict[2, 1] = MEAN_RATE
        demand_dict[1, 0] = MEAN_RATE
        
    for i, j in list(demand_dict.keys()):
        start_nodes.append(end_host_map[i])
        end_nodes.append(end_host_map[j])
        amt_relative.append(demand_dict[i, j])	
    amt_absolute = [SCALE_AMOUNT * x for x in amt_relative]

    print("generated workload") 

    max_circ = max_circulation(demand_dict)
    if total != 0:
        print("ALERT!", "maximum circulation: ", max_circ, " or ", float(max_circ)/total)

    if generate_json_also:
        generate_json_files(filename, whole_graph, inside_graph, start_nodes, end_nodes, amt_absolute)


    if "weird" not in filename:
        print("generting txns here")
        write_txns_to_file(filename + '_workload.txt', start_nodes, end_nodes, amt_absolute,\
            workload_type, total_time, log_normal, kaggle_size, txn_size_mean, timeout_value)

    else:
        kaggle_size = False
        start_nodes_circ, end_nodes_circ, amt_relative_circ = [], [], []
        for i, j in list(demand_dict_circ.keys()):
            start_nodes_circ.append(end_host_map[i])
            end_nodes_circ.append(end_host_map[j])
            amt_relative_circ.append(demand_dict_circ[i, j])
        amt_absolute_circ = [SCALE_AMOUNT * x for x in amt_relative_circ]
        
        # circ for 1000s
        if dag_frac == 0.20 or dag_frac == 0.45 or dag_frac == 0.8:
            # dag plus circ for 2000s
            write_txns_to_file(filename + '_workload.txt', start_nodes, end_nodes, amt_absolute,\
                workload_type, 2000, log_normal, kaggle_size, txn_size_mean, timeout_value)

            # circ again for 1000s
            write_txns_to_file(filename + '_workload.txt', start_nodes_circ, end_nodes_circ, amt_absolute_circ,\
                workload_type, 1000, log_normal, kaggle_size, txn_size_mean, timeout_value, "a", 2000)

        else:
            write_txns_to_file(filename + '_workload.txt', start_nodes_circ, end_nodes_circ, amt_absolute_circ,\
            workload_type, 1000, log_normal, kaggle_size, txn_size_mean, timeout_value)

            # dag plus circ for 1000s
            write_txns_to_file(filename + '_workload.txt', start_nodes, end_nodes, amt_absolute,\
                workload_type, 1000, log_normal, kaggle_size, txn_size_mean, timeout_value, "a", 1000)

            # circ again for 1000s
            write_txns_to_file(filename + '_workload.txt', start_nodes_circ, end_nodes_circ, amt_absolute_circ,\
                workload_type, 1000, log_normal, kaggle_size, txn_size_mean, timeout_value, "a", 2000)


# parse a given line of edge relationships from the topology file
# and return whether this is a router node and its identifier
def parse_node(node_name):
    try:
        val = int(node_name[:-1])
        if node_name[-1] == 'r':
            return True, val
        if node_name[-1] == 'e':
            return False, val
        return -1
    except:
        return -1

# parse topology file to get graph structure
def parse_topo(topo_filename):
    g = nx.Graph()
    router_graph = nx.Graph()
    end_host_map = dict()

    line_num = 0
    with open(topo_filename) as topo_file:
        for line in topo_file:
            line_num += 1

            # landmark line
            if line_num == 1:
                continue

            if line == '\n':
                continue
            n1 = parse_node(line.split()[0])
            n2 = parse_node(line.split()[1])
            if n1 == -1 or n2 == -1:
                print("Bad line " + line)
                continue

            g.add_edge(n1[1], n2[1])

            if n1[0] and n2[0]: 
                router_graph.add_edge(n1[1], n2[1])
            elif n1[0]:
                end_host_map[n1[1]] = n2[1]
            elif n2[0]:
                end_host_map[n2[1]] = n1[1]
    
    return g, router_graph, end_host_map



# generate circulation demand for node ids mentioned in node_list,
# with average total demand at a node equal to 'mean', and a 
# perturbation of 'std_dev' 
def circ_demand(node_list, mean, std_dev):
    print("MEAN DEMAND", mean)

    assert type(mean) is int
    assert type(std_dev) is int

    demand_dict = {}
    num_nodes = len(node_list)

    """ sum of 'mean' number of random permutation matrices """
    """ note any permutation matrix is a circulation demand """
    """ matrix indices are shifted by number of nodes to account """
    for i in range(mean):
        perm = np.random.permutation(node_list)
        for j, k in enumerate(perm):
            if (j, k) in list(demand_dict.keys()):
                demand_dict[j, k] += 1
            else:
                demand_dict[j, k] = 1

    """ add 'std_dev' number of additional cycles to the demand """
    for i in range(std_dev):
        cycle_len = np.random.choice(list(range(1, num_nodes+1)))
        cycle = np.random.choice(node_list, cycle_len)
        cycle = set(cycle)
        cycle = list(cycle)
        cycle.append(cycle[0])
        for j in range(len(cycle[:-1])):
            if (cycle[j], cycle[j+1]) in list(demand_dict.keys()):
                demand_dict[cycle[j], cycle[j+1]] += 1
            else:
                demand_dict[cycle[j], cycle[j+1]] = 1			

    """ remove diagonal entries of demand matrix """
    for (i, j) in list(demand_dict.keys()):
        if i == j:
            del demand_dict[i, j]

    return demand_dict

# generate dag for node ids mentioned in node_list,
# with average total demand out of a node equal to 'mean', and a 
# perturbation of 'std_dev' 
def dag_demand(node_list, mean, std_dev, skew_param=5,gen_method="topological_sort"):
        print("DAG_DEMAND", mean)

        assert type(mean) is int
        assert type(std_dev) is int

        demand_dict = {}

        if gen_method == "src_skew":
            """ sample receiver uniformly at random and source from exponential distribution """
            for i in range(len(node_list) * mean):

                sender = len(node_list)
                while sender >= len(node_list):
                    sender = int(np.random.exponential(len(node_list)/skew_param))
                
                receiver_list = np.random.permutation(node_list)
                receiver_index = len(node_list)
                while receiver_index >= len(node_list):
                    receiver_index = int(np.random.exponential(len(node_list)/skew_param))
                receiver = receiver_list[receiver_index]

                demand_dict[sender, receiver] = demand_dict.get((sender, receiver), 0) + 1
        else:
            perm = np.random.permutation(node_list)
            print("root is ", perm[0])

            """ use a random ordering of the nodes """
            """ as the topological sort of the DAG demand to produce """
            """ generate demand from a node to only nodes higher """
            """ than it in the random ordering """
            for i, node in enumerate(perm[:-1]):
                receiver_node_list = perm[i + 1:]
                total_demand_from_node = mean + np.random.choice([std_dev, -1*std_dev])

                for j in range(total_demand_from_node):
                    receiver = np.random.choice(receiver_node_list)
                    demand_dict[node, receiver] = demand_dict.get((node, receiver), 0) + 1

        """ remove diagonal entries of demand matrix """
        for (i, j) in list(demand_dict.keys()):
                if i == j:
                        del demand_dict[i, j]

        return demand_dict


# parse arguments
parser = argparse.ArgumentParser(description="Create arbitrary txn workloads to run the omnet simulator on")
parser.add_argument('--graph-topo', \
        choices=['hotnets_topo', 'simple_line', 'simple_deadlock', 'custom', 'hardcoded_circ', 'toy_dctcp', 'dag_example', 'parallel_graph'],\
        help='type of graph (Small world or scale free or custom topology)', default='simple_line')
parser.add_argument('--payment-graph-dag-percentage', type=int,\
	help='percentage of circulation to put in the payment graph', default=0)
parser.add_argument('--topo-filename', dest='topo_filename', type=str, \
        help='name of topology file to generate worklooad for')
parser.add_argument('output_file_prefix', type=str, help='name of the output workload file', \
        default='simple_workload.txt')
parser.add_argument('interval_distribution', choices=['uniform', 'poisson','kaggle'],\
        help='time between transactions is determine by this', default='poisson')
parser.add_argument('--experiment-time', dest='total_time', type=int, \
        help='time to generate txns for', default=30)
parser.add_argument('--txn-size-mean', dest='txn_size_mean', type=int, \
        help='mean_txn_size', default=1)
parser.add_argument('--log-normal', action='store_true', help='should txns be exponential in size')
parser.add_argument('--kaggle-size', action='store_true', help='should txns be kaggle in size')
parser.add_argument('--generate-json-also', action="store_true", help="do you need to generate json file also \
        for the custom topology")
parser.add_argument('--balance-list', type=int, nargs='+', dest='balance_list', default=[100])
parser.add_argument('--timeout-value', type=float, help='generic time out for all transactions', default=5)
parser.add_argument('--scale-amount', type=int, help='how much to scale the mean deamnd by', default=5)
parser.add_argument('--run-num', type=int, help='influences the seed', default=1)


args = parser.parse_args()

output_prefix = args.output_file_prefix
circ_frac = (100 - args.payment_graph_dag_percentage) / 100.0
distribution = args.interval_distribution
total_time = args.total_time
txn_size_mean = args.txn_size_mean
log_normal = args.log_normal
kaggle_size = args.kaggle_size
topo_filename = args.topo_filename
generate_json_also = args.generate_json_also
graph_topo = args.graph_topo
balance_list = args.balance_list
timeout_value = args.timeout_value
SCALE_AMOUNT = args.scale_amount

if kaggle_size:
    log_normal = False


# generate workloads
np.random.seed(SEED_LIST[args.run_num])
random.seed(SEED_LIST[args.run_num])
if graph_topo != 'custom':
    generate_workload_standard(output_prefix, graph_topo, distribution, \
            total_time, log_normal, kaggle_size, txn_size_mean, timeout_value, generate_json_also, circ_frac)
elif topo_filename is None:
    raise Exception("Topology needed for custom file")
else:
    whole_graph, inside_graph, end_host_map = parse_topo(topo_filename)
    generate_workload_for_provided_topology(output_prefix, inside_graph, whole_graph, end_host_map,\
            distribution, total_time, log_normal, kaggle_size,\
            txn_size_mean, timeout_value, generate_json_also, circ_frac)


# File Path: \\wsl.localhost\Ubuntu-20.04\home\ad\omnetpp-5.6\samples\spider_omnet-Copy\scripts\gather_sw_sf_data.sh

for topo in "sw_50_routers" "sf_50_routers"
    # path data
    python parse_final_summary_stats.py --topo ${topo}_lndCap --payment-graph-type circ --credit-list 3200 \
        --demand 30 --scheme-list DCTCPQ --save ${topo}_lnd_credit3200_pathtypes_data --num-max 4 \
        --path-type-list oblivious kspYen widest
    
    python parse_final_summary_stats.py --topo ${topo}_lndCap --payment-graph-type circ --credit-list 3200 \
        --demand 30 --scheme-list DCTCPQ --save ${topo}_lnd_credit3200_pathnum_data --num-max 4 \
        --path-num-list 1 2 8

    for credit in "uniform" "gaussian" "lnd"
        # size succ stats
        suffix=""
        amt=800
        credit1=100
        credit2=200
        
        if [ $credit == "lnd" ]; then
            suffix="_lndCap"
            amt=3200
            credit1=6400
            credit2=12800
        elif [ $credit == "gaussian" ]; then
            suffix="_randomCap"
        fi

        # summary data
        python parse_final_summary_stats.py --topo ${topo}${suffix} --payment-graph-type circ \
            --credit-list $credit1 $credit2 400 800 1600 3200 --demand 30 \
            --scheme-list priceSchemeWindow waterfilling lndBaseline DCTCPQ landmarkRouting \
            shortestPath \
            --save ${topo}_${credit}_credit_data --num-max 4


        python parse_probability_size_stats.py --topo ${topo}${suffix} --payment-graph-type circ \
            --credit ${amt} --demand 30 --scheme-list waterfilling lndBaseline DCTCPQ landmarkRouting \
            shortestPath priceSchemeWindow --save ${topo}_${credit}_credit${amt}_prob_data --num-max 4 \
            --num-buckets 8

# File Path: \\wsl.localhost\Ubuntu-20.04\home\ad\omnetpp-5.6\samples\spider_omnet-Copy\scripts\generate_analysis_plots_for_single_run.py

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import argparse
import os
import json
import numpy as np
from parse_vec_files import *
from parse_sca_files import *
from matplotlib.backends.backend_pdf import PdfPages
from cycler import cycler
from config import *
from itertools import cycle
import collections

parser = argparse.ArgumentParser('Analysis Plots')
parser.add_argument('--detail',
        type=str, 
        help='whether to just summarize or plot individual things', default='True')
parser.add_argument('--vec_file',
        type=str,
        required=True,
        help='Single vector file for a particular run using the omnet simulator')
parser.add_argument('--sca_file',
        type=str,
        required=True,
        help='Single scalar file for a particular run using the omnet simulator')
parser.add_argument('--balance',
        action='store_true',
        help='Plot balance information for all routers')
parser.add_argument('--time_inflight',
        action='store_true',
        help='Plot information on time spent in flight for all routers\' channels ')
parser.add_argument('--inflight',
        action='store_true',
        help='Plot inflight funds information for all routers (need to be used with --balance to work)')
parser.add_argument('--num_sent_per_channel',
        action='store_true',
        help='Plot number sent per channel for all routers')
parser.add_argument('--queue_info',
        action='store_true',
        help='Plot queue information for all routers')
parser.add_argument('--timeouts',
        action='store_true',
        help='Plot timeout information for all source destination pairs')
parser.add_argument('--timeouts_sender',
        action='store_true',
        help='Plot number of timeouts at the sender for all source destination pairs')
parser.add_argument('--frac_completed',
        action='store_true',
        help='Plot fraction completed for all source destination pairs')
parser.add_argument('--frac_completed_window',
        action='store_true',
        help='Plot fraction completed for all source destination pairs in every window')
parser.add_argument('--path',
        action='store_true',
        help='Plot path index used for all source destination pairs')
parser.add_argument('--waiting',
        action='store_true',
        help='Plot number of waiting txns at a the sender at a point in time all source destination pairs')
parser.add_argument('--probabilities',
        action='store_true',
        help='Plot probabilities of picking each path at a given point in time all source destination pairs')
parser.add_argument('--bottlenecks',
        action='store_true',
        help='Plot bottlenecks on different paths at a given point in time all source destination pairs')
parser.add_argument('--lambda_val',
        action='store_true',
        help='Plot the per channel capacity related price when price based scheme is used')
parser.add_argument('--mu_local',
        action='store_true',
        help='Plot the per imbalance related price when price based scheme is used')
parser.add_argument('--mu_remote',
        action='store_true',
        help='Plot the imbalance related price at the remote end')
parser.add_argument('--x_local',
        action='store_true',
        help='Plot the per channel rate of sending related price when price based scheme is used')
parser.add_argument('--n_local',
        action='store_true',
        help='Plot the per channel number of txns when price based scheme is used')
parser.add_argument('--service_arrival_ratio',
        action='store_true',
        help='Plot the per channel  service rate when price based scheme is used')
parser.add_argument('--inflight_outgoing',
        action='store_true',
        help='Plot the per channel number of outgoing txns price when price based scheme is used')
parser.add_argument('--inflight_incoming',
        action='store_true',
        help='Plot the per channel number of incoming txns price when price based scheme is used')
parser.add_argument('--rate_to_send',
        action='store_true',
        help='Plot the per path rate to send when price based scheme is used')
parser.add_argument('--rate_sent',
        action='store_true',
        help='Plot the per path rate actually sent when price based scheme is used')
parser.add_argument('--rate_acked',
        action='store_true',
        help='Plot the per path rate at which acks are received')
parser.add_argument('--measured_rtt',
        action='store_true',
        help='Plot the per path rtt')
parser.add_argument('--fraction_marked',
        action='store_true',
        help='Plot the per path marked acks out of all acks received')
parser.add_argument('--amt_inflight_per_path',
        action='store_true',
        help='Plot the per path amt inflight when price based scheme is used')
parser.add_argument('--price',
        action='store_true',
        help='Plot the per channel price to send when price based scheme is used')
parser.add_argument('--demand',
        action='store_true',
        help='Plot the per dest estimated demand when price based scheme is used')
parser.add_argument('--numCompleted',
        action='store_true',
        help='Plot the per dest completion rate')
parser.add_argument('--queue_delay',
        action='store_true',
        help='Plot the perchannel queueing delay')
parser.add_argument('--fake_rebalance_queue',
        action='store_true',
        help='Plot the perchannel fake queue delay')
parser.add_argument('--rebalancing-amt',
        action='store_true',
        help='Plot the implicit and explicit rebalancing amt')
parser.add_argument('--capacity',
        action='store_true',
        help='Plot the capacity of payment channels')
parser.add_argument('--bank',
        action='store_true',
        help='Plot the bank')
parser.add_argument('--cpi',
        action='store_true',
        help='Plot the cpi weights per channel per destination')
parser.add_argument('--perDestQueue',
        action='store_true',
        help='Plot the size of the per dest queue at every intermediary celer router')
parser.add_argument('--queueTimedOut',
        action='store_true',
        help='Plot the number timed out per destination queue at every router/host')
parser.add_argument('--kStar',
        action='store_true',
        help='Plot celer kstar destination on every payment channel')

parser.add_argument('--save',
        type=str,
        required=True,
        help='The pdf file prefix to which to write the figures')

args = parser.parse_args()
num_paths = []
ggplot = open(args.save + "_ggplot.txt", "w+")
tag = "balance" if "imbalance" in args.save else "capacity"
#fmts = ['r--', 'b-', 'g-.']

# returns a dictionary of the necessary stats where key is a router node
# and value is another dictionary where the key is the partner node 
# and value is the time series of the signal_type recorded for this pair of nodes
def aggregate_info_per_node(all_timeseries, vec_id_to_info_map, signal_type, is_router, aggregate_per_path=False, is_both=False, aggregate_per_dest=False):
    node_signal_info = dict()
   #all_timeseries, vec_id_to_info_map = parse_vec_file(filename, signal_type)
    
    #for key, value in vec_id_to_info_map.items():
    #print "key: ", key, "; value: ", value

    # aggregate information on a per router node basis
    # and then on a per channel basis for that router node
    for vec_id, timeseries in list(all_timeseries.items()):
        #print "vec_id: ", vec_id, "; timeseries: ", timeseries
        vector_details = vec_id_to_info_map[vec_id]
        src_node = vector_details[0]
        src_node_type = vector_details[1]
        dest_node_type = vector_details[4]
        dest_node = vector_details[3]
                
        if is_both:
            if src_node_type == "host":
                src_node = -1 * src_node if src_node > 0 else 10000
            elif dest_node_type == "host":
                dest_node = -1 * dest_node if dest_node > 0 else 10000
        else:
            if is_router and (src_node_type != "router" or dest_node_type != "router"):
                continue

            if not is_router and (src_node_type != "host" or dest_node_type != "host"):
                continue
            '''elif not is_router:
                src_node = src_node + 2
                dest_node = dest_node + 2'''

        signal_name = vector_details[2]
        if signal_type not in signal_name:
            continue

        signal_values =  timeseries

        '''if signal_type == "numInQueue":
            ggplot_file = open("ggplot_op", "a+")
            time = [t[0] for t in timeseries]
            value = [t[1] for t in timeseries]
            if src_node == 0:
                queue_id = 1
            elif src_node == 2:
                queue_id = 4
            elif dest_node == 0:
                queue_id = 2
            else:
                queue_id = 3
            for t, v in zip(time, value):
                if t > 295 and t < 310:
                    ggplot_file.write("Q" + str(queue_id) + "," + str(t - 295) + "," + str(v) + "\n")
            ggplot_file.close()'''

        if aggregate_per_path:
            path_id = int(signal_name.split("_")[1])
            cur_info = node_signal_info.get(src_node, dict())
            dest_info = cur_info.get(dest_node, dict())
            dest_info[path_id] = signal_values
            cur_info[dest_node] = dest_info
            node_signal_info[src_node] = cur_info
        elif aggregate_per_dest:
            # the stats so far have been separated per channel and the udnerscore denotes the dest
            channel_node = dest_node
            channel_node_type = dest_node_type
            channel_node_key = (channel_node, channel_node*-1)[channel_node_type == "host" and \
                    channel_node != 10000 and not is_both]
            
            dest_node = int(signal_name.split("_")[1])
            cur_info = node_signal_info.get(src_node, dict())
            channel_info = cur_info.get(channel_node_key, dict())
            channel_info[dest_node] = timeseries
            cur_info[channel_node_key] = channel_info
            node_signal_info[src_node] = cur_info
        else: 
            cur_info = node_signal_info.get(src_node, dict())
            cur_info[dest_node] = signal_values
            node_signal_info[src_node] = cur_info
        
    return node_signal_info

# use the balance information to get amount inlfight on every channel
def aggregate_inflight_info(bal_timeseries):
    node_signal_info = dict()

    for router, channel_info in list(bal_timeseries.items()):
        inflight_info = dict()
        for partner, router_partner_TS in list(channel_info.items()):
            if router < partner:
                inflight_TS = []
                partner_router_TS = bal_timeseries[partner][router]

                for i, (time, forward_bal) in enumerate(router_partner_TS):
                    backward_bal = partner_router_TS[i][1]
                    inflight_TS.append((time, ROUTER_CAPACITY - forward_bal - backward_bal))
                
                inflight_info[partner] = inflight_TS
        node_signal_info[router] = inflight_info
    return node_signal_info


# use the successful and attempted information to get fraction of successful txns in every interval
def aggregate_frac_successful_info(success, attempted):
    node_signal_info = dict()

    for router, channel_info in list(success.items()):
        frac_successful = dict()
        for partner, success_TS in list(channel_info.items()):
            frac_succ_TS = []
            attempt_TS = attempted[router][partner]

            for i, (time, num_succeeded) in enumerate(success_TS):
                num_attempted = attempt_TS[i][1]
                if num_attempted == 0:
                    frac_succ_TS.append((time, 0))
                else:
                    frac_succ_TS.append((time, num_succeeded/float(num_attempted)))
                frac_successful[partner] = frac_succ_TS
        node_signal_info[router] = frac_successful
    
    return node_signal_info


# plot time series of tpt
def plot_tpt_timeseries(num_completed, num_arrived, pdf):
    total_arrived, total_completed  = dict(), dict()
    for src, dest_info in list(num_arrived.items()):
        for dest, info in list(dest_info.items()):
            for i, (time, value) in enumerate(info[1:-1]):
                cur_completed = total_completed.get(time, 0)
                cur_arrived = total_arrived.get(time, 0)
                total_completed[time] = cur_completed + num_completed[src][dest][i + 1][1]
                total_arrived[time] = cur_arrived + value 


    times = sorted(total_completed.keys()) 
    tpts = []
    for time in times:
        if total_arrived[time] < total_completed[time]:
            total_completed[time] = total_arrived[time]
        tpt = float(100*total_completed[time])/max(total_arrived[time],1)
        tpts.append(tpt)

    plt.figure()
    plt.plot(times, tpts)
    plt.xlabel("Time")
    plt.ylim(0,100)
    plt.ylabel("Throughput %")
    plt.grid()
    pdf.savefig()  # saves the current figure into a pdf page
    plt.close()

    tpt_file = open(args.save + "_tpttimeseries.txt", "w+")
    tpt_file.write("topo,time,tpt\n")
    for time, tpt in zip(times, tpts):
        tpt_file.write("sw," + str(time) + "," + str(tpt) + "\n")
    tpt_file.close()
           




# plots every router's signal_type info in a new pdf page
# and add a router wealth plot separately
def plot_relevant_stats(data, pdf, signal_type, compute_router_wealth=False, per_path_info=False, window_info=None):
    color_opts = ['#fa9e9e', '#a4e0f9', '#57a882', '#ad62aa']
    router_wealth_info =[]
    sum_vals = 0
    
    for router, channel_info in list(data.items()):
        channel_bal_timeseries = []
        plt.figure()
        plt.rc('axes', prop_cycle = (cycler('color', ['r', 'g', 'b', 'y', 'c', 'm', 'y', 'k']) +
            cycler('linestyle', ['-']*8)))
                #, '--', ':', '-.', '-', '--', ':', '-.'])))

        
        i = 0
        for channel, info in list(channel_info.items()):
            # making labels and titles sensible if there are both end host and router data
            if router < 0:
                router_name = "e" + str(-1 * router)
            elif router == 10000:
                router_name = "e0"
            else:
                router_name = str(router)
            if channel < 0:
                channel_name = "e" + str(-1*channel)
            elif channel == 10000:
                channel_name = "e0"
            else:
                channel_name = str(channel)

            # plot one plot per src dest pair and multiple lines per path
            if per_path_info:
                color_cycle_for_path = cycle(['r', 'g', 'b', 'y', 'c', 'm', 'y', 'k'])
                if signal_type == "Price per path":
                    num_paths.append(len(list(info.items())))
                sum_values, num_values = 0.0, 0.0
                for path, path_signals in list(info.items()):
                    time = [t[0] for t in path_signals]
                    values = [t[1] for t in path_signals]
                    
                    c = next(color_cycle_for_path)

                    if window_info is not None:
                        window_signals = window_info[router][channel][path]
                        window_val = [t[1] for t in window_signals]
                        start = int(len(window_val)/4)
                        label_name = str(path) + "_Window" + "(" + str(np.average(window_val[start:])) + ")"
                        plt.plot(time, window_val, label=label_name, linestyle="--", color=c)
                    
                    label_name = str(path)
                    start = int(len(values)/4)
                    plt.plot(time, values, \
                            label=label_name + "(" + str(np.average(values[start:])) + ")", color=c)
                    
                    if (signal_type == "Rate acknowledged"):
                        for t, v in zip(time, values):
                            if router == 0 and path == 0:
                                router_not = "endhost a"
                            elif router == 1 and path == 1:
                                router_not = "endhost b"
                            elif router == 2 and path == 0:
                                router_not = "endhost c"
                            else:
                                break
                            ggplot.write(tag + "," + str(router_not) + ",rate," + str(t) + "," + str(v) + "\n")


                    sum_values += np.average(values[start:])
                    num_values += 1
                plt.title(signal_type + " " + str(router_name) + "->" + str(channel_name) + \
                        "(" + str(sum_values/num_values) + ")")
                plt.xlabel("Time")
                plt.ylabel(signal_type)
                plt.legend()
                plt.grid()
                pdf.savefig()  # saves the current figure into a pdf page
                plt.close()

            # one plot per src with multiple lines per dest
            else:
                time = [t[0] for t in info]
                values = [t[1] for t in info]
                label_name = router_name + "->" + channel_name

                if compute_router_wealth:
                    channel_bal_timeseries.append((time, values))

                if signal_type == "Balance" and "toy" in args.save:
                    if (router == 0 and channel != 1) or (router == 1 and channel != 0):
                        continue


                start = int(len(values)/4)
                plt.plot(time, values, label=label_name + \
                        "(" + str(np.average(values[start:])) + ")")

                if ("Explicit" in signal_type):
                    sum_vals += abs(np.average(values[start:]))

                if (signal_type == "Queue Size"):
                    for t, v in zip(time, values):
                        if router == 0:
                            router_not = "router a" 
                        elif router == 2:
                            router_not = "router c"
                        else:
                            break
                        ggplot.write(tag + "," + str(router_not) + ",queue," + str(t) + "," + str(v) + "\n")

                
                i += 1

        # aggregate info for all router's wealth
        if compute_router_wealth:
            router_wealth = []
            for i, time in enumerate(channel_bal_timeseries[0][0]):
                wealth = 0
                for j in channel_bal_timeseries:
                    wealth += j[1][i]
                router_wealth.append(wealth)
            router_wealth_info.append((router,channel_bal_timeseries[0][0], router_wealth))
        
        # close plot after every router unless it is per path information
        if not per_path_info:
            plt.title(signal_type + " for Router " + str(router_name))
            plt.xlabel("Time")
            plt.ylabel(signal_type)
            plt.legend()
            plt.grid()
            pdf.savefig()  # saves the current figure into a pdf page
            plt.close()

    # one giant plot for all router's wealth
    if compute_router_wealth:
        for (r, time, wealth) in router_wealth_info:
            plt.plot(time, wealth, label=str(r))
        plt.title("Router Wealth Timeseries")
        plt.xlabel("Time")
        plt.ylabel("Router Wealth")
        plt.legend()
        plt.grid()
        pdf.savefig()  # saves the current figure into a pdf page
        plt.close()


    if ("Explicit" in signal_type):
        print("explicit sum", sum_vals)


# see if infligh plus balance was ever more than capacity
def find_problem(balance_timeseries, inflight_timeseries) :
    for router, channel_info in list(balance_timeseries.items()):
            for channel, info in list(channel_info.items()):
                for i, (time, value) in enumerate(info):
                    my_balance = value
                    try:
                        remote_balance = balance_timeseries[channel][router][i][1]
                        inflight = inflight_timeseries[channel][router][i][1]
                        # num inflight might be a little inconsistent depending on clear state
                        # but others should tally up
                        if my_balance + remote_balance > ROUTER_CAPACITY:
                            print(" problem at router", router, "with ", channel,   " at time " , time)
                            print(my_balance, remote_balance, inflight)
                    except:
                        print(channel, router, i)
        


# plot per router channel information on a per router basis depending on the type of signal wanted
def plot_per_payment_channel_stats(args, text_to_add):
    color_opts = ['#fa9e9e', '#a4e0f9', '#57a882', '#ad62aa']
    dims = plt.rcParams["figure.figsize"]
    plt.rcParams["figure.figsize"] = dims
    data_to_plot = dict()

    with PdfPages(args.save + "_per_channel_info.pdf") as pdf:
        all_timeseries, vec_id_to_info_map, parameters = parse_vec_file(args.vec_file, "per_channel_plot")
        firstPage = plt.figure()
        firstPage.clf()
        txt = 'Parameters:\n' + parameters
        txt += str(text_to_add[0]) + "\n"
        txt += "Completion over arrival " + str(text_to_add[1]) + "\n"
        txt += "Completion over attempted " + str(text_to_add[2]) + "\n"

        firstPage.text(0.5, 0, txt, transform=firstPage.transFigure, ha="center")
        pdf.savefig()
        plt.close()

        isboth = args.rebalancing_amt

        if args.balance: 
            data_to_plot = aggregate_info_per_node(all_timeseries, vec_id_to_info_map, "balance", True, is_both=isboth)
            plot_relevant_stats(data_to_plot, pdf, "Balance", compute_router_wealth=True)

            inflight = aggregate_info_per_node(all_timeseries, vec_id_to_info_map, "numInflight", True,is_both=isboth)
            plot_relevant_stats(inflight, pdf, "Inflight funds")
            #find_problem(data_to_plot, inflight)

            '''if args.inflight:
                inflight_data_to_plot = aggregate_inflight_info(data_to_plot)
                plot_relevant_stats(inflight_data_to_plot, pdf, "Inflight Funds")'''

        if args.time_inflight:
            data_to_plot = aggregate_info_per_node(all_timeseries, vec_id_to_info_map, "timeInFlight", True)
            plot_relevant_stats(data_to_plot, pdf, "Time spent in flight per channel(s)")
            
        if args.queue_info:
            data_to_plot = aggregate_info_per_node(all_timeseries, vec_id_to_info_map, "numInQueue", True, is_both=True)
            plot_relevant_stats(data_to_plot, pdf, "Queue Size")

        if args.queue_delay:
            data_to_plot = aggregate_info_per_node(all_timeseries, vec_id_to_info_map, "queueDelayEWMA", True)
            plot_relevant_stats(data_to_plot, pdf, "Delay through queue")

        if args.fake_rebalance_queue:
            data_to_plot = aggregate_info_per_node(all_timeseries, vec_id_to_info_map, "fakeRebalanceQ", True)
            plot_relevant_stats(data_to_plot, pdf, "fake queue size")

        if args.num_sent_per_channel:
            data_to_plot = aggregate_info_per_node(all_timeseries, vec_id_to_info_map, "numSent", True)
            plot_relevant_stats(data_to_plot, pdf, "Number Sent")

        if args.lambda_val:
            data_to_plot = aggregate_info_per_node(all_timeseries, vec_id_to_info_map, "lambda", True, is_both=False)
            plot_relevant_stats(data_to_plot, pdf, "Lambda")
        
        if args.mu_local:
            data_to_plot = aggregate_info_per_node(all_timeseries, vec_id_to_info_map, "muLocal", True, is_both=False)
            plot_relevant_stats(data_to_plot, pdf, "Mu Local")
        
        if args.mu_remote:
            data_to_plot = aggregate_info_per_node(all_timeseries, vec_id_to_info_map, "muRemote", True, is_both=False)
            plot_relevant_stats(data_to_plot, pdf, "Mu Remote")
        
        if args.x_local:
            data_to_plot = aggregate_info_per_node(all_timeseries, vec_id_to_info_map, "xLocal", True, is_both=False)
            plot_relevant_stats(data_to_plot, pdf, "xLocal")
        
        if args.n_local:
            data_to_plot = aggregate_info_per_node(all_timeseries, vec_id_to_info_map, "nValue", True, is_both=False)
            plot_relevant_stats(data_to_plot, pdf, "nValue")

        if args.service_arrival_ratio:
            data_to_plot = aggregate_info_per_node(all_timeseries, vec_id_to_info_map, "serviceRate", True, is_both=False)
            plot_relevant_stats(data_to_plot, pdf, "service arrival ratio")


        if args.inflight_outgoing:
            data_to_plot = aggregate_info_per_node(all_timeseries, vec_id_to_info_map, "inflightOutgoing", True, is_both=False)
            plot_relevant_stats(data_to_plot, pdf, "inflight outgoing on channel")


        if args.inflight_incoming:
            data_to_plot = aggregate_info_per_node(all_timeseries, vec_id_to_info_map, "inflightIncoming", True, is_both=False)
            plot_relevant_stats(data_to_plot, pdf, "inflight incoming on channel")

        if args.rebalancing_amt:
            data_to_plot = aggregate_info_per_node(all_timeseries, vec_id_to_info_map, "implicitRebalancingAmt", \
                    True, is_both=True)
            plot_relevant_stats(data_to_plot, pdf, "Implicit amount rebalanced")

            data_to_plot = aggregate_info_per_node(all_timeseries, vec_id_to_info_map, "explicitRebalancingAmt", \
                    True, is_both=True)
            plot_relevant_stats(data_to_plot, pdf, "Explicit amount rebalanced")

        if args.capacity:
            data_to_plot = aggregate_info_per_node(all_timeseries, vec_id_to_info_map, "capacity", \
                    True, is_both=True)
            plot_relevant_stats(data_to_plot, pdf, "Capacity")

        if args.bank:
            data_to_plot = aggregate_info_per_node(all_timeseries, vec_id_to_info_map, "bank", \
                    True, is_both=True)
            plot_relevant_stats(data_to_plot, pdf, "Bank")

        if args.kStar:
            data_to_plot = aggregate_info_per_node(all_timeseries, vec_id_to_info_map, "kStar", \
                    True, is_both=True)
            plot_relevant_stats(data_to_plot, pdf, "kstar")

    #print("http://" + EC2_INSTANCE_ADDRESS + ":" + str(PORT_NUMBER) + "/scripts/figures/" + \
    #        os.path.basename(args.save) + "_per_channel_info.pdf")
    


# plot per router channel information on a per router basis depending on the type of signal wanted
def plot_per_src_dest_stats(args, text_to_add):
    color_opts = ['#fa9e9e', '#a4e0f9', '#57a882', '#ad62aa']
    dims = plt.rcParams["figure.figsize"]
    plt.rcParams["figure.figsize"] = dims
    data_to_plot = dict()

    with PdfPages(args.save + "_per_src_dest_stats.pdf") as pdf:
        all_timeseries, vec_id_to_info_map, parameters = parse_vec_file(args.vec_file, "per_src_dest_plot")
 
        firstPage = plt.figure()
        firstPage.clf()
        txt = 'Parameters:\n' + parameters
        txt += text_to_add[0] + "\n"
        txt += "Completion over arrival " + str(text_to_add[1]) + "\n"
        txt += "Completion over attempted " + str(text_to_add[2]) + "\n"

        firstPage.text(0.5, 0, txt, transform=firstPage.transFigure, ha="center")
        pdf.savefig()
        plt.close()
         
        num_completed = aggregate_info_per_node(all_timeseries, vec_id_to_info_map, "numCompleted", False)
        num_arrived = aggregate_info_per_node(all_timeseries, vec_id_to_info_map, "numArrived", False)
        plot_tpt_timeseries(num_completed, num_arrived, pdf)   
        
        if args.timeouts: 
            data_to_plot = aggregate_info_per_node(all_timeseries, vec_id_to_info_map,  "numTimedOutPerDest", False)
            plot_relevant_stats(data_to_plot, pdf, "Number of Transactions Timed Out")
        
        if args.frac_completed_window: 
            successful = aggregate_info_per_node(all_timeseries, vec_id_to_info_map, "rateCompleted", False)
            attempted = aggregate_info_per_node(all_timeseries, vec_id_to_info_map, "rateArrived", False)
            data_to_plot = aggregate_frac_successful_info(successful, attempted)
            plot_relevant_stats(data_to_plot, pdf, "Fraction of successful txns in each window")

        if args.frac_completed:
            data_to_plot = aggregate_info_per_node(all_timeseries, vec_id_to_info_map, "fracSuccessful", False)
            plot_relevant_stats(data_to_plot, pdf, "Fraction of successful txns")

        if args.path:
            data_to_plot = aggregate_info_per_node(all_timeseries, vec_id_to_info_map, "pathPerTrans", False)
            plot_relevant_stats(data_to_plot, pdf, "Path Per Transaction")

        if args.timeouts_sender:
            data_to_plot = aggregate_info_per_node(all_timeseries, vec_id_to_info_map, "numTimedOutAtSender", False)
            plot_relevant_stats(data_to_plot, pdf, "Number of Transactions Timed Out At Sender")

        if args.waiting:
            data_to_plot = aggregate_info_per_node(all_timeseries, vec_id_to_info_map, "numWaiting", False)
            plot_relevant_stats(data_to_plot, pdf, "Number of Transactions Waiting at sender To Given Destination")

        if args.probabilities:
            data_to_plot = aggregate_info_per_node(all_timeseries, vec_id_to_info_map, "probability", False, True)
            plot_relevant_stats(data_to_plot, pdf, "Probability of picking paths", per_path_info=True)

        if args.bottlenecks:
            data_to_plot = aggregate_info_per_node(all_timeseries, vec_id_to_info_map, "bottleneck", False, True)
            plot_relevant_stats(data_to_plot, pdf, "Bottleneck Balance", per_path_info=True)

        if args.rate_to_send:
            data_to_plot = aggregate_info_per_node(all_timeseries, vec_id_to_info_map, "rateToSendTrans", False, True)
            plot_relevant_stats(data_to_plot, pdf, "Rate to send per path", per_path_info=True)

        if args.rate_sent:
            data_to_plot = aggregate_info_per_node(all_timeseries, vec_id_to_info_map, "rateSent", False, True)
            plot_relevant_stats(data_to_plot, pdf, "Rate actually sent per path", per_path_info=True)
        
        if args.rate_acked:
            data_to_plot = aggregate_info_per_node(all_timeseries, vec_id_to_info_map, "rateOfAcks", False, True)
            plot_relevant_stats(data_to_plot, pdf, "Rate acknowledged", per_path_info=True)    

        if args.fraction_marked:
            data_to_plot = aggregate_info_per_node(all_timeseries, vec_id_to_info_map, "fractionMarked", False, True)
            plot_relevant_stats(data_to_plot, pdf, "Fraction of packets marked", per_path_info=True)  
            
            data_to_plot = aggregate_info_per_node(all_timeseries, vec_id_to_info_map, "smoothedFractionMarked", \
                    False, True)
            plot_relevant_stats(data_to_plot, pdf, "Fraction of packets marked in interval", per_path_info=True)  
 
        if args.measured_rtt:
            data_to_plot = aggregate_info_per_node(all_timeseries, vec_id_to_info_map, "measuredRTT", False, True)
            plot_relevant_stats(data_to_plot, pdf, "Observed RTT", per_path_info=True)  
        
        if args.amt_inflight_per_path:
            data_to_plot = aggregate_info_per_node(all_timeseries, vec_id_to_info_map, \
                    "sumOfTransUnitsInFlight", False, True)
            window_info = aggregate_info_per_node(all_timeseries, vec_id_to_info_map, \
                    "window", False, True)
            plot_relevant_stats(data_to_plot, pdf, "Amount Inflight/Window per path", per_path_info=True, \
                    window_info=window_info)
        
        if args.price:
            data_to_plot = aggregate_info_per_node(all_timeseries, vec_id_to_info_map, "priceLastSeen", False, True)
            plot_relevant_stats(data_to_plot, pdf, "Price per path", per_path_info=True)

        if args.demand:
            data_to_plot = aggregate_info_per_node(all_timeseries, vec_id_to_info_map, "demandEstimate", False)
            plot_relevant_stats(data_to_plot, pdf, "Demand Estimate per Path")

        if args.numCompleted:
            data_to_plot = aggregate_info_per_node(all_timeseries, vec_id_to_info_map, "rateCompleted", False)
            plot_relevant_stats(data_to_plot, pdf, "number of txns completed")

        if args.perDestQueue:
            data_to_plot = aggregate_info_per_node(all_timeseries, vec_id_to_info_map, "destQueue", \
                    True, is_both=True)
            plot_relevant_stats(data_to_plot, pdf, "Per destination queue sizes")


        if args.queueTimedOut:
            data_to_plot = aggregate_info_per_node(all_timeseries, vec_id_to_info_map, "queueTimedOut", \
                    True, is_both=True)
            plot_relevant_stats(data_to_plot, pdf, "Per destination timed out in queue")



    #print("http://" + EC2_INSTANCE_ADDRESS + ":" + str(PORT_NUMBER) + "/scripts/figures/" + \
    #        os.path.basename(args.save) + "_per_src_dest_stats.pdf")


# plot per router channel information on a per destination basis depending on the type of signal wanted
# mostly applicable to celer
def plot_per_channel_dest_stats(args, text_to_add):
    color_opts = ['#fa9e9e', '#a4e0f9', '#57a882', '#ad62aa']
    dims = plt.rcParams["figure.figsize"]
    plt.rcParams["figure.figsize"] = dims
    data_to_plot = dict()

    with PdfPages(args.save + "_per_channel_dest_stats.pdf") as pdf:
        all_timeseries, vec_id_to_info_map, parameters = parse_vec_file(args.vec_file, "per_channel_dest_plot")
 
        firstPage = plt.figure()
        firstPage.clf()
        txt = 'Parameters:\n' + parameters
        txt += text_to_add[0] + "\n"
        txt += "Completion over arrival " + str(text_to_add[1]) + "\n"
        txt += "Completion over attempted " + str(text_to_add[2]) + "\n"

        firstPage.text(0.5, 0, txt, transform=firstPage.transFigure, ha="center")
        pdf.savefig()
        plt.close()
         
        if args.cpi: 
            data_to_plot = aggregate_info_per_node(all_timeseries, vec_id_to_info_map,  "cpi", True, False,
                    is_both=True, aggregate_per_dest=True)
            plot_relevant_stats(data_to_plot, pdf, "cpi", per_path_info = True)

    #print("http://" + EC2_INSTANCE_ADDRESS + ":" + str(PORT_NUMBER) + "/scripts/figures/" + \
    #        os.path.basename(args.save) + "_per_channel_dest_stats.pdf")


def main():
    '''matplotlib.rcParams['figure.figsize'] = [15, 10]
    plt.rc('font', size=40)          # controls default text sizes
    plt.rc('axes', titlesize=42)     # fontsize of the axes title
    plt.rc('axes', labelsize=40)    # fontsize of the x and y labels
    plt.rc('xtick', labelsize=32)    # fontsize of the tick labels
    plt.rc('ytick', labelsize=32)    # fontsize of the tick labels
    plt.rc('legend', fontsize=34)    # legend fontsize'''
    summary_stats = parse_sca_files_overall(args.sca_file) 
    f = open(args.save + "_summary.txt", "w+")
    f.write(summary_stats[2])
    f.close()

    if args.detail == 'true':
        text_to_add = parse_sca_files(args.sca_file)
        plot_per_payment_channel_stats(args, text_to_add)
        plot_per_src_dest_stats(args, text_to_add)
        plot_per_channel_dest_stats(args, text_to_add)
        path_dist = collections.Counter(num_paths)
        print(path_dist)
    ggplot.close()


main()



# File Path: \\wsl.localhost\Ubuntu-20.04\home\ad\omnetpp-5.6\samples\spider_omnet-Copy\scripts\generate_circulation_benchmarks.sh

#!/bin/bash
PATH_NAME="/home/ubuntu/omnetpp-5.4.1/samples/spider_omnet/benchmarks/circulations/"
GRAPH_PATH="/home/ubuntu/omnetpp-5.4.1/samples/spider_omnet/scripts/figures/"

num_nodes=("2" "2" "3" "4" "5" "5" "5" "0" "0" "10" "20" "50" "60" "80" "100" "200" "400" "600" "800" "1000" \
    "10" "20" "50" "60" "80" "100" "200" "400" "600" "800" "1000" "40" "10" "20" "30" "40" "0" "0" "0" "4")


prefix=("two_node_imbalance" "two_node_capacity" "three_node" "four_node" "five_node_hardcoded" \
    "hotnets" "five_line" "lnd_dec4_2018" "lnd_dec4_2018lessScale" \
    "sw_10_routers" "sw_20_routers" "sw_50_routers" "sw_60_routers" "sw_80_routers"  \
    "sw_100_routers" "sw_200_routers" "sw_400_routers" "sw_600_routers" \
    "sw_800_routers" "sw_1000_routers"\
    "sf_10_routers" "sf_20_routers" \
    "sf_50_routers" "sf_60_routers" "sf_80_routers"  \
    "sf_100_routers" "sf_200_routers" "sf_400_routers" "sf_600_routers" \
    "sf_800_routers" "sf_1000_routers" "tree_40_routers" "random_10_routers" "random_20_routers"\
    "random_30_routers" "sw_sparse_40_routers" "lnd_gaussian" "lnd_uniform" "lnd_july15_2019"\
    "parallel_graphs")


demand_scale=("3") # 10 for implementation comparison 
random_init_bal=false
random_capacity=false
lnd_capacity=false


#general parameters that do not affect config names
simulationLength=5100
statCollectionRate=100
timeoutClearRate=1
timeoutEnabled=true
signalsEnabled=true
loggingEnabled=false

# scheme specific parameters
eta=0.025
alpha=0.2
kappa=0.025
updateQueryTime=1.5
minPriceRate=0.25
zeta=0.01
rho=0.04
tau=10
normalizer=100
xi=1
routerQueueDrainTime=5
serviceArrivalWindow=300


mkdir -p ${PATH_NAME}
for suffix in "Base" "Waterfilling" "LndBaseline" "PriceScheme" "DCTCP"
do
    cp hostNode${suffix}.ned ${PATH_NAME}
    cp routerNode${suffix}.ned ${PATH_NAME}
done
cp hostNodeLandmarkRouting.ned ${PATH_NAME}
cp hostNodePropFairPriceScheme.ned ${PATH_NAME}
cp routerNodeDCTCPBal.ned ${PATH_NAME}

arraylength=${#prefix[@]}
PYTHON="/usr/bin/python"
mkdir -p ${PATH_NAME}

# TODO: find the indices in prefix of the topologies you want to run on and then specify them in array
# adjust experiment time as needed
array=(20) 
for i in "${array[@]}" 
do
    for balance in 100 200 400 # 50 100 200 400      
        # 900 1350 2750 4000 8750
    do
        if [ $random_capacity == "true" ]; then
            prefix_to_use="${prefix[i]}_randomCap"
            echo "random"
        elif [ $lnd_capacity == "true" ]; then
            prefix_to_use="${prefix[i]}_lndCap"
            echo "lnd"
        else
            prefix_to_use="${prefix[i]}"
        fi

        network="${prefix_to_use}_circ_net"
        topofile="${PATH_NAME}${prefix_to_use}_topo${balance}.txt"

        # identify graph type for topology
        if [ ${prefix_to_use:0:2} == "sw" ]; then
            graph_type="small_world"
        elif [ ${prefix_to_use:0:2} == "sf" ]; then
            graph_type="scale_free"
        elif [ ${prefix_to_use:0:4} == "tree" ]; then
            graph_type="tree"
        elif [ ${prefix_to_use:0:3} == "lnd" ]; then
            graph_type=${prefix_to_use}
        elif [ ${prefix_to_use} == "hotnets" ]; then
            graph_type="hotnets_topo"
        elif [ ${prefix_to_use:0:6} == "random" ]; then
            graph_type="random"
        elif [ ${prefix_to_use:0:8} == "parallel" ]; then
            graph_type="parallel_graph"
        else
            graph_type="simple_topologies"
        fi
        
        # set delay amount
        if [ ${prefix_to_use:0:3} == "two" ]; then
            delay="120"
        else
            # delay="150" for implementation comparison
            delay="30"
        fi
        
        # STEP 1: create topology
        $PYTHON scripts/create_topo_ned_file.py $graph_type\
                --network-name ${PATH_NAME}$network\
                --topo-filename $topofile\
                --num-nodes ${num_nodes[i]}\
                --balance-per-channel $balance\
                --separate-end-hosts \
                --delay-per-channel $delay \
                --randomize-start-bal $random_init_bal\
                --random-channel-capacity $random_capacity\
                --lnd-channel-capacity $lnd_capacity
    done

    # create 5 workload files for 5 runs
    for num in {0..4}
    do

        # create workload files and run different demand levels
        for scale in "${demand_scale[@]}"
        do

            # generate the graph first to ned file
            workloadname="${prefix[i]}_circ${num}_demand${scale}"
            workload="${PATH_NAME}$workloadname"
            inifile="${PATH_NAME}${workloadname}_default.ini"
            payment_graph_topo="custom"
            
            # figure out payment graph/workload topology
            if [ ${prefix_to_use:0:9} == "five_line" ]; then
                payment_graph_topo="simple_line"
            elif [ ${prefix_to_use:0:4} == "five" ]; then
                payment_graph_topo="hardcoded_circ"
            elif [ ${prefix_to_use:0:7} == "hotnets" ]; then
                payment_graph_topo="hotnets_topo"
            fi

            echo $network
            echo $topofile
            echo $inifile
            echo $graph_type

            # STEP 2: create transactions corresponding to this experiment run
            $PYTHON scripts/create_workload.py $workload poisson \
                    --graph-topo $payment_graph_topo \
                    --payment-graph-dag-percentage 0\
                    --topo-filename $topofile\
                    --experiment-time $simulationLength \
                    --balance-list $balance \
                    --generate-json-also \
                    --timeout-value 5 \
                    --scale-amount $scale\
                    --kaggle-size \
                    --run-num ${num}
        done
    done
done
 
 # File Path: \\wsl.localhost\Ubuntu-20.04\home\ad\omnetpp-5.6\samples\spider_omnet-Copy\scripts\generate_dag_benchmarks.sh

 #!/bin/bash
PATH_PREFIX="/home/ubuntu/omnetpp-5.4.1/samples/spider_omnet/benchmarks/"
GRAPH_PATH="/home/ubuntu/omnetpp-5.4.1/samples/spider_omnet/scripts/figures/"

num_nodes=("2" "2" "3" "4" "5" "5" "5" "0" "0" "10" "20" "50" "50" "60" "80" "30" "40" "0" "0" "50" "50" "0")


prefix=("two_node_imbalance" "two_node_capacity" "three_node" "four_node" "five_node_hardcoded" \
    "hotnets" "five_line" "lnd_dec4_2018" "lnd_dec4_2018lessScale" \
    "sw_10_routers" "sw_20_routers" "sw_50_routers" \
   "sf_50_routers" "sf_60_routers" "sf_80_routers"  \
    "random_30_routers" "sw_sparse_40_routers" "lnd_july15_2019" "lnd_uniform" \
    "sw_weird_combo" "sf_weird_combo", "lnd_weird_combo")

scale=3
random_init_bal=false
random_capacity=false
lnd_capacity=true


#general parameters that do not affect config names
simulationLength=2010
statCollectionRate=100
timeoutClearRate=1
timeoutEnabled=true
signalsEnabled=false
loggingEnabled=false

# scheme specific parameters
eta=0.025
alpha=0.2
kappa=0.025
updateQueryTime=1.5
minPriceRate=0.25
zeta=0.01
rho=0.04
tau=10
normalizer=100
xi=1
routerQueueDrainTime=5
serviceArrivalWindow=300



arraylength=${#prefix[@]}
PYTHON="/usr/bin/python"
mkdir -p ${PATH_PREFIX}

dag_percent=("45") # "20" "65")
balance=4000

# TODO: find the indices in prefix of the topologies you want to run on and then specify them in array
# adjust experiment time as needed
array=(20) 
for i in "${array[@]}" 
do    
    # create workload files and run different demand levels
    for dag_amt in "${dag_percent[@]}"
    do
        # generate the graph first to ned file
        PATH_NAME="${PATH_PREFIX}dag${dag_amt}/"
        mkdir -p ${PATH_NAME}
        for suffix in "Base" "Waterfilling" "LndBaseline" "PriceScheme" "DCTCP"
        do
            cp hostNode${suffix}.ned ${PATH_NAME}
            cp routerNode${suffix}.ned ${PATH_NAME}
        done
        cp hostNodeLandmarkRouting.ned ${PATH_NAME}
        cp hostNodePropFairPriceScheme.ned ${PATH_NAME}
        cp routerNodeDCTCPBal.ned ${PATH_NAME}
        
        if [ $random_capacity == "true" ]; then
            prefix_to_use="${prefix[i]}_randomCap"
            echo "random"
        elif [ $lnd_capacity == "true" ]; then
            prefix_to_use="${prefix[i]}_lndCap"
            echo "lnd"
        else
            prefix_to_use="${prefix[i]}"
        fi
        
        network="${prefix_to_use}_dag${dag_amt}_net"
        topofile="${PATH_NAME}${prefix_to_use}_topo${balance}.txt"

        # identify graph type for topology
        if [ ${prefix[i]:0:2} == "sw" ]; then
            graph_type="small_world"
        elif [ ${prefix[i]:0:2} == "sf" ]; then
            graph_type="scale_free"
        elif [ ${prefix[i]:0:4} == "tree" ]; then
            graph_type="tree"
        elif [ ${prefix[i]:0:11} == "lnd_uniform" ]; then
            graph_type="lnd_uniform"
        elif [ ${prefix[i]:0:3} == "lnd" ]; then
            graph_type="lnd_july15_2019"
        elif [ ${prefix[i]} == "hotnets" ]; then
            graph_type="hotnets_topo"
        elif [ ${prefix[i]:0:6} == "random" ]; then
            graph_type="random"
        else
            graph_type="simple_topologies"
        fi
        
        # set delay amount
        if [ ${prefix[i]:0:3} == "two" ]; then
            delay="120"
        else
            delay="30"
        fi
        
        # STEP 1: create topology
        $PYTHON scripts/create_topo_ned_file.py $graph_type\
                --network-name ${PATH_NAME}$network\
                --topo-filename $topofile\
                --num-nodes ${num_nodes[i]}\
                --balance-per-channel $balance\
                --separate-end-hosts \
                --delay-per-channel $delay\
                --randomize-start-bal $random_init_bal\
                --random-channel-capacity $random_capacity \
                --lnd-channel-capacity $lnd_capacity

        # identify graph type for topology
        if [ ${prefix_to_use:0:2} == "sw" ]; then
            graph_type="small_world"
        elif [ ${prefix_to_use:0:2} == "sf" ]; then
            graph_type="scale_free"
        elif [ ${prefix_to_use:0:4} == "tree" ]; then
            graph_type="tree"
        elif [ ${prefix_to_use:0:3} == "lnd" ]; then
            graph_type=${prefix_to_use}
        elif [ ${prefix_to_use} == "hotnets" ]; then
            graph_type="hotnets_topo"
        elif [ ${prefix_to_use:0:6} == "random" ]; then
            graph_type="random"
        else
            graph_type="simple_topologies"
        fi
        
        # set delay amount
        if [ ${prefix_to_use:0:3} == "two" ]; then
            delay="120"
        else
            delay="30"
        fi       

        echo $network
        echo $topofile
        echo $inifile
        echo $graph_type
    
        # create 5 workload files for 5 runs
        for num in #{0..4}
        do
            echo "generating dag ${num} for ${dag_amt}"
            workloadname="${prefix[i]}_demand${scale}_dag${dag_amt}_num${num}"
            workload="${PATH_NAME}$workloadname"
            inifile="${PATH_NAME}${workloadname}_default.ini"
            payment_graph_topo="custom"

            # STEP 2: create transactions corresponding to this experiment run
            $PYTHON scripts/create_workload.py $workload poisson \
                    --graph-topo $payment_graph_topo \
                    --payment-graph-dag-percentage ${dag_amt}\
                    --topo-filename $topofile\
                    --experiment-time $simulationLength \
                    --balance-list $balance\
                    --generate-json-also \
                    --timeout-value 5 \
                    --kaggle-size \
                    --scale-amount $scale \
                    --run-num ${num}
        done
    done
    #rm $topofile
done

 # File Path: \\wsl.localhost\Ubuntu-20.04\home\ad\omnetpp-5.6\samples\spider_omnet-Copy\scripts\generate_rebalancing_benchmarks.sh

 #!/bin/bash
PATH_PREFIX="/home/ubuntu/omnetpp-5.4.1/samples/spider_omnet/benchmarks/"
GRAPH_PATH="/home/ubuntu/omnetpp-5.4.1/samples/spider_omnet/scripts/figures/"

num_nodes=("3" "3" "4" "5" "5" "5" "0" "0" "10" "20" "50" "50" "60" "80" "30" "40" "0" "0" "50" "50")


prefix=("dag_example" "three_node" "four_node" "five_node_hardcoded" \
    "hotnets" "five_line" "lnd_dec4_2018" "lnd_dec4_2018lessScale" \
    "sw_10_routers" "sw_20_routers" "sw_50_routers" \
   "sf_50_routers" "lnd_july15_2019" "lnd_uniform" \
    "sw_weird_combo" "sf_weird_combo" "dag_example")

scale=1 
random_init_bal=false
random_capacity=false
lnd_capacity=true


#general parameters that do not affect config names
simulationLength=2010
statCollectionRate=10
timeoutClearRate=1
timeoutEnabled=true
signalsEnabled=false
loggingEnabled=false

# scheme specific parameters
eta=0.025
alpha=0.2
kappa=0.025
updateQueryTime=1.5
minPriceRate=0.25
zeta=0.01
rho=0.04
tau=10
normalizer=100
xi=1
routerQueueDrainTime=5
serviceArrivalWindow=300



arraylength=${#prefix[@]}
PYTHON="/usr/bin/python"
mkdir -p ${PATH_PREFIX}

dag_percent=("45") # "20" "65")
balance=4000

# TODO: find the indices in prefix of the topologies you want to run on and then specify them in array
# adjust experiment time as needed
array=(10) 
for i in "${array[@]}" 
do    
    # create workload files and run different demand levels
    for dag_amt in "${dag_percent[@]}"
    do
        # generate the graph first to ned file
        PATH_NAME="${PATH_PREFIX}dag${dag_amt}/"
        mkdir -p ${PATH_NAME}
        for suffix in "Base" "Waterfilling" "LndBaseline" "PriceScheme" "DCTCP"
        do
            cp hostNode${suffix}.ned ${PATH_NAME}
            cp routerNode${suffix}.ned ${PATH_NAME}
        done
        cp hostNodeLandmarkRouting.ned ${PATH_NAME}
        cp hostNodePropFairPriceScheme.ned ${PATH_NAME}
        cp routerNodeDCTCPBal.ned ${PATH_NAME}
        
        if [ $random_capacity == "true" ]; then
            prefix_to_use="${prefix[i]}_randomCap"
            echo "random"
        elif [ $lnd_capacity == "true" ]; then
            prefix_to_use="${prefix[i]}_lndCap"
            echo "lnd"
        else
            prefix_to_use="${prefix[i]}"
        fi
        
        network="${prefix_to_use}_dag${dag_amt}_net"
        topofile="${PATH_NAME}${prefix_to_use}_topo${balance}_rb.txt"

        # identify graph type for topology
        if [ ${prefix[i]:0:2} == "sw" ]; then
            graph_type="small_world"
        elif [ ${prefix[i]:0:2} == "sf" ]; then
            graph_type="scale_free"
        elif [ ${prefix[i]:0:4} == "tree" ]; then
            graph_type="tree"
        elif [ ${prefix[i]:0:11} == "lnd_uniform" ]; then
            graph_type="lnd_uniform"
        elif [ ${prefix[i]:0:3} == "lnd" ]; then
            graph_type=${prefix[i]}
        elif [ ${prefix[i]:0:3} == "dag" ]; then
            graph_type=${prefix[i]}
        elif [ ${prefix[i]} == "hotnets" ]; then
            graph_type="hotnets_topo"
        elif [ ${prefix[i]:0:6} == "random" ]; then
            graph_type="random"
        else
            graph_type="simple_topologies"
        fi
        
        # set delay amount
        if [ ${prefix[i]:0:3} == "two" ]; then
            delay="120"
        else
            delay="30"
        fi
        
        # STEP 1: create topology
        $PYTHON scripts/create_topo_ned_file.py $graph_type\
                --network-name ${PATH_NAME}$network\
                --topo-filename $topofile\
                --num-nodes ${num_nodes[i]}\
                --balance-per-channel $balance\
                --separate-end-hosts \
                --delay-per-channel $delay\
                --randomize-start-bal $random_init_bal\
                --random-channel-capacity $random_capacity \
                --lnd-channel-capacity $lnd_capacity \
                --rebalancing-enabled true 

        # identify graph type for topology
        if [ ${prefix_to_use:0:2} == "sw" ]; then
            graph_type="small_world"
        elif [ ${prefix_to_use:0:2} == "sf" ]; then
            graph_type="scale_free"
        elif [ ${prefix_to_use:0:4} == "tree" ]; then
            graph_type="tree"
        elif [ ${prefix_to_use:0:3} == "lnd" ]; then
            graph_type=${prefix_to_use}
        elif [ ${prefix_to_use:0:3} == "dag" ]; then
            graph_type=${prefix_to_use}
        elif [ ${prefix_to_use} == "hotnets" ]; then
            graph_type="hotnets_topo"
        elif [ ${prefix_to_use:0:6} == "random" ]; then
            graph_type="random"
        else
            graph_type="simple_topologies"
        fi
        
        # set delay amount
        if [ ${prefix_to_use:0:3} == "two" ]; then
            delay="120"
        else
            delay="30"
        fi       

        echo $network
        echo $topofile
        echo $inifile
        echo $graph_type
    
        # create 5 workload files for 5 runs
        for num in {0..4}
        do
            echo "generating dag ${num} for ${dag_amt}"
            workloadname="${prefix[i]}_demand${scale}_dag${dag_amt}_num${num}"
            workload="${PATH_NAME}$workloadname"
            inifile="${PATH_NAME}${workloadname}_default.ini"
            payment_graph_topo="custom"

            # STEP 2: create transactions corresponding to this experiment run
            $PYTHON scripts/create_workload.py $workload poisson \
                    --graph-topo $payment_graph_topo \
                    --payment-graph-dag-percentage ${dag_amt}\
                    --topo-filename $topofile\
                    --experiment-time $simulationLength \
                    --generate-json-also \
                    --timeout-value 5 \
                    --kaggle-size \
                    --scale-amount $scale \
                    --run-num ${num}
        done
    done
    #rm $topofile
done



# File Path: \\wsl.localhost\Ubuntu-20.04\home\ad\omnetpp-5.6\samples\spider_omnet-Copy\scripts\generate_summary_cdf.py


import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import argparse
import os
import json
import numpy as np
from parse_vec_files import *

parser = argparse.ArgumentParser('CDF')
parser.add_argument('--vec_files',
        type=str,
        required=True,
        nargs='+',
        help='Each vector file should be generated from a run using the omnet simulator')
parser.add_argument('--labels',
        type=str,
        required=True,
        nargs='+')
parser.add_argument('--save',
        type=str,
        required=True,
        help='The file to which to write the figure')

args = parser.parse_args()

fmts = ['r--o', 'b-^']



def compute_avg_path_completion_rates(filename, shortest=True):
    completion_fractions = []
    all_timeseries, vec_id_to_info_map, parameters = parse_vec_file(filename, "completion_rate_cdfs")

    # dictionary of src dest pairs to a map denoting the attempting rate or completion rate on each of
    # MAX_K paths each identified by path id
    avg_rate_attempted = dict()
    avg_rate_completed = dict()
    total_completed = 0
    total_attempted = 0

    # aggregate information on a per src_Dest pair and per path level
    for vec_id, timeseries in list(all_timeseries.items()):
        vector_details = vec_id_to_info_map[vec_id]
        src_dest_pair = (vector_details[0], vector_details[3])

        signal_name = vector_details[2]
        if "Path" in signal_name:
            continue
        path_id = 0

        '''if "Path" not in signal_name and not shortest:
            continue
        elif not shortest:
            path_id = int(signal_name.split("_")[1])
        else:
            path_id = 0'''
        signal_values = [t[1] for t in timeseries]


        if "rateArrived" in signal_name:
            cur_info = avg_rate_attempted.get(src_dest_pair, dict())
            cur_info[path_id] = np.average(np.array(signal_values))
            avg_rate_attempted[src_dest_pair] = cur_info
            total_attempted += np.sum(signal_values)
        elif "rateCompleted" in signal_name:
            cur_info = avg_rate_completed.get(src_dest_pair, dict())
            cur_info[path_id] = np.average(np.array(signal_values))
            avg_rate_completed[src_dest_pair] = cur_info
            total_completed += np.sum(signal_values)
        else: 
            print("UNINTERESTING TIME SERIES OF VECTOR of type", signal_name, "FOUND")


    # aggregate all the information across all source destination pairs and paths onto 
    # a single list with the completion fractions for all of them
    for key in list(avg_rate_attempted.keys()):
        rates_attempted_across_paths = avg_rate_attempted[key]
        rates_completed_across_paths = avg_rate_completed.get(key, dict())

        for path_id, attempt_rate in list(rates_attempted_across_paths.items()):
            if attempt_rate > 0: 
                completion_fractions.append(rates_completed_across_paths.get(path_id, 0)/attempt_rate)

    overall_success = total_completed/float(total_attempted)
    print(filename, ": ")
    print("Average across source destinations: ", np.average(np.array(completion_fractions)))
    print("Overall success rate: ", total_completed, float(total_attempted))
    return overall_success, completion_fractions


# plot cdf of completion fractions across all paths used between different sources and destination pairs
# for the given set of algorithms
def plot_completion_rate_cdf(args):
    def bar_x_coord(bw, ix):
        return bw - (len(args.summaries)/2.0 - ix)*args.bar_width

    color_opts = ['#fa9e9e', '#a4e0f9', '#57a882', '#ad62aa']
    dims = plt.rcParams["figure.figsize"]
    plt.rcParams["figure.figsize"] = dims
    for i in range(len(args.vec_files)):
        scale = 1
        # returns all the data points for the completion rates for individual paths to every source dest pair
        overall_succ, res = compute_avg_path_completion_rates(args.vec_files[i], "waterfilling" not in args.vec_files[i])
        keys = sorted(res)
        ys = np.linspace(0.1, 1.0, 100)
        
        # if you want error bars
        #xerr_low = [k - data[k][0] for k in keys]
        #xerr_high = [data[k][1] - k for k in keys]
        #plt.errorbar(keys, ys, xerr=[keys, keys], fmt=fmts[i], label=args.labels[i], linewidth=3, markersize=15)
        n, bins, patches = plt.hist(keys, bins = 100, density='True', cumulative='True', label=args.labels[i] \
                + "(" + str(overall_succ) + ")", \
                linewidth=3, histtype='step')
        patches[0].set_xy(patches[0].get_xy()[:-1])

    plt.title('Completion rate cdfs') # TODO: put topology file here
    plt.xlabel('Achieved completion rates')
    plt.ylabel('CDF')
    plt.ylim(0,1.1)
    plt.legend(loc="upper left")
    plt.tight_layout()
    plt.savefig(args.save)


def main():
    matplotlib.rcParams['figure.figsize'] = [15, 10]
    plt.rc('font', size=40)          # controls default text sizes
    plt.rc('axes', titlesize=42)     # fontsize of the axes title
    plt.rc('axes', labelsize=40)    # fontsize of the x and y labels
    plt.rc('xtick', labelsize=32)    # fontsize of the tick labels
    plt.rc('ytick', labelsize=32)    # fontsize of the tick labels
    #plt.rc('legend', fontsize=34)    # legend fontsize for paper
    plt.rc('legend', fontsize=20)    # legend fontsize
    plot_completion_rate_cdf(args)

main()



# File Path: \\wsl.localhost\Ubuntu-20.04\home\ad\omnetpp-5.6\samples\spider_omnet-Copy\scripts\generate_tpt_benchmarks.sh

#!/bin/bash
PATH_NAME="/home/ubuntu/omnetpp-5.4.1/samples/spider_omnet/benchmarks/circulations/"


demand_scale=("10" "20" "40" "80" "160") # "60" "90")
capacity_list=("20" "40" "80" "160" "320")
random_init_bal=false
random_capacity=false
simulationLength=4100


cp hostNodeBase.ned ${PATH_NAME}
cp hostNodeWaterfilling.ned ${PATH_NAME}
cp hostNodeLandmarkRouting.ned ${PATH_NAME}
cp hostNodePriceScheme.ned ${PATH_NAME}
cp hostNodeLndBaseline.ned ${PATH_NAME}
cp routerNode.ned ${PATH_NAME}

arraylength=${#prefix[@]}
PYTHON="/usr/bin/python"
mkdir -p ${PATH_NAME}


prefix="lnd_uniform"
length=${#capacity_list[@]}

for (( i=0; i<$length; i++ ));
do
    capacity=${capacity_list[i]}
    scale=${demand_scale[i]}
    network="${prefix}_cap${capacity}_circ_net"
    topofile="${PATH_NAME}${prefix}_cap${capacity}_topo.txt"
    graph_type="lnd_uniform"
    delay="30"

    echo $capacity
    echo $scale
    
    # STEP 1: create topology
    $PYTHON scripts/create_topo_ned_file.py $graph_type\
            --network-name ${PATH_NAME}$network\
            --topo-filename $topofile\
            --num-nodes 102\
            --balance-per-channel $capacity\
            --separate-end-hosts \
            --delay-per-channel $delay\
            --randomize-start-bal $random_init_bal\
            --random-channel-capacity $random_capacity


    # create workload files and run different demand levels
    workloadname="${prefix}_circ_demand${scale}"
    workload="${PATH_NAME}$workloadname"
    inifile="${PATH_NAME}${workloadname}_default.ini"
    payment_graph_topo="custom"
        


    echo $network
    echo $topofile
    echo $inifile
    echo $graph_type

        # STEP 2: create transactions corresponding to this experiment run
        $PYTHON scripts/create_workload.py $workload poisson \
                --graph-topo $payment_graph_topo \
                --payment-graph-dag-percentage 0\
                --topo-filename $topofile\
                --experiment-time $simulationLength \
                --balance-per-channel $capacity\
                --timeout-value 5 \
                --scale-amount $scale 
done



# File Path: \\wsl.localhost\Ubuntu-20.04\home\ad\omnetpp-5.6\samples\spider_omnet-Copy\scripts\get-queue-numbers-capacityFactor-1.sh

for demand in 1 3 6
do
    python parse_vec_files_queueLines.py /home/ubuntu/omnetpp-5.4.1/samples/spider_omnet/benchmarks/circulations/results/capacity-factor-1-queue-1000/sw_50_routers_circ_net_priceScheme_demand${demand}_shortest_4-#0.vec $demand woWin 3000 5000 figures/timeouts/capacity-factor-1-queue-1000/queue-sw50-kaggle-demand${demand}-woWin-capacityFactor1.txt
    #python parse_vec_files_queueLines.py /home/ubuntu/omnetpp-5.4.1/samples/spider_omnet/benchmarks/circulations/results/capacity-factor-1-queue-1000/sw_50_routers_circ_net_priceSchemeWindow_demand${demand}_shortest_4-#0.vec $demand regular 3000 5000 figures/timeouts/capacity-factor-1-queue-1000/queue-sw50-kaggle-demand${demand}-regular-capacityFactor1.txt
    #python parse_vec_files_queueLines.py /home/ubuntu/omnetpp-5.4.1/samples/spider_omnet/benchmarks/circulations/results/capacity-factor-1-queue-1000/sw_50_routers_circ_net_priceSchemeWindowNoQueue_demand${demand}_shortest_4-#0.vec $demand woQ 3000 5000 figures/timeouts/capacity-factor-1-queue-1000/queue-sw50-kaggle-demand${demand}-woQ-capacityFactor1.txt
done

for demand in 9
do
    #python parse_vec_files_queueLines.py /home/ubuntu/omnetpp-5.4.1/samples/spider_omnet/benchmarks/circulations/results/capacity-factor-1-queue-1000/sw_50_routers_circ_net_priceScheme_demand${demand}_shortest_4-#0.vec $demand woWin 3000 5000 figures/timeouts/capacity-factor-1-queue-1000/queue-sw50-kaggle-demand${demand}-woWin-capacityFactor1.txt
    #python parse_vec_files_queueLines.py /home/ubuntu/omnetpp-5.4.1/samples/spider_omnet/benchmarks/circulations/results/capacity-factor-1-queue-1000/sw_50_routers_circ_net_priceSchemeWindow_demand${demand}_shortest_4-#0.vec $demand regular 3000 5000 figures/timeouts/capacity-factor-1-queue-1000/queue-sw50-kaggle-demand${demand}-regular-capacityFactor1.txt
    python parse_vec_files_queueLines.py /home/ubuntu/omnetpp-5.4.1/samples/spider_omnet/benchmarks/circulations/results/capacity-factor-1-queue-1000/sw_50_routers_circ_net_priceSchemeWindowNoQueue_demand${demand}_shortest_4-#0.vec $demand woQ 3000 5000 figures/timeouts/capacity-factor-1-queue-1000/queue-sw50-kaggle-demand${demand}-woQ-capacityFactor1.txt
done


# File Path: \\wsl.localhost\Ubuntu-20.04\home\ad\omnetpp-5.6\samples\spider_omnet-Copy\scripts\kshortestpaths.py

""" algorithm to compute k shortest paths from given source to destination
in a graph using Yen's algorithm: https://en.wikipedia.org/wiki/Yen%27s_algorithm """

import argparse
import copy 

import cPickle as pickle 
import networkx as nx 
import operator
import numpy as np

import sys
sys.path.insert(1, '../paths')
import parse

def ksp_yen(graph, node_start, node_end, max_k=2):

    graph = copy.deepcopy(graph)

    A = []
    B = []

    try:
        path = nx.shortest_path(graph, source=node_start, target=node_end)
    except:
        print "No path found!"
        return None 

    A.append(path)    
    
    for k in range(1, max_k):
        for i in range(0, len(A[-1])-1):

            node_spur = A[-1][i]
            path_root = A[-1][:i+1]

            edges_removed = []
            for path_k in A:
                curr_path = path_k
                if len(curr_path) > i and path_root == curr_path[:i+1]:
                    if (curr_path[i], curr_path[i+1]) in graph.edges() or \
                        (curr_path[i+1], curr_path[i]) in graph.edges():
                        graph.remove_edge(curr_path[i], curr_path[i+1])
                        edges_removed.append([curr_path[i], curr_path[i+1]])
            
            nodes_removed = []
            for rootpathnode in path_root:
                if rootpathnode != node_spur:
                    graph.remove_node(rootpathnode)
                    nodes_removed.append(rootpathnode)

            try:
                path_spur = nx.shortest_path(graph, source=node_spur, target=node_end)
            except:
                path_spur = None
            
            if path_spur:
                path_total = path_root[:-1] + path_spur            
                potential_k = path_total            
                if not (potential_k in B):
                    B.append(potential_k)
            
            for node in nodes_removed:
                graph.add_node(node)

            for edge in edges_removed:
                graph.add_edge(edge[0], edge[1])
        
        if len(B):
            B.sort(key=len)
            A.append(B[0])
            B.pop(0)
        else:
            break
    
    return A

def ksp_edge_disjoint(graph, node_start, node_end, max_k=2):
    """ compute k edge disjoint shortest paths """
    graph = copy.deepcopy(graph)

    A = []

    try:
        path = nx.shortest_path(graph, source=node_start, target=node_end)
    except:
        print "No path found!"
        return None 

    A.append(path)    
    
    for k in range(1, max_k):
        prev_path = A[-1]
        for i, j in zip(prev_path[:-1], prev_path[1:]):
            if (i, j) in graph.edges() or (j, i) in graph.edges():
                graph.remove_edge(i, j)

        try:
            path = nx.shortest_path(graph, source=node_start, target=node_end)
        except:
            path = None

        if path:
            A.append(path)
                
    return A    

def kwp_edge_disjoint(graph, node_start, node_end, max_k, credit_mat):
    """ compute k edge disjoint widest paths """
    """ using http://www.cs.cmu.edu/~avrim/451f08/lectures/lect1007.pdf """

    graph = copy.deepcopy(graph)
    capacity_mat = credit_mat
    A = []

    try:
        path = nx.shortest_path(graph, source=node_start, target=node_end)
    except:
        print "No path found!"
        return None 

    for k in range(max_k):
        widthto = {}
        pathto = {}
        tree_nodes = []
        tree_neighbors = []
        tree_nodes_membership_indicator = {v: False for v in graph.nodes()}
        tree_neighbors_membership_indicator = {v: False for v in graph.nodes()}
        
        widthto[node_end] = np.inf
        pathto[node_end] = None
        tree_nodes.append(node_end)
        tree_nodes_membership_indicator[node_end] = True
        tree_neighbors = [v for v in graph.neighbors(node_end)]
        for v in graph.neighbors(node_end):
            tree_neighbors_membership_indicator[v] = True

        while tree_neighbors and (tree_nodes_membership_indicator[node_start] is False):
            x = tree_neighbors.pop(0)
            tree_neighbors_membership_indicator[x] = False
            
            max_width = -1.
            max_width_neighbor = None            
            for v in graph.neighbors(x):
                if tree_nodes_membership_indicator[v] is True:
                    if np.min([widthto[v], capacity_mat[x, v]]) > max_width:
                        max_width = np.min([widthto[v], capacity_mat[x, v]])
                        max_width_neighbor = v
                else:
                    if tree_neighbors_membership_indicator[v] is False:
                        tree_neighbors.append(v)
                        tree_neighbors_membership_indicator[v] = True

            widthto[x] = max_width
            pathto[x] = max_width_neighbor
            tree_nodes.append(x)
            tree_nodes_membership_indicator[x] = True

        if tree_nodes_membership_indicator[node_start] is True:
            node = node_start
            path = [node]
            while node != node_end:
                node = pathto[node]
                path.append(node)
            A.append(path)

        prev_path = A[-1]
        for i, j in zip(prev_path[:-1], prev_path[1:]):
            if (i, j) in graph.edges() or (j, i) in graph.edges():
                graph.remove_edge(i, j)            

    return A 

# get the best paths amongst the intermediary paths passed in based
# on bottleneck capacity / total rtt
def heuristic(intermediary_paths, capacity_mat, prop_mat, max_k=2):
    final_paths = []
    path_metric_dict = {}
    for i, path in enumerate(intermediary_paths):
        sum_rtt = 0.0
        min_capacity = capacity_mat[path[0], path[1]]
        for u,v in zip(path[:-1], path[1:]):
            sum_rtt += prop_mat[u,v]
            if capacity_mat[u,v] < min_capacity:
                min_capacity = capacity_mat[u,v]
        path_metric_dict[i] =  min_capacity / sum_rtt

    for key, _ in sorted(path_metric_dict.items(), key=operator.itemgetter(1), reverse=True):
        final_paths.append(intermediary_paths[key])
        if len(final_paths) == max_k:
            return final_paths
    return final_paths


def raeke(node_start, node_end):
    with open('./lnd_oblivious.pkl', 'rb') as input:
        paths = pickle.load(input)

    """ change node index """
    new_paths = []
    for path in paths[node_start, node_end]:
        new_path = [i-102 for i in path[1:-1]]
        new_paths.append(new_path)

    return new_paths

def main():

    parser = argparse.ArgumentParser()
    parser.add_argument('--credit_type', help='uniform or random or lnd credit on links')
    parser.add_argument('--graph_type', help='small_world or scale_free or txt or lnd or edgelist')
    parser.add_argument('--path_type', help='ksp_yen or ksp_edge_disjoint or kwp_edge_disjoint or heuristic')
    parser.add_argument('--topo_txt_file', help='filename to parse topology from', \
            default="../topology/sf_50_routers_lndCap_topo2750.txt")
    parser.add_argument('--max_num_paths', help='number of paths to consider (integer > 0)')
    args = parser.parse_args()

    n = 50
    CREDIT_AMT = 100.0
    RAND_SEED = 23
    delay = 1

    """ construct graph """
    if args.graph_type == 'scale_free':
        graph = nx.barabasi_albert_graph(n, 8, seed=23)
        graph = nx.Graph(graph)
        graph.remove_edges_from(graph.selfloop_edges())

    elif args.graph_type == 'small_world':
        graph = nx.watts_strogatz_graph(n, k=8, p=0.25, seed=23)
        graph = nx.Graph(graph)
        graph.remove_edges_from(graph.selfloop_edges())

    elif args.graph_type == 'edgelist':
        graph = nx.read_edgelist("../oblivious_routing/lnd_dec4_2018_reducedsize.edgelist")
        rename_dict = {v: int(str(v)) for v in graph.nodes()}
        graph = nx.relabel_nodes(graph, rename_dict)
        for e in graph.edges():
            graph.edges[e]['capacity'] = int(str(graph.edges[e]['capacity']))
        graph = nx.Graph(graph)
        graph.remove_edges_from(graph.selfloop_edges())
        n = nx.number_of_nodes(graph)        

    elif args.graph_type == 'txt':
        graph = parse.parse_txt_topology_file(args.topo_txt_file)
        n = nx.number_of_nodes(graph)

    else:
        print "Error! Graph type invalid."

    assert nx.is_connected(graph)

    """ construct credit matrix """
    if args.credit_type == 'uniform':
        credit_mat = np.ones([n, n])*CREDIT_AMT
        prop_mat = np.ones([n, n])*delay

    elif args.credit_type == 'random':
        np.random.seed(RAND_SEED)
        credit_mat = np.triu(np.random.rand(n, n), 1) * 2 * CREDIT_AMT
        credit_mat += credit_mat.transpose()
        credit_mat = credit_mat.astype(int)
        prop_mat = np.ones([n, n])*delay

    elif args.credit_type == 'txt' or args.credit_type == 'edgelist':
        credit_mat = np.zeros([n, n])
        prop_mat = np.zeros([n, n])
        for e in graph.edges():
            credit_mat[e[0], e[1]] = graph[e[0]][e[1]]['capacity'] 
            credit_mat[e[1], e[0]] = graph[e[1]][e[0]]['capacity']
            prop_mat[e[0], e[1]] = graph[e[0]][e[1]]['hop_delay'] 
            prop_mat[e[1], e[0]] = graph[e[1]][e[0]]['hop_delay']

    else:
        print "Error! Credit matrix type invalid."

    """ get paths and store in dict """
    paths = {}
    for i in range(n):
        for j in range(n):
            if i != j:
                if args.path_type == 'ksp_yen':
                    ret_paths = ksp_yen(graph, i, j, int(args.max_num_paths))
                elif args.path_type == 'ksp_edge_disjoint':
                    ret_paths = ksp_edge_disjoint(graph, i, j, int(args.max_num_paths))
                elif args.path_type == 'kwp_edge_disjoint':
                    ret_paths = kwp_edge_disjoint(graph, i, j, int(args.max_num_paths), credit_mat)
                elif args.path_type == 'heuristic':
                    intermediary_paths = ksp_yen(graph, i, j, 10000)
                    print "found", len(intermediary_paths), "between", i, "and", j
                    ret_paths = heuristic(intermediary_paths, credit_mat, prop_mat, int(args.max_num_paths))
                else:
                    print "Error! Path type invalid."

                new_paths = []
                for ret_path in ret_paths: 
                    new_path = []
                    new_path.append(i)
                    new_path = new_path + [u + n for u in ret_path]
                    new_path.append(j)
                    new_paths.append(new_path)

                paths[i, j] = new_paths

    print paths

    with open('./paths/' + args.graph_type + '_' + args.path_type + '.pkl', 'wb') as output:
        pickle.dump(paths, output, pickle.HIGHEST_PROTOCOL)

if __name__=='__main__':
    main()


# File Path: \\wsl.localhost\Ubuntu-20.04\home\ad\omnetpp-5.6\samples\spider_omnet-Copy\scripts\max_circulation.py


""" function to compute the maximum circulation value in a 
given demand matrix """

from gurobipy import *

def max_circulation(demand_dict):

	m = Model()
	m.setParam('OutputFlag', 0)
	m.setParam('TimeLimit', TIME_LIMIT)

	xvar = {}

	""" create variables """
	for i, j in list(demand_dict.keys()):
		xvar[i, j] = m.addVar(vtype=GRB.CONTINUOUS, lb=0.0, ub=demand_dict[i, j], obj=1.0)

	""" add circulation constraints """
	nonzero_demand_nodes = set()
	nonzero_demand_nodes = nonzero_demand_nodes | {i for i, j in list(demand_dict.keys())}
	nonzero_demand_nodes = nonzero_demand_nodes | {j for i, j in list(demand_dict.keys())}
	nonzero_demand_nodes = list(nonzero_demand_nodes)

	for k in nonzero_demand_nodes:
		incoming = list({i for i, j in list(demand_dict.keys()) if j == k})
		outgoing = list({j for i, j in list(demand_dict.keys()) if i == k})
		expr = 0.0
		for i in incoming:
			expr += xvar[i, k]
		for j in outgoing:
			expr -= xvar[k, j]
		m.addConstr(expr == 0.0)

	""" optimize """
	m.update()
	m.setAttr("ModelSense", -1)
	m.optimize()

	""" return computed solution """
	obj = 0.0
	for i, j in list(demand_dict.keys()):
		obj += xvar[i, j].X 

	return obj

TIME_LIMIT = 120

def main():
	demand_dict = {}
	demand_dict[0, 1] = 1.0
	demand_dict[1, 2] = 1.0
	demand_dict[2, 3] = 1.0
	demand_dict[3, 0] = 1.0
	demand_dict[3, 1] = 1.0
	demand_dict[2, 0] = 1.0
	print(max_circulation(demand_dict))

if __name__=='__main__':
	main()



# File Path: \\wsl.localhost\Ubuntu-20.04\home\ad\omnetpp-5.6\samples\spider_omnet-Copy\scripts\parse_completion_times.py

import sys
import argparse
import statistics as stat
from config import *
import shlex
import numpy as np
import math

# figure out what the size buckets should be for a given number of buckets
# say you want 20 buckets, you want to make them equally sized in the number
# of transactions in a bucket (based on the skew of transaction sizes), so the
# larger transactions span a wider range but at the smaller end, the buckets
# are narrower
def compute_buckets(num_buckets, dist_filename):
    amt_dist = np.load(dist_filename)
    num_amts = amt_dist.item().get('p').size
    pdf = amt_dist.item().get('p')
    cdf = np.cumsum(pdf)

    gap = 1.0 / num_buckets
    break_point = gap
    buckets = []

    # return all the bucket end markers
    for i, c in enumerate(cdf):
        if c >= break_point:
            print(break_point, i, c)
            buckets.append(int(round(amt_dist.item().get('bins')[i], 1)))
            break_point += gap
    # buckets.append(int(round(amt_dist.item().get('bins')[-1], 1)))
    print(buckets, len(buckets))
    return buckets

delay = 30

parser = argparse.ArgumentParser('Completion Time Plots')
parser.add_argument('--topo',
        type=str, 
        required=True,
        help='what topology to generate size summary for')
parser.add_argument('--payment-graph-type',
        type=str, 
        help='what graph type topology to generate summary for', default="circ")
parser.add_argument('--credit',
        type=int,
        help='Credit to collect stats for', default=10)
parser.add_argument('--demand',
        type=int,
        help='Single number denoting the demand to collect data for', default="30")
parser.add_argument('--path-type',
        type=str,
        help='types of paths to collect data for', default="shortest")
parser.add_argument('--path-num',
        type=int,
        help='number of paths to collect data for', default=4)
parser.add_argument('--scheme-list',
        nargs="*",
        help='set of schemes to aggregate results for', default=["priceSchemeWindow"])
parser.add_argument('--save',
        type=str, 
        required=True,
        help='file name to save data in')
parser.add_argument('--num-max',
        type=int,
        help='Single number denoting the maximum number of runs to aggregate data over', default="5")
parser.add_argument('--num-buckets',
        type=int,
        help='Single number denoting the maximum number of buckets to group txn sizes into', default="20")
parser.add_argument('--lnd-retry-data',
        action='store_true',
        help='whether to parse overall lnd retry information')

# collect all arguments
args = parser.parse_args()
topo = args.topo
credit = args.credit
demand = args.demand
path_type = args.path_type
num_paths = args.path_num
scheme_list = args.scheme_list

succ_retries, fail_retries = [],[]
output_file = open(GGPLOT_DATA_DIR + args.save, "w+")
output_file.write("Topo,CreditType,Scheme,Credit,SizeStart,SizeEnd,Point,AvgCompTime,TailCompTime90,TailCompTime99,Demand\n")

buckets = compute_buckets(args.num_buckets, KAGGLE_AMT_DIST_FILENAME)

if "sw" in args.topo or "sf" in args.topo:
    topo_type = args.save[:2]
else:
    topo_type = args.save[:3]

if "lnd_uniform" in args.topo: 
    credit_type = "uniform"
elif "lnd_july15" in args.topo or "lndCap" in args.topo:
    credit_type = "lnd"
else:
    credit_type = "uniform"

# parse and print the retry data for LND
def collect_retry_stats(succ_retry_file, fail_retry_file):
    for i, file_name in enumerate([succ_retry_file, fail_retry_file]):
        retries = fail_retries if i else succ_retries
        try:
            with open(RESULT_DIR + file_name) as f:
                for line in f:
                    parts = line.split()
                    for t in parts:
                        retries.append(float(t))
        except IOError:
            print("error with", file_name)
            continue



# go through all relevant files and aggregate probability by size
for scheme in scheme_list:
    size_to_comp_times = {} 
    for run_num in range(0, args.num_max + 1):
        if credit_type != "uniform" and (scheme == "waterfilling" or scheme == "DCTCPQ"):
            path_type = "widest"
        else:
            path_type = "shortest"

        file_name = topo + "_" + args.payment_graph_type + "_net_" + str(credit) + "_" + scheme + "_" + \
                args.payment_graph_type + str(run_num) + \
            "_demand" + str(demand/10) + "_" + path_type
        if scheme != "shortestPath":
            file_name += "_" + str(num_paths)

        if scheme == 'lndBaseline' and args.lnd_retry_data:
            succ_retry_file = file_name + "_LIFO_succRetries.txt"
            fail_retry_file = file_name + "_LIFO_failRetries.txt"
            collect_retry_stats(succ_retry_file, fail_retry_file)
            
        file_name += "_LIFO_tailCompBySize.txt"
        try:
            with open(RESULT_DIR + file_name) as f:
                for line in f:
                    parts = line.split()
                    size = float(parts[0][:-1])
                    bucket = buckets[np.searchsorted(buckets, size)]
                    comp_times = size_to_comp_times.get(bucket, [])
                    for t in parts[1:]:
                        comp_times.append(float(t))
                    size_to_comp_times[bucket] = comp_times
        except IOError:
            print("error with", file_name)
            continue
     

    sorted_sizes = [5]
    sorted_sizes.extend(sorted(size_to_comp_times.keys()))
    print(sorted_sizes)
    if scheme == 'lndBaseline' and args.lnd_retry_data:
        print("Successful transaction LND retries Average:", np.average(np.array(succ_retries)), \
                "99%:" , np.percentile(np.array(succ_retries), 99))
        print("Failed transaction retries LND Average:", np.average(np.array(fail_retries)), \
                "99%:" , np.percentile(np.array(fail_retries), 99))
    
    for i, size in enumerate(sorted_sizes[1:]):
        comp_times = np.array(size_to_comp_times[size])
        output_file.write(topo_type + "," + credit_type + "," + \
                str(SCHEME_CODE[scheme]) +  "," + str(credit) +  "," + \
            "%f,%f,%f,%f,%f,%f,%f\n" % (sorted_sizes[i], size, \
                    math.sqrt(size * sorted_sizes[i]), \
                    np.average(comp_times), np.percentile(comp_times, 90), np.percentile(comp_times, 99),
                     demand))

output_file.close()



# File path : \\wsl.localhost\Ubuntu-20.04\home\ad\omnetpp-5.6\samples\spider_omnet-Copy\scripts\parse_final_summary_stats.py

import sys
import argparse
import statistics as stat
from config import *
import os

delay = 30

parser = argparse.ArgumentParser('Analysis Plots')
parser.add_argument('--topo',
        type=str, 
        required=True,
        help='what topology to generate summary for')
parser.add_argument('--payment-graph-type',
        type=str, 
        help='what graph type topology to generate summary for', default="circ")
parser.add_argument('--credit-list',
        nargs="+",
        required=True,
        help='Credits to collect stats for')
parser.add_argument('--demand',
        type=int,
        help='Single number denoting the demand to collect data for', default="30")
parser.add_argument('--path-type-list',
        nargs="*",
        help='types of paths to collect data for', default=["shortest"])
parser.add_argument('--scheduling-alg-list',
        nargs="*",
        help='scheduling algorithms to collect info for', default=[None])
parser.add_argument('--queue-threshold-list',
        nargs="*",
        help='queue thresholds to collect info for', default=[None])
parser.add_argument('--dag-percent-list',
        nargs="*",
        help='dag percents to collect info for', default=[None])
parser.add_argument('--path-num-list',
        nargs="*",
        help='number of paths to collect data for', default=[4])
parser.add_argument('--scheme-list',
        nargs="*",
        help='set of schemes to aggregate results for', default=["priceSchemeWindow"])
parser.add_argument('--save',
        type=str, 
        required=True,
        help='file name to save data in')
parser.add_argument('--num-max',
        type=int,
        help='Single number denoting the maximum number of runs to aggregate data over', default="5")

# collect all arguments
args = parser.parse_args()
topo = args.topo
credit_list = args.credit_list
demand = args.demand
path_type_list = args.path_type_list
scheme_list = args.scheme_list
path_num_list = args.path_num_list
queue_threshold_list = args.queue_threshold_list
dag_percent_list = args.dag_percent_list
scheduling_algorithms = args.scheduling_alg_list

output_file = open(GGPLOT_DATA_DIR + args.save, "w+")
if args.payment_graph_type == "circ":
    output_file.write("Scheme,Credit,")
else:
    output_file.write("Scheme,Credit,DAGAmt,")

output_file.write("Topo,CreditType,NumPaths,PathType,SchedulingAlg," + \
        "Threshold,SuccRatio,SuccRatioMin,SuccRatioMax,SuccVolume," + \
        "SuccVolumeMin," +\
        "SuccVolumeMax,CompTime,CompTimeMin,CompTimeMax\n")

# determine topology and credit type
if "sw" in args.topo or "sf" in args.topo:
    topo_type = args.save[:2]
else:
    topo_type = args.save[:3]

if "lnd_uniform" in args.topo: 
    credit_type = "uniform"
elif "lnd_july15" in args.topo or "lndCap" in args.topo:
    credit_type = "lnd"
else:
    credit_type = "uniform"


# go through all relevant files and aggregate info
for credit in credit_list:
    for scheme in scheme_list:
        for path_type in path_type_list:
            if path_type == "widest" and scheme not in ["waterfilling", "DCTCPQ"]:
                continue
            if path_type == "shortest" and len(scheme_list) > 1 and scheme in ["waterfilling", "DCTCPQ"] and \
                    credit_type == "lnd":
                continue

            for queue_threshold in queue_threshold_list:
                for num_paths in path_num_list:
                    for percent in dag_percent_list:
                        for alg in scheduling_algorithms:
                            succ_ratios, succ_vols,comp_times = [], [], []
                            for run_num in range(0, args.num_max  + 1):
                                if args.payment_graph_type == "circ" or percent == '0':
                                    file_name = topo + str(credit) + "_circ" + str(run_num)
                                else:
                                    file_name = topo + "_dag" + str(percent) + "_" + str(credit) + "_num" + \
                                            str(run_num) 
                                
                                file_name += "_delay" + str(delay) + "_demand" + str(demand) + "_" + scheme + \
                                        "_" + path_type 

                                if scheme != "shortestPath":
                                    file_name += "_" + str(num_paths) 
                                if alg is not None:
                                    file_name += "_" + alg
                                elif scheme == "celer":
                                    file_name += "_FIFO"
                                else:
                                    file_name += "_LIFO"
                                
                                if queue_threshold is not None and percent != '0' and scheme == "DCTCPQ":
                                    file_name += "_qd" + str(queue_threshold)
                                file_name += "_summary.txt"
                                
                                try: 
                                    with open(SUMMARY_DIR + file_name) as f:
                                        for line in f:
                                            if line.startswith("Success ratio"):
                                                succ_ratio = float(line.split(" ")[4])
                                            elif line.startswith("Success volume"):
                                                succ_volume = float(line.split(" ")[5])
                                            elif line.startswith("Avg completion time"):
                                                comp_time = float(line.split(" ")[3][:-1])
                                        succ_ratios.append(succ_ratio * 100)
                                        succ_vols.append(succ_volume * 100)
                                        comp_times.append(comp_time)
                                except IOError:
                                    print("error with " , file_name)
                                    continue

                            if "lndtopo" in args.save and "lnd_credit" in args.save:
                                capacity = int(credit) * 650
                            elif "lndnewtopo" in args.save and "lnd_credit" in args.save:
                                capacity = int(credit) * 422
                            else:
                                capacity = int(credit)



                            
                            if len(succ_ratios) > 0:
                                if args.payment_graph_type == "circ":
                                    output_file.write(SCHEME_CODE[scheme] + "," + str(capacity) +  ",")
                                else:
                                    output_file.write(SCHEME_CODE[scheme] + "," + str(capacity) +  "," + \
                                            str(PERCENT_MAPPING[percent]) + ",")

                                output_file.write(topo_type + "," + credit_type + "," \
                                        + str(num_paths) + "," \
                                    + str(path_type) + "," \
                                    + str(alg) + "," \
                                    + str(queue_threshold) + "," \
                                    + ("%f,%f,%f,%f,%f,%f,%f,%f,%f\n" % (stat.mean(succ_ratios), min(succ_ratios), \
                                    max(succ_ratios), stat.mean(succ_vols), min(succ_vols),  max(succ_vols), \
                                    stat.mean(comp_times), min(comp_times), max(comp_times))))
    
    if args.payment_graph_type == 'dag':
        for percent in dag_percent_list:
            if "lndtopo" in args.save and "lnd_credit" in args.save:
                capacity = int(credit) * 650
            else:
                capacity = int(credit)

            output_file.write("Circ," + str(capacity) +  "," + \
                                        str(PERCENT_MAPPING[percent]) + ",")   
            ideal = 100 - PERCENT_MAPPING[percent]
            output_file.write(topo_type + "," + credit_type + ",4,ideal,0," \
                                + ("%f,%f,%f,%f,%f,%f,0,0,0\n" % (ideal, ideal, ideal, ideal, ideal, ideal)))


output_file.close()



# File path : \\wsl.localhost\Ubuntu-20.04\home\ad\omnetpp-5.6\samples\spider_omnet-Copy\scripts\parse_lnd_data.py


import json
import networkx as nx
from config import *
import matplotlib.pyplot as plt
import numpy as np
import collections

# read the relevant JSON and produce some stats
# on the channel distribution
def read_file(filename, file_format="LND"):
    if file_format == "LND":
        lnd_graph, capacity_list = read_file_lnd(filename)
    else:
        lnd_graph, capacity_list =  read_file_c_lightning(filename)
    
    print("Connectedness", nx.is_connected(lnd_graph))
    print("Number of nodes in lnd graph:", lnd_graph.number_of_nodes())
    print("Number of edge in lnd graph:", lnd_graph.number_of_edges())

    capacities = nx.get_edge_attributes(lnd_graph, "capacity")
    capacities = [float(str(c))/SAT_TO_EUR for c in list(capacities.values()) if float(str(c))/SAT_TO_EUR > 2.0]
    print(len(capacities))
    plt.hist(capacities, bins=100, density=True, cumulative=True)
    print(np.mean(np.array(capacities)), "stddev" , np.std(np.array(capacities)), "min", min(capacities), "max", max(capacities))
    plt.show()

    return lnd_graph

# read a json captured from listchannels and listnodes from c-lightning
def read_file_c_lightning(filename):
    capacity_list = []
    node_list = dict()
    lnd_graph = nx.Graph()
    
    with open(filename + "_nodes.json") as f:
        data = json.load(f)
        for i, node in enumerate(data):
            node_list[node["nodeid"]] = i
            
    with open(filename + "_channels.json") as f:
        data = json.load(f)
        
        for edge in data:
            n1 = edge["source"]
            n2 = edge["destination"]
            cap = edge["satoshis"]
            capacity_list.append(cap)

            try: 
                n1_id = node_list[n1]
                n2_id = node_list[n2]

            except:
                print("nodes for edge ", n1, "->", n2, "not found")
                continue

            lnd_graph.add_edge(n1_id, n2_id, capacity=cap)
    return lnd_graph, capacity_list

    
# read a json produced by lncli describegraph from LND
def read_file_lnd(filename):
    capacity_list = []
    with open(filename + ".json") as f:
        lnd_graph = nx.Graph()
        data = json.load(f)

        node_list = dict()

        for i, node in enumerate(data["nodes"]):
            node_list[node["pub_key"]] = i
        
        for edge in data["edges"]:
            n1 = edge["node1_pub"]
            n2 = edge["node2_pub"]
            cap = edge["capacity"]
            capacity_list.append(cap)

            try: 
                n1_id = node_list[n1]
                n2_id = node_list[n2]

            except:
                print("nodes for edge ", n1, "->", n2, "not found")
                continue

            lnd_graph.add_edge(n1_id, n2_id, capacity=cap)
    return lnd_graph, capacity_list

# retrieve the subgraph of nodes that have degree more than the passed number
def remove_nodes_based_on_degree(graph, degree):
    new_nodes = [n for n in graph.nodes() if graph.degree[n] > degree]
    new_graph = graph.subgraph(new_nodes)
    print("Number of nodes in sub graph:", new_graph.number_of_nodes())
    print("Number of edge in sub graph:", new_graph.number_of_edges())

    """ 
    capacities = nx.get_edge_attributes(new_graph, "capacity")
    capacities = [float(str(c))/SAT_TO_EUR for c in capacities.values() if float(str(c))/SAT_TO_EUR > 2.0]
    plt.hist(capacities, bins=100, normed=True, cumulative=True)
    print np.mean(np.array(capacities)), "stddev" , np.std(np.array(capacities)), "min", min(capacities)
    plt.show()
    """

    return new_graph

# plot a histogram of degree distribution for given graph
# from: https://networkx.github.io/documentation/stable/auto_examples/drawing/plot_degree_histogram.html
def plot_degree_distribution(graph):
    degree_cap = {}
    for n, d in graph.degree():
        current_node_sum = 0
        for edge in graph.edges(n, data=True):
            current_node_sum += float(edge[2]["capacity"])/SAT_TO_EUR
        sum_for_this_degree = degree_cap.get(d, 0)
        degree_cap[d] = sum_for_this_degree + current_node_sum

    degree_sequence = sorted([d for n, d in graph.degree()], reverse=True)  # degree sequence
    degreeCount = collections.Counter(degree_sequence)
    deg, cnt = list(zip(*list(degreeCount.items())))
    print(degreeCount)
    print(degree_cap)
    fig, ax = plt.subplots()
    plt.plot(deg, cnt)

    plt.title("Degree Histogram")
    plt.ylabel("Count")
    plt.xlabel("Degree")
    plt.xscale("log")
    plt.show()


# plot a histogram of shortest path between randomly sampled
# sender receiver pairs
def plot_shortest_path_distribution(graph):
    random_tries = 10000
    connected_components = list(nx.connected_components(graph))
    connected_components = [list(c) for c in connected_components if len(c) > 2]
    print(len(connected_components))
    shortest_paths = []
    
    for i in range(random_tries):
        comp_id = np.random.choice(len(connected_components))
        srcdest = np.random.choice(connected_components[comp_id], 2, replace = False)
        src = srcdest[0]
        dest = srcdest[1]
        path = nx.shortest_path(graph, source=src, target=dest)
        shortest_paths.append(len(path) - 1)
    
    shortest_paths.sort()
    path_len_counter = collections.Counter(shortest_paths)
    plen, cnt = list(zip(*list(path_len_counter.items())))
    fig, ax = plt.subplots()
    plt.plot(plen, cnt)

    plt.title("Path Length Histogram")
    plt.ylabel("Count")
    plt.xlabel("Path Length")
    plt.xscale("log")
    plt.show()

#lnd_file_list = ["lnd_dec4_2018", "lnd_dec28_2018", "lnd_july15_2019", "clightning_oct5_2020"]
lnd_file_list = ["clightning_oct5_2020"]
for filename in lnd_file_list:
    file_format = "LND" if "lnd" in filename else "c-lightning"
    graph = read_file(LND_FILE_PATH + filename, file_format)
    new_graph = remove_nodes_based_on_degree(graph, 6)
    plot_degree_distribution(graph)
    plot_shortest_path_distribution(graph)
    nx.write_edgelist(graph, LND_FILE_PATH + filename + ".edgelist")




# File path : \\wsl.localhost\Ubuntu-20.04\home\ad\omnetpp-5.6\samples\spider_omnet-Copy\scripts\parse_per_flow_success.py

import sys
import argparse
import statistics as stat
from config import *
import shlex
import math

delay = 30

parser = argparse.ArgumentParser('Analysis Plots')
parser.add_argument('--topo',
        type=str, 
        required=True,
        help='what topology to generate size summary for')
parser.add_argument('--payment-graph-type',
        type=str, 
        help='what graph type topology to generate summary for', default="circ")
parser.add_argument('--credit',
        type=int,
        help='Credit to collect stats for', default=10)
parser.add_argument('--demand',
        type=int,
        help='Single number denoting the demand to collect data for', default="30")
parser.add_argument('--path-type',
        type=str,
        help='types of paths to collect data for', default="shortest")
parser.add_argument('--path-num',
        type=int,
        help='number of paths to collect data for', default=4)
parser.add_argument('--scheme-list',
        nargs="*",
        help='set of schemes to aggregate results for', default=["priceSchemeWindow"])
parser.add_argument('--save',
        type=str, 
        required=True,
        help='file name to save data in')
parser.add_argument('--num-max',
        type=int,
        help='Single number denoting the maximum number of runs to aggregate data over', default="5")
parser.add_argument('--measurement-interval',
        type=int,
        help='Number of seconds over which success was measured', default="200")


# collect all arguments
args = parser.parse_args()
topo = args.topo
credit = args.credit
demand = args.demand
path_type = args.path_type
num_paths = args.path_num
scheme_list = args.scheme_list


# determine topology and credit type
if "sw" in args.topo or "sf" in args.topo:
    topo_type = args.save[:2]
else:
    topo_type = args.save[:3]

if "lnd_uniform" in args.topo: 
    credit_type = "uniform"
elif "lnd_july15" in args.topo or "lndCap" in args.topo:
    credit_type = "lnd"
else:
    credit_type = "uniform"




output_file = open(GGPLOT_DATA_DIR + args.save, "w+")
output_file.write("Topo,CreditType,Scheme,Credit,SuccVol,Demand\n")
print("scheme objective numberOfZeroes")


# go through all relevant files and aggregate probability by size
for i, scheme in enumerate(scheme_list):
    flow_succ_list, flow_arrival_list = [], []
    for run_num in range(0, args.num_max + 1):
        if credit_type != "uniform" and (scheme == "waterfilling" or scheme == "DCTCPQ" or scheme == "priceSchemeWindow"):
            path_type = "widest"
        else:
            path_type = "shortest"

        file_name = topo + "_" + args.payment_graph_type + "_net_" + str(credit) + "_" + scheme + "_" + \
                args.payment_graph_type + str(run_num) + \
            "_demand" + str(demand/10) + "_" + path_type
        if scheme != "shortestPath":
            file_name += "_" + str(num_paths)
        file_name += "-#0.sca"
        
        # num_completed will always be populated first and the next
        # entry will be arrived for the same flow
        # based on file structure
        with open(RESULT_DIR + file_name) as f:
            for line in f:
                if "->" in line:
                    if "amtCompleted" in line:
                        parts = shlex.split(line)
                        num_completed = float(parts[-1])
                    if "amtArrived" in line:
                        parts = shlex.split(line)
                        num_arrived = float(parts[-1])
                        flow_arrival_list.append(num_arrived / args.measurement_interval)
                        #flow_succ_list.append(num_completed * 100 /num_arrived)
                        flow_succ_list.append(num_completed/ args.measurement_interval)

    sum_fairness, count_zero = 0.0, 0.0
    for entry in sorted(flow_succ_list):
        if entry == 0:
            count_zero += 1
        else:
            sum_fairness += math.log(entry, 2)
        output_file.write(topo_type + "," + credit_type + "," + \
                str(SCHEME_CODE[scheme]) +  "," + str(credit) +  "," + \
            "%f,%f\n" % (entry, demand))
    print(scheme, sum_fairness/(args.num_max + 1), count_zero)

    if i == 0:
        for entry in sorted(flow_arrival_list):
            output_file.write(topo_type + "," + credit_type + \
                    ",arrival," + str(credit) +  "," + \
                "%f,%f\n" % (entry, demand))

output_file.close()



# File Path : \\wsl.localhost\Ubuntu-20.04\home\ad\omnetpp-5.6\samples\spider_omnet-Copy\scripts\parse_probability_size_stats.py

import sys
import argparse
import statistics as stat
from config import *
import shlex
import numpy as np
import math

# figure out what the size buckets should be for a given number of buckets
# say you want 20 buckets, you want to make them equally sized in the number
# of transactions in a bucket (based on the skew of transaction sizes), so the
# larger transactions span a wider range but at the smaller end, the buckets
# are narrower
def compute_buckets(num_buckets, dist_filename):
    amt_dist = np.load(dist_filename)
    num_amts = amt_dist.item().get('p').size
    pdf = amt_dist.item().get('p')
    cdf = np.cumsum(pdf)

    gap = 1.0 / num_buckets
    break_point = gap
    buckets = []

    # return all the bucket end markers
    for i, c in enumerate(cdf):
        if c >= break_point:
            print(break_point, i, c)
            buckets.append(int(round(amt_dist.item().get('bins')[i], 1)))
            break_point += gap
    # buckets.append(int(round(amt_dist.item().get('bins')[-1], 1)))
    print(buckets, len(buckets))
    return buckets

delay = 30

parser = argparse.ArgumentParser('Analysis Plots')
parser.add_argument('--topo',
        type=str, 
        required=True,
        help='what topology to generate size summary for')
parser.add_argument('--payment-graph-type',
        type=str, 
        help='what graph type topology to generate summary for', default="circ")
parser.add_argument('--credit',
        type=int,
        help='Credit to collect stats for', default=10)
parser.add_argument('--demand',
        type=int,
        help='Single number denoting the demand to collect data for', default="30")
parser.add_argument('--path-type',
        type=str,
        help='types of paths to collect data for', default="shortest")
parser.add_argument('--path-num',
        type=int,
        help='number of paths to collect data for', default=4)
parser.add_argument('--scheme-list',
        nargs="*",
        help='set of schemes to aggregate results for', default=["priceSchemeWindow"])
parser.add_argument('--save',
        type=str, 
        required=True,
        help='file name to save data in')
parser.add_argument('--num-max',
        type=int,
        help='Single number denoting the maximum number of runs to aggregate data over', default="5")
parser.add_argument('--num-buckets',
        type=int,
        help='Single number denoting the maximum number of buckets to group txn sizes into', default="20")

# collect all arguments
args = parser.parse_args()
topo = args.topo
credit = args.credit
demand = args.demand
path_type = args.path_type
num_paths = args.path_num
scheme_list = args.scheme_list


output_file = open(GGPLOT_DATA_DIR + args.save, "w+")
output_file.write("Topo,CreditType,Scheme,Credit,SizeStart,SizeEnd,Point,Prob,Demand\n")

buckets = compute_buckets(args.num_buckets, KAGGLE_AMT_DIST_FILENAME)

if "sw" in args.topo or "sf" in args.topo:
    topo_type = args.save[:2]
else:
    topo_type = args.save[:3]

if "lnd_uniform" in args.topo: 
    credit_type = "uniform"
elif "lnd_july15" in args.topo or "lndCap" in args.topo:
    credit_type = "lnd"
else:
    credit_type = "uniform"


# go through all relevant files and aggregate probability by size
for scheme in scheme_list:
    size_to_arrival = {} 
    size_to_completion = {}
    for run_num in range(0, args.num_max + 1):
        if credit_type != "uniform" and (scheme == "waterfilling" or scheme == "DCTCPQ"):
            path_type = "widest"
        else:
            path_type = "shortest"

        file_name = topo + "_" + args.payment_graph_type + "_net_" + str(credit) + "_" + scheme + "_" + \
                args.payment_graph_type + str(run_num) + \
            "_demand" + str(demand/10) + "_" + path_type
        if scheme != "shortestPath":
            file_name += "_" + str(num_paths)
        file_name += "-#0.sca"
        
        try:
            with open(RESULT_DIR + file_name) as f:
                for line in f:
                    if "size" in line:
                        parts = shlex.split(line)
                        num_completed = float(parts[-1])
                        sub_parts = parts[-2].split()
                        size = int(sub_parts[1][:-1])
                        num_arrived = float(sub_parts[3][1:-1]) + 1
                        bucket = buckets[np.searchsorted(buckets, size)]
                        if num_arrived > 0:
                            if num_arrived < num_completed:
                                print("problem with ", scheme, " on run ", run_num)
                                print("Num arrived", num_arrived, "num completed", num_completed, "for size", size)
                                num_arrived = num_completed
                            size_to_arrival[bucket] = size_to_arrival.get(bucket, 0) + num_arrived
                            size_to_completion[bucket] = size_to_completion.get(bucket, 0) + num_completed
        except IOError:
            print("error with", file_name)
            continue
     

    sorted_sizes = [5]
    sorted_sizes.extend(sorted(size_to_completion.keys()))
    print(sorted_sizes)
    for i, size in enumerate(sorted_sizes[1:]):
        output_file.write(topo_type + "," + credit_type + "," + \
                str(SCHEME_CODE[scheme]) +  "," + str(credit) +  "," + \
            "%f,%f,%f,%f,%f\n" % (sorted_sizes[i], size, \
                    math.sqrt(size * sorted_sizes[i]), \
                    size_to_completion[size]/size_to_arrival[size], demand))
output_file.close()



# File path : \\wsl.localhost\Ubuntu-20.04\home\ad\omnetpp-5.6\samples\spider_omnet-Copy\scripts\parse_rebalancing_stats.py

import sys
import argparse
import statistics as stat
from config import *
import os

delay = 30

parser = argparse.ArgumentParser('Analysis Plots')
parser.add_argument('--topo',
        type=str, 
        required=True,
        help='what topology to generate summary for')
parser.add_argument('--payment-graph-type',
        type=str, 
        help='what graph type topology to generate summary for', default="circ")
parser.add_argument('--credit-list',
        nargs="+",
        required=True,
        help='Credits to collect stats for')
parser.add_argument('--demand',
        type=int,
        help='Single number denoting the demand to collect data for', default="30")
parser.add_argument('--path-type-list',
        nargs="*",
        help='types of paths to collect data for', default=["shortest"])
parser.add_argument('--scheduling-alg-list',
        nargs="*",
        help='scheduling algorithms to collect info for', default=[None])
parser.add_argument('--queue-threshold-list',
        nargs="*",
        help='queue thresholds to collect info for', default=[None])
parser.add_argument('--dag-percent-list',
        nargs="*",
        help='dag percents to collect info for', default=[None])
parser.add_argument('--path-num-list',
        nargs="*",
        help='number of paths to collect data for', default=[4])
parser.add_argument('--scheme-list',
        nargs="*",
        help='set of schemes to aggregate results for', default=["priceSchemeWindow"])
parser.add_argument('--save',
        type=str, 
        required=True,
        help='file name to save data in')
parser.add_argument('--num-max',
        type=int,
        help='Single number denoting the maximum number of runs to aggregate data over', default="5")
parser.add_argument('--rebalancing-rate-list',
        nargs="*",
        type=int,
        help='list of rebalancing frequencies', default=[None])

# collect all arguments
args = parser.parse_args()
topo = args.topo
credit_list = args.credit_list
demand = args.demand
path_type_list = args.path_type_list
scheme_list = args.scheme_list
path_num_list = args.path_num_list
queue_threshold_list = args.queue_threshold_list
dag_percent_list = args.dag_percent_list
scheduling_algorithms = args.scheduling_alg_list
rebalancing_rate_list = args.rebalancing_rate_list

output_file = open(GGPLOT_DATA_DIR + args.save, "w+")
if args.payment_graph_type == "circ":
    output_file.write("Scheme,Credit,")
else:
    output_file.write("Scheme,Credit,DAGAmt,")

output_file.write("Topo,CreditType,NumPaths,PathType,SchedulingAlg," + \
        "Threshold,RebalancingRate,SuccRatio,SuccRatioMin,SuccRatioMax,SuccVolume," + \
        "SuccVolumeMin," +\
        "SuccVolumeMax,CompTime,CompTimeMin,CompTimeMax," +
        "RebalancingAmt,RebalancingAmtMin,RebalancingAmtMax,Offloading,OffloadingMin,OffloadingMax\n")

# determine topology and credit type
if "sw" in args.topo or "sf" in args.topo:
    topo_type = args.save[:2]
    num_nodes = 50
else:
    topo_type = args.save[:3]
    num_nodes = 106

if "lnd_uniform" in args.topo: 
    credit_type = "uniform"
elif "lnd_july15" in args.topo or "lndCap" in args.topo:
    credit_type = "lnd"
else:
    credit_type = "uniform"


# go through all relevant files and aggregate info
for credit in credit_list:
    for scheme in scheme_list:
        for path_type in path_type_list:
            if path_type == "widest" and scheme not in ["waterfilling", "DCTCPQ"]:
                continue
            if path_type == "shortest" and len(scheme_list) > 1 and scheme in ["waterfilling", "DCTCPQ"] and \
                    credit_type == "lnd":
                continue

            for queue_threshold in queue_threshold_list:
                for num_paths in path_num_list:
                    for percent in dag_percent_list:
                        for alg in scheduling_algorithms:
                            for rr in rebalancing_rate_list:
                                succ_ratios, succ_vols,comp_times, rebalances, offloadings = [], [], [], [], []
                                for run_num in range(0, args.num_max  + 1):
                                    if args.payment_graph_type == "circ" or percent == '0':
                                        file_name = topo + str(credit) + "_circ" + str(run_num)
                                    else:
                                        file_name = topo + "_dag" + str(percent) + "_" + str(credit) + "_num" + \
                                                str(run_num) 
                                    
                                    file_name += "_delay" + str(delay) + "_demand" + str(demand) + "_" + \
                                            scheme + \
                                            "_" + path_type 

                                    if scheme != "shortestPath":
                                        file_name += "_" + str(num_paths) 
                                        if alg is not None:
                                            file_name += "_" + alg

                                    if rr is not None:
                                        file_name += "_rebalancing1_rate" + str(rr)
                                    
                                    if queue_threshold is not None and percent != '0' and scheme == "DCTCPQ":
                                        file_name += "_qd" + str(queue_threshold)


                                    file_name += "_summary.txt"
                                    
                                    try: 
                                        with open(SUMMARY_DIR + file_name) as f:
                                            for line in f:
                                                if line.startswith("Success ratio"):
                                                    succ_ratio = float(line.split(" ")[4])
                                                elif line.startswith("Success volume"):
                                                    succ_volume = float(line.split(" ")[5])
                                                elif line.startswith("Avg completion time"):
                                                    comp_time = float(line.split(" ")[3][:-1])
                                                elif line.startswith("Amt rebalanced"):
                                                    amt_rebalanced = float(line.split(" ")[2][:-1])/num_nodes
                                                elif line.startswith("Num rebalanced"):
                                                    num_rebalanced = float(line.split(" ")[2][:-1])*MEASUREMENT_INTERVAL
                                                elif line.startswith("Num arrived"):
                                                    num_arrived = float(line.split(" ")[2][:-1])
                                                elif line.startswith("Num completed"):
                                                    num_completed = float(line.split(" ")[2][:-1])
                                            succ_ratios.append(succ_ratio * 100)
                                            succ_vols.append(succ_volume * 100)
                                            comp_times.append(comp_time)
                                            rebalances.append(amt_rebalanced)
                                            
                                            num_failed = num_arrived - num_completed
                                            offloading = (num_rebalanced + num_failed)/num_completed
                                            offloadings.append(offloading)
                                    except IOError:
                                        print("error with " , file_name)
                                        continue

                                if "lndtopo" in args.save and "lnd_credit" in args.save:
                                    capacity = int(credit) * 650
                                elif "lndnewtopo" in args.save and "lnd_credit" in args.save:
                                    capacity = int(credit) * 422
                                else:
                                    capacity = int(credit)



                                
                                if len(succ_ratios) > 0:
                                    if args.payment_graph_type == "circ":
                                        output_file.write(SCHEME_CODE[scheme] + "," + str(capacity) +  ",")
                                    else:
                                        output_file.write(SCHEME_CODE[scheme] + "," + str(capacity) +  "," + \
                                                str(PERCENT_MAPPING[percent]) + ",")

                                    output_file.write(topo_type + "," + credit_type + "," \
                                            + str(num_paths) + "," \
                                        + str(path_type) + "," \
                                        + str(alg) + "," \
                                        + str(queue_threshold) + "," \
                                        + str(rr) + "," \
                                        + ("%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n" % \
                                            (stat.mean(succ_ratios), \
                                        min(succ_ratios), \
                                        max(succ_ratios), stat.mean(succ_vols), min(succ_vols),  \
                                        max(succ_vols), \
                                        stat.mean(comp_times), min(comp_times), max(comp_times), \
                                        stat.mean(rebalances), min(rebalances), max(rebalances),
                                        stat.mean(offloadings), min(offloadings), max(offloadings))))
    

output_file.close()


# File path : \\wsl.localhost\Ubuntu-20.04\home\ad\omnetpp-5.6\samples\spider_omnet-Copy\scripts\parse_sca_files.py

import sys
import shlex
import numpy as np

# parse a line in the scalar file that starts with scalar
def parse_sca_parameter_line(line):
    data = shlex.split(line)
    scalar = data[len(data) - 2]
    value = data[len(data) - 1]
    return scalar + " " + value + "\n"



# parse one line of statistic and update the 
def parse_sca_stat_line(line):
    info = shlex.split(line)
    sender = int(info[1].split('[')[1].split(']')[0])

    signal_type = info[2]
    if "rateArrivedPerDest_Total" in signal_type:
        stat_type = "arrived"
    elif "rateAttemptedPerDest_Total" in signal_type:
        stat_type = "attempted"
    elif "rateCompletedPerDest_Total" in signal_type:
        stat_type = "completed"
    else:
        stat_type = "irrelevant"
        return sender, 0, stat_type

    receiver = int(signal_type.split(" ")[2].split(")")[0])
    return sender, receiver, stat_type

# parse the scalar file in its entirety
# and return the overall success rate among the total arrived and the total attempted
def parse_sca_files(filename):
    parameters = ""
    data = dict()
    with open(filename) as f:
        line = f.readline()
        while line:
            if line.startswith("scalar"):
                if not("queueSize" in line or "completionTime" in line or "amt" in line or "rate" in line or \
                        "Rebalancing" in line or "Amt" in line or "size" in line or "time " in line):
                    parameters += parse_sca_parameter_line(line)

            elif line.startswith("statistic"):
                sender, receiver, stat_type = parse_sca_stat_line(line)
                for i in range(7):
                    line = f.readline()
                    relevant = (stat_type != "irrelevant")
                    if i == 5 and relevant and line.startswith("field"):
                        # field line with sum, remove new line part
                        sum_value = line.split(" ")[2][:-1]
                        cur_data = data.get((sender, receiver), [])
                        cur_data.append((stat_type, sum_value))
                        data[(sender, receiver)] = cur_data    
                    else:
                        continue
            
            line = f.readline()

    # compute completion as a fraction of arrival and attempte
    sum_attempted = 0.0
    sum_arrived = 0.0
    sum_completed = 0.0
    for src_dst_pair, stat in list(data.items()):
        for s in stat:
            stat_type = s[0]
            value = float(s[1])

            if stat_type == "attempted":
                sum_attempted += value
            elif stat_type == "completed":
                sum_completed += value
            else:
                sum_arrived += value

    return parameters, sum_completed/max(sum_arrived, 1.0), sum_completed/max(sum_attempted, 1.0)

def parse_overall_stat_line(line):
    data = shlex.split(line)
    scalar_name = data[len(data) - 2]
    parts = scalar_name.split()
    stat_type = parts[0]
    sender = int(parts[1])
    receiver = int(parts[3])
    value = data[len(data) - 1]
    return sender, receiver, stat_type, value

def parse_simple_stat_line(line):
    data = shlex.split(line)
    scalar_name = data[len(data) - 2]
    parts = scalar_name.split()
    stat_type = parts[0]
    sender = int(parts[-1])
    value = data[len(data) - 1]
    return sender, stat_type, value


def parse_sca_files_overall(filename):
    parameters = ""
    stats = dict()
    stats_3000 = dict()
    amt_added, num_rebalancing = 0, 0
    num_retries, comp_times = [], []
    num_rebalancing_list, amt_rebalanced_list = [], []
    
    with open(filename) as f:
        line = f.readline()
        flag = False
        while line:
            if line.startswith("scalar") and "->" in line:
                #print line, flag
                sender, receiver, stat_name, value = parse_overall_stat_line(line)
                if flag:
                    temp = stats_3000.get((sender, receiver), [])
                    temp.append((stat_name, value))
                    stats_3000[sender, receiver] = temp
                else:
                    temp = stats.get((sender, receiver), [])
                    temp.append((stat_name, value))
                    stats[sender, receiver] = temp
                if "completionTime" in line:
                    flag = False
            elif line.startswith("scalar") and "totalNumRebalancingEvents" in line:
                sender, receiver, stat_name, value = parse_overall_stat_line(line)
                num_rebalancing_list.append(float(value))
                num_rebalancing += float(value)
            elif line.startswith("scalar") and "retries" in line:
                sender, stat_name, value = parse_simple_stat_line(line)
                num_retries.append(float(value))
            elif line.startswith("scalar") and "completion times" in line:
                sender, stat_name, value = parse_simple_stat_line(line)
                comp_times.append(float(value))
            elif line.startswith("scalar") and "totalAmtAdded" in line:
                sender, receiver, stat_name, value = parse_overall_stat_line(line)
                amt_rebalanced_list.append(float(value))
                amt_added += float(value)

            elif "time 3000" in line:
                flag = True
            else:
                flag = False
            line = f.readline()

    # compute completion as a fraction of arrival and attempte
    vol_attempted, num_attempted = 0.0, 0.0
    vol_arrived, num_arrived = 0.0, 0.0
    vol_completed, num_completed = 0.0, 0.0
    completion_time = 0.0

    # clean tail compl times and tries
    comp_times = np.array(comp_times)
    comp_times = comp_times[comp_times != 0]
    num_retries = np.array(num_retries)
    num_retries = num_retries[num_retries != 0]

    for src_dst_pair, stat in list(stats_3000.items()):
        for s in stat:
            stat_type = s[0]
            value = float(s[1])

            if stat_type == "rateAttempted":
                num_attempted += value
            elif stat_type == "amtAttempted":
                vol_attempted += value
            elif stat_type == "rateCompleted":
                num_completed += value
            elif stat_type == "amtArrived":
                vol_arrived += value
            elif stat_type == "amtCompleted":
                vol_completed += value
            elif stat_type == "rateArrived":
                num_arrived += value
            else:
                completion_time += value

    
    if num_arrived > 0 and vol_arrived > 0:
        print("Stats for first 3000 seconds of  ", filename)
        print(" Success ratio over arrived: ", num_completed/num_arrived, " over attempted", \
                num_completed/num_attempted)
        print(" Success volume  over arrived: ", vol_completed/vol_arrived, \
                " over attempted", vol_completed/vol_attempted)
        print(" Avg completion time ", completion_time/num_completed)
        print("Success Rate " + str(num_completed/1000.0))


    vol_attempted, num_attempted = 0.0, 0.0
    vol_arrived, num_arrived = 0.0, 0.0
    vol_completed, num_completed = 0.0, 0.0
    completion_time = 0.0
    str_to_add = ""
    
    for src_dst_pair, stat in list(stats.items()):
        for s in stat:
            stat_type = s[0]
            value = float(s[1])

            if stat_type == "rateAttempted":
                num_attempted += value
            elif stat_type == "amtAttempted":
                vol_attempted += value
            elif stat_type == "rateCompleted":
                num_completed += value
            elif stat_type == "amtArrived":
                vol_arrived += value
            elif stat_type == "amtCompleted":
                vol_completed += value
            elif stat_type == "rateArrived":
                num_arrived += value
            else:
                completion_time += value

    print("Stats for last part of", filename)
    print(" Success ratio over arrived: ", num_completed/num_arrived, " over attempted", num_completed/num_attempted)
    print(" Success volume  over arrived: ", vol_completed/vol_arrived, \
            " over attempted", vol_completed/vol_attempted)
    print(" Avg completion time ", completion_time/num_completed)
    print("Success Rate " + str(num_completed/1000.0))
    print("Amt rebalanced " + str(amt_added)) 
    print("Num rebalanced " + str(num_rebalancing))
    print("Num arrived " + str(num_arrived)) 
    print("Num completed " + str(num_completed)) 

    if len(num_retries) > 0:
        str_to_add = "\nNum retries percentile (99.9) " + str(np.percentile(num_retries, 90))
        print(str_to_add)


    stats_str = "Stats for " + filename + "\nSuccess ratio over arrived: " + str(num_completed/num_arrived) +\
            " over attempted" + str(num_completed/num_attempted) + \
            "\nSuccess volume  over arrived: " + str(vol_completed/vol_arrived) + \
            " over attempted" + str(vol_completed/vol_attempted) + \
            "\nAvg completion time " + str(completion_time/max(num_completed, 1.0)) + \
            "\nSuccess Rate " + str(vol_completed/1000.0) + \
            "\nAmt rebalanced " + str(amt_added) + \
            "\nNum rebalanced " + str(num_rebalancing) + \
            "\nNum arrived " + str(num_arrived) + \
            "\nNum completed " + str(num_completed) + \
            str_to_add 

    return (num_rebalancing_list, amt_rebalanced_list, stats_str) 


# File path : \\wsl.localhost\Ubuntu-20.04\home\ad\omnetpp-5.6\samples\spider_omnet-Copy\scripts\parse_sca_files.py

import sys
import shlex
import numpy as np

# parse a line in the scalar file that starts with scalar
def parse_sca_parameter_line(line):
    data = shlex.split(line)
    scalar = data[len(data) - 2]
    value = data[len(data) - 1]
    return scalar + " " + value + "\n"



# parse one line of statistic and update the 
def parse_sca_stat_line(line):
    info = shlex.split(line)
    sender = int(info[1].split('[')[1].split(']')[0])

    signal_type = info[2]
    if "rateArrivedPerDest_Total" in signal_type:
        stat_type = "arrived"
    elif "rateAttemptedPerDest_Total" in signal_type:
        stat_type = "attempted"
    elif "rateCompletedPerDest_Total" in signal_type:
        stat_type = "completed"
    else:
        stat_type = "irrelevant"
        return sender, 0, stat_type

    receiver = int(signal_type.split(" ")[2].split(")")[0])
    return sender, receiver, stat_type

# parse the scalar file in its entirety
# and return the overall success rate among the total arrived and the total attempted
def parse_sca_files(filename):
    parameters = ""
    data = dict()
    with open(filename) as f:
        line = f.readline()
        while line:
            if line.startswith("scalar"):
                if not("queueSize" in line or "completionTime" in line or "amt" in line or "rate" in line or \
                        "Rebalancing" in line or "Amt" in line or "size" in line or "time " in line):
                    parameters += parse_sca_parameter_line(line)

            elif line.startswith("statistic"):
                sender, receiver, stat_type = parse_sca_stat_line(line)
                for i in range(7):
                    line = f.readline()
                    relevant = (stat_type != "irrelevant")
                    if i == 5 and relevant and line.startswith("field"):
                        # field line with sum, remove new line part
                        sum_value = line.split(" ")[2][:-1]
                        cur_data = data.get((sender, receiver), [])
                        cur_data.append((stat_type, sum_value))
                        data[(sender, receiver)] = cur_data    
                    else:
                        continue
            
            line = f.readline()

    # compute completion as a fraction of arrival and attempte
    sum_attempted = 0.0
    sum_arrived = 0.0
    sum_completed = 0.0
    for src_dst_pair, stat in list(data.items()):
        for s in stat:
            stat_type = s[0]
            value = float(s[1])

            if stat_type == "attempted":
                sum_attempted += value
            elif stat_type == "completed":
                sum_completed += value
            else:
                sum_arrived += value

    return parameters, sum_completed/max(sum_arrived, 1.0), sum_completed/max(sum_attempted, 1.0)

def parse_overall_stat_line(line):
    data = shlex.split(line)
    scalar_name = data[len(data) - 2]
    parts = scalar_name.split()
    stat_type = parts[0]
    sender = int(parts[1])
    receiver = int(parts[3])
    value = data[len(data) - 1]
    return sender, receiver, stat_type, value

def parse_simple_stat_line(line):
    data = shlex.split(line)
    scalar_name = data[len(data) - 2]
    parts = scalar_name.split()
    stat_type = parts[0]
    sender = int(parts[-1])
    value = data[len(data) - 1]
    return sender, stat_type, value


def parse_sca_files_overall(filename):
    parameters = ""
    stats = dict()
    stats_3000 = dict()
    amt_added, num_rebalancing = 0, 0
    num_retries, comp_times = [], []
    num_rebalancing_list, amt_rebalanced_list = [], []
    
    with open(filename) as f:
        line = f.readline()
        flag = False
        while line:
            if line.startswith("scalar") and "->" in line:
                #print line, flag
                sender, receiver, stat_name, value = parse_overall_stat_line(line)
                if flag:
                    temp = stats_3000.get((sender, receiver), [])
                    temp.append((stat_name, value))
                    stats_3000[sender, receiver] = temp
                else:
                    temp = stats.get((sender, receiver), [])
                    temp.append((stat_name, value))
                    stats[sender, receiver] = temp
                if "completionTime" in line:
                    flag = False
            elif line.startswith("scalar") and "totalNumRebalancingEvents" in line:
                sender, receiver, stat_name, value = parse_overall_stat_line(line)
                num_rebalancing_list.append(float(value))
                num_rebalancing += float(value)
            elif line.startswith("scalar") and "retries" in line:
                sender, stat_name, value = parse_simple_stat_line(line)
                num_retries.append(float(value))
            elif line.startswith("scalar") and "completion times" in line:
                sender, stat_name, value = parse_simple_stat_line(line)
                comp_times.append(float(value))
            elif line.startswith("scalar") and "totalAmtAdded" in line:
                sender, receiver, stat_name, value = parse_overall_stat_line(line)
                amt_rebalanced_list.append(float(value))
                amt_added += float(value)

            elif "time 3000" in line:
                flag = True
            else:
                flag = False
            line = f.readline()

    # compute completion as a fraction of arrival and attempte
    vol_attempted, num_attempted = 0.0, 0.0
    vol_arrived, num_arrived = 0.0, 0.0
    vol_completed, num_completed = 0.0, 0.0
    completion_time = 0.0

    # clean tail compl times and tries
    comp_times = np.array(comp_times)
    comp_times = comp_times[comp_times != 0]
    num_retries = np.array(num_retries)
    num_retries = num_retries[num_retries != 0]

    for src_dst_pair, stat in list(stats_3000.items()):
        for s in stat:
            stat_type = s[0]
            value = float(s[1])

            if stat_type == "rateAttempted":
                num_attempted += value
            elif stat_type == "amtAttempted":
                vol_attempted += value
            elif stat_type == "rateCompleted":
                num_completed += value
            elif stat_type == "amtArrived":
                vol_arrived += value
            elif stat_type == "amtCompleted":
                vol_completed += value
            elif stat_type == "rateArrived":
                num_arrived += value
            else:
                completion_time += value

    
    if num_arrived > 0 and vol_arrived > 0:
        print("Stats for first 3000 seconds of  ", filename)
        print(" Success ratio over arrived: ", num_completed/num_arrived, " over attempted", \
                num_completed/num_attempted)
        print(" Success volume  over arrived: ", vol_completed/vol_arrived, \
                " over attempted", vol_completed/vol_attempted)
        print(" Avg completion time ", completion_time/num_completed)
        print("Success Rate " + str(num_completed/1000.0))


    vol_attempted, num_attempted = 0.0, 0.0
    vol_arrived, num_arrived = 0.0, 0.0
    vol_completed, num_completed = 0.0, 0.0
    completion_time = 0.0
    str_to_add = ""
    
    for src_dst_pair, stat in list(stats.items()):
        for s in stat:
            stat_type = s[0]
            value = float(s[1])

            if stat_type == "rateAttempted":
                num_attempted += value
            elif stat_type == "amtAttempted":
                vol_attempted += value
            elif stat_type == "rateCompleted":
                num_completed += value
            elif stat_type == "amtArrived":
                vol_arrived += value
            elif stat_type == "amtCompleted":
                vol_completed += value
            elif stat_type == "rateArrived":
                num_arrived += value
            else:
                completion_time += value

    print("Stats for last part of", filename)
    print(" Success ratio over arrived: ", num_completed/num_arrived, " over attempted", num_completed/num_attempted)
    print(" Success volume  over arrived: ", vol_completed/vol_arrived, \
            " over attempted", vol_completed/vol_attempted)
    print(" Avg completion time ", completion_time/num_completed)
    print("Success Rate " + str(num_completed/1000.0))
    print("Amt rebalanced " + str(amt_added)) 
    print("Num rebalanced " + str(num_rebalancing))
    print("Num arrived " + str(num_arrived)) 
    print("Num completed " + str(num_completed)) 

    if len(num_retries) > 0:
        str_to_add = "\nNum retries percentile (99.9) " + str(np.percentile(num_retries, 90))
        print(str_to_add)


    stats_str = "Stats for " + filename + "\nSuccess ratio over arrived: " + str(num_completed/num_arrived) +\
            " over attempted" + str(num_completed/num_attempted) + \
            "\nSuccess volume  over arrived: " + str(vol_completed/vol_arrived) + \
            " over attempted" + str(vol_completed/vol_attempted) + \
            "\nAvg completion time " + str(completion_time/max(num_completed, 1.0)) + \
            "\nSuccess Rate " + str(vol_completed/1000.0) + \
            "\nAmt rebalanced " + str(amt_added) + \
            "\nNum rebalanced " + str(num_rebalancing) + \
            "\nNum arrived " + str(num_arrived) + \
            "\nNum completed " + str(num_completed) + \
            str_to_add 

    return (num_rebalancing_list, amt_rebalanced_list, stats_str) 



\\wsl.localhost\Ubuntu-20.04\home\ad\omnetpp-5.6\samples\spider_omnet-Copy\scripts\parse_vec_files_for_ggplot.py

import os
import sys
from config import *
from parse_vec_files import *
import argparse
import numpy as np
import math

parser = argparse.ArgumentParser(prog='timeseries or relevant stats in ggplot')
parser.add_argument('--filename',
        type=str,
        help='Single vector file for a particular run using the omnet simulator')
parser.add_argument('--output-prefix',
        type=str,
        required=True,
        help='output prefix')
parser.add_argument('--scheme-list',
        nargs="*",
        help='set of schemes to aggregate results for', default=["priceSchemeWindow"])
parser.add_argument('--demand',
        type=int,
        help='Demand amount used for this experiment', default=30)
parser.add_argument('--start-time',
        type=int,
        required=True,
        help='Consider points after this second')
parser.add_argument('--credit-list',
        nargs="+",
        required=True,
        help='Credits to collect stats for')
parser.add_argument('--end-time',
        type=int,
        required=True,
        help='Consider points until this second')
parser.add_argument('--num-max',
        type=int,
        help='Single number denoting the maximum number of runs to aggregate data over', default="5")
parser.add_argument('--attr-list',
        nargs='+',
        help='List of attributes to plot')
parser.add_argument('--summary',
        action='store_true',
        help='whether to produce summary across all src dest')
parser.add_argument('--topo',
        type=str, 
        help='what topology to generate size summary for')

args = parser.parse_args()
num_paths = 4

signal_map={"queue": "numInQueue", "inflight": "sumOfTransUnitsInFlight", "rate": "rateToSendTransPerDest"}


# parse the entire file starting from the declarations making a map of them
# and then parsing the data lines one by one
# everytime store the data in a map, so its easy to extract all the useful signals at the end in one shot
def parse_for_timeseries(filename, start_time, end_time, node_type, src, dest, relevant_signal):
    vec_id_map = dict()
    last_vec_id = -1 
    dataPoints = list()
    parameters = ""

    with open(filename) as f:
        for line in f:
            if line.startswith("param"):
                parameters += (line.split("**")[1])
            if line.startswith("vector"):
                vec_id, vec_info = parse_vec_declaration(line)
                vec_id_map[vec_id] = vec_info

            if line[0].isdigit():
                columns = line.split("\t")
                if len(columns) == 4:
                    cur_vec_id = columns[0]
                    signal_name = vec_id_map[int(columns[0])][2]
                    if (node_type == "host" and "_" in signal_name and "Total" not in signal_name):
                        path_id = int(signal_name.split("_")[1])
                    else: 
                        path_id = 0
                    if (cur_vec_id != last_vec_id and is_relevant(int(columns[0]), vec_id_map, relevant_signal)):
                        cur_vec_id = last_vec_id
                    if cur_vec_id == last_vec_id:
                        data = parse_vec_data_line(columns)
                        if data[0] != None:
                            tv_val = data[1]
                            vec_id = data[0]
                            if((vec_id_map[vec_id][1] == node_type) and (vec_id_map[vec_id][4] == node_type)
                                and ((tv_val[0] >= start_time) and (tv_val[0]<= end_time))):
                                    if (src == dest or 
                                            (vec_id_map[vec_id][0] == src and vec_id_map[vec_id][3] == dest)):
                                        dataPoints.append((tv_val[0], tv_val[1], path_id))
    return dataPoints


# check if this field is actually one of the fields we want to process
def is_relevant(vec_id, vec_id_to_info_map, relevant_signal):
    vec_info = vec_id_to_info_map[vec_id]

    if vec_info[2].startswith(relevant_signal):
        return True
    return False


""" collect time inflight data across schemes and edges at different credit list"""
def aggregate_across_files(scheme_list, credit_list, topo):
    out_file = open(GGPLOT_DATA_DIR + args.output_prefix + "_token_time_inflight_data", 'w+')
    out_file.write("scheme,credit,values\n")
    for credit in credit_list:
        for scheme in scheme_list:
            path_type = "shortest" #if scheme in ["waterfilling", "DCTCPQ"] else "shortest"
            scheduling_alg = "FIFO" if scheme in ["celer"] else "LIFO"

            for run_num in range(0, args.num_max  + 1):
                file_name = topo + "_circ_net_" + str(credit) + "_" + str(scheme) + "_circ"
                file_name += str(run_num) + "_demand" + str(demand/10) + "_" + path_type
                if scheme != "shortestPath":
                    file_name += "_" + str(num_paths) 
                file_name += "_" + scheduling_alg + "-#0.vec"
                                
                try: 
                    all_timeseries, vec_id_to_info_map, parameters = parse_vec_file(RESULT_DIR + file_name, \
                            "timeInFlight")
                    
                    for vec_id, timeseries in list(all_timeseries.items()):
                        vector_details = vec_id_to_info_map[vec_id]
                        src_node = vector_details[0]
                        src_node_type = vector_details[1]
                        dest_node_type = vector_details[4]
                        dest_node = vector_details[3]
                        
                        if ("timeInFlight" not in vector_details[2]):
                            continue
                        if (src_node_type != "router" or dest_node_type != "router"):
                            continue

                        values = [t[1] for t in timeseries if t[0] > args.start_time and 
                                t[0] < args.end_time and not math.isnan(t[1])]
                        if len(values) > 0:
                            out_file.write(scheme + "," + str(credit) + "," + str(np.average(values)) + "\n")
                except IOError:
                    print("error with " , file_name)
                    continue
    out_file.close()


if __name__ == "__main__":
    demand = args.demand
    start_time = args.start_time
    end_time = args.end_time
    
    """ parse single vector file """
    if args.filename is not None:
        vec_filename = args.filename
        scheme = args.scheme_list[0]
        new_list = []
        attr_list = []
        attr_set = set()
        summary = args.summary
        for i, a in enumerate(args.attr_list):
            new_list.append(a)
            if (i + 1)  % 3 == 0:
                attr_list.append(new_list)
                new_list = []
            if (i % 3) == 0:
                attr_set.add(a)

        for e in attr_set:
            os.system('rm ' + args.output_prefix + '_' + e + "_data")
            out_file = open(args.output_prefix + "_" + e + "_data", 'w+')
            if summary:
                out_file.write("scheme,demand,")
            else:
                out_file.write("scheme,demand,src,dest,")
            if e != "queue":
                out_file.write("path,time,value\n")
            else:
                out_file.write("time,value\n")

        if summary:
            for e in attr_set:
                parameter = e
                print(signal_map[parameter])
                node_type = "router" if parameter == "queue" else "host"
                data_points = parse_for_timeseries(vec_filename, start_time, end_time, node_type, 0, 0, \
                        signal_map[parameter])
                out_file = open(args.output_prefix + "_" + parameter + "_data", 'a+')
                for x in data_points:
                    path = x[2]
                    out_file.write(scheme + "," + str(demand) + ",")
                    if node_type == "host":
                        out_file.write("P" + str(path) + "," + str(x[0]) + "," + str(x[1]) + "\n")
                    else:
                        out_file.write(str(x[0]) + "," + str(x[1]) + "\n")
                out_file.close()
                print(max(data_points))
        else:
            for entry in attr_list:
                parameter = entry[0]
                src, dest = int(entry[1]), int(entry[2])
                print(signal_map[parameter])
                node_type = "router" if parameter == "queue" else "host"
                data_points = parse_for_timeseries(vec_filename, start_time, end_time, node_type, src, dest, \
                        signal_map[parameter])
                out_file = open(args.output_prefix + "_" + parameter + "_data", 'a+')
                for x in data_points:
                    path = x[2]
                    out_file.write(scheme + "," + str(demand) + "," + str(src) + "," + str(dest) + ",")
                    if node_type == "host":
                        out_file.write("P" + str(path) + "," + str(x[0]) + "," + str(x[1]) + "\n")
                    else:
                        out_file.write(str(x[0]) + "," + str(x[1]) + "\n")
                out_file.close()
                print(max(data_points))
                
    else:
        """ aggregate stats for a bunch of files with a common prefix and for a set of schemes """
        aggregate_across_files(args.scheme_list, args.credit_list, args.topo)


\\wsl.localhost\Ubuntu-20.04\home\ad\omnetpp-5.6\samples\spider_omnet-Copy\scripts\parse_vec_files.py


import os
from config import *
from parse_vec_files import *

# parse a vector declaration line that maps a vector id to the signal that it is recording
def parse_vec_declaration(line):
    words = line.split(" ")
    if words[0] != "vector":
        print("Invalid line no vector")
        return None, "Invalid line"


    vector_id = int(words[1])
    owner_node = int(words[2].split("[")[1].split("]")[0])
    owner_node_type = words[2].split(".")[1].split("[")[0]

    signal_info = words[3].split('"')[1].split("(")[0] if'"' in words[3] else words[3].split(":")[0] 
    dest_node = -1
    dest_node_type = None
    if words[3].endswith("router"):
        dest_node = int(words[4]) 
        dest_node_type = "router"
        # the actual node number int(words[5].split("[")[1].split("]")[0])
    elif not signal_info.startswith("completionTime"): # dont know what this is
        if words[4] == 'node':
            dest_node = int(words[5].split(")")[0])
        else: 
            dest_node = int(words[4].split(")")[0])
        dest_node_type = "host"


    return vector_id, (owner_node, owner_node_type, signal_info, dest_node, dest_node_type)


# parse a line that is guaranteed to comprise of 4 columns the first of which is the vector id, second is
# event ID (that we are ignoring right now), third is the timestamp at which the signal
# was recorded and the 4th is the value recorded
def parse_vec_data_line(columns):
    try:
        vec_id = int(columns[0])
        event_id = float(columns[1])
        time = float(columns[2])
        value = float(columns[3])
        return vec_id, (time, value)

    except:
        print("Bad line for reading vec data: ", columns)
        return None, None, None

# parse the entire file starting from the declarations making a map of them
# and then parsing the data lines one by one
# everytime store the data in a map, so its easy to extract all the useful signals at the end in one shot
def parse_vec_file(filename, plot_type):
    vec_id_map = dict()
    timeseries = dict()
    last_vec_id = -1 
    parameters = ""

    with open(filename) as f:
        for line in f:
            if line.startswith("param"):
                parameters += (line.split("**")[1])
            if line.startswith("vector"):
                vec_id, vec_info = parse_vec_declaration(line)
                vec_id_map[vec_id] = vec_info

            if line[0].isdigit():
                columns = line.split("\t")
                if len(columns) == 4:
                    cur_vec_id = columns[0]
                    if (cur_vec_id != last_vec_id and is_interesting(int(columns[0]), vec_id_map, plot_type)):
                        cur_vec_id = last_vec_id
                    if cur_vec_id == last_vec_id:
                        data = parse_vec_data_line(columns)
                        if data[0] != None:
                            cur_timeseries = timeseries.get(data[0], [])
                            cur_timeseries.append(data[1])
                            timeseries[data[0]] = cur_timeseries

    return timeseries, vec_id_map, parameters


# check if this field is actually one of the fields we want to process
def is_interesting(vec_id, vec_id_to_info_map, plot_type):
    vec_info = vec_id_to_info_map[vec_id]

    for s in INTERESTING_SIGNALS[plot_type]:
        if vec_info[2].startswith(s):
            return True
    return False


\\wsl.localhost\Ubuntu-20.04\home\ad\omnetpp-5.6\samples\spider_omnet-Copy\scripts\plot_rebalancing_dist.py

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import sys
from parse_sca_files import *
import argparse
from config import *
import numpy

delay = 30
RESULT_PATH_PREFIX = "../benchmarks/dag"

parser = argparse.ArgumentParser('Analysis Plots')
parser.add_argument('--topo',
        type=str, 
        required=True,
        help='what topology to generate summary for')
parser.add_argument('--credit-list',
        nargs="+",
        required=True,
        help='Credits to collect stats for')
parser.add_argument('--demand',
        type=int,
        help='Single number denoting the demand to collect data for', default="30")
parser.add_argument('--scheduling-alg',
        type=str,
        help='scheduling algorithm to collect info for', default="LIFO")
parser.add_argument('--path-type',
        type=str,
        help='types of paths to collect info for', default="widest")
parser.add_argument('--queue-threshold',
        type=int,
        help='queue threshold to collect info for', default=None)
parser.add_argument('--dag-percent',
        type=int,
        help='dag percents to collect info for', default=[None])
parser.add_argument('--path-num',
        type=int,
        help='number of paths to collect data for', default=4)
parser.add_argument('--scheme-list',
        nargs="*",
        help='set of schemes to aggregate results for', default=["DCTCPQ"])
parser.add_argument('--save',
        type=str, 
        required=True,
        help='file name to save data in')
parser.add_argument('--num-max',
        type=int,
        help='Single number denoting the maximum number of runs to aggregate data over', default="5")
parser.add_argument('--rebalancing-rate-list',
        nargs="*",
        type=int,
        help='list of rebalancing frequencies', default=[None])

# collect all arguments
args = parser.parse_args()
topo = args.topo
credit_list = args.credit_list
demand = args.demand
path_type = args.path_type
scheme_list = args.scheme_list
num_paths = args.path_num
queue_threshold = args.queue_threshold
dag_percent = args.dag_percent
scheduling_alg = args.scheduling_alg
rebalancing_rate_list = args.rebalancing_rate_list

if "lnd_uniform" in args.topo: 
    credit_type = "uniform"
elif "lnd_july15" in args.topo or "lndCap" in args.topo:
    credit_type = "lnd"
else:
    credit_type = "uniform"

# compute how many nodes contribute to how much of the rebalancing
def compute_x_y(array):
    sorted_array = sorted(array)
    cum_sum = numpy.cumsum([i / sum(sorted_array) for i in sorted_array])
    new_x = [float(i) / len(array) for i, _ in enumerate(array)]
    return new_x, cum_sum


# plots histogram
def plot_rebalancing_dist(amt_list, num_list, alg):
    n_bins = 50
    fig, axs = plt.subplots(1, 2, tight_layout=True)

    amt_x_y = compute_x_y(amt_list)
    axs[0].plot(amt_x_y[0], amt_x_y[1])
    axs[0].set_xlabel("Fraction of nodes")
    axs[0].set_ylabel("Contribution to total amount rebalanced")
    
    num_x_y = compute_x_y(num_list)
    axs[1].plot(num_x_y[0], num_x_y[1])
    axs[1].set_xlabel("Fraction of nodes")
    axs[1].set_ylabel("Contribution to number of rebalancing events")
   
    st = plt.suptitle("Rebalancing Distribution for " + alg)
    # shift subplots down:
    st.set_y(0.98)
    fig.subplots_adjust(top=0.65)
    fig.savefig(args.save)

# go through all relevant files and aggregate probability by size
aggregate_rebalancing_amt_list = []
aggregate_rebalancing_num_list = []
for scheme in scheme_list:
    for run_num in range(0, args.num_max + 1):
        for credit in credit_list:
            for rebalancing_rate in rebalancing_rate_list:
                if credit_type != "uniform" and (scheme == "waterfilling" or scheme == "DCTCPQ"):
                    path_type = "widest"
                else:
                    path_type = "shortest"

                file_name = topo + "_dag" +  str(dag_percent) + "_net_" + str(credit) + "_" + scheme + "_" + \
                        "dag" + str(run_num) + \
                    "_demand" + str(demand/10) + "_" + path_type
                if scheme != "shortestPath":
                    file_name += "_" + str(num_paths)
                file_name += "_" + scheduling_alg + "_rebalancing1_rate" + str(rebalancing_rate) 
                if queue_threshold is not None and scheme == "DCTCPQ":
                    file_name += "_qd" + str(queue_threshold)
                file_name += "-#0.sca"

                # collect stats
                result_path = RESULT_PATH_PREFIX + str(dag_percent) + "/results/"
                (rebalancing_amt_list, rebalancing_num_list, stats_str) = parse_sca_files_overall(result_path + file_name)
                norm_rebalancing_amt = [x / sum(rebalancing_amt_list)  for x in rebalancing_amt_list]
                norm_rebalancing_num = [x / sum(rebalancing_num_list)  for x in rebalancing_num_list]
                aggregate_rebalancing_amt_list.extend(norm_rebalancing_amt)
                aggregate_rebalancing_num_list.extend(norm_rebalancing_num)
    
    # plot distribution
    alg = "Spider" if scheme == "DCTCPQ" else "LND"
    plot_rebalancing_dist(aggregate_rebalancing_amt_list, aggregate_rebalancing_num_list, alg)



\\wsl.localhost\Ubuntu-20.04\home\ad\omnetpp-5.6\samples\spider_omnet-Copy\scripts\README.md

#### Benchmark Generation
To generate the benchmarks, run one of the two commands depending on your needs.
```
    generate_circulation_benchmarks.sh
    generate_dag_benchmarks.sh
```
Make sure that the scheme that is run is "DCTCPQ" in order to reproduce Spider's algorithm. Some more examples of a cleaner experiment routine
can be found in the `refactor-scripts` branch.
These two basically go through all the types of graphs: really small ones to much larger ones and generate
the topology, workload, ini files and ned files for every one of them. The circulations are placed in 
`../benchmarks/circulations` and the dags are placed in `../benchmarks/dagx` where `x` denotes the dag percentage
which is also supplied in line 18 of the `generate_dag_benchmarks.sh`. If you want fewer topologies to be newly generated, tamper only with the `prefix` array.


#### Running benchmarks
To run a subset of these benchmarks, two sample scripts exist in the parent directory to this. It is important
you run them only from the parent directory since otherwise, the directory indexing gets messed with.
```
    run_circulations.sh
    run_dag.sh
```
They both ensure that the hostNed and routerNed files are supplied in the right directory and then run the 
spiderNet executable on appropriate workloads. The results generated go into the corresponding 
`benchmarks/x/results/` where `x` depends on whether it is a circulation or a dag. The filenames of the .vec files
are determined by the config that was run.

To run a particular topology, workload or config file (let's say its prefix is `sample` and it 
is a circulation workload), 
run this command from the parent directory to this directory,
```
./spiderNet -u Cmdenv -f benchmarks/circulations/sample.ini -c sample -n benchmarks/circulations
```
The results of this run will be in `benchmarks/circulations/results/sample-#0.vec`.


#### Visualizing CDFs
To visualize the CDFs and compare them across a few different runs, you can use the following command:
```
python generate_summary_cdf.py --vec_files ../benchmarks/circulations/results/wf/sample-#0.vec ../benchmarks/circulations/results/shortest/sample-#0.vec --labels "wf" "shortest"  --save 400_nodes_sw.pdf
```
The --vec_files specifies the vector files holding the results. Make sure to put the waterfilling run in a `wf` subfolder as of now, since that is how the plotting
script differnetiates between waterfilling and shortest paths. The --labels then mention the legend names and the
--save denotes the pdf file to plot to. Following this, the pdf file can be viewed/downloaded and so on.


#### Analyzing a single run similar to IDE
An example command to generate the analysis plots for the above described `sample` run would be:
```
python generate_analysis_plots_for_single_run.py --vec_file ../benchmarks/circulations/results/sample-#0.vec --balance --save sample --queue_info --timeouts --frac_completed
```
In particular, this generates two pdf files. The first will be called `sample_per_channel_info.pdf` which will contain the time series of the balances and the queue information across all payment channels. Every router has its own page and unique plot in this pdf with all of its payment channels on that plot. If you want more information other than the balances
and the queue, you can look at other flags supported or add your own and include support for parsing the right signals associated with it.

 The second will be called `sample_per_src_dest_info.pdf` which will contain the time series of the fraction of transactions completed and the number of transactions that timed out between every source and destination. Every host has its own page and unique plot in this pdf with all of its destinations on that plot. If you want more information other than the timeouts or fraction completed you can look at other flags supported or add your own and include support for parsing the right signals associated with it.




    
 
\\wsl.localhost\Ubuntu-20.04\home\ad\omnetpp-5.6\samples\spider_omnet-Copy\scripts\snowball.py

""" code to sample a small subgraph of a given input graph via 
the snowball sampling method.
Ref: https://arxiv.org/pdf/1308.5865.pdf """


import matplotlib.pyplot as plt 

import copy
import networkx as nx 
import random 
from config import *
import matplotlib.pyplot as plt
import numpy as np

random.seed(SEED)

def sample_ngbr(graph, ngbr_set, v_set, k):
    """ for each vertex in ngbr_set sample k random neighbors
    in graph """
    
    new_ngbr_set = set() 
    new_edge_set = set()

    for v in ngbr_set:
        ngbr = [u for u in graph.neighbors(v)]
        if len(ngbr) > k:
            ngbr_sample = set(random.sample(ngbr, k))
        else:
            ngbr_sample = set(ngbr)
        new_ngbr_set = new_ngbr_set | ngbr_sample
        new_edge_set = new_edge_set | {(u, v) for u in ngbr_sample}
    
    new_ngbr_set = new_ngbr_set - v_set
    return new_ngbr_set, new_edge_set



def snowball_sample(graph, init_seed, max_sample_size=100, k=4):
    """ sample a subgraph no more than max_sample_size starting with 
    vertices in init_seed """

    if len(graph.nodes()) <= max_sample_size:
        return graph

    assert len(init_seed) <= max_sample_size

    v_set = copy.deepcopy(init_seed)
    e_set = set()
    old_v_set = v_set
    old_e_set = e_set
    ngbr_set = copy.deepcopy(init_seed)

    while len(v_set) <= max_sample_size:
        new_ngbr_set, new_edge_set = sample_ngbr(graph, ngbr_set, v_set, k)
        old_v_set = v_set.copy()
        old_e_set = e_set.copy()
        v_set = v_set | new_ngbr_set
        e_set = e_set | new_edge_set
        ngbr_set = new_ngbr_set

    sampled_graph = nx.Graph()
    sampled_graph.add_nodes_from(list(old_v_set))
    sampled_graph.add_edges_from(list(old_e_set))

    for e in sampled_graph.edges():
        sampled_graph.edges[e]['capacity'] = graph.edges[e]['capacity']

    return sampled_graph



def prune_deg_one_nodes(sampled_graph):
    """ prune out degree one nodes from graph """
    deg_one_nodes = []
    for v in sampled_graph.nodes():
        if sampled_graph.degree(v) == 1:
            deg_one_nodes.append(v)

    for v in deg_one_nodes:
        sampled_graph.remove_node(v)

    return sampled_graph



def write_capacities_to_file(filename, capacities):
    with open(filename, "w+") as f:
        f.write("values\n")
        for c in capacities:
            f.write(str(c) + "\n")


#lnd_file_list = ["lnd_dec4_2018", "lnd_dec28_2018"]
#lnd_file_list = ["lnd_july15_2019"]
lnd_file_list = ["clightning_oct5_2020"]
for filename in lnd_file_list: 
    graph = nx.read_edgelist(LND_FILE_PATH + filename + ".edgelist")

    rename_dict = {v: int(str(v)) for v in graph.nodes()}
    graph = nx.relabel_nodes(graph, rename_dict)

    # convert all capacities to EUROS
    count = 0
    for e in graph.edges():
        edge_cap = round(graph.edges[e]['capacity'] / SAT_TO_EUR)
        if edge_cap < 10:
            edge_cap = 10
            count += 1
        graph.edges[e]['capacity'] = edge_cap
    print("massaged", count, "out of", len(graph.edges()), "edges")
    
    init_seed = {784, 549, 989}

    """ max_sample_size is the maximum size of sampled graph. Returned graph 
    might be smaller than that. k is how many neighbors to sample (by each node)
    in each round """
    sampled_graph = snowball_sample(graph, init_seed, max_sample_size=1000, k=12)

    """ prune out degree one nodes until there are no degree one nodes """
    graph_size = len(sampled_graph.nodes()) + 1
    while graph_size > len(sampled_graph.nodes()):
        graph_size = len(sampled_graph.nodes())
        sampled_graph = prune_deg_one_nodes(sampled_graph)

    
    """ make all node numbers start from 0 """
    numbered_graph = nx.convert_node_labels_to_integers(sampled_graph)
    print("graph size: ", numbered_graph.number_of_nodes(), " nodes" , \
            numbered_graph.number_of_edges(), " edges")

    nx.write_edgelist(numbered_graph, LND_FILE_PATH + filename + "_reducedsize" + ".edgelist")
        
    capacities = nx.get_edge_attributes(sampled_graph, 'capacity')
    capacities = [float(c) for c in list(capacities.values())]
    write_capacities_to_file(filename + "_data_min25", capacities)
    #plt.hist(capacities, bins=100, normed=True, cumulative=True)
    print(np.mean(np.array(capacities)), "stddev" , np.std(np.array(capacities),), min(capacities), \
            np.median(np.array(capacities)), np.percentile(np.array(capacities), 25))
    #plt.show()




\\wsl.localhost\Ubuntu-20.04\home\ad\omnetpp-5.6\samples\spider_omnet-Copy\scripts\visualize_kaggle.py


import numpy as np
import matplotlib.pyplot as plt
import math

amt_dist = np.load('data/amt_dist.npy')
sizes = amt_dist.item().get('bins')
prob = amt_dist.item().get('p')
index = -1

cdf = []
cdf.append(prob[0])
for i in range(1, len(prob)):
    cdf.append(prob[i] + cdf[i - 1])
    if sizes[i] > 250 and index == -1:
        index = i

#plt.plot(sizes, cdf)
#plt.show()

cut_off_sizes = sizes[:index + 1]
cut_off_pdfs = prob[:index + 1]
new_pdf = [x/sum(cut_off_pdfs) for x in cut_off_pdfs]
mean = np.average(cut_off_sizes, weights=new_pdf)

np.save('data/amt_dist_cutoff.npy', np.array({'p':np.array(new_pdf), 'bins':np.array(cut_off_sizes)}))



new_cdf = []
new_cdf.append(new_pdf[0])
exp_x2 = ((sizes[0] - mean) ** 2) * new_pdf[0]
for i in range(1, len(new_pdf)):
    new_cdf.append(new_pdf[i] + new_cdf[i - 1])
    exp_x2 += ((sizes[i] - mean) ** 2) * new_pdf[i]

#plt.plot(cut_off_sizes, new_cdf)
#plt.show()

print(math.sqrt(exp_x2))
print(mean)



