//#include "ansi.h"
#include <iostream>



//#include "moench03T1ZmqData.h"
#ifdef NEWRECEIVER
#include "moench03T1ReceiverDataNew.h"
#endif
#ifdef CSAXS_FP
#include "moench03T1ReceiverData.h"
#endif 
#ifdef OLDDATA
#include "moench03Ctb10GbT1Data.h"
#endif 

// #include "interpolatingDetector.h"
//#include "etaInterpolationPosXY.h"
// #include "linearInterpolation.h"
// #include "noInterpolation.h"
#include "multiThreadedAnalogDetector.h"
#include "singlePhotonDetector.h"
//#include "interpolatingDetector.h"

#include <stdio.h>
#include <map>
#include <fstream>
#include <sys/stat.h>

#include <ctime>
using namespace std;


int main(int argc, char *argv[]) {


  if (argc<4) {
    cout << "Usage is " << argv[0] << "indir outdir fname [pedfile] [threshold]" << endl;
    return 1;
  }
  int p=10000;
  int fifosize=1000;
  int nthreads=8;
  int nsubpix=25;
  int etabins=nsubpix*10;
  double etamin=-1, etamax=2;
  int csize=3;
  int nx=400, ny=400;
  int save=1;
  int nsigma=5;
  int nped=1000;
  int ndark=100;
  int ok;
  int iprog=0;



#ifdef NEWRECEIVER
  moench03T1ReceiverDataNew *decoder=new  moench03T1ReceiverDataNew();
  cout << "RECEIVER DATA WITH ONE HEADER!"<<endl;
#endif

#ifdef CSAXS_FP
  moench03T1ReceiverData *decoder=new  moench03T1ReceiverData();
  cout << "RECEIVER DATA WITH ALL HEADERS!"<<endl;
#endif

#ifdef OLDDATA
  moench03Ctb10GbT1Data *decoder=new  moench03Ctb10GbT1Data();
  cout << "OLD RECEIVER DATA!"<<endl;
#endif

#ifndef ANALOG
  //moench03T1ZmqData *decoder=new  moench03T1ZmqData();
  singlePhotonDetector *filter=new singlePhotonDetector(decoder,csize, nsigma, 1, 0, nped, 200);
  //  char tit[10000];
  cout << "filter " << endl;
#endif

#ifdef ANALOG
  //moench03T1ZmqData *decoder=new  moench03T1ZmqData();
  analogDetector<uint16_t> *filter=new analogDetector<uint16_t>(decoder, 1, 0, nped);
  //  char tit[10000];
  cout << "filter " << endl;
#endif



  // filter->readPedestals("/scratch/ped_100.tiff");
  // interp->readFlatField("/scratch/eta_100.tiff",etamin,etamax);
  // cout << "filter "<< endl;
  

  int size = 327680;////atoi(argv[3]);
  
  int* image;
	//int* image =new int[327680/sizeof(int)];
  filter->newDataSet();


  int ff, np;
  int dsize=decoder->getDataSize();
  cout << " data size is " << dsize;
  

  char data[dsize];

  ifstream filebin;
  char *indir=argv[1];
  char *outdir=argv[2];
  char *fformat=argv[3];
  char *pedfile=NULL;
  if (argc>=5) {
    pedfile=argv[4];
  }
  double thr=0;
  if (argc>=6) {
    thr=atoi(argv[5]);
  }
  

  char fname[10000];
  char imgfname[10000];
  //  strcpy(pedfname,argv[6]);
  char fn[10000];
  
  std::time_t end_time;

  FILE *of=NULL;
  cout << "input directory is " << indir << endl;
  cout << "output directory is " << outdir << endl;
  cout << "input file is " << fformat << endl;
  if (pedfile)
    cout << "pedestal file is " << pedfile << endl;
  if (thr>0) {
    cout << "threshold is " << thr << endl;
    filter->setThreshold(thr);
  }

  filter->setROI(0,150,0,150);

  std::time(&end_time);
  cout << std::ctime(&end_time) <<   endl;
 







  char* buff;
  multiThreadedAnalogDetector *mt=new multiThreadedAnalogDetector(filter,nthreads,fifosize);

 
  mt->StartThreads();
  mt->popFree(buff);


  cout << "mt " << endl;

  int ifr=0;
 









  for (int irun=0; irun<2; irun++) {
    if (irun>0) {
      mt->setFrameMode(eFrame);
      // sprintf(fn,fformat,irun);
      sprintf(fname,"%s/%s.raw",indir,fformat);
    //  sprintf(outfname,"%s/%s.clust",outdir,fn);
      sprintf(imgfname,"%s/%s.tiff",outdir,fformat);
    } else {
      mt->setFrameMode(ePedestal);
      // sprintf(fn,fformat,irun);
      sprintf(fname,"%s/%s.raw",indir,pedfile);
    //  sprintf(outfname,"%s/%s.clust",outdir,fn);
      // sprintf(imgfname,"%s/%s.tiff",outdir,fn);
    }
    cout << fname << endl;
    std::time(&end_time);
    cout << std::ctime(&end_time) <<    endl;
    //  cout <<  fname << " " << outfname << " " << imgfname <<  endl;
    filebin.open((const char *)(fname), ios::in | ios::binary);
    //      //open file
    if (filebin.is_open()){
      // of=fopen(outfname,"w");
      // if (of) {
      // 	mt->setFilePointer(of);
      // 	//	cout << "file pointer set " << endl;
      // } else {
      // 	cout << "Could not open "<< outfname << " for writing " << endl;
      // 	mt->setFilePointer(NULL);
      // 	return 1;
      // }
      //     //while read frame 
      ff=-1;
      while (decoder->readNextFrame(filebin, ff, np,buff)) {
	//	cout << "*"<<ifr++<<"*"<<ff<< endl;
	//	cout << ff << " " << np << endl;
  	//         //push
	mt->pushData(buff);
  // 	//         //pop
	mt->nextThread();
  // // 		//	cout << " " << (void*)buff;
	mt->popFree(buff);
	ifr++;
	if (ifr%10000==0) cout << ifr << " " << ff << endl;
	ff=-1;
      }
      cout << "--" << endl;
      filebin.close();	 
      //      //close file 
      //     //join threads
      while (mt->isBusy()) {;}//wait until all data are processed from the queues
      // if (of)
      // 	fclose(of);
      if (irun>0) {
	cout << "Writing tiff to " << imgfname << endl;
	mt->writeImage(imgfname);
	//	mt->clearImage();
      }
      std::time(&end_time);
      cout << std::ctime(&end_time) <<   endl;

    } else 
     cout << "Could not open "<< fname << " for reading " << endl;
      
    
   }
    

  return 0;
}
