/*******************************************************************

Date:       $Date$
Revision:   $Rev$
Author:     $Author$
URL:        $URL$
ID:         $Id$

********************************************************************/



#include "multiSlsDetector.h"
#include "slsDetector.h"
#include "slsDetectorCommand.h"
#include "usersFunctions.h"
#include  <sys/types.h>
#include  <sys/ipc.h>
#include  <sys/shm.h>
#include  <iostream>
#include  <string>
using namespace std;



int multiSlsDetector::freeSharedMemory() {
  // Detach Memory address
    if (shmdt(thisMultiDetector) == -1) {
      perror("shmdt failed\n");
      return FAIL;
    }
    printf("Shared memory %d detached\n", shmId);
    // remove shared memory
    if (shmctl(shmId, IPC_RMID, 0) == -1) {
      perror("shmctl(IPC_RMID) failed\n");
      return FAIL;
    }
    printf("Shared memory %d deleted\n", shmId);
    return OK;

}



int multiSlsDetector::initSharedMemory(int id=0) {
  
  key_t     mem_key=DEFAULT_SHM_KEY+MAXDET+id;
  int       shm_id;
  int sz;



  sz=sizeof(sharedMultiSlsDetector);


   #ifdef VERBOSE
  std::cout<<"multiSlsDetector: Size of shared memory is "<< sz << " - id " << mem_key << std::endl;
#endif
   shm_id = shmget(mem_key,sz,IPC_CREAT  | 0666); // allocate shared memory

  if (shm_id < 0) {
    std::cout<<"*** shmget error (server) ***"<< shm_id << std::endl;
    return shm_id;
  }
  
   /**
      thisMultiDetector pointer is set to the memory address of the shared memory
   */

  thisMultiDetector = (sharedMultiSlsDetector*) shmat(shm_id, NULL, 0);  /* attach */
  
  if (thisMultiDetector == (void*)-1) {
    std::cout<<"*** shmat error (server) ***" << std::endl;
    return shm_id;
  }
    /**
      shm_id returns -1 is shared memory initialization fails
   */ 

  return shm_id;

}




multiSlsDetector::multiSlsDetector(int id) :  slsDetectorUtils(), shmId(-1) 
{
  while (shmId<0) {
    shmId=initSharedMemory(id);
    id++;
  }
  id--;
  if (thisMultiDetector->alreadyExisting==0) {


    thisMultiDetector->onlineFlag = ONLINE_FLAG;
    thisMultiDetector->numberOfDetectors=0;
    for (int id=0; id<MAXDET; id++) {
      thisMultiDetector->detectorIds[id]=-1;
      thisMultiDetector->offsetX[id]=0;
      thisMultiDetector->offsetY[id]=0;
    }
    thisMultiDetector->masterPosition=-1;
    thisMultiDetector->dataBytes=0;
    thisMultiDetector->numberOfChannels=0;

    thisMultiDetector->maxNumberOfChannels=0;

    

     /** set trimDsdir, calDir and filePath to default to home directory*/
     strcpy(thisMultiDetector->filePath,getenv("HOME"));
     /** set fileName to default to run*/
     strcpy(thisMultiDetector->fileName,"run");
     /** set fileIndex to default to 0*/
     thisMultiDetector->fileIndex=0;
     /** set progress Index to default to 0*/
     thisMultiDetector->progressIndex=0;
     /** set total number of frames to be acquired to default to 1*/
     thisMultiDetector->totalProgress=1;




     /** set correction mask to 0*/
     thisMultiDetector->correctionMask=0;
     /** set deat time*/
     thisMultiDetector->tDead=0;
     /** sets bad channel list file to none */
     strcpy(thisMultiDetector->badChanFile,"none");
     /** sets flat field correction directory */
     strcpy(thisMultiDetector->flatFieldDir,getenv("HOME"));
     /** sets flat field correction file */
     strcpy(thisMultiDetector->flatFieldFile,"none");
     /** set angular direction to 1*/
     thisMultiDetector->angDirection=1;
     /** set fine offset to 0*/
     thisMultiDetector->fineOffset=0;
     /** set global offset to 0*/
     thisMultiDetector->globalOffset=0;



     /** set threshold to -1*/
     thisMultiDetector->currentThresholdEV=-1;
     // /** set clockdivider to 1*/
     // thisMultiDetector->clkDiv=1;
     /** set number of positions to 0*/
     thisMultiDetector->numberOfPositions=0;
     /** sets angular conversion file to none */
     strcpy(thisMultiDetector->angConvFile,"none");
     /** set binsize*/
     thisMultiDetector->binSize=0;
     thisMultiDetector->stoppedFlag=0;
     
     thisMultiDetector->threadedProcessing=1;

     thisMultiDetector->actionMask=0;


     for (int ia=0; ia<MAX_ACTIONS; ia++) {
       //thisMultiDetector->actionMode[ia]=0;
       strcpy(thisMultiDetector->actionScript[ia],"none");
       strcpy(thisMultiDetector->actionParameter[ia],"none");
     }


     for (int iscan=0; iscan<MAX_SCAN_LEVELS; iscan++) {
       
       thisMultiDetector->scanMode[iscan]=0;
       strcpy(thisMultiDetector->scanScript[iscan],"none");
       strcpy(thisMultiDetector->scanParameter[iscan],"none");
       thisMultiDetector->nScanSteps[iscan]=0;
       thisMultiDetector->scanPrecision[iscan]=0;
     }





    thisMultiDetector->alreadyExisting=1;
  }


  for (int i=0; i<thisMultiDetector->numberOfDetectors; i++) {
#ifdef VERBOSE
    cout << thisMultiDetector->detectorIds[i] << endl;
#endif
    detectors[i]=new slsDetector(thisMultiDetector->detectorIds[i]);
  }
  for (int i=thisMultiDetector->numberOfDetectors; i<MAXDET; i++)
    detectors[i]=NULL;



   /** modifies the last PID accessing the detector system*/
  thisMultiDetector->lastPID=getpid();
   getPointers(&thisMultiDetector->stoppedFlag,				\
	       &thisMultiDetector->threadedProcessing,			\
	       &thisMultiDetector->actionMask,				\
	       thisMultiDetector->actionScript,				\
	       thisMultiDetector->actionParameter,			\
	       thisMultiDetector->nScanSteps,				\
	       thisMultiDetector->scanMode,				\
	       thisMultiDetector->scanScript,				\
	       thisMultiDetector->scanParameter,			\
	       thisMultiDetector->scanSteps,				\
	       thisMultiDetector->scanPrecision,			\
	       &thisMultiDetector->numberOfPositions,			\
	       thisMultiDetector->detPositions,				\
	       thisMultiDetector->angConvFile,				\
	       &thisMultiDetector->correctionMask,			\
	       &thisMultiDetector->binSize,				\
	       &thisMultiDetector->fineOffset,				\
	       &thisMultiDetector->globalOffset,			\
	       &thisMultiDetector->angDirection,			\
	       thisMultiDetector->flatFieldDir,				\
	       thisMultiDetector->flatFieldFile,			\
	       thisMultiDetector->badChanFile,				\
	       thisMultiDetector->timerValue,				\
	       &thisMultiDetector->currentSettings,			\
	       &thisMultiDetector->currentThresholdEV,			\
	       thisMultiDetector->filePath,				\
	       thisMultiDetector->fileName,				\
	       &thisMultiDetector->fileIndex);

  
// #ifdef VERBOSE
//    cout << "filling bad channel mask" << endl;
// #endif   
//    /** fill the BadChannelMask \sa  fillBadChannelMask */
//    fillBadChannelMask();
   
// #ifdef VERBOSE
//    cout << "done" << endl;
// #endif 

}

multiSlsDetector::~multiSlsDetector() {
  //removeSlsDetector();

}

int multiSlsDetector::addSlsDetector(int id, int pos) {
  int j=thisMultiDetector->numberOfDetectors;
 

  if (slsDetector::exists(id)==0) {
    cout << "Detector " << id << " does not exist - You should first create it to determine type etc." << endl;
  }
  
#ifdef VERBOSE
  cout << "Adding detector " << id << " in position " << pos << endl;
#endif

  if (pos<0)
    pos=j;

  if (pos>j)
    pos=thisMultiDetector->numberOfDetectors;
  


  //check that it is not already in the list
  
  for (int i=0; i<thisMultiDetector->numberOfDetectors; i++) {
    //check that it is not already in the list, in that case move to new position
    if (detectors[i]) {
      if (detectors[i]->getDetectorId()==id) { 
	cout << "Detector " << id << "already part of the multiDetector in position " << i << "!" << endl << "Remove it before adding it back in a new position!"<< endl;
	return -1;
      }
    }
  }
  


  if (pos!=thisMultiDetector->numberOfDetectors) {
    for (int ip=thisMultiDetector->numberOfDetectors-1; ip>=pos; ip--) {
#ifdef VERBOSE
      cout << "Moving detector " << thisMultiDetector->detectorIds[ip] << " from position " << ip << " to " << ip+1 << endl;
#endif
      thisMultiDetector->detectorIds[ip+1]=thisMultiDetector->detectorIds[ip];
      detectors[ip+1]=detectors[ip];
    }
  }
#ifdef VERBOSE
  cout << "Creating new detector " << pos << endl;
#endif

  detectors[pos]=new slsDetector(id);
  thisMultiDetector->detectorIds[pos]=detectors[pos]->getDetectorId();
  thisMultiDetector->numberOfDetectors++;



  thisMultiDetector->dataBytes+=detectors[pos]->getDataBytes();
 
  thisMultiDetector->numberOfChannels+=detectors[pos]->getTotalNumberOfChannels();
  thisMultiDetector->maxNumberOfChannels-=detectors[j]->getMaxNumberOfChannels();
 
#ifdef VERBOSE
  cout << "Detector added " << thisMultiDetector->numberOfDetectors<< endl;

  for (int ip=0; ip<thisMultiDetector->numberOfDetectors; ip++) {
    cout << "Detector " << thisMultiDetector->detectorIds[ip] << " position " << ip << " "  << detectors[ip]->getHostname() << endl;
  }
#endif

  return thisMultiDetector->numberOfDetectors;

}


