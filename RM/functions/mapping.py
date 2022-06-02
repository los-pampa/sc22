import os
from classes.rm_classes import *


def rmIsCoreAvailable(threadMapping):
    for i in threadMapping.statusMapCores:
        if(i == 0):
            return True
    return False


def rmDefineInitialMask(threadMapping, application):
    maskFinal = " "
    cont = 0
    for i in threadMapping.statusMapCores:
        if(i == 0): #core is available
            maskFinal = maskFinal + threadMapping.mapCores[cont]+","
        cont = cont + 1
    maskFinal = maskFinal[:-1]
    return maskFinal


def rmMapContainers(architecture, listContainers, threadMapping):
    newMask = " "
    for index in listContainers:
        if(index.mapped == False):
            index.mapped = True
            cont = 0
            i = 0
            while(i < index.tlp):
                while(cont < architecture.total and threadMapping.statusMapCores[cont] == 1):
                    cont = cont + 1
                    if(cont == architecture.total):
                        break
                if(cont != architecture.total):
                    newMask = newMask + threadMapping.mapCores[cont]+","
                    threadMapping.statusMapCores[cont] = 1
                    threadMapping.nameAppMapCores[cont] = index.name
                i = i + architecture.cluster
            newMask = newMask[:-1]
            command = "docker update "+index.id+" --cpuset-cpus "+newMask
           # print(command)
            os.system(command)
            

def rmUpdateMappingAfterKill(name, threadMapping, architecture):
    i = 0
    for index in threadMapping.nameAppMapCores:
        if(index == name):
            threadMapping.statusMapCores[i] = 0
        i = i + 1
    i = 0
    while(i < architecture.total):
        if(threadMapping.statusMapCores[i] == 0):
            threadMapping.nameAppMapCores[i] = "Null"
        i = i + 1

def rmAdjustMapping(threadMapping, application, architecture, listContainers, dockerEnv):
    #check if container needs adjustment
    i=0
    free = False
    newMask = " "
    for index in dockerEnv.containers.list():
        id = index.id
        newMask = " "
        free = False
        for listC in listContainers:
    	    if(listC.id == id):
                fileContainer = "/sys/fs/cgroup/cpu/docker/"+id+"/tasks" #For reproducibility, if the driver used is systemd, please, change here accordingly.
                newTLP = sum(1 for line in open(fileContainer)) - 1
                print("checking app "+listC.name+" with TLP = "+str(listC.tlp)+" and newTLP = "+str(newTLP))
                newDiv = (int)(newTLP/architecture.cluster)
                div = (int) (listC.tlp/architecture.cluster)
                if(newTLP < listC.tlp and div != newDiv):
                    print("Adjusting the number of resources assigned to application"+listC.name)
                    i = 0
                    for index_app in threadMapping.nameAppMapCores:
                        if(index_app == listC.name):
                            threadMapping.statusMapCores[i] = 0
                        i = i + 1
                    i = 0
                    while(i < architecture.total):
                        if(threadMapping.statusMapCores[i] == 0):
                            threadMapping.nameAppMapCores[i] = "Null"
                        i = i + 1
                    i = 0
                    cont = 0
                    while(i < newTLP):
                        while(cont < architecture.total and threadMapping.statusMapCores[cont] == 1):
                            cont = cont + 1
                            if(cont == architecture.total):
                                break
                        if(cont != architecture.total):
                            newMask = newMask + threadMapping.mapCores[cont]+","
                            threadMapping.statusMapCores[cont] = 1
                            threadMapping.nameAppMapCores[cont] = index.name
                        i = i + architecture.cluster
                    newMask = newMask[:-1]
                    listC.tlp = newTLP
                    command = "docker update "+listC.id+" --cpuset-cpus "+newMask
                    os.system(command)
                elif(newTLP > listC.tlp and div != newDiv):
                    free = False
                    i = 0
                    while(i < architecture.total):
                        if(threadMapping.statusMapCores[i] == 0):
                            free = True
                            break
                        i = i + 1
                    if(free == True):
                        print("Adjusting the number of resources assigned to application"+listC.name)
                        i = 0
                        for index_app in threadMapping.nameAppMapCores:
                            if(index_app == listC.name):
                                threadMapping.statusMapCores[i] = 0
                            i = i + 1
                        i = 0
                        while(i < architecture.total):
                            if(threadMapping.statusMapCores[i] == 0):
                                threadMapping.nameAppMapCores[i] = "Null"
                            i = i + 1
                        i = 0
                        cont = 0
                        while(i < newTLP):
                            while(cont < architecture.total and threadMapping.statusMapCores[cont] == 1):
                                cont = cont + 1
                                if(cont == architecture.total):
                                    break
                            if(cont != architecture.total):
                                newMask = newMask + threadMapping.mapCores[cont]+","
                                threadMapping.statusMapCores[cont] = 1
                                threadMapping.nameAppMapCores[cont] = index.name
                            i = i + architecture.cluster
                        newMask = newMask[:-1]
                        listC.tlp = newTLP
                        command = "docker update "+listC.id+" --cpuset-cpus "+newMask
                        os.system(command)
