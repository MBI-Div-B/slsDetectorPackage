#ifdef SLS_DETECTOR_FUNCTION_LIST


#include <stdio.h>
#include <unistd.h> //to gethostname
#include <string.h>


#include "slsDetectorFunctionList.h"
#include "gitInfoEiger.h"
#include "EigerHighLevelFunctions.c"
#include "EigerBackEndFunctions.c"


enum detectorSettings thisSettings;
//static const string dacNames[16] = {"Svp","Svn","Vtr","Vrf","Vrs","Vtgstv","Vcmp_ll","Vcmp_lr","Cal","Vcmp_rl","Vcmp_rr","Rxb_rb","Rxb_lb","Vcp","Vcn","Vis"};

sls_detector_module *detectorModules=NULL;
int *detectorChips=NULL;
int *detectorChans=NULL;
dacs_t *detectorDacs=NULL;
dacs_t *detectorAdcs=NULL;



int initDetector(){
	  int imod,i,n;
	  n = getNModBoard(1);

  printf("This is the EIGER Server\n");

  //#ifdef VERBOSE
  printf("Board is for %d half modules\n",n);
  //#endif


  detectorModules=malloc(n*sizeof(sls_detector_module));
  detectorChips=malloc(n*NCHIP*sizeof(int));

  detectorChans=malloc(n*NCHIP*NCHAN*sizeof(int));
  detectorDacs=malloc(n*NDAC*sizeof(dacs_t));
  detectorAdcs=malloc(n*NADC*sizeof(dacs_t));
#ifdef VERBOSE
  printf("modules from 0x%x to 0x%x\n",detectorModules, detectorModules+n);
  printf("chips from 0x%x to 0x%x\n",detectorChips, detectorChips+n*NCHIP);
  printf("chans from 0x%x to 0x%x\n",detectorChans, detectorChans+n*NCHIP*NCHAN);
  printf("dacs from 0x%x to 0x%x\n",detectorDacs, detectorDacs+n*NDAC);
  printf("adcs from 0x%x to 0x%x\n",detectorAdcs, detectorAdcs+n*NADC);
#endif
  for (imod=0; imod<n; imod++) {
    (detectorModules+imod)->dacs=detectorDacs+imod*NDAC;
    (detectorModules+imod)->adcs=detectorAdcs+imod*NADC;
    (detectorModules+imod)->chipregs=detectorChips+imod*NCHIP;
    (detectorModules+imod)->chanregs=detectorChans+imod*NCHIP*NCHAN;
    (detectorModules+imod)->ndac=NDAC;
    (detectorModules+imod)->nadc=NADC;
    (detectorModules+imod)->nchip=NCHIP;
    (detectorModules+imod)->nchan=NCHIP*NCHAN;
    (detectorModules+imod)->module=imod;
    (detectorModules+imod)->gain=0;
    (detectorModules+imod)->offset=0;
    (detectorModules+imod)->reg=0;
    /* initialize registers, dacs, retrieve sn, adc values etc */
  }
  thisSettings = STANDARD;/**UNITIALIZED*/
  /*sChan=noneSelected;
  sChip=noneSelected;
  sMod=noneSelected;
  sDac=noneSelected;
  sAdc=noneSelected;
*/

  //get dac values
  int retval[2];
	for(i=0;i<(detectorModules)->ndac;i++){
		setDAC((enum detDacIndex)i,-1,(detectorModules)->module,0,retval);
		(detectorModules)->dacs[i] = retval[0];
	}

  /* initialize dynamic range etc. */


  //set number of frames to 1
  setTimer(FRAME_NUMBER,1);
  setTimer(ACQUISITION_TIME,1E9);
  setTimer(ACQUISITION_TIME,1E9);
  setDynamicRange(16);
  setThresholdEnergy(8000,0);
  setReadOutFlags(PARALLEL);
  setSpeed(0,1);//clk_devider,half speed
 setHighVolage(150,0);
  setIODelay(675,0);
  setTiming(AUTO_TIMING);
  int enable[2] = {0,1};
  setExternalGating(enable);//disable external gating
   return 1;
}


