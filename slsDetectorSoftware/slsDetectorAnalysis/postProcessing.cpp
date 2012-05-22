#include "postProcessing.h"


postProcessing::postProcessing(){							
  pthread_mutex_t mp1 = PTHREAD_MUTEX_INITIALIZER;
  mp=mp1;
  pthread_mutex_init(&mp, NULL);  
  mg=mp1;
  pthread_mutex_init(&mg, NULL);  
}


int postProcessing::flatFieldCorrect(float datain, float errin, float &dataout, float &errout, float ffcoefficient, float fferr){
  float e;

  dataout=datain*ffcoefficient;

  if (errin==0 && datain>=0) 
    e=sqrt(datain);
  else
    e=errin;
  
  if (dataout>0)
    errout=sqrt(e*ffcoefficient*e*ffcoefficient+datain*fferr*datain*fferr);
  else
    errout=1.;
  
  return 0;
};


 int postProcessing::rateCorrect(float datain, float errin, float &dataout, float &errout, float tau, float t){

   // float data;
   float e;
 
   dataout=(datain*exp(tau*datain/t));
   
   if (errin==0 && datain>=0) 
     e=sqrt(datain);
   else
     e=errin;
   
   if (dataout>0)
     errout=e*dataout*sqrt((1/(datain*datain)+tau*tau/(t*t)));
   else 
     errout=1.;
   return 0;

};


int postProcessing::setBadChannelCorrection(string fname, int &nbad, int *badlist){
  ifstream infile;
  string str;
  int interrupt=0;
  int ich;
  int chmin,chmax;
#ifdef VERBOSE
  std::cout << "Setting bad channel correction to " << fname << std::endl;
#endif

  if (fname=="" || fname=="none") {
    nbad=0;
    return 0;
  } else { 
    infile.open(fname.c_str(), ios_base::in);
    if (infile.is_open()==0) {
      std::cout << "could not open file " << fname <<std::endl;
      return -1;
    }
    nbad=0;
    while (infile.good() and interrupt==0) {
      getline(infile,str);
#ifdef VERBOSE
      std::cout << str << std::endl;
#endif
      istringstream ssstr;
      ssstr.str(str);
      if (ssstr.bad() || ssstr.fail() || infile.eof()) {
	interrupt=1;
	break;
      }
      if (str.find('-')!=string::npos) {
	ssstr >> chmin ;
	ssstr.str(str.substr(str.find('-')+1,str.size()));
	ssstr >> chmax;
#ifdef VERBOSE
	std::cout << "channels between"<< chmin << " and " << chmax << std::endl;
#endif
	for (ich=chmin; ich<=chmax; ich++) {
	  if (nbad<MAX_BADCHANS) {
	    badlist[nbad]=ich;
	    nbad++;
#ifdef VERBOSE
	    std::cout<< nbad << " Found bad channel "<< ich << std::endl;
#endif
	  } else
	    interrupt=1;
	}
      } else {
	ssstr >> ich;
#ifdef VERBOSE
	std::cout << "channel "<< ich << std::endl;
#endif
	if (nbad<MAX_BADCHANS) {
	  badlist[nbad]=ich;
	  nbad++;
#ifdef VERBOSE
	  std::cout << nbad << " Found bad channel "<< ich << std::endl;
#endif
	} else
	  interrupt=1;
      }
    }
  }
  infile.close();
  if (nbad>0 && nbad<MAX_BADCHANS) {
    return nbad;
  } else
  return 0;
}


void postProcessing::processFrame(int *myData, int delflag) {

  string fname;
  // float *fdata=NULL;
  
  
 incrementProgress();
 
 /** decode data */
 
 fdata=decodeData(myData, fdata);
 
 fname=createFileName();
 
  
 //uses static function?!?!?!?
 writeDataFile (fname+string(".raw"),fdata, NULL, NULL, 'i'); 
 
 doProcessing(fdata,delflag, fname);

  delete [] myData;
  myData=NULL;
  fdata=NULL;

#ifdef VERBOSE
  cout << "Pop data queue " << *fileIndex << endl;
#endif
  
  pthread_mutex_lock(&mp);
  dataQueue.pop(); //remove the data from the queue
  queuesize=dataQueue.size();
  pthread_mutex_unlock(&mp);
  
  
}