string multiSlsDetector::setHostname(char* name, int pos){

  int id=0;
  string s;
  if (pos>=0) {
    addSlsDetector(name, pos);
    if (detectors[pos])
      return detectors[pos]->getHostname();
  } else {
    size_t p1=0;
    s=string(name);
    size_t p2=s.find('+',p1);
    char hn[1000];
    while (p2!=string::npos) {

      strcpy(hn,s.substr(p1,p2-p1).c_str());
      addSlsDetector(hn, pos);
      s=s.substr(p2+1);
      p2=s.find('+');
    }
  }
  return getHostname(pos);
}


string multiSlsDetector::getHostname(int pos) {
  
#ifdef VERBOSE
  cout << "returning hostname" << pos << endl;
#endif
  if (pos>=0) {
    if (detectors[pos])
      return detectors[pos]->getHostname();
  } else {
    string s=string("");
    for (int ip=0; ip<thisMultiDetector->numberOfDetectors; ip++) {
#ifdef VERBOSE
  cout << "detector " << ip << endl;
#endif
      if (detectors[ip]) {
	s+=detectors[ip]->getHostname();
	s+=string("+");
      }
#ifdef VERBOSE
  cout << "hostname " << s << endl;
#endif
    }
    return s;
  }
}

int multiSlsDetector::getDetectorId(int pos) {
  
#ifdef VERBOSE
  cout << "Getting detector ID " << pos << endl;
#endif

  if (pos>=0) {
    if (detectors[pos])
      return detectors[pos]->getDetectorId();
  } 
  return -1;
}



int multiSlsDetector::setDetectorId(int ival, int pos){

  if (pos>=0) {
    addSlsDetector(ival, pos);
    if (detectors[pos])
      return detectors[pos]->getDetectorId();
  } else {
    return -1;
  }

 
}


int multiSlsDetector::addSlsDetector(char *name, int pos) {
  

  detectorType t=GENERIC;
  slsDetector *s=NULL;
  int id;
#ifdef VERBOSE
  cout << "Adding detector "<<name << " in position " << pos << endl;
#endif


  for (int i=0; i<thisMultiDetector->numberOfDetectors; i++) {
    if (detectors[i]) {
     if (detectors[i]->getHostname()==string(name)) {
	cout << "Detector " << name << "already part of the multiDetector in position " << i << "!" << endl<<  "Remove it before adding it back in a new position!"<< endl;
	return -1;	
     }
   }
  }
   
   //checking that the detector doesn't already exists

  for (id=0; id<MAXDET; id++) {
    cout << id << endl;
     if (slsDetector::exists(id)>0) {
       s=new slsDetector(id);
       if (s->getHostname()==string(name))
	 break;
       delete s;
       s=NULL;
       id++;
     }
   }

   if (s==NULL) {
     t=slsDetector::getDetectorType(name, DEFAULT_PORTNO);
     if (t==GENERIC) {
       cout << "Detector " << name << "does not exist in shared memory and could not connect to it to determine the type!" << endl;
       return -1;
     }
     //#ifdef VERBOSE
     else
       cout << "Detector type is " << t << endl;
     //#endif  

     for (id=0; id<MAXDET; id++) {
       if (slsDetector::exists(id)==0) {
	 break;
       }
     }
     
     s=new slsDetector(t, id);
     s->setTCPSocket(name);
     delete s;
   }

   return addSlsDetector(id, pos);


  }











int multiSlsDetector::getDetectorOffset(int pos, int &ox, int &oy) {
  ox=-1;
  oy=-1;
  int ret=FAIL;
  if (pos>=0 && pos<thisMultiDetector->numberOfDetectors) {
    if (detectors[pos]) {
      ox=thisMultiDetector->offsetX[pos];
      oy=thisMultiDetector->offsetY[pos];
      ret=OK;
    }
  }
  return ret;
}

int multiSlsDetector::setDetectorOffset(int pos, int ox, int oy) {
 
 
  int ret=FAIL;
 
  if (pos>=0 && pos<thisMultiDetector->numberOfDetectors) {
    if (detectors[pos]) {
      if (ox!=-1)
	thisMultiDetector->offsetX[pos]=ox;
      if (oy!=-1) 
	thisMultiDetector->offsetY[pos]=oy;
      ret=OK;
    }
  }
  return ret;
}



int multiSlsDetector::removeSlsDetector(char *name){
  for (int id=0; id<thisMultiDetector->numberOfDetectors; id++) {	
    if (detectors[id]) {						
      if (detectors[id]->getHostname()==string(name)) {			
	removeSlsDetector(id);						
      }									
    }									
  }									
  return thisMultiDetector->numberOfDetectors;
};




int multiSlsDetector::removeSlsDetector(int pos) {
  int j;
  
#ifdef VERBOSE
  cout << "Removing detector in position " << pos << endl;
#endif

  if (pos<0 )
    pos=thisMultiDetector->numberOfDetectors-1;

  if (pos>=thisMultiDetector->numberOfDetectors)
    return thisMultiDetector->numberOfDetectors;

  j=pos;

  if (detectors[j]) {

  thisMultiDetector->dataBytes-=detectors[j]->getDataBytes();
  thisMultiDetector->numberOfChannels-=detectors[j]->getTotalNumberOfChannels();
  thisMultiDetector->maxNumberOfChannels-=detectors[j]->getMaxNumberOfChannels();

    delete detectors[j];
    thisMultiDetector->numberOfDetectors--;
    
    

    
    for (int i=j+1; i<thisMultiDetector->numberOfDetectors+1; i++) {
      detectors[i-1]=detectors[i];
      thisMultiDetector->detectorIds[i-1]=thisMultiDetector->detectorIds[i];
    }
    detectors[thisMultiDetector->numberOfDetectors]=NULL;
    thisMultiDetector->detectorIds[thisMultiDetector->numberOfDetectors]=-1;
  }
  return thisMultiDetector->numberOfDetectors;
}







 
int multiSlsDetector::setMaster(int i) {

  int ret=-1, slave=0;
  masterFlags f;
#ifdef VERBOSE
  cout << "settin master in position " << i << endl;
#endif
  if (i>=0 && i<thisMultiDetector->numberOfDetectors) {
    if (detectors[i]) {
#ifdef VERBOSE
      cout << "detector position " << i << " ";
#endif
       thisMultiDetector->masterPosition=i;
       detectors[i]->setMaster(IS_MASTER);
    }
    for (int id=0; id<thisMultiDetector->numberOfDetectors; id++) {
      if (i!=id) {
	if (detectors[id]) {
#ifdef VERBOSE
      cout << "detector position " << id << " ";
#endif
	  detectors[id]->setMaster(IS_SLAVE);
	}
      }
    }

  } else if (i==-2) {
    for (int id=0; id<thisMultiDetector->numberOfDetectors; id++) {
	if (detectors[id]) {
#ifdef VERBOSE
      cout << "detector position " << id << " ";
#endif
	  detectors[id]->setMaster(NO_MASTER);
	}
    }
    
  }

  // check return value

  for (int id=0; id<thisMultiDetector->numberOfDetectors; id++) {
    if (detectors[id]) {
#ifdef VERBOSE
      cout << "detector position " << id << " ";
#endif
      f=detectors[id]->setMaster(GET_MASTER);
      switch (f) {
      case NO_MASTER:
	if (ret!=-1)
	  ret=-2;
	break;
      case IS_MASTER:
	if (ret==-1)
	  ret=id;
	else
	  ret=-2;
	break;
      case IS_SLAVE:
	slave=1;
	break;
      default:
	ret=-2;
      }
    }
  }
  if (slave>0 && ret<0)
    ret=-2;
  
  if (ret<0)
    ret=-1;
  
  thisMultiDetector->masterPosition=ret;

  return thisMultiDetector->masterPosition;
}

//   enum synchronyzationMode {
//     GET_SYNCHRONIZATION_MODE=-1, /**< the multidetector will return its synchronization mode */
//     NONE, /**< all detectors are independent (no cabling) */
//     MASTER_GATES, /**< the master gates the other detectors */
//     MASTER_TRIGGERS, /**< the master triggers the other detectors */
//     SLAVE_STARTS_WHEN_MASTER_STOPS /**< the slave acquires when the master finishes, to avoid deadtime */
//   }
  
  /** 
      Sets/gets the synchronization mode of the various detectors
      \param sync syncronization mode
      \returns current syncronization mode
  */
synchronizationMode multiSlsDetector::setSynchronization(synchronizationMode sync) {

  
  synchronizationMode ret=GET_SYNCHRONIZATION_MODE, ret1=GET_SYNCHRONIZATION_MODE;
  
  for (int id=0; id<thisMultiDetector->numberOfDetectors; id++) {
    if (detectors[id]) {
      ret1=detectors[id]->setSynchronization(sync);
      if (id==0)
	ret=ret1;
      else if (ret!=ret1)
	ret=GET_SYNCHRONIZATION_MODE;

    }
  }
  
  thisMultiDetector->syncMode=ret;

  return thisMultiDetector->syncMode;
  
}



























int multiSlsDetector::setOnline(int off) {
  
  if (off!=GET_ONLINE_FLAG) {
    thisMultiDetector->onlineFlag=off;
    for (int i=0; i<thisMultiDetector->numberOfDetectors+1; i++) {
      if (detectors[i])
	detectors[i]->setOnline(off);
    }
  }
  return thisMultiDetector->onlineFlag;

};


int multiSlsDetector::exists() {
  return thisMultiDetector->alreadyExisting;
}






  // Initialization functions



  

int multiSlsDetector::getThresholdEnergy(int pos) {
 

  int i, posmin, posmax;
  int ret1=-100, ret;

  if (pos<0) {
    posmin=0;
    posmax=thisMultiDetector->numberOfDetectors;
  } else {
    posmin=pos;
    posmax=pos+1;
  }

  for (i=posmin; i<posmax; i++) {
    if (detectors[i]) {
      ret=detectors[i]->getThresholdEnergy();
      if (ret1==-100)
	ret1=ret;
      else if (ret<(ret1-200) || ret>(ret1+200))
	ret1=FAIL;
      
    }
   
  }
  thisMultiDetector->currentThresholdEV=ret1;
  return ret1;


}  