int setNMod(int nm, enum dimension dim){
	return 1;
}



int getNModBoard(enum dimension arg){
	return 1;
}



int64_t getModuleId(enum idMode arg, int imod){

/**/
	return -1;
}




int64_t getDetectorId(enum idMode arg){
	int64_t retval = -1;

	switch(arg){
	case DETECTOR_SERIAL_NUMBER:
		retval =  getDetectorNumber();/** to be implemented with mac? */
		break;
	case DETECTOR_FIRMWARE_VERSION:
		return FIRMWAREREV;/** to be implemented */
	case DETECTOR_SOFTWARE_VERSION:
		retval= SVNREV;
		retval= (retval <<32) | SVNDATE;
		break;
	default:
		break;
	}

	return retval;
}



int getDetectorNumber(){
	int res=0;
    char hostname[100];
    if (gethostname(hostname, sizeof hostname) == 0)
        puts(hostname);
    else
        perror("gethostname");
    sscanf(hostname,"%x",&res);
    return res;


/*
	char output[255]="";
	int res=0;
	FILE* sysFile = popen("hostname", "r");
	fgets(output, sizeof(output), sysFile);
	pclose(sysFile);
	sscanf(output,"%x",&res);
	return res;
	*/

}


u_int64_t  getDetectorMAC() {
  /*
	char output[255],mac[255]="";
	u_int64_t res=0;
	FILE* sysFile = popen("ifconfig eth0 | grep HWaddr | cut -d \" \" -f 11", "r");
	fgets(output, sizeof(output), sysFile);
	pclose(sysFile);
	//getting rid of ":"
	char * pch;
	pch = strtok (output,":");
	while (pch != NULL){
		strcat(mac,pch);
		pch = strtok (NULL, ":");
	}
	sscanf(mac,"%llx",&res);
	printf("mac:%llx\n",res);
	return res;
  */
  return 0;
}

int moduleTest( enum digitalTestMode arg, int imod){
	//template testShiftIn from mcb_funcs.c

	//CHIP_TEST
	//testShiftIn
	//testShiftOut
	//testShiftStSel
	//testDataInOutMux
	//testExtPulseMux
	//testOutMux
	//testFpgaMux

	return OK;
}





int detectorTest( enum digitalTestMode arg){
	//templates from firmware_funcs.c

	//DETECTOR_FIRMWARE_TEST:testFpga()
	//DETECTOR_MEMORY_TEST:testRAM()
	//DETECTOR_BUS_TEST:testBus()
	//DETECTOR_SOFTWARE_TEST:testFpga()
	return OK;
}






void setDAC(enum detDacIndex ind, int val, int imod, int mV, int retval[]){
	char iname[10];
	strcpy(iname,EigerGetDACName((int)ind));
#ifdef VERBOSE
	if(val >= 0)
		printf("Setting dac %d: %s to %d ",ind, iname,val);
	else
		printf("Getting dac %d: %s ",ind, iname);
	if(mV)
		printf("in mV\n");
	else
		printf("in dac units\n");
#endif
	if(val >= 0)
		EigerSetDAC(iname,val,mV);
	retval[0] = EigerGetDAC(iname);
	retval[1] = EigerGetDACmV(iname);

	(detectorModules)->dacs[ind] = retval[0];

}


int setHighVolage(int val, int imod){
	if(val!=-1){
		printf(" Setting High Voltage: %d\n",val);
		EigerSetHighVoltage(val);
	}
	return EigerGetHighVoltage();
}


int getADC(enum detDacIndex ind,  int imod){
	//get adc value
	return 0;
}


int setIODelay(int val, int imod){
	if(val!=-1){
		printf(" Setting IO Delay: %d\n",val);
		EigerSetIODelay(val);
	}
	return EigerGetIODelay();
}


int enableTenGigabitEthernet(int val){
	if(val!=-1){
		if(val>0)
			SetTenGigbaBitEthernet(1);
		else
			SetTenGigbaBitEthernet(0);
		//configuremac called from client
	}
	return GetTenGigbaBitEthernet();
}


