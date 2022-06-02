from classes.rm_classes import *
from functions.mapping import *
from os.path import exists
import docker


def rmStatusContainers(dockerEnv, architecture, listContainers):
    for index in dockerEnv.containers.list():
        id = index.id
        found = False
        for i in listContainers:
            if (i.id == id):
                found = True
                break
        if(found == False):
            name = index.name
            fileContainer = "/sys/fs/cgroup/cpu/docker/"+id+"/tasks" #for reproducibility, please change here if the information of docker is not stored in this path
            if (os.path.exists(fileContainer)):
                tlp = sum(1 for line in open(fileContainer)) -1
                mapped = False
                status = index.status
                listContainers.append(rmContainer(id, name, tlp, mapped, status))



def rmPrintMapping(threadMapping):
    print(threadMapping.mapCores)
    print(threadMapping.statusMapCores)
    print(threadMapping.nameAppMapCores)


def rmFinalize(threadMapping, architecture, listApps):
    i = 0
    while(i < architecture.total):
        if(threadMapping.statusMapCores[i] != 0):
            return True
        i= i + 1
    
    if len(listApps):
        return True        

    return False


def rmCheckContainers(listContainers, architecture, dockerEnv, threadMapping):
    for indexCont in listContainers:
        alive = False
        for indexDock in dockerEnv.containers.list():
            if(indexCont.id == indexDock.id):
                alive = True
                indexCont.status = "Running"
                break
        if(alive == False):
            name = indexCont.name
            listContainers.remove(indexCont)
            rmUpdateMappingAfterKill(name, threadMapping, architecture)