void postProcessing::doProcessing(float *lfdata, int delflag, string fname) {


  /** write raw data file */	   
  if (*correctionMask==0 && delflag==1) {
    //  delete [] fdata;
    ;
  } else {


    

    float *rcdata=NULL, *rcerr=NULL;
    float *ffcdata=NULL, *ffcerr=NULL;
    float *ang=NULL;
    // int imod;
    int np;
    //string fname;
    detectorData *thisData;

    
    string ext=".dat";
    // fname=createFileName();

    /** rate correction */
    if (*correctionMask&(1<<RATE_CORRECTION)) {
      rcdata=new float[getTotalNumberOfChannels()]; 
      rcerr=new float[getTotalNumberOfChannels()];
      rateCorrect(lfdata,NULL,rcdata,rcerr);
      delete [] lfdata;
    } else {
      rcdata=lfdata;
    }
    lfdata=NULL;
    
	

 	  
    /** flat field correction */
    if (*correctionMask&(1<<FLAT_FIELD_CORRECTION)) {
      
      ffcdata=new float[getTotalNumberOfChannels()]; 
      ffcerr=new float[getTotalNumberOfChannels()];
      flatFieldCorrect(rcdata,rcerr,ffcdata,ffcerr);
      delete [] rcdata;
      rcdata=NULL;
      if (rcerr)	    
	delete [] rcerr;
      rcerr=NULL;
    } else {
      ffcdata=rcdata;
      ffcerr=rcerr;
      rcdata=NULL;
      rcerr=NULL;
    }

    // writes angualr converted files

    if (*correctionMask!=0) {
      if (*correctionMask&(1<< ANGULAR_CONVERSION))
	ang=convertAngles();
      writeDataFile (fname+ext,  ffcdata, ffcerr,ang);
    }
  
    if (*correctionMask&(1<< ANGULAR_CONVERSION) && getNumberOfPositions()>0) {
#ifdef VERBOSE
      cout << "**************Current position index is " << getCurrentPositionIndex() << endl;
#endif
      // if (*numberOfPositions>0) {
      if (getCurrentPositionIndex()<=1) {
	   
#ifdef VERBOSE
	cout << "reset merging " << endl;
#endif
	resetMerging();
      }
      
#ifdef VERBOSE
      cout << "add to merging "<< getCurrentPositionIndex() << endl;
#endif

      if (*correctionMask&(1<< ANGULAR_CONVERSION))
	addToMerging(ang, ffcdata, ffcerr, badChannelMask );
     
      
#ifdef VERBOSE
      cout << getCurrentPositionIndex() << " " << getNumberOfPositions() << endl;
	
#endif
      
      
      //  cout << "lock 1" << endl;
      pthread_mutex_lock(&mp);
      if ((getCurrentPositionIndex()>=getNumberOfPositions() && posfinished==1 && queuesize==1)) {
	  
#ifdef VERBOSE
	cout << "finalize merging " << getCurrentPositionIndex()<< endl;
#endif
	np=finalizeMerging();
	/** file writing */
	incrementPositionIndex();
	//	cout << "unlock 1" << endl;
	pthread_mutex_unlock(&mp);
	
	
	fname=createFileName();
	
#ifdef VERBOSE
	cout << "writing merged data file" << endl;
#endif
	writeDataFile (fname+ext,np,getMergedCounts(), getMergedErrors(), getMergedPositions(),'f');
#ifdef VERBOSE
	cout << " done" << endl;
#endif
		
	

	if (delflag) {
	  deleteMerging();
	} else {
	  thisData=new detectorData(getMergedCounts(),getMergedErrors(),getMergedPositions(),getCurrentProgress(),(fname+ext).c_str(),np);
	  
	  // cout << "lock 2" << endl;
	  pthread_mutex_lock(&mg);
	  finalDataQueue.push(thisData);
	  // cout << "unlock 2" << endl;
	  pthread_mutex_unlock(&mg);
	}
	//	cout << "lock 3" << endl;
	pthread_mutex_lock(&mp);
      }
      // cout << "unlock 3" << endl;
      pthread_mutex_unlock(&mp);
	
      if (ffcdata)
	delete [] ffcdata;
     
      ffcdata=NULL;

      if (ffcerr) 
	delete [] ffcerr;
      ffcerr=NULL;

      if (ang)
	delete [] ang;
      ang=NULL;
      
    }   else { 
      if (delflag) {
	if (ffcdata)
	  delete [] ffcdata;
	if (ffcerr)
	  delete [] ffcerr;
	if ( ang)
	  delete [] ang;
      } else {
	thisData=new detectorData(ffcdata,ffcerr,NULL,getCurrentProgress(),(fname+ext).c_str(),getTotalNumberOfChannels());
	pthread_mutex_lock(&mg);
	finalDataQueue.push(thisData);  
	pthread_mutex_unlock(&mg);
      }
    }
  }

  incrementFileIndex();
#ifdef VERBOSE
  cout << "fdata is " << fdata << endl;
#endif
  
}





