import os
import docker
import time
import multiprocessing
import sys
import psutil

from classes.rm_classes import *
from functions.mapping import *
from functions.deploy import *
from functions.apps import *
from functions.stats import *


        
##############################################################################################################################
#main function
##############################################################################################################################


strategy = rmStrategy(sys.argv[1]) # rm run with: "python3 rm AMD64" for example

#init info from machine
architecture = rmMachine(sys.argv[2], strategy) #AMD24 or AMD64 or INTEL20 or INTEL44

if(architecture.name == "AMD64"): #if is is an AMD architecture, will get energy consumption fromthe script
    os.system("./AMD/get_energy.sh &")

#init info to be used to run a new container
dockerRun = rmDockerCommand(architecture)

#initialize client do DOCKER API
dockerEnv = docker.from_env()

#initialize thread mapping
threadMapping = rmMapping(architecture, strategy)

#Variable to keep the list of applications that will be executed
listApps = []
listContainers = []


print("############################################################## TT-Autoscaling ##############################################################")


while True:
    #read applications from the list
    rmInitializeApps(listApps)
    #print initial mapping
    rmPrintMapping(threadMapping)
    #deploy a new container if there is enough available resources
    rmDeployContainer(dockerRun, listApps, architecture, threadMapping)

    #sleep time to get the informations from the infrastructure
    time.sleep(15) 

    #get the status of all active containers
    rmStatusContainers(dockerEnv, architecture, listContainers)
    #adjust the mapping of containers
    rmMapContainers(architecture, listContainers, threadMapping)
    #check and remove containers that were already executed from the list
    rmCheckContainers(listContainers, architecture, dockerEnv, threadMapping)
    #adjust the mapping of containers after removing a container from execution
    rmAdjustMapping(threadMapping, listApps, architecture, listContainers, dockerEnv)

    #check if there is any active container
    isRun = rmFinalize(threadMapping, architecture, listApps)
    if(isRun == False):
        break
    

#terminates the get_energy process that was reading energy consumption for the entire execution.
if(architecture.name == "AMD64"):
    processName = "get_energy"
    for proc in psutil.process_iter():
        if processName.lower() in proc.name().lower():
            print("Kill "+str(proc.name))
            os.system("kill "+str(proc.pid))