int multiSlsDetector::setThresholdEnergy(int e_eV, int pos, detectorSettings isettings) {

  int i, posmin, posmax;
  int ret1=-100, ret;

  if (pos<0) {
    posmin=0;
    posmax=thisMultiDetector->numberOfDetectors;
  } else {
    posmin=pos;
    posmax=pos+1;
  }

  for (i=posmin; i<posmax; i++) {
    if (detectors[i]) {
      ret=detectors[i]->setThresholdEnergy(e_eV,-1,isettings);
#ifdef VERBOSE
      cout << "detetcor " << i << " threshold " << ret << endl;
#endif
      if (ret1==-100)
	ret1=ret;
      else if (ret<(ret1-200) || ret>(ret1+200))
	ret1=FAIL;
      
#ifdef VERBOSE
      cout << "return value " << ret1 << endl;
#endif
    }
   
  }
  thisMultiDetector->currentThresholdEV=ret1;
  return ret1;

}
 
detectorSettings multiSlsDetector::getSettings(int pos) {

  int i, posmin, posmax;
  detectorSettings ret1=GET_SETTINGS, ret;

  if (pos<0) {
    posmin=0;
    posmax=thisMultiDetector->numberOfDetectors;
  } else {
    posmin=pos;
    posmax=pos+1;
  }

  for (i=posmin; i<posmax; i++) {
    if (detectors[i]) {
      ret=detectors[i]->getSettings();
      if (ret1==GET_SETTINGS)
	ret1=ret;
      else if (ret!=ret1)
	ret1=GET_SETTINGS;
      
    }
   
  }
  thisMultiDetector->currentSettings=ret1;
  return ret1;
}

detectorSettings multiSlsDetector::setSettings(detectorSettings isettings, int pos) {


  int i, posmin, posmax;
  detectorSettings ret1=GET_SETTINGS, ret;

  if (pos<0) {
    posmin=0;
    posmax=thisMultiDetector->numberOfDetectors;
  } else {
    posmin=pos;
    posmax=pos+1;
  }

  for (i=posmin; i<posmax; i++) {
    if (detectors[i]) {
      ret=detectors[i]->setSettings(isettings);
      if (ret1==GET_SETTINGS)
	ret1=ret;
      else if (ret!=ret1)
	ret1=GET_SETTINGS;
      
    }
   
  }
  thisMultiDetector->currentSettings=ret1;
  return ret1;

}






















  /* Communication to server */



// Acquisition functions
/* change these funcs accepting also ok/fail */

int multiSlsDetector::startAcquisition(){
 
  int i=0;
  int ret=OK, ret1=OK;
  for (i=0; i<thisMultiDetector->numberOfDetectors; i++) {
    if (i!=thisMultiDetector->masterPosition)
      if (detectors[i]) {
	ret=detectors[i]->startAcquisition();
	if (ret!=OK)
	  ret1=FAIL;
      }
  }
  i=thisMultiDetector->masterPosition;
  if (thisMultiDetector->masterPosition>=0) {
    if (detectors[i]) {
      ret=detectors[i]->startAcquisition();
      if (ret!=OK)
	ret1=FAIL;
    }
  }
  return ret1;
     
};




int multiSlsDetector::stopAcquisition(){

  int i=0;
  int ret=OK, ret1=OK;  
 
  i=thisMultiDetector->masterPosition;
  if (thisMultiDetector->masterPosition>=0) {
    if (detectors[i]) {
      ret=detectors[i]->stopAcquisition();
      if (ret!=OK)
	ret1=FAIL;
    }
  }
  for (i=0; i<thisMultiDetector->numberOfDetectors; i++) {
    if (detectors[i]) {
      ret=detectors[i]->stopAcquisition();
      if (ret!=OK)
	ret1=FAIL;
    }
  }
  return ret1;


};

int multiSlsDetector::startReadOut(){

 int i=0;
  int ret=OK, ret1=OK;
  i=thisMultiDetector->masterPosition;
  if (i>=0) {
    if (detectors[i]) {
      ret=detectors[i]->startReadOut();
      if (ret!=OK)
	ret1=FAIL;
    }
  }
  for (i=0; i<thisMultiDetector->numberOfDetectors; i++) {
    if (detectors[i]) {
      ret=detectors[i]->startReadOut();
      if (ret!=OK)
	ret1=FAIL;
    }
  }

  return ret1;
  

};



int* multiSlsDetector::getDataFromDetector() {

  int nel=thisMultiDetector->dataBytes/sizeof(int);
  int n;
  int* retval=new int[nel];
  int *retdet, *p=retval;
  int nodata=1, nodatadet=-1;;
 

  for (int id=0; id<thisMultiDetector->numberOfDetectors; id++) {
    if (detectors[id]) {
      retdet=detectors[id]->getDataFromDetector();
      n=detectors[id]->getDataBytes();
      if (retdet) {
	nodata=0;
#ifdef VERBOSE
	cout << "Detector " << id << " returned " << n << " bytes " << endl;
#endif
	memcpy(p,retdet,n);
#ifdef VERBOSE
	cout << "Copied to pointer "<< p  << endl;
#endif
	delete [] retdet;
      } else {
	nodatadet=id;
	cout << "Detector " << id << " does not have data left " << endl;
	break;
      }      
      p+=n/sizeof(int);
    }
  }
  if (nodatadet>=0) {
    for (int id=0; id<thisMultiDetector->numberOfDetectors; id++) {
      if (id!=nodatadet) {
	if (detectors[id]) {
#ifdef VERBOSE
	  cout << "Stopping detector "<< id << endl;
#endif
	  detectors[id]->stopAcquisition();
	  while ((retdet=detectors[id]->getDataFromDetector())) {
	    
#ifdef VERBOSE
	    cout << "Detector "<< id << " still sent data " << endl;
#endif
	    delete [] retdet;
	  }
	}
      }
    }
    delete [] retval;
    return NULL;
  }

  return retval;
};


int* multiSlsDetector::readFrame(){
  int nel=thisMultiDetector->dataBytes/sizeof(int);
  int n;
  int* retval=new int[nel];
  int *retdet, *p=retval;
  
  /** probably it's always better to have one integer per channel in any case! */

  for (int id=0; id<thisMultiDetector->numberOfDetectors; id++) {
    if (detectors[id]) {
      retdet=detectors[id]->readFrame();
      if (retdet) {
	n=detectors[id]->getDataBytes();
	memcpy(p,retdet,n);
	delete [] retdet;
	p+=n/sizeof(int);
      } else {
#ifdef VERBOSE
	cout << "Detector " << id << " does not have data left " << endl;
#endif
	delete [] retval;
	return NULL;
      }
    }
  }
  dataQueue.push(retval);
  return retval;

};



int* multiSlsDetector::readAll(){
  
  /** Thread for each detector?!?!?! */

  // int fnum=F_READ_ALL;
  int* retval; // check what we return!
  // int ret=OK, ret1=OK;

  int i=0;
#ifdef VERBOSE
  std::cout<< "Reading all frames "<< std::endl;
#endif
  if (thisMultiDetector->onlineFlag==ONLINE_FLAG) {
    
    for (int id=0; id<thisMultiDetector->numberOfDetectors; id++) {
      if (detectors[id]) {
	detectors[id]->readAllNoWait();
      }
    }
    while ((retval=getDataFromDetector())){
      i++;
#ifdef VERBOSE
      // std::cout<< i << std::endl;
#endif
      dataQueue.push(retval);
    }
    for (int id=0; id<thisMultiDetector->numberOfDetectors; id++) {
      if (detectors[id]) {
	detectors[id]->disconnectControl();
      }
    }  
 
 }

#ifdef VERBOSE
  std::cout<< "received "<< i<< " frames" << std::endl;
#endif
  return dataQueue.front(); // check what we return!

};

int* multiSlsDetector::startAndReadAll(){

  /** Thread for each detector?!?!?! */
#ifdef VERBOSE
  cout << "Start and read all " << endl;
#endif 


  int* retval;
  int i=0;
  if (thisMultiDetector->onlineFlag==ONLINE_FLAG) {
    
    startAndReadAllNoWait();
   
    while ((retval=getDataFromDetector())){
      i++;
#ifdef VERBOSE
      std::cout<< i << std::endl;
#endif
      dataQueue.push(retval);
    }

    for (int id=0; id<thisMultiDetector->numberOfDetectors; id++) {
      if (detectors[id]) {
	detectors[id]->disconnectControl();
      }
    }  
 
 }

#ifdef VERBOSE
  std::cout<< "MMMM recieved "<< i<< " frames" << std::endl;
#endif
  return dataQueue.front(); // check what we return!

  
};


int multiSlsDetector::startAndReadAllNoWait(){


  int i=0;
  int ret=OK, ret1=OK;

 for (i=0; i<thisMultiDetector->numberOfDetectors; i++) {
      if (i!=thisMultiDetector->masterPosition)
	if (detectors[i]) {
	  ret=detectors[i]->startAndReadAllNoWait();
	if (ret!=OK)
	  ret1=FAIL;
	}
    }
    i=thisMultiDetector->masterPosition;
    if (thisMultiDetector->masterPosition>=0) {
      if (detectors[i]) {
	ret=detectors[i]->startAndReadAllNoWait();
	if (ret!=OK)
	  ret1=FAIL;
      }
    }
    return ret1;

}


/**
   get run status
   \returns status mask
*/
runStatus  multiSlsDetector::getRunStatus() {

  runStatus s,s1;

  if (thisMultiDetector->masterPosition>=0)
    if (detectors[thisMultiDetector->masterPosition])
      return detectors[thisMultiDetector->masterPosition]->getRunStatus();


  if (detectors[0]) s=detectors[0]->getRunStatus(); 

  for (int i=0; i<thisMultiDetector->numberOfDetectors; i++) {
    s1=detectors[i]->getRunStatus(); 
    if (s1==ERROR)
      s=ERROR;
    if (s1==IDLE && s!=IDLE)
      s=ERROR;
    
  }
  return s;
}


int* multiSlsDetector::popDataQueue() {
  int *retval=NULL;
  if( !dataQueue.empty() ) {
    retval=dataQueue.front();
    dataQueue.pop();
  }
  return retval;
}