int postProcessing::fillBadChannelMask() {

  int nbad=0;

  if (*correctionMask&(1<< DISCARD_BAD_CHANNELS)) {
    nbad=getBadChannelCorrection();
#ifdef VERBOSE
    cout << "number of bad channels is " << nbad << endl;
#endif
    if (nbad>0) {
      
      int *badChansList=new int[nbad];
      getBadChannelCorrection(badChansList);

      if (badChannelMask) 
	delete [] badChannelMask;
      badChannelMask=new int[getTotalNumberOfChannels()];

#ifdef VERBOSE
      cout << " pointer to bad channel mask is " << badChannelMask << endl;
#endif
      for (int ichan=0; ichan<getTotalNumberOfChannels(); ichan++)
	badChannelMask[ichan]=0;
#ifdef VERBOSE
      cout << " badChanMask has be reset" << badChannelMask << endl;
#endif
      for (int ichan=0; ichan<nbad; ichan++) {
	if (badChansList[ichan]<getTotalNumberOfChannels() && badChansList[ichan]>=0 ) {
	  if (badChannelMask[badChansList[ichan]]==0)
	    nbad++;
	  badChannelMask[badChansList[ichan]]=1;
	  
	}
      }
      delete [] badChansList;

    } else {
      if (badChannelMask) {
#ifdef VERBOSE
      cout << "deleting bad channel mask beacuse number of bad channels is 0" << endl;
#endif
	
	delete [] badChannelMask;
	badChannelMask=NULL;
      }
    }

  } else {
#ifdef VERBOSE
    cout << "bad channel correction is disabled " << nbad << endl;
#endif
    if (badChannelMask) {
#ifdef VERBOSE
      cout << "deleting bad channel mask beacuse no bad channel correction is selected" << endl;
#endif
      delete [] badChannelMask;
      badChannelMask=NULL;
    }
  }

#ifdef VERBOSE
    cout << "number of bad channels is " << nbad << endl;
#endif
  return  nbad;
}






void* postProcessing::processData(int delflag) {


#ifdef VERBOSE
  std::cout<< " processing data - threaded mode " << *threadedProcessing << endl;
#endif



  setTotalProgress();
  pthread_mutex_lock(&mp);
  queuesize=dataQueue.size();
  pthread_mutex_unlock(&mp);

  int *myData;
  int dum=1;

  fdata=NULL;


  while(dum | *threadedProcessing) { // ????????????????????????
    
    
    /* IF THERE ARE DATA PROCESS THEM*/
    pthread_mutex_lock(&mp);
    while((queuesize=dataQueue.size())>0) {
      /** Pop data queue */
      myData=dataQueue.front(); // get the data from the queue 
      pthread_mutex_unlock(&mp);
     
      if (myData) {
	processFrame(myData,delflag);
	//usleep(1000);
      }
      pthread_mutex_lock(&mp);

    }
    pthread_mutex_unlock(&mp);
   
    /* IF THERE ARE NO DATA look if acquisition is finished */
    pthread_mutex_lock(&mp);
    if (jointhread) {
      if (dataQueue.size()==0) {
	pthread_mutex_unlock(&mp);
	break;
      }
      pthread_mutex_unlock(&mp);
    } else {
      pthread_mutex_unlock(&mp);
    }
    dum=0;
  }

  if (fdata) {
#ifdef VERBOSE
    cout << "delete fdata" << endl;
#endif
    delete [] fdata;
#ifdef VERBOSE
    cout << "done " << endl;
#endif
  }
  return 0;
}


int* postProcessing::popDataQueue() {
  int *retval=NULL;
  if( !dataQueue.empty() ) {
    retval=dataQueue.front();
    dataQueue.pop();
  }
  return retval;
}

detectorData* postProcessing::popFinalDataQueue() {
  detectorData *retval=NULL;
  pthread_mutex_unlock(&mg);
  if( !finalDataQueue.empty() ) {
    retval=finalDataQueue.front();
    finalDataQueue.pop();
  }
  pthread_mutex_unlock(&mg);
  return retval;
}

void postProcessing::resetDataQueue() {
  int *retval=NULL;
  while( !dataQueue.empty() ) {
    retval=dataQueue.front();
    dataQueue.pop();
    delete [] retval;
  }
 
}

void postProcessing::resetFinalDataQueue() {
  detectorData *retval=NULL;
  pthread_mutex_lock(&mg);
  while( !finalDataQueue.empty() ) {
    retval=finalDataQueue.front();
    finalDataQueue.pop();
    delete retval;
  }
  pthread_mutex_unlock(&mg);
}


void postProcessing::startThread(int delflag) {
  pthread_attr_t tattr;
  int ret;
  sched_param param, mparam;
  int policy= SCHED_OTHER;


  // set the priority; others are unchanged
  //newprio = 30;
  mparam.sched_priority =1;
  param.sched_priority =1;   


   /* Initialize and set thread detached attribute */
   pthread_attr_init(&tattr);
   pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_JOINABLE);



  // param.sched_priority = 5;
  // scheduling parameters of main thread 
  ret = pthread_setschedparam(pthread_self(), policy, &mparam);
  //#ifdef VERBOSE
    // printf("current priority is %d\n",param.sched_priority);
  //#endif
  if (delflag)
    ret = pthread_create(&dataProcessingThread, &tattr,startProcessData, (void*)this);
  else
    ret = pthread_create(&dataProcessingThread, &tattr,startProcessDataNoDelete, (void*)this);
    
  pthread_attr_destroy(&tattr);
   // scheduling parameters of target thread
  ret = pthread_setschedparam(dataProcessingThread, policy, &param);
  
}