int setModule(sls_detector_module myMod){
	int retval[2];
  #ifdef VERBOSE
	printf("Setting module with settings %d\n",myMod.reg);
#endif
	int i;
	for(i=0;i<myMod.ndac;i++)
		setDAC((enum detDacIndex)i,myMod.dacs[i],myMod.module,0,retval);


	thisSettings = (enum detectorSettings)myMod.reg;
	thisSettings = 0;
/** set trimbits*/
	if (detectorModules)
		copyModule(detectorModules,&myMod);
  return 0;
}


int getModule(sls_detector_module *myMod){
	  if (detectorModules)
	    copyModule(myMod,detectorModules);/*copyModule(myMod,detectorModules+iMod);*/
	  else
	    return FAIL;
	return OK;
}






int getThresholdEnergy(int imod){
	printf(" Getting Threshold energy\n");
	return EigerGetPhotonEnergy();
}


int setThresholdEnergy(int thr, int imod){
	printf(" Setting threshold energy:%d\n",thr);
	EigerSetPhotonEnergy(thr);
	return EigerGetPhotonEnergy();
}



enum detectorSettings setSettings(enum detectorSettings sett, int imod){
	if(sett != GET_SETTINGS)
		thisSettings = sett;
	return thisSettings;
}





int startStateMachine(){
	printf("Going to start acquisition\n");
	EigerStartAcquisition();
	RequestImages();
	return OK;
}


int stopStateMachine(){
	printf("Going to stop acquisition\n");
	EigerStopAcquisition();
	return OK;
}


int startReadOut(){
	RequestImages();
	return OK;
}


enum runStatus getRunStatus(){
	int i = EigerRunStatus();
	if(i== 0){
		printf("IDLE\n");
		return IDLE;
	}else{
		printf("RUNNING\n");
		return RUNNING;
	}
}



char *readFrame(int *ret, char *mess){
	EigerWaitForAcquisitionFinish();
	/*
	int i = EigerRunStatus();
	while(i){
		i = EigerRunStatus();
		usleep(1000);
	}*/
	*ret = (int)FINISHED;
	return NULL;
}







int64_t setTimer(enum timerIndex ind, int64_t val){

	switch(ind){
	case FRAME_NUMBER:
		if(val >= 0){
			printf(" Setting number of frames: %d\n",(unsigned int)val);
			EigerSetNumberOfExposures((unsigned int)val);
			SetDestinationParameters(EigerGetNumberOfExposures()*EigerGetNumberOfCycles());
		}return EigerGetNumberOfExposures();
	case ACQUISITION_TIME:
		if(val >= 0){
			printf(" Setting exp time: %fs\n",val/(1E9));
			EigerSetExposureTime(val/(1E9));
		}return (EigerGetExposureTime()*(1E9));
	case FRAME_PERIOD:
		if(val >= 0){
			printf(" Setting acq period: %fs\n",val/(1E9));
			EigerSetExposurePeriod(val/(1E9));
		}return (EigerGetExposurePeriod()*(1E9));
/*	case DELAY_AFTER_TRIGGER:
		if(val >= 0)
			EigerSetNumberOfExposures((unsigned int)val);
		return EigerGetNumberOfExposures();

	case GATES_NUMBER:
		if(val >= 0)
			EigerSetNumberOfGates((unsigned int)val);
		return EigerGetNumberOfGates();

	case PROBES_NUMBER:
		if(val >= 0)
			EigerSetNumberOfExposures((unsigned int)val);
		return EigerGetNumberOfExposures();*/
	case CYCLES_NUMBER:
		if(val >= 0){
			printf(" Setting number of triggers: %d\n",(unsigned int)val);
			EigerSetNumberOfCycles((unsigned int)val);
			SetDestinationParameters(EigerGetNumberOfExposures()*EigerGetNumberOfCycles());
		}return EigerGetNumberOfCycles();
	default:
		printf("unknown timer index: %d\n",ind);
		break;
	}

	return -1;
}