detectorData* multiSlsDetector::popFinalDataQueue() {
  detectorData *retval=NULL;
  if( !finalDataQueue.empty() ) {
    retval=finalDataQueue.front();
    finalDataQueue.pop();
  }
  return retval;
}

void multiSlsDetector::resetDataQueue() {
  int *retval=NULL;
  while( !dataQueue.empty() ) {
    retval=dataQueue.front();
    dataQueue.pop();
    delete [] retval;
  }
 
}

void multiSlsDetector::resetFinalDataQueue() {
  detectorData *retval=NULL;
  while( !finalDataQueue.empty() ) {
    retval=finalDataQueue.front();
    finalDataQueue.pop();
    delete retval;
  }

}



  /* 
     set or read the acquisition timers 
     enum timerIndex {
     FRAME_NUMBER,
     ACQUISITION_TIME,
     FRAME_PERIOD,
     DELAY_AFTER_TRIGGER,
     GATES_NUMBER,
     PROBES_NUMBER
     CYCLES_NUMBER,
     GATE_INTEGRATED_TIME
     }
  */
int64_t multiSlsDetector::setTimer(timerIndex index, int64_t t){
  int i;
  int64_t ret1=-100, ret;


  
  for (i=0; i<thisMultiDetector->numberOfDetectors; i++) {
    if (detectors[i]) {
      ret=detectors[i]->setTimer(index,t);
      if (ret1==-100)
	ret1=ret;
      else if (ret!=ret1)
	ret1=FAIL;
      
    }
  }
// check return values!!!
  
  thisMultiDetector->timerValue[index]=ret1;
  
  return ret1;
};

 
int64_t multiSlsDetector::getTimeLeft(timerIndex index){
  int i;
  int64_t ret1=-100, ret;
  

  if (thisMultiDetector->masterPosition>=0)
    if (detectors[thisMultiDetector->masterPosition])
      return detectors[thisMultiDetector->masterPosition]->getTimeLeft(index);
  
  
  for (i=0; i<thisMultiDetector->numberOfDetectors; i++) {
    if (detectors[i]) {
      ret=detectors[i]->getTimeLeft(index);
      if (ret1==-100)
	ret1=ret;
      else if (ret!=ret1)
	ret1=-1;
    }
  }
  
  return ret1;
  
}



int multiSlsDetector::setSpeed(speedVariable index, int value){
  int i;
  int64_t ret1=-100, ret;
  

  
  for (i=0; i<thisMultiDetector->numberOfDetectors; i++) {
    if (detectors[i]) {
      ret=detectors[i]->setSpeed(index,value);
      if (ret1==-100)
	ret1=ret;
      else if (ret!=ret1)
	ret1=FAIL;
    }
  }
  
  return ret1;
  
}


















  // Flags
int multiSlsDetector::setDynamicRange(int n, int pos){

  int imi, ima, i;
  int ret, ret1=-100;

  if (pos<0) {
    imi=0;
    ima=thisMultiDetector->numberOfDetectors;
  } else {
    imi=pos;
    ima=pos+1;
  }
 
  for (i=imi; i<ima; i++) {
    if (detectors[i]) {
      thisMultiDetector->dataBytes-=detectors[i]->getDataBytes();
      ret=detectors[i]->setDynamicRange(n);
      if (ret1==-100)
	ret1=ret;
      else if (ret!=ret1)
	ret1=FAIL;
      thisMultiDetector->dataBytes+=detectors[i]->getDataBytes();
    }
  }

  return thisMultiDetector->dataBytes;
};

/*

int multiSlsDetector::setROI(int nroi, int *xmin, int *xmax, int *ymin, int *ymax){


};
*/


float* multiSlsDetector::decodeData(int *datain) {
  float *dataout=new float[thisMultiDetector->numberOfChannels];
  int ich=0;
  float *detp;
  int  *datap=datain;


  for (int i=0; i<thisMultiDetector->numberOfDetectors; i++) {
    if (detectors[i]) {
      detp=detectors[i]->decodeData(datap);
      datap+=detectors[i]->getDataBytes()/sizeof(int);
      for (int j=0; j<detectors[i]->getTotalNumberOfChannels(); j++) {
	dataout[ich]=detp[j];
	ich++;
      }
      delete [] detp;
    }
  }

  return dataout;
}
 
//Correction
  /*
    enum correctionFlags {
    DISCARD_BAD_CHANNELS,
    AVERAGE_NEIGHBOURS_FOR_BAD_CHANNELS,
    FLAT_FIELD_CORRECTION,
    RATE_CORRECTION,
    ANGULAR_CONVERSION
    }
  */


///////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////







int multiSlsDetector::setFlatFieldCorrection(string fname){
  float data[thisMultiDetector->numberOfChannels],  xmed[thisMultiDetector->numberOfChannels];
  float ffcoefficients[thisMultiDetector->numberOfChannels], fferrors[thisMultiDetector->numberOfChannels];
  int nmed=0;
  int idet=0, ichdet=-1;
  char ffffname[MAX_STR_LENGTH*2];
  int nbad=0, nch;
  int badlist[MAX_BADCHANS];
  int im=0;


  if (fname=="") {
#ifdef VERBOSE
   std::cout<< "disabling flat field correction" << std::endl;
#endif
    thisMultiDetector->correctionMask&=~(1<<FLAT_FIELD_CORRECTION);
    strcpy(thisMultiDetector->flatFieldFile,"none");
    for (int i=0; i<thisMultiDetector->numberOfDetectors; i++) {
      if (detectors[i])
	detectors[i]->setFlatFieldCorrection(NULL, NULL);
    }
  } else { 
#ifdef VERBOSE
   std::cout<< "Setting flat field correction from file " << fname << std::endl;
#endif
   sprintf(ffffname,"%s/%s",thisMultiDetector->flatFieldDir,fname.c_str());
   nch=readDataFile(string(ffffname),data);

   if (nch>thisMultiDetector->numberOfChannels)
     nch=thisMultiDetector->numberOfChannels;

   if (nch>0) {
      strcpy(thisMultiDetector->flatFieldFile,fname.c_str());
     


      for (int ichan=0; ichan<nch; ichan++) {
	if (detectors[idet]) {
	  if (ichdet>=detectors[idet]->getTotalNumberOfChannels()) {
	    ichdet=0;
	    detectors[idet]->setBadChannelCorrection(nbad,badlist,1);
	    idet++;
	    nbad=0;
	  } else
	    ichdet++;
	}

	if (data[ichan]>0) {
	  /* add to median */
	  im=0;
	  while ((im<nmed) && (xmed[im]<data[ichan])) 
	    im++;
	  for (int i=nmed; i>im; i--) 
	    xmed[i]=xmed[i-1];
	  xmed[im]=data[ichan];
	  nmed++;
        } else {
	  if (nbad<MAX_BADCHANS) {
	    badlist[nbad]=ichdet;
	    nbad++;
	  }
	}
      }
      if (detectors[idet]) 
	detectors[idet]->setBadChannelCorrection(nbad,badlist,1);
	
      if (nmed>1 && xmed[nmed/2]>0) {
#ifdef VERBOSE
	std::cout<< "Flat field median is " << xmed[nmed/2] << " calculated using "<< nmed << " points" << std::endl;
#endif
	
	thisMultiDetector->correctionMask|=(1<<FLAT_FIELD_CORRECTION);

	// add to ff coefficients and errors of single detectors

	  

	idet=0; 
	ichdet=0;
	int detoff=0;

	for (int ichan=0; ichan<nch; ichan++) {

	  if (detectors[idet]) {
	    if (ichdet>=detectors[idet]->getTotalNumberOfChannels()) {
#ifdef VERBOSE
	      cout << "Set flat field detector " << idet << "(offset "<< detoff << ")" << endl; 
#endif
	      detectors[idet]->setFlatFieldCorrection(ffcoefficients+detoff, fferrors+detoff);
	      ichdet=0;//ichan;
	      detoff=ichan;
	      idet++;
	    } 
	  }


	  if (data[ichan]>0) {
	    ffcoefficients[ichan]=xmed[nmed/2]/data[ichan];
	    fferrors[ichan]=ffcoefficients[ichan]*sqrt(data[ichan])/data[ichan];
	  } else {
	    ffcoefficients[ichan]=0.;
	    fferrors[ichan]=1.;
	  }
	  ichdet++;
	}
	if (detectors[idet]) {
#ifdef VERBOSE
	      cout << "**Set flat field detector " << idet << "(offset "<< detoff << ")" <<  endl; 
#endif
	  detectors[idet]->setFlatFieldCorrection(ffcoefficients+detoff, fferrors+detoff);
	}
      } else {
	std::cout<< "Flat field data from file " << fname << " are not valid (" << nmed << "///" << xmed[nmed/2] << std::endl;
	thisMultiDetector->correctionMask&=~(1<<FLAT_FIELD_CORRECTION);
	for (int i=0; i<thisMultiDetector->numberOfDetectors; i++) {
	  if (detectors[i])
	    detectors[i]->setFlatFieldCorrection(NULL, NULL);
	}
	return -1;
      }
   } else {
      std::cout<< "Flat field from file " << fname << " is not valid " << nch << std::endl;  
      thisMultiDetector->correctionMask&=~(1<<FLAT_FIELD_CORRECTION);
      for (int i=0; i<thisMultiDetector->numberOfDetectors; i++) {
	if (detectors[i])
	  detectors[i]->setFlatFieldCorrection(NULL, NULL);
      }
      return -1;
    } 
  }
  return thisMultiDetector->correctionMask&(1<<FLAT_FIELD_CORRECTION);
}
 




int multiSlsDetector::setFlatFieldCorrection(float *corr, float *ecorr) {
  int ichdet=0;
  float *p, *ep;
  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
    if (detectors[idet]) {
      if (corr!=NULL)
	p=corr+ichdet;
      else
	p=NULL;
      if (ecorr!=NULL)
	ep=ecorr+ichdet;
      else
	ep=NULL;
      detectors[idet]->setFlatFieldCorrection(p, ep);
      ichdet+=detectors[idet]->getTotalNumberOfChannels();
    }
  }
  return 0;
}









