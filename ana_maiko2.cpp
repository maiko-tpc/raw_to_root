#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <fstream>
#include <TFile.h>
#include <TTree.h>
#include <TStyle.h>
#include <TROOT.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <signal.h>

using namespace std;

TFile* ofile;
TTree* tree;

#define N_ac 2
#define N_hit_ch 768

bool quit_flag=false;
void stop_ana(int sig){
  quit_flag=true;
}

int main(int argc, char *argv[]){
  signal(SIGINT, stop_ana);
  const int used_board[12]={0,1,1,1,1,1, 1,1,1,1,1,1};    
  int runno = atoi(argv[1]);
  int stop_evt=-1;
  if(argc>2) stop_evt=atoi(argv[2]);
  
  string ac[2] = {"anode","cathode"};
  int all_different_evt=0;
  int ac_id = 0;
  int board_id = 0;
  int boardno = 0;
  
  int evt_num[12]; 
  int evt_cnt = 1;
  
  //unsigned int fadc_data[1025][24];
   unsigned int fadc_data[2][1025][24];
  int fadc_ch;
  
  unsigned int hitdata;
  unsigned int hit_data_a[24][2048]; // 768/32
  unsigned int hit_data_c[24][2048];
  //unsigned int hit_data[2][2048][768];

  int hit_ch;
  unsigned int hit_clk;
  unsigned int hitclk[2048];  
  unsigned int N_hit_clk[12];
  int hit_cnt =0;
  
  int clk_cnt[12];
  int sca_cnt[12];

  int flag=-1;
  
  /*
  unsigned int*** hit_data;
  hit_data = (unsigned int***)malloc(sizeof(unsigned int**)*N_ac);
  if(hit_data==NULL) exit(1);
  for(int i=0; i<N_ac; i++){
    hit_data[i] = (unsigned int**)malloc(sizeof(unsigned int*)*N_hit_clk);
    if(hit_data[i]==NULL) exit(1);
    for(int j=0; j<N_hit_clk; j++){
      hit_data[i][j] = (unsigned int*)malloc(sizeof(unsigned int)*N_hit_ch);
      if(hit_data[i][j]==NULL) exit(1);
    }
  }
  */
  
  ofile = new TFile(Form("./rootfile/MAIKo_%05d.root",runno),"RECREATE");
  tree  = new TTree("tree","tree"); 
  
  //tree->Branch("ac_id",&ac_id,"ac_id/I");
  //tree->Branch("board_no",&boardno,"boardno/I");
  tree->Branch("evt_num",&evt_num,"evt_num[12]/I");
  tree->Branch("evt_cnt",&evt_cnt,"evt_cnt/I");
  tree->Branch("clk_cnt",&clk_cnt,"clk_cnt[12]/I");
  tree->Branch("sca_cnt",&sca_cnt,"sca_cnt[12]/I");
  //tree->Branch("fadc_data",fadc_data,"fadc_data[1025][24]/i");
  tree->Branch("fadc_data",fadc_data,"fadc_data[2][1025][24]/i");
  //tree->Branch("fadc_clk",&fadc_clk,"fadc_clk/I");
  //tree->Branch("hit_data",&hit_data,"hit_data/I");
  tree->Branch("hitclk",hitclk,"hitclk[2048]/I");
  tree->Branch("N_hit_clk",N_hit_clk,"N_hit_clk[12]/i");
  tree->Branch("hit_cnt",&hit_cnt,"hit_cnt/I");
  tree->Branch("hit_data_a",hit_data_a,"hit_data_a[24][2048]/i");
  tree->Branch("hit_data_c",hit_data_c,"hit_data_c[24][2048]/i");
  //tree->Branch("hit_data",hit_data,"hit_data[ac_id][hit_clk][hit_ch]/I");

  
  ifstream fin[12];
  
  for(board_id=0; board_id < 12; board_id++){     
    if(used_board[board_id]==1){
    //printf("ac_id=%d\n",ac_id);
    //printf("board_id=%d\n",board_id);
    //printf("boardno=%d\n",boardno);
    
    //fin[board_id].open(Form("./uTPC_%04d_%s%d_00000.raw",runno,ac[ac_id].c_str(),boardno), ios::in|ios::binary);
    fin[board_id].open(Form("/home/quser/exp/maiko/2024Nov/maikodaq/data/uTPC_%04d_%s%d_00000.raw",runno,ac[ac_id].c_str(),boardno), ios::in|ios::binary);
      // fin[board_id].open(Form("/home/quser/exp/test_raw/uTPC_%04d_%s%d_00000.raw",runno,ac[ac_id].c_str(),boardno), ios::in|ios::binary);

    // fin[board_id].seekg(ios_base::beg);
    
    if(!fin[board_id].is_open()){
      cerr << "Can't open " << ac[ac_id] << boardno <<" raw file." << endl;
    }
    
    if(fin[board_id].is_open()){
      cerr << " open " << ac[ac_id] << boardno <<" raw file." << endl;
    }
    }
    if(board_id == 5){
      printf("ac change\n");
      ac_id = 1;
      boardno = -1;
    }    
    boardno++;      
  }

  char data;
  int i_data;
  
  
  //      while(evt_cnt<5){
    while(stop_evt==-1 || evt_cnt<stop_evt){
    
    boardno = 0;
    //ac_id=0;
    //board_id =0;    
	

    for(int i=0; i<2;i++){
      for(int j=0; j<1025;j++){
	for(int k=0; k<24;k++){
	  //fadc_data[j][k]=0;
	  fadc_data[i][j][k]=0;
	}
      }
    }
    //for(int i=0; i<N_ac;i++){

    for(int k=0; k<24;k++){
      for(int j=0; j<2048;j++){
	hit_data_a[k][j] = 0;
	hit_data_c[k][j] = 0;
	//hit_data[i][j][k] = 0;
      }
    }
    //}

    for(int k=0; k<12;k++){
      sca_cnt[k]=0;
      clk_cnt[k]=0;
      evt_num[k]=0;
    }

      
      
    for(board_id=0; board_id<12 ; board_id++){
      //if(fin[board_id].eof()) continue;
      //      if (evt_cnt==0 and board_id==6) continue;
      //     if (evt_cnt==0 and board_id==7) continue;
      //2023/12/19 updated by LIN for Board 6,7 mismatch
      //These 2 boards' data start from evt1 so i want them skip once
      //  flag = -1;
      
      if(board_id >5){
	ac_id = 1;
      }
      if(board_id<6){
	ac_id=0;
      }
      
      if(used_board[board_id]==1) {
	flag = -1;
	while(flag== -1){		
	

	if(!fin[board_id].read((char*)&data, 1)){
	  flag=-2;
	  printf("a");
	  break;
	}
	
	//	printf("data1=%x\n",data);
	//	printf("data_size=%ld\n",sizeof(data));
	//if((char*)data == NULL) break;
	
	if((data & 0xff) == 0xeb){
	  fin[board_id].read(&data, 1);
	  if((data & 0xff) == 0x90){
	    flag = 1;
	  }
	}
      }
      if(flag==-2) break;

      fin[board_id].read(&data, 1);

      if((data & 0xff) == 0x19){
	fin[board_id].read(&data, 1);
	if((data & 0xff) == 0x64){

   
    
        
      
      int evtcnt;
		
	    fin[board_id].read((char*)&evtcnt, 4);
	    evt_num[board_id] = htonl(evtcnt);  
	    if(evt_cnt < evt_num[board_id]){
	      printf("wrongboardid=%d\n",board_id);
	      printf("evt_cnt=%d\n",evt_cnt);
	      printf("evt_num=%d\n",evt_num[board_id]);
	      fin[board_id].seekg(-8,ios::cur);
	      //	      fin[board_id].read(&data,1);
	      //	      printf("data=%d\n",data);
	      // cout << hex << (data&0xff) << endl;
	      continue;
	    }
	    	    
	    if(evt_cnt==evt_num[board_id]) flag=2 ;
	    printf("flag=%d\n",flag);
	    printf("evt_cnt=%d\n",evt_cnt);
	    printf("boardid=%d\n",board_id);
	    printf("evt_num=%d\n",evt_num[board_id]);
	  //printf("evt_num[0]=%d\n",evt_num[0]);
	  //printf("evt_num[1]=%d\n",evt_num[1]);

	  
	  int clkcnt;
	  int scacnt;

	  int fadc_cnt = 1;
		
	  fin[board_id].read((char*)&clkcnt, 4);
	  
	  clk_cnt[board_id] = htonl(clkcnt);  
	  //	  if(board_id==7) printf("clk_cnt=%n\n",clk_cnt);
	  //printf("clk_cnt=%d\n",clk_cnt[board_id]);

	  fin[board_id].read((char*)&scacnt, 4);
	  sca_cnt[board_id] = htonl(scacnt);  
	  //printf("sca_cnt=%d\n",sca_cnt);
	  // if(board_id==7) printf("sca_cnt=%n\n",sca_cnt);
	  fin[board_id].read((char*)&i_data, 4);
	  // printf("check1");
	  // if(board_id==7) printf("check1");
	  if(htonl(i_data) != 0x75504943){
	      
	    //FADC data
	    // int fadc_lenth=0;
	    while(htonl(i_data) != 0x160317ff ) { //fadc_end 
	    if (board_id==7 && htonl(i_data)  == 0x16031400) { // 检查是否开始hitdata
	        break; // 若遇到8000标志，则结束FADC数据处理
	       }
	      int fdata;

	      fdata = (i_data & 0xffff);
	      // if(board_id==7) printf("fdata=%d\n",fdata);
	      //printf("fdata=%x\n",fdata);
	      //printf("htonl fdata 16=%x\n",htonl(fdata)>>16);
	      //  printf("check2");
	      int f_ch,f_data;
		  
	      f_ch = ((htonl(fdata) >> 28) & 0xf) -4;
	      fadc_ch = (boardno*4)+f_ch;
	      f_data = (htonl(fdata)>>16) & 0x3ff;
	      // printf("check3");
	      //printf("fadc_ch=%d\n",fadc_ch);
	      //printf("f_data=%d\n",f_data);
	      //printf("fadc_cnt=%d\n",fadc_cnt);
	      // if(board_id==7) printf("fadc_ch=%d\n",fadc_ch);
	      // if(board_id==7) printf("fadc_cnt=%d\n",fadc_cnt);
	      // printf("fadc_ch=%d\n",fadc_ch);
	      // printf("fadc_cnt=%d\n",fadc_cnt);
	      //fadc_cnt++;
	      // if(board_id==7) printf("i_data=%d\n",i_data);
	      // if(board_id==7) printf("evt_cnt=%d\n",evt_cnt);
	      //fadc_data[fadc_cnt][fadc_ch]=f_data;
	      //printf("true fadc_lenth=%d\n",fadc_lenth);
	      // if(board_id==7) printf("fadc_lenth=%d\n",fadc_lenth);
	      
	      fadc_data[ac_id][fadc_cnt][fadc_ch]=f_data;
 
	      //printf("fadcdata=%d\n",fadc_data[fadc_cnt][fadc_ch]);
	      //printf("fadcdata=%d\n",fadc_data[ac_id][fadc_cnt][fadc_ch]);
	      // if(board_id==7) printf("check2\n");
	      fdata = (i_data >> 16) & 0xffff;

	      f_ch = ((htonl(fdata) >> 28) & 0xf) -4;
	      fadc_ch = (boardno*4)+f_ch;
	      f_data = (htonl(fdata)>> 16) & 0x3ff;
	      // if(board_id==7) printf("check3\n");
	      //fadc_cnt++;

	      fadc_data[ac_id][fadc_cnt][fadc_ch]=f_data;		    		    

	      //fin[board_id].seekg(-4,ios_base::cur);
	      //fin[board_id].read((char*)&fdata,2);
	      //if(board_id==7) printf("check4\n");
	      for(int f=0; f<2; f++){

		fin[board_id].read((char*)&fdata,2);
				
		f_ch = ((htonl(fdata) >> 28) & 0xf) -4;
		fadc_ch = (boardno*4)+f_ch;
		f_data = (htonl(fdata) >> 16)& 0x3ff;
		
		//fadc_cnt++;

		fadc_data[ac_id][fadc_cnt][fadc_ch]=f_data;

	      }
	      fadc_cnt++;
		
	      fin[board_id].read((char*)&i_data, 4);
	      if(fin[board_id].eof()){
		flag=-2;
		break;
	      }
	      //if(fadc_cnt>1025) goto FADC_end;
	    }
	  } //fadc end
    
	  //printf("fadc data end\n");
	      
	  //if(htonl(i_data) == 0x75504943) continue;	
	  
	    //if(evt_num%1000==0) printf("board_id=%d\n",board_id);
	    //if(evt_num%1000==0) printf("hit data begin\n");
	  	  if(htonl(i_data) == 0x160317ff){ 
     // ----  Hit data --------------------------------------------------------	     
	      
	    fin[board_id].read((char*)&i_data, 4);
	    //printf("htonl i_data2=%x\n",htonl(i_data));
        }
	    while(htonl(i_data) >> 16  == 0x8000){
	  
	      hit_clk= (htonl(i_data) & 0x07ff);		      
	      //printf("hit_clk = %d\n",hit_clk);

	      hitclk[hit_clk]=hit_clk;
	      N_hit_clk[board_id]=hit_clk;
       	
	      for(int i=0; i<4; i++){
		fin[board_id].read((char*)&hitdata, 4);
		//printf("hitdata1 =%x\n",hitdata);
		hitdata = htonl(hitdata);
		  
		//for(int j=0; j<32; j++){  

		hit_ch = boardno*4+i;
		//printf("hit_ch=%d\n",hit_ch);
		    
		if(board_id<6){
		  hit_data_a[hit_ch][hit_clk] = hitdata;
		}

		if(board_id>5){
		  hit_data_c[hit_ch][hit_clk] = hitdata;
		}

		//printf("hit_data_a = %u\n",hit_data_a[hit_ch][hit_clk]);

		//if(hit_data_c[hit_ch][hit_clk]!=0){
		//printf("hit_data_c = %d\n",hit_data_c[hit_ch][hit_clk]);
		//}
	      }

	      fin[board_id].read((char*)&i_data, 4);
	      if(fin[board_id].eof()){
		flag=-2;
		break;
	      }	      
	    }
	      
	    //printf("hit data end\n");
	    hit_cnt++;
		  
	    //printf("htonl i_data2=%x\n",htonl(i_data));
	  
	  
	
      }//check 90eb_6419 end

      }  
      //if((board_id==11) & (evt_num%1000==0)) printf("evt_num=%d\n",evt_num);
      //if((board_id==11) & (evt_num%1000==0)) printf("evt_cnt=%d\n",evt_cnt);
      }
      if(boardno==5) boardno= -1;
      boardno++;     

      if(flag==-2) break;
    }//board12 loop end
    
    bool all_different = true;
    for (int num : evt_num){
      if (num == evt_cnt){
	all_different = false;
	//	all_different_evt+=1;
	break;
      }
    }
    if (all_different){
      evt_cnt+=1;
      all_different_evt += 1;
    }
     if(flag==2){
    // if(flag){
      //     if(evt_cnt >= 10) {
    tree->Fill();
    //  }
    evt_cnt++; 
      //   if(evt_cnt%1000==0){
	printf("\rtree fill: %d",evt_cnt);
	fflush(stdout);
	//  }
      flag=-1;
    
    }
    //if(htonl(i_data) == 0x75504943) continue;

    if(flag==-2) break;
    if(quit_flag) break;
  }//while end    
  //  printf("\rtree fill: %d\n",evt_cnt);

  
  for(int i= 0; i<12; i++){
    if(used_board[i]==1){
    fin[i].close();
  }
  }
  printf("file close\n");
  printf("alldifferent_evt=%d\n",all_different_evt);
  tree->Write();
  ofile->Close();

  return 0;
}