int64_t getTimeLeft(enum timerIndex ind){

	return -1;
}



int setDynamicRange(int dr){
	int r;
	if(dr > 0){
		printf(" Setting dynamic range: %d\n",dr);
		EigerSetDynamicRange(dr);
		EigerSetBitMode(dr);
	}
	//make sure back end and front end have the same bit mode
	r= EigerGetDynamicRange();
	if(r != EigerGetBitMode())
		EigerSetBitMode(r);

	return r;
}



enum readOutFlags setReadOutFlags(enum readOutFlags val){
	int ret;
	if(val!=GET_READOUT_FLAGS){
		switch(val){
		case PARALLEL:  	val=0; break;
		case NONPARALLEL:	val=1; break;
		case SAFE:			val=2; break;
		default:  			val=0; break;
		}
		printf(" Setting Read out Flag: %d\n",val);
		EigerSetReadoutMode(val);
	}
	switch(EigerGetReadoutMode()){
	case 0: ret=PARALLEL; 		break;
	case 1:	ret=NONPARALLEL; 	break;
	case 2:	ret=SAFE; 			break;
	default:ret=-1; 			break;
	}

  return ret;
}




int setROI(int n, ROI arg[], int *retvalsize, int *ret){
	return FAIL;
}



int setSpeed(enum speedVariable arg, int val){
	if(val != -1){
		printf(" Setting Read out Speed: %d\n",val);
		EigerSetReadoutSpeed(val);
	}
	return 	EigerGetReadoutSpeed();
}



int executeTrimming(enum trimMode mode, int par1, int par2, int imod){
	return FAIL;
}


int configureMAC(int ipad, long long int macad, long long int detectormacadd, int detipad, int udpport, int ival){
	EigerSetupTableEntryLeft(ipad, macad, detectormacadd, detipad, udpport);
	EigerSetupTableEntryRight(ipad, macad, detectormacadd, detipad, udpport);
	SetDestinationParameters(EigerGetNumberOfExposures()*EigerGetNumberOfCycles());
	return 0;
}


int calculateDataBytes(){
	return setDynamicRange(-1)*16*1040;
}


int copyModule(sls_detector_module *destMod, sls_detector_module *srcMod){

	int ichip, idac,  ichan, iadc;
	int ret=OK;

#ifdef VERBOSE
	printf("Copying module %x to module %x\n",srcMod,destMod);
#endif

	if (srcMod->module>=0) {
#ifdef VERBOSE
		printf("Copying module number %d to module number %d\n",srcMod->module,destMod->module);
#endif
		destMod->module=srcMod->module;
	}
	if (srcMod->serialnumber>=0){

		destMod->serialnumber=srcMod->serialnumber;
	}
	if ((srcMod->nchip)>(destMod->nchip)) {
		printf("Number of chip of source is larger than number of chips of destination\n");
		return FAIL;
	}
	if ((srcMod->nchan)>(destMod->nchan)) {
		printf("Number of channels of source is larger than number of channels of destination\n");
		return FAIL;
	}
	if ((srcMod->ndac)>(destMod->ndac)) {
		printf("Number of dacs of source is larger than number of dacs of destination\n");
		return FAIL;
	}
	if ((srcMod->nadc)>(destMod->nadc)) {
		printf("Number of dacs of source is larger than number of dacs of destination\n");
		return FAIL;
	}

#ifdef VERBOSE
	printf("DACs: src %d, dest %d\n",srcMod->ndac,destMod->ndac);
	printf("ADCs: src %d, dest %d\n",srcMod->nadc,destMod->nadc);
	printf("Chips: src %d, dest %d\n",srcMod->nchip,destMod->nchip);
	printf("Chans: src %d, dest %d\n",srcMod->nchan,destMod->nchan);

#endif
	destMod->ndac=srcMod->ndac;
	destMod->nadc=srcMod->nadc;
	destMod->nchip=srcMod->nchip;
	destMod->nchan=srcMod->nchan;
	if (srcMod->reg>=0)
		destMod->reg=srcMod->reg;
#ifdef VERBOSE
	printf("Copying register %x (%x)\n",destMod->reg,srcMod->reg );
#endif
	if (srcMod->gain>=0)
		destMod->gain=srcMod->gain;
	if (srcMod->offset>=0)
		destMod->offset=srcMod->offset;

	for (ichip=0; ichip<(srcMod->nchip); ichip++) {
		if (*((srcMod->chipregs)+ichip)>=0)
			*((destMod->chipregs)+ichip)=*((srcMod->chipregs)+ichip);
	}
	for (ichan=0; ichan<(srcMod->nchan); ichan++) {
		if (*((srcMod->chanregs)+ichan)>=0)
			*((destMod->chanregs)+ichan)=*((srcMod->chanregs)+ichan);
	}
	for (idac=0; idac<(srcMod->ndac); idac++) {
		if (*((srcMod->dacs)+idac)>=0)
			*((destMod->dacs)+idac)=*((srcMod->dacs)+idac);
	}
	for (iadc=0; iadc<(srcMod->nadc); iadc++) {
		if (*((srcMod->adcs)+iadc)>=0)
			*((destMod->adcs)+iadc)=*((srcMod->adcs)+iadc);
	}
	return ret;
}