int multiSlsDetector::getFlatFieldCorrection(float *corr, float *ecorr) {
  int ichdet=0;
  float *p, *ep;
  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
    if (detectors[idet]) {
      if (corr!=NULL)
	p=corr+ichdet;
      else
	p=NULL;
      if (ecorr!=NULL)
	ep=ecorr+ichdet;
      else
	ep=NULL;
      detectors[idet]->getFlatFieldCorrection(p, ep);
      ichdet+=detectors[idet]->getTotalNumberOfChannels();
    }
  }
  return 0;
}























int multiSlsDetector::flatFieldCorrect(float* datain, float *errin, float* dataout, float *errout){  

  int ichdet=0;
  float *pdata, *perr=errin;
  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
    if (detectors[idet]) {
#ifdef VERBOSE
      cout << " detector " << idet << " offset " << ichdet << endl;
#endif
      if (errin)
	perr+=ichdet;
      detectors[idet]->flatFieldCorrect(datain+ichdet, perr, dataout+ichdet, errout+ichdet);
      ichdet+=detectors[idet]->getTotalNumberOfChannels();//detectors[idet]->getNChans()*detectors[idet]->getNChips()*detectors[idet]->getNMods();
    }
  }
  return 0;
};






int multiSlsDetector::setRateCorrection(float t){
  float tdead[]=defaultTDead;

  if (t==0) {
    thisMultiDetector->correctionMask&=~(1<<RATE_CORRECTION);
  } else {
    thisMultiDetector->correctionMask|=(1<<RATE_CORRECTION);
    
    for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
      
      if (detectors[idet]) {
	detectors[idet]->setRateCorrection(t);
      }
    }
#ifdef VERBOSE
    std::cout<< "Setting rate correction with dead time "<< thisMultiDetector->tDead << std::endl;
#endif
  }
  return thisMultiDetector->correctionMask&(1<<RATE_CORRECTION);
}


int multiSlsDetector::getRateCorrection(float &t){

  if (thisMultiDetector->correctionMask&(1<<RATE_CORRECTION)) {
#ifdef VERBOSE
    std::cout<< "Rate correction is enabled with dead time "<< thisMultiDetector->tDead << std::endl;
#endif
    //which t should we return if they are all different?
    return 1;
  } else
    t=0;
#ifdef VERBOSE
    std::cout<< "Rate correction is disabled " << std::endl;
#endif
    return 0;
};

float multiSlsDetector::getRateCorrectionTau(){

  if (thisMultiDetector->correctionMask&(1<<RATE_CORRECTION)) {
#ifdef VERBOSE
    std::cout<< "Rate correction is enabled with dead time "<< thisMultiDetector->tDead << std::endl;
#endif
    //which t should we return if they are all different?




    return 1;
  } else
#ifdef VERBOSE
    std::cout<< "Rate correction is disabled " << std::endl;
#endif
    return 0;
};



int multiSlsDetector::getRateCorrection(){

  if (thisMultiDetector->correctionMask&(1<<RATE_CORRECTION)) {
    return 1;
  } else
    return 0;
};




int multiSlsDetector::rateCorrect(float* datain, float *errin, float* dataout, float *errout){

  int ichdet=0;
  float *perr=errin;
  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
    if (detectors[idet]) {
      if (errin)
	perr+=ichdet;
      detectors[idet]->rateCorrect(datain+ichdet, perr, dataout+ichdet, errout+ichdet);
      ichdet+=detectors[idet]->getTotalNumberOfChannels();
    }
  }
  return 0;
};


int multiSlsDetector::setBadChannelCorrection(string fname){

  int badlist[MAX_BADCHANS], badlistdet[MAX_BADCHANS];
  int nbad=0, nbaddet=0, choff=0, idet=0;

  if (fname=="default")
    fname=string(thisMultiDetector->badChanFile);

  int ret=setBadChannelCorrection(fname, nbad, badlist);
#ifdef VERBOSE
  cout << "file contained " << ret << " badcahns" << endl; 
#endif
  if (ret==0) {
    thisMultiDetector->correctionMask&=~(1<<DISCARD_BAD_CHANNELS);
    nbad=0;
  } else {
    thisMultiDetector->correctionMask|=(1<<DISCARD_BAD_CHANNELS);
    strcpy(thisMultiDetector->badChanFile,fname.c_str());
  }

  return setBadChannelCorrection(nbad,badlist,0);

}



int multiSlsDetector::setBadChannelCorrection(int nbad, int *badlist, int ff) {
  

  int  badlistdet[MAX_BADCHANS];
  int nbaddet=0, choff=0, idet=0;

  if (nbad>0) {
    thisMultiDetector->correctionMask|=(1<<DISCARD_BAD_CHANNELS);
    
    for (int ich=0; ich<nbad; ich++) {
      if (detectors[idet]) {
	if ((badlist[ich]-choff)>=detectors[idet]->getMaxNumberOfChannels()) {
#ifdef VERBOSE
	  cout << "setting " << nbaddet << " badchans to detector " << idet << endl;
#endif
	  detectors[idet]->setBadChannelCorrection(nbaddet,badlist,0);
	  choff+=detectors[idet]->getMaxNumberOfChannels();
	  nbaddet=0;
	  idet++;
	  if (detectors[idet]==NULL)
	    break;
	}
	badlistdet[nbaddet]=(badlist[ich]-choff);
	nbaddet++;
	cout << nbaddet << " " << badlist[ich] << " " << badlistdet[nbaddet-1] << endl;
      }
    }
    if (nbaddet>0) {

      if (detectors[idet]) {
#ifdef VERBOSE
	cout << "setting " << nbaddet << " badchans to detector " << idet << endl;
#endif
	detectors[idet]->setBadChannelCorrection(nbaddet,badlist,0);
	choff+=detectors[idet]->getMaxNumberOfChannels();
	nbaddet=0;
	idet++;
      }
    }
    nbaddet=0;
    for (int i=idet; i<thisMultiDetector->numberOfDetectors; i++) {
#ifdef VERBOSE
	  cout << "setting " << 0 << " badchans to detector " << i << endl;
#endif
      if (detectors[i]) {
	detectors[i]->setBadChannelCorrection(nbaddet,badlist,0);
      }
    }
    
  } else {
    nbaddet=0;
    for (int i=0; i<thisMultiDetector->numberOfDetectors; i++) {
      if (detectors[idet]) {
#ifdef VERBOSE
	  cout << "setting " << 0 << " badchans to detector " << idet << endl;
#endif
	detectors[idet]->setBadChannelCorrection(nbaddet,badlist,0);
      }
    }
    thisMultiDetector->correctionMask&=~(1<<DISCARD_BAD_CHANNELS);
  }  
#ifdef VERBOSE 
  cout << (thisMultiDetector->correctionMask&(1<<DISCARD_BAD_CHANNELS)) << endl;
#endif
  return thisMultiDetector->correctionMask&(1<<DISCARD_BAD_CHANNELS);

}


int multiSlsDetector::setAngularConversion(string fname) {
  if (fname=="") {
    thisMultiDetector->correctionMask&=~(1<< ANGULAR_CONVERSION);
    //strcpy(thisDetector->angConvFile,"none");
    //#ifdef VERBOSE
     std::cout << "Unsetting angular conversion" <<  std::endl;
    //#endif
  } else {
    if (fname=="default") {
      fname=string(thisMultiDetector->angConvFile);
    }
    
    //#ifdef VERBOSE
    std::cout << "Setting angular conversion to" << fname << std:: endl;
    //#endif
    if (readAngularConversion(fname)>=0) {
      thisMultiDetector->correctionMask|=(1<< ANGULAR_CONVERSION);
      strcpy(thisMultiDetector->angConvFile,fname.c_str());
    }
  }
  return thisMultiDetector->correctionMask&(1<< ANGULAR_CONVERSION);
}

int multiSlsDetector::readAngularConversion(string fname) {

  
  ifstream infile;
  int nm=0;
  infile.open(fname.c_str(), ios_base::in);
  if (infile.is_open()) {

    for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
      if (detectors[idet]) {
#ifdef VERBOSE
	cout << " detector " << idet << endl;
#endif
	detectors[idet]->readAngularConversion(infile);
      }
    }
    infile.close();
  } else {
    std::cout<< "Could not open calibration file "<< fname << std::endl;
    return -1;
  }
  return 0;

}


int multiSlsDetector::writeAngularConversion(string fname) {

  
  ofstream outfile;
  int nm=0;
  outfile.open(fname.c_str(), ios_base::out);
  if (outfile.is_open()) {

    for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
      if (detectors[idet]) {
	detectors[idet]->writeAngularConversion(outfile);
      }
    }
    outfile.close();
  } else {
    std::cout<< "Could not open calibration file "<< fname << std::endl;
    return -1;
  }
  return 0;

}

int  multiSlsDetector::getAngularConversion(int &direction,  angleConversionConstant *angconv) {

  int dir=-100, dir1;
  angleConversionConstant *a1=angconv;

  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
    if (detectors[idet]) {
      detectors[idet]->getAngularConversion(dir1,a1);
      if (dir==-100)
	dir = dir1;
      if (dir!=dir1)
	dir=0;
      if (angconv) {
	a1+=detectors[idet]->getNMods();
      }
    }
  }
  direction=dir;
  
  if (thisMultiDetector->correctionMask&(1<< ANGULAR_CONVERSION)) {
    return 1;
  } 
  return 0;
 

}



float multiSlsDetector::setDAC(float val, dacIndex idac, int imod) {
  float ret, ret1=-100;
  
  int id=-1, im=-1;
  int dmi=0, dma=thisMultiDetector->numberOfDetectors;
  
  if (decodeNMod(imod, id, im)>=0) {
    dmi=id;
    dma=dma+1;
  }

 for (int idet=dmi; idet<dma; idet++) {
   if (detectors[idet]) {
     ret=detectors[idet]->setDAC(val, idac, im);
     if (ret1==-100)
       ret1=ret;
     else if (ret!=ret1)
       ret1=-1;
   }
 }
 return ret1;
}

