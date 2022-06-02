import multiprocessing

class rmStrategy:
    def __init__(self, name):
        self.name = name


#definition of class rmContainer
class rmContainer:
    def __init__(self, id, name, tlp, mapped, status):
        self.id = id
        self.name = name
        self.tlp = tlp
        self.mapped = mapped
        self.status = status



# class to store information of the machine.
# For reproducibility on a different machine, please do the following:
#  - give a name to the new architecture (e.g., AMD24); 
#  - define the granularity in which the mapping strategy will act. For example, definining a self.total = 6 and self.cluster = 4, it means that there will be 12 clusters, each one with 2 cores. Theis value is used by the algorithm in the mapping function. 
#  - the AMD24 architecture was defined in the example.
class rmMachine:
    def __init__(self, name, strategy):
        self.name = name
        if(name == "AMD64"):
                self.total = 16
                self.cluster = 4
        elif(name == "INTEL20"):
                self.total = 10
                self.cluster = 2
        elif(name == "INTEL44"):
                self.total = 22
                self.cluster = 2
        #elif(name == "AMD24"):
        #        self.total = 6  
        #        self.cluster = 4


# For reproducibility, please, change the lines below in order to run on a given architecture.
class rmDockerCommand:
    def __init__(self, architecture):
            self.command = "docker run -it -d --rm -v /home/ipdps/SC_REPRODUCIBILITY/benchmarks/:/home -w /home -v /dev/cpu/0:/dev/cpu/0 --privileged " #command used to deploy the container
            self.image = "sc22" #for reproducibility, please, change the name for the correspondent image name 



class rmApps:
    def __init__(self, path, name, priority):
        self.path = path
        self.name = name
        self.priority = priority


# defines the mapping accordingly to the organization defined in rmMachine class.
# For reproducibility, please, do the following:
#  - Add the architecture defined in the class rmMachine
#  - Define the coreIDs on each cluster. In the example, there will be 6 clusters, each one with 4 cores.
#  - Define the status of mapping: 0 means that there is no container mapped to the respective cores.
#  - Define the name of apps mapped to the cores: "null" means that there is no app running on the respective container.
#  - The example considers the AMD24 architecture.
class rmMapping:
    def __init__(self, architecture, strategy):
        if(architecture.name == "AMD64"):
            self.mapCores = ['0-3','4-7','8-11','12-15','16-19','20-23','24-27','28-31','32-35','36-39','40-43','44-47','48-51','52-55','56-59','60-63']
            self.statusMapCores = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
            self.nameAppMapCores = ['Null', 'Null', 'Null', 'Null', 'Null', 'Null', 'Null', 'Null', 'Null', 'Null', 'Null', 'Null', 'Null', 'Null', 'Null', 'Null']
        elif(architecture.name == "INTEL44"):
            self.mapCores = ['0-1','2-3','4-5','6-7','8-9','10-11','12-13','14-15','16-17','18-19','20-21','22-23','24-25','26-27','28-29','30-31','32-33','34-35','36-37','38-39','40-41','42-43']
            self.statusMapCores = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
            self.nameAppMapCores = ['Null', 'Null', 'Null', 'Null', 'Null', 'Null', 'Null', 'Null', 'Null', 'Null', 'Null', 'Null', 'Null', 'Null', 'Null', 'Null', 'Null', 'Null', 'Null', 'Null', 'Null', 'Null']            
        elif(architecture.name == "INTEL20"):
            self.mapCores=['0-1','2-3','4-5','6-7','8-9','10-11','12-13','14-15','16-17','18-19']
            self.statusMapCores = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
            self.nameAppMapCores = ['Null','Null','Null','Null','Null','Null','Null','Null','Null','Null']
        #elif(architecture.name == "AMD24"):
        #    self.mapCores = ['0-3','4-7','8-11','12-15','16-19','20-23']
        #    self.statusMapCores = [0, 0, 0, 0, 0, 0]
        #    self.nameAppMapCores = ['Null', 'Null', 'Null', 'Null', 'Null', 'Null']