int getTotalNumberOfChannels(){return getNumberOfChannelsPerModule();};//NCHIP*NCHAN*nModBoard;}
int getTotalNumberOfChips(){return 4;};//NCHIP*nModBoard;}
int getTotalNumberOfModules(){return 1;}//nModBoard;}
int getNumberOfChannelsPerChip(){return  (256*256);}//NCHAN;}
int getNumberOfChannelsPerModule(){return  getNumberOfChannelsPerChip() * getTotalNumberOfChips();}//NCHAN*NCHIP;}
int getNumberOfChipsPerModule(){return  4;}//NCHIP;}
int getNumberOfDACsPerModule(){return  16;}//NDAC;}
int getNumberOfADCsPerModule(){return  0;}//NADC;}







enum externalSignalFlag getExtSignal(int signalindex){
	return GET_EXTERNAL_SIGNAL_FLAG;
}





enum externalSignalFlag setExtSignal(int signalindex,  enum externalSignalFlag flag){
	return getExtSignal(signalindex);
}






enum externalCommunicationMode setTiming( enum externalCommunicationMode arg){
	enum externalCommunicationMode ret=GET_EXTERNAL_COMMUNICATION_MODE;
	if(arg != GET_EXTERNAL_COMMUNICATION_MODE){
		switch((int)arg){
		case AUTO_TIMING:			ret = 0;	break;
		case TRIGGER_EXPOSURE:		ret = 2;	break;
		case TRIGGER_READOUT:		ret = 1;	break;
		case GATE_FIX_NUMBER:		ret = 3;	break;
		}
		printf(" Setting Triggering Mode: %d\n",(int)ret);
		EigerSetTriggerMode(ret);
	}

	ret = EigerGetTriggerMode();
	switch((int)ret){
	case 0:		ret = AUTO_TIMING;		break;
	case 2:		ret = TRIGGER_EXPOSURE; break;
	case 1:		ret = TRIGGER_READOUT;	break;
	case 3:		ret = GATE_FIX_NUMBER;	break;
	default:
		printf("Unknown trigger mode found %d\n",ret);
		ret = 0;
	}
	return ret;
}


void setExternalGating(int enable[]){
	if(enable>=0)
		EigerSetExternalGating(enable[0], enable[1]);//enable = 0 or 1, polarity = 0 or 1 , where 1 is positive
	enable[0] = EigerGetExternalGatingPolarity();
	enable[1] = EigerGetExternalGating();
}


enum masterFlags setMaster(enum masterFlags arg){
	return NO_MASTER;
}



enum synchronizationMode setSynchronization(enum synchronizationMode arg){
	return NO_SYNCHRONIZATION;
}



#endif