float multiSlsDetector::getADC(dacIndex idac, int imod) {
  float ret, ret1=-100;
  
  int id=-1, im=-1;
  int dmi=0, dma=thisMultiDetector->numberOfDetectors;
  
  if (decodeNMod(imod, id, im)>=0) {
    dmi=id;
    dma=dma+1;
  }

 for (int idet=dmi; idet<dma; idet++) {
   if (detectors[idet]) {
     ret=detectors[idet]->getADC(idac, im);
     if (ret1==-100)
       ret1=ret;
     else if (ret!=ret1)
       ret1=-1;
   }
 }
 return ret1;
}

int multiSlsDetector::setChannel(long long reg, int ichan, int ichip, int imod) {
  int ret, ret1=-100;
  int id=-1, im=-1;
  int dmi=0, dma=thisMultiDetector->numberOfDetectors;
  
  if (decodeNMod(imod, id, im)>=0) {
    dmi=id;
    dma=dma+1;
  }
  for (int idet=dmi; idet<dma; idet++) {
    if (detectors[idet]) {
      ret=detectors[idet]->setChannel(reg, ichan, ichip, im);
      if (ret1==-100)
	ret1=ret;
      else if (ret!=ret1)
	ret1=-1;
    }
  }
  return ret1;
   
}



   /**
     sets the value of s angular conversion parameter
     \param c can be ANGULAR_DIRECTION, GLOBAL_OFFSET, FINE_OFFSET, BIN_SIZE
     \param v the value to be set
     \returns the actual value
  */

float multiSlsDetector::setAngularConversionParameter(angleConversionParameter c, float v) {
  float ret=slsDetectorUtils::setAngularConversionParameter(c,v);
  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
    
      if (detectors[idet]) {
	
	detectors[idet]->setAngularConversionParameter(c,v);
      }
  }
  return ret;
}



float* multiSlsDetector::convertAngles(float pos) {
  float *ang=new float[thisMultiDetector->numberOfChannels];

  float *p=ang;
  int choff=0;

  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
    
    if (detectors[idet]) {
      p=detectors[idet]->convertAngles(pos);
      for (int ich=0; ich<detectors[idet]->getTotalNumberOfChannels(); ich++) {
	ang[choff+ich]=p[ich];
	}
      choff+=detectors[idet]->getTotalNumberOfChannels();
      delete [] p;
    }
  }
  return ang;
}


int multiSlsDetector::getBadChannelCorrection(int *bad) {
  int ichan;
  int *bd, nd, ntot=0, choff=0;;

  
  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
    if (detectors[idet]) {
      nd=detectors[idet]->getBadChannelCorrection();
      bd = new int[nd];
      nd=detectors[idet]->getBadChannelCorrection(bd);
      for (int id=0; id<nd; id++) {
	if (bd[id]<detectors[idet]->getTotalNumberOfChannels()) {
	  if (bad) bad[ntot]=choff+bd[id];
	  ntot++;
	}
      }
      choff+=detectors[idet]->getTotalNumberOfChannels();
      delete [] bd;
    }
  }
  return ntot;

}


int multiSlsDetector::exitServer() {

  int ival=FAIL, iv;
  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
    if (detectors[idet]) {
      iv=detectors[idet]->exitServer();
      if (iv==OK)
	ival=iv;
    }
  }
  return ival;
}


  /** returns the detector trimbit/settings directory  \sa sharedSlsDetector */
char* multiSlsDetector::getSettingsDir() {
  string s0="", s1="", s;
  

  char ans[1000];
  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
    if (detectors[idet]) {
      s=detectors[idet]->getSettingsDir();

      if (s0=="")
	s0=s;
      else
	s0+=string("+")+s;
      if (s1=="")
	s1=s;
      else if (s1!=s)
	s1="bad";
    }
  }
  if (s1=="bad")
    strcpy(ans,s0.c_str());
  else
    strcpy(ans,s1.c_str());
  return ans;
}



  /** sets the detector trimbit/settings directory  \sa sharedSlsDetector */
char* multiSlsDetector::setSettingsDir(string s){

  if (s.find('+')==string::npos) {
    for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
      if (detectors[idet]) {
	detectors[idet]->setSettingsDir(s);
      }
    }
  } else {
    size_t p1=0;
    size_t p2=s.find('+',p1);
    int id=0;
    while (p2!=string::npos) {

      if (detectors[id]) {
	detectors[id]->setSettingsDir(s.substr(p1,p2-p1));
      }
      id++;
      s=s.substr(p2+1);
      p2=s.find('+');
      if (id>=thisMultiDetector->numberOfDetectors)
	break;
    }

  }
  return getSettingsDir();


}




int multiSlsDetector::setTrimEn(int ne, int *ene) {

  int ret=-100, ret1;

  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
    if (detectors[idet]) {
      ret1=detectors[idet]->setTrimEn(ne,ene);
      if (ret==-100)
	ret=ret1;
      else if (ret!=ret1)
	ret=-1;
    }
  }
  return ret;

}



int multiSlsDetector::getTrimEn(int *ene) {

  int ret=-100, ret1;

  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
    if (detectors[idet]) {
      ret1=detectors[idet]->getTrimEn(ene);
      if (ret==-100)
	ret=ret1;
      else if (ret!=ret1)
	ret=-1;
    }
  }
  return ret;

}









 /**
     returns the location of the calibration files
  \sa  sharedSlsDetector
  */
char* multiSlsDetector::getCalDir() {
  string s0="", s1="", s;
  char ans[1000];
  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
    if (detectors[idet]) {
      s=detectors[idet]->getCalDir();

      if (s0=="")
	s0=s;
      else
	s0+=string("+")+s;
      if (s1=="")
	s1=s;
      else if (s1!=s)
	s1="bad";
    }
  }
  if (s1=="bad")
    strcpy(ans,s0.c_str());
  else
    strcpy(ans,s1.c_str());
  return ans;
}


   /**
      sets the location of the calibration files
  \sa  sharedSlsDetector
  */
char* multiSlsDetector::setCalDir(string s){

  if (s.find('+')==string::npos) {
    for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
      if (detectors[idet]) {
	detectors[idet]->setCalDir(s);
      }
    }
  } else {
    size_t p1=0;
    size_t p2=s.find('+',p1);
    int id=0;
    while (p2!=string::npos) {

      if (detectors[id]) {
	detectors[id]->setCalDir(s.substr(p1,p2-p1));
      }
      id++;
      s=s.substr(p2+1);
      p2=s.find('+');
      if (id>=thisMultiDetector->numberOfDetectors)
	break;
    }

  }
  return getCalDir();

} 

 /**
     returns the location of the calibration files
  \sa  sharedSlsDetector
  */
char* multiSlsDetector::getNetworkParameter(networkParameter p) {
  string s0="", s1="",s ;
  
  char ans[1000];
  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
    if (detectors[idet]) {
      s=detectors[idet]->getNetworkParameter(p);

      if (s0=="")
	s0=s;
      else
	s0+=string("+")+s;
      if (s1=="")
	s1=s;
      else if (s1!=s)
	s1="bad";
    }
  }
  if (s1=="bad")
    strcpy(ans,s0.c_str());
  else
    strcpy(ans,s1.c_str());
  return ans;
}


   /**
      sets the location of the calibration files
  \sa  sharedSlsDetector
  */
char* multiSlsDetector::setNetworkParameter(networkParameter p, string s){

  if (s.find('+')==string::npos) {
    for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
      if (detectors[idet]) {
	detectors[idet]->setNetworkParameter(p,s);
      }
    }
  } else {
    size_t p1=0;
    size_t p2=s.find('+',p1);
    int id=0;
    while (p2!=string::npos) {

      if (detectors[id]) {
	detectors[id]->setCalDir(s.substr(p1,p2-p1));
      }
      id++;
      s=s.substr(p2+1);
      p2=s.find('+');
      if (id>=thisMultiDetector->numberOfDetectors)
	break;
    }

  }
  return getNetworkParameter(p);

} 

int multiSlsDetector::setPort(portType t, int p) {

  int ret=-100, ret1;

  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
    if (detectors[idet]) {
      ret1=detectors[idet]->setPort(t,p);
      if (ret==-100)
	ret=ret1;
      else if (ret!=ret1)
	ret=-1;
    }
  }
  return ret;

}

int multiSlsDetector::lockServer(int p) {

  int ret=-100, ret1;

  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
    if (detectors[idet]) {
      ret1=detectors[idet]->lockServer(p);
      if (ret==-100)
	ret=ret1;
      else if (ret!=ret1)
	ret=-1;
    }
  }

  return ret;

}

string multiSlsDetector::getLastClientIP() {
  string s0="", s1="",s ;
  
  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
    if (detectors[idet]) {
      s=detectors[idet]->getLastClientIP();

      if (s0=="")
	s0=s;
      else
	s0+=string("+")+s;
      if (s1=="")
	s1=s;
      else if (s1!=s)
	s1="bad";
    }
  }
  if (s1=="bad")
   return s0;
  else
    return s1;
}






int multiSlsDetector::setReadOutFlags(readOutFlags flag) {

  int ret=-100, ret1;

  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
    if (detectors[idet]) {
      ret1=detectors[idet]->setReadOutFlags(flag);
      if (ret==-100)
	ret=ret1;
      else if (ret!=ret1)
	ret=-1;
    }
  }

  return ret;


}


externalCommunicationMode multiSlsDetector::setExternalCommunicationMode(externalCommunicationMode pol) {

  externalCommunicationMode  ret, ret1;

  if (detectors[0])
    ret=detectors[0]->setExternalCommunicationMode(pol);

  
  for (int idet=1; idet<thisMultiDetector->numberOfDetectors; idet++) {
    if (detectors[idet]) {
      ret1=detectors[idet]->setExternalCommunicationMode(pol);
      if (ret!=ret1)
	ret=GET_EXTERNAL_COMMUNICATION_MODE;
    }
  }

  setMaster();
  setSynchronization();
  return ret;

}



externalSignalFlag multiSlsDetector::setExternalSignalFlags(externalSignalFlag pol, int signalindex) {

  externalSignalFlag  ret, ret1;

  if (detectors[0])
    ret=detectors[0]->setExternalSignalFlags(pol,signalindex);

  for (int idet=1; idet<thisMultiDetector->numberOfDetectors; idet++) {
    if (detectors[idet]) {
      ret1=detectors[idet]->setExternalSignalFlags(pol,signalindex);
      if (ret!=ret1)
	ret=GET_EXTERNAL_SIGNAL_FLAG;
    }
  }

  setMaster();
  setSynchronization();
  return ret;


}












const char * multiSlsDetector::getSettingsFile() {

   string s0="", s1="",s ;
  
  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
    if (detectors[idet]) {
      s=detectors[idet]->getSettingsFile();

      if (s0=="")
	s0=s;
      else
	s0+=string("+")+s;
      if (s1=="")
	s1=s;
      else if (s1!=s)
	s1="bad";
    }
  }
  if (s1=="bad")
    return s0.c_str();
  else
    return s1.c_str();

}


int multiSlsDetector::configureMAC() {

  int ret=-100, ret1;

  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
    if (detectors[idet]) {
      ret1=detectors[idet]->configureMAC();
      if (ret==-100)
	ret=ret1;
      else if (ret!=ret1)
	ret=-1;
    }
  }

  return ret;

}


int multiSlsDetector::setDynamicRange(int p) {

  int ret=-100, ret1;
  thisMultiDetector->dataBytes=0;
  thisMultiDetector->numberOfChannels=0;
  
  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
    if (detectors[idet]) {
      ret1=detectors[idet]->setDynamicRange(p);
      thisMultiDetector->dataBytes+=detectors[idet]->getDataBytes();
      thisMultiDetector->numberOfChannels+=detectors[idet]->getTotalNumberOfChannels();
      if (ret==-100)
	ret=ret1;
      else if (ret!=ret1)
	ret=-1;
    }
  }
 
  return ret;

}


int multiSlsDetector::getMaxNumberOfModules(dimension d) {

  int ret=0, ret1;
  
  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
    if (detectors[idet]) {
      ret1=detectors[idet]->getMaxNumberOfModules();
      ret+=ret1;
    }
  }
  return ret;

}

int multiSlsDetector::setNumberOfModules(int p, dimension d) {

  int ret=0, ret1;
  int nm, mm, nt=p;

  thisMultiDetector->dataBytes=0;
  thisMultiDetector->numberOfChannels=0;

  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
    if (detectors[idet]) {
      if (p<0)
	nm=p;
      else {
	mm=detectors[idet]->getMaxNumberOfModules();
	if (nt>mm) {
	  nm=mm;
	  nt-=nm;
	} else {
	  nm=nt;
	  nt-=nm;
	}
      }
      ret+=detectors[idet]->setNumberOfModules(nm);
      thisMultiDetector->dataBytes+=detectors[idet]->getDataBytes();
      thisMultiDetector->numberOfChannels+=detectors[idet]->getTotalNumberOfChannels();
    }
  }  
  return ret;

}

int multiSlsDetector::decodeNMod(int i, int &id, int &im) {
#ifdef VERBOSE
    cout << " Module " << i << " belongs to detector ";
#endif

  if (i<0 || i>=getMaxNumberOfModules()) {
    id=-1;
    im=-1;
#ifdef VERBOSE
    cout  << id << " position " << im << endl;
#endif

    return -1;
  }
  int nm;
  for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
    if (detectors[idet]) {
      nm=detectors[idet]->getNMods();
      if (nm>i) {
	id=idet;
	im=i;
#ifdef VERBOSE
    cout  << id << " position " << im << endl;
#endif
	return im;
      } else {
	i-=nm;
      }
    }
  }
  id=-1;
  im=-1;
#ifdef VERBOSE
    cout  << id << " position " << im << endl;
#endif
  return -1;
  

}


 
int64_t multiSlsDetector::getId(idMode mode, int imod) {

  int id, im;

  if (decodeNMod(imod, id, im)>=0) {
    if (detectors[id]) {
      return detectors[id]->getId(mode, im);
    }
  }

  return -1;

}

int multiSlsDetector::digitalTest(digitalTestMode mode, int imod) {

  int id, im;

  if (decodeNMod(imod, id, im)>=0) {
    if (detectors[id]) {
      return detectors[id]->digitalTest(mode, im);
    }
  }

  return -1;

}





int multiSlsDetector::executeTrimming(trimMode mode, int par1, int par2, int imod) {
  int id, im, ret;


  if (decodeNMod(imod, id, im)>=0) {
    if (detectors[id]) {
      return detectors[id]->executeTrimming(mode, par1, par2, im);
    }
  }  else if (imod<0) {
    for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
      if (detectors[idet]) {
	ret=detectors[idet]->executeTrimming(mode, par1, par2, imod);
      }
    }
    return ret;
  }
  return -1;
}





int multiSlsDetector::loadSettingsFile(string fname, int imod) {
  int id, im, ret;

  if (decodeNMod(imod, id, im)>=0) {
    if (detectors[id]) {
      return detectors[id]->loadSettingsFile(fname, im);
    }
  } else if (imod<0) {
    for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
      if (detectors[idet]) {
	ret=detectors[idet]->loadSettingsFile(fname, imod);
      }
    }
    return ret;
  }
  return -1;

}


int multiSlsDetector::saveSettingsFile(string fname, int imod) {
  int id, im, ret;

  if (decodeNMod(imod, id, im)>=0) {
    if (detectors[id]) {
      return detectors[id]->saveSettingsFile(fname, im);
    }
  } else if (imod<0) {
    for (int idet=0; idet<thisMultiDetector->numberOfDetectors; idet++) {
      if (detectors[idet]) {
	ret=detectors[idet]->saveSettingsFile(fname, imod);
      }
    }
    return ret;
  }
  return -1;

}

int multiSlsDetector::writeRegister(int addr, int val){

  int imi, ima, i;
  int ret, ret1=-100;

  for (i=0; i<thisMultiDetector->numberOfDetectors; i++) {
    if (detectors[i]) {
      ret=detectors[i]->writeRegister(addr,val);
      if (ret1==-100)
	ret1=ret;
      else if (ret!=ret1)
	ret1=-1;
    }
  }

  return ret1;
};


int multiSlsDetector::readRegister(int addr){

  int imi, ima, i;
  int ret, ret1=-100;

  for (i=0; i<thisMultiDetector->numberOfDetectors; i++) {
    if (detectors[i]) {
      ret=detectors[i]->readRegister(addr);
      if (ret1==-100)
	ret1=ret;
      else if (ret!=ret1)
	ret1=-1;
    }
  }

  return ret1;
};


int multiSlsDetector::readConfigurationFile(string const fname){

  

  slsDetectorCommand *cmd=new slsDetectorCommand(this);
  char ext[100];



    string ans;
    string str;
    ifstream infile;
    int iargval;
    int interrupt=0;
    char *args[100];
    for (int ia=0; ia<100; ia++) {
      args[ia]=new char[1000];
    }
    
    
    string sargname, sargval;
    int iline=0;
    std::cout<< "config file name "<< fname << std::endl;
    infile.open(fname.c_str(), ios_base::in);
    if (infile.is_open()) {


    while (infile.good() and interrupt==0) {
	sargname="none";
	sargval="0";
	getline(infile,str);
	iline++;
#ifdef VERBOSE
	std::cout<<  str << std::endl;
#endif
	if (str.find('#')!=string::npos) {
#ifdef VERBOSE
	  std::cout<< "Line is a comment " << std::endl;
	  std::cout<< str << std::endl;
#endif
	  continue;
	} else if (str.length()<2) {
#ifdef VERBOSE
	  std::cout<< "Empty line " << std::endl;
#endif
	  continue;
	} else {
	  istringstream ssstr(str);
	  iargval=0;
	  while (ssstr.good()) {
	    ssstr >> sargname;
	    //if (ssstr.good()) {
#ifdef VERBOSE 
	    std::cout<< iargval << " " << sargname  << std::endl;
#endif
	    strcpy(args[iargval],sargname.c_str());
	    iargval++;
	    //}
	}
	  ans=cmd->executeLine(iargval,args,PUT_ACTION);
#ifdef VERBOSE 
	  std::cout<< ans << std::endl;
#endif
	}
	iline++;
      }




      infile.close();

      
      for (int i=0; i<thisMultiDetector->numberOfDetectors; i++) {
	sprintf(ext,".det%d",i);
	if (detectors[i]) {
	  detectors[i]->readConfigurationFile(fname+string(ext));
	}
      }







    } else {
      std::cout<< "Error opening configuration file " << fname << " for reading" << std::endl;
      return FAIL;
    }
#ifdef VERBOSE
    std::cout<< "Read configuration file of " << iline << " lines" << std::endl;
#endif


    delete cmd;
    return iline;


};  





int multiSlsDetector::writeConfigurationFile(string const fname){




  slsDetectorCommand *cmd=new slsDetectorCommand(this);

  string names[]={				\
    "hostname",					\
    "master",					\
    "sync",					\
    "caldir",					\
    "settingsdir",				\
    "trimen",					\
    "outdir",					\
    "ffdir",					\
    "headerbefore",				\
    "headerafter",				\
    "headerbeforepar",				\
    "headerafterpar",				\
    "nmod",					\
    "badchannels",				\
    "angconv",					\
    "globaloff",				\
    "binsize",					\
    "threaded"				};

  int nvar=18;
 
  char ext[100];
  
  int iv=0;
  char *args[100];
  for (int ia=0; ia<100; ia++) {
    args[ia]=new char[1000];
  }
  
  

  ofstream outfile;
  int ret;
  
  outfile.open(fname.c_str(),ios_base::out);
  if (outfile.is_open()) {

    for (iv=0; iv<nvar; iv++) {
      strcpy(args[0],names[iv].c_str());
      outfile << names[iv] << " " << cmd->executeLine(1,args,GET_ACTION) << std::endl;
    }
    
    
  outfile.close();
  

  for (int i=0; i<thisMultiDetector->numberOfDetectors; i++) {
    sprintf(ext,".det%d",i);
    if (detectors[i]) {
      detectors[i]->writeConfigurationFile(fname+string(ext));
    }
  }

  }
  else {
    std::cout<< "Error opening configuration file " << fname << " for writing" << std::endl;
    return FAIL;
  }
#ifdef VERBOSE
  std::cout<< "wrote " <<ret << " lines to configuration file " << std::endl;
#endif
  delete cmd;
  return iv;

};












int multiSlsDetector::dumpDetectorSetup(string const fname, int level){

  slsDetectorCommand *cmd=new slsDetectorCommand(this);

  string names[]={
    "fname",\
    "index",\
    "flags",\
    "dr",\
    "settings",\
    "threshold",\
    "exptime",\
    "period",\
    "delay",\
    "gates",\
    "frames",\
    "cycles",\
    "probes",\
    "timing",\
    "fineoff",\
    "ratecorr",\
    "startscript",\
    "startscriptpar",\
    "stopscript",\
    "stopscriptpar",\
    "scriptbefore",\
    "scriptbeforepar",\
    "scriptafter",\
    "scriptafterpar",\
    "headerbefore",\
    "headerbeforepar",\
    "headerafter",\
    "headerafterpar",\
    "scan0script",\
    "scan0par",\
    "scan0prec",\
    "scan0steps",\
    "scan1script",\
    "scan1par",\
    "scan1prec",\
    "scan1steps",\
    "flatfield",\
    "badchannels",\
    "angconv"
  };
  int nvar=39;



    char ext[100];

  int iv=0;
  string fname1;



  ofstream outfile;
  char *args[2];
  for (int ia=0; ia<2; ia++) {
    args[ia]=new char[1000];
  }
  int nargs;
  if (level==2)
    nargs=2;
  else
    nargs=1;


  if (level==2) {
    fname1=fname+string(".config");
    writeConfigurationFile(fname1);
    fname1=fname+string(".det");
  } else
    fname1=fname;



  outfile.open(fname1.c_str(),ios_base::out);
  if (outfile.is_open()) {
    for (iv=0; iv<nvar-5; iv++) {
      strcpy(args[0],names[iv].c_str());
      outfile << names[iv] << " " << cmd->executeLine(1,args,GET_ACTION) << std::endl;
    }


    strcpy(args[0],names[iv].c_str());
    if (level==2) {
      fname1=fname+string(".ff");
      strcpy(args[1],fname1.c_str());
    }
    outfile << names[iv] << " " << cmd->executeLine(nargs,args,GET_ACTION) << std::endl;
    iv++;

    strcpy(args[0],names[iv].c_str());
    if (level==2) {
      fname1=fname+string(".bad");
      strcpy(args[1],fname1.c_str());
    }
    outfile << names[iv] << " " << cmd->executeLine(nargs,args,GET_ACTION) << std::endl;
    iv++;

      
    strcpy(args[0],names[iv].c_str());
    if (level==2) {
      fname1=fname+string(".angoff");
      strcpy(args[1],fname1.c_str());
    }
    outfile << names[iv] << " " << cmd->executeLine(nargs,args,GET_ACTION) << std::endl;
    iv++;

    outfile.close();




    for (int i=0; i<thisMultiDetector->numberOfDetectors; i++) {
      sprintf(ext,".det%d",i);
      if (detectors[i]) {
	detectors[i]->dumpDetectorSetup(fname+string(ext), level);
      }
    }

  }
  else {
    std::cout<< "Error opening parameters file " << fname1 << " for writing" << std::endl;
    return FAIL;
  }
  
#ifdef VERBOSE
  std::cout<< "wrote " <<iv << " lines to  "<< fname1 << std::endl;
#endif
  
  delete cmd;
  return 0;

} 




int multiSlsDetector::retrieveDetectorSetup(string const fname1, int level){



  slsDetectorCommand *cmd=new slsDetectorCommand(this);


    char ext[100];
   
   string fname;
   string str;
   ifstream infile;
   int iargval;
   int interrupt=0;
  char *args[2];
  for (int ia=0; ia<2; ia++) {
    args[ia]=new char[1000];
  }
  string sargname, sargval;
  int iline=0;
  
  if (level==2) {
    fname=fname1+string(".config");
    readConfigurationFile(fname);
    //cout << "config file read" << endl;
    fname=fname1+string(".det");
  }  else
    fname=fname1;

  infile.open(fname.c_str(), ios_base::in);
  if (infile.is_open()) {
    while (infile.good() and interrupt==0) {
      sargname="none";
      sargval="0";
      getline(infile,str);
      iline++;
#ifdef VERBOSE
      std::cout<<  str << std::endl;
#endif
      if (str.find('#')!=string::npos) {
#ifdef VERBOSE
	std::cout<< "Line is a comment " << std::endl;
	std::cout<< str << std::endl;
#endif
	continue;
      } else {
	istringstream ssstr(str);
	iargval=0;
	while (ssstr.good()) {
	  ssstr >> sargname;
	  //  if (ssstr.good()) {
	    strcpy(args[iargval],sargname.c_str());
#ifdef VERBOSE
      std::cout<< args[iargval]  << std::endl;
#endif
	    iargval++;
	    // }
	}
	if (level==2) {
	  cmd->executeLine(iargval,args,PUT_ACTION);
	} else {
	  if (string(args[0])==string("flatfield"))
	    ;
	  else if  (string(args[0])==string("badchannels"))
	    ;
	  else if  (string(args[0])==string("angconv"))
	    ;
	  else if (string(args[0])==string("trimbits"))
	    ;
	  else {
	    ;
	    cmd->executeLine(iargval,args,PUT_ACTION);
	  }
	}
      }
      iline++;
    }
    infile.close();
    


    for (int i=0; i<thisMultiDetector->numberOfDetectors; i++) {
      sprintf(ext,".det%d",i);
      if (detectors[i]) {
	detectors[i]->retrieveDetectorSetup(fname1+string(ext), level);
      }
    }


  } else {
    std::cout<< "Error opening  " << fname << " for reading" << std::endl;
    return FAIL;
  }
#ifdef VERBOSE
  std::cout<< "Read  " << iline << " lines" << std::endl;
#endif
  delete cmd;
  return iline;


}



int multiSlsDetector::loadImageToDetector(imageType t, string s) {

}
int multiSlsDetector::testFunction(int times) {

}
  

int multiSlsDetector::writeDataFile(string fname, float *data, float *err, float *ang, char dataformat, int nch) {

#ifdef VERBOSE
  cout << "using overloaded multiSlsDetector function to write formatted data file " << endl;
#endif


  ofstream outfile;
  int idata, choff=0, off=0;
  float *pe=err, *pa=ang;
  int nch_left=nch, n;

  if (nch_left<=0)
    nch_left=getTotalNumberOfChannels();


  if (data==NULL)
    return FAIL;

  //  args|=0x10; // one line per channel!

  outfile.open (fname.c_str(),ios_base::out);
  if (outfile.is_open())
  {
    
   for (int i=0; i<thisMultiDetector->numberOfDetectors; i++) {
     if (detectors[i]) {
       n=detectors[i]->getTotalNumberOfChannels();
       if (nch_left<n)
	 n=nch_left;
       detectors[i]->writeDataFile(outfile,n, data+off, pe, pa, dataformat, choff);
       nch_left-=n;
       choff+=detectors[i]->getMaxNumberOfChannels();
       off+=n;
       if (pe)
	 pe=pe+off;
       if (pa)
	 pa=pa+off;
     }
   }

   outfile.close();
   return OK;
  } else {
    std::cout<< "Could not open file " << fname << "for writing"<< std::endl;
    return FAIL;
  }
}


int multiSlsDetector::writeDataFile(string fname, int *data) {
  ofstream outfile;
  int choff=0, off=0;

#ifdef VERBOSE
  cout << "using overloaded multiSlsDetector function to write raw data file " << endl;
#endif

  if (data==NULL)
    return FAIL;

  outfile.open (fname.c_str(),ios_base::out);
  if (outfile.is_open())
  {
   for (int i=0; i<thisMultiDetector->numberOfDetectors; i++) {
     if (detectors[i]) {
       detectors[i]->writeDataFile(outfile, detectors[i]->getTotalNumberOfChannels(), data+off, choff);
	choff+=detectors[i]->getMaxNumberOfChannels();
	off+=detectors[i]->getTotalNumberOfChannels();
      }
    }


    outfile.close();
    return OK;
  } else {
    std::cout<< "Could not open file " << fname << "for writing"<< std::endl;
    return FAIL;
  }
}


int multiSlsDetector::readDataFile(string fname, float *data, float *err, float *ang, char dataformat){

#ifdef VERBOSE
  cout << "using overloaded multiSlsDetector function to read formatted data file " << endl;
#endif

  ifstream infile;
  int ichan, iline=0;
  int interrupt=0;
  string str;
  int choff=0, off=0;
  float *pe=err, *pa=ang;

#ifdef VERBOSE
  std::cout<< "Opening file "<< fname << std::endl;
#endif
  infile.open(fname.c_str(), ios_base::in);
  if (infile.is_open()) {

    for (int i=0; i<thisMultiDetector->numberOfDetectors; i++) {
      if (detectors[i]) {
	iline+=detectors[i]->readDataFile(detectors[i]->getTotalNumberOfChannels(), infile, data+off, pe, pa, dataformat, choff);
	choff+=detectors[i]->getMaxNumberOfChannels();
	off+=detectors[i]->getTotalNumberOfChannels();
       if (pe)
	 pe=pe+off;
       if (pa)
	 pa=pa+off;
      }
    }


    infile.close();
  } else {
    std::cout<< "Could not read file " << fname << std::endl;
    return -1;
  }
  return iline;

}

int multiSlsDetector::readDataFile(string fname, int *data) {

#ifdef VERBOSE
  cout << "using overloaded multiSlsDetector function to read raw data file " << endl;
#endif

  ifstream infile;
  int ichan, iline=0;
  int interrupt=0;
  string str;
  int choff=0, off=0;

#ifdef VERBOSE
  std::cout<< "Opening file "<< fname << std::endl;
#endif
  infile.open(fname.c_str(), ios_base::in);
  if (infile.is_open()) {

    for (int i=0; i<thisMultiDetector->numberOfDetectors; i++) {
      if (detectors[i]) {
	iline+=detectors[i]->readDataFile(infile, data+off,detectors[i]->getTotalNumberOfChannels(), choff);
	choff+=detectors[i]->getMaxNumberOfChannels();
	off+=detectors[i]->getTotalNumberOfChannels();
      }
    }
    infile.close();
  } else {
    std::cout<< "Could not read file " << fname << std::endl;
    return -1;
  }
  return iline;
}
