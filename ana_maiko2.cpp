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

bool quit_flag = false;
void stop_ana(int sig) {
  quit_flag = true;
}

// 用于同步板卡文件指针至下一个 event header 的函数
void sync_to_next_event(ifstream &fin) {
  char tmp;
  // 持续读取直到找到 header（0xeb,0x90）或遇到文件结束
  while(fin.read(&tmp, 1)) {
    if((tmp & 0xff) == 0xeb) {
      char second;
      if(fin.read(&second, 1)) {
        if((second & 0xff) == 0x90) {
          // 定位到 header 起始处，以便下一次读取时能重新检测 header
          fin.seekg(-2, ios::cur);
          break;
        } else {
          // 将文件指针退回一字节，以免遗漏可能的 header 起始字节
          fin.seekg(-1, ios::cur);
        }
      }
    }
  }
}

int main(int argc, char *argv[]){
  signal(SIGINT, stop_ana);
  const int used_board[12] = {1,1,1,1,1,1, 1,1,1,1,1,1};    

  // error massage or useage
  if(argc==1){
    printf("useage: ./ana_maiko2 run_num [stop_evt]\n");
    exit(1);
  }

  int runno = atoi(argv[1]);
  int stop_evt = -1;


  if(argc > 2) stop_evt = atoi(argv[2]);
  
  string ac[2] = {"anode", "cathode"};
  int all_different_evt = 0;
  int ac_id = 0;
  int board_id = 0;
  int boardno = 0;
  
  int evt_num[12]; 
  // evt_cnt will be initialized from the first board's evt_num
  int evt_cnt = 1;
  
  // Count of successfully parsed events (i.e. tree->Fill calls)
  int successful_evt_count = 0;
  
  unsigned int fadc_data[2][1025][24];
  int fadc_ch;
  
  unsigned int hitdata;
  unsigned int hit_data_a[24][2048]; // for anode
  unsigned int hit_data_c[24][2048]; // for cathode

  int hit_ch;
  unsigned int hit_clk;
  unsigned int hitclk[2048];  
  unsigned int N_hit_clk[12];
  int hit_cnt = 0;
  int trig_rq = 0;
  int trig_acpt = 0;
  int clk_rq = 0;
  int clk_acpt = 0;
  int clk_cnt[12];
  int sca_cnt[12];

  int flag = -1;
  
  ofile = new TFile(Form("./rootfile/MAIKo_%05d.root", runno), "RECREATE");
  tree  = new TTree("tree", "tree"); 
  
  tree->Branch("evt_num", evt_num, "evt_num[12]/I");
  tree->Branch("evt_cnt", &evt_cnt, "evt_cnt/I");
  tree->Branch("clk_cnt", clk_cnt, "clk_cnt[12]/I");
  tree->Branch("sca_cnt", sca_cnt, "sca_cnt[12]/I");
  tree->Branch("fadc_data", fadc_data, "fadc_data[2][1025][24]/i");
  tree->Branch("hitclk", hitclk, "hitclk[2048]/I");
  tree->Branch("N_hit_clk", N_hit_clk, "N_hit_clk[12]/i");
  tree->Branch("hit_cnt", &hit_cnt, "hit_cnt/I");
  tree->Branch("hit_data_a", hit_data_a, "hit_data_a[24][2048]/i");
  tree->Branch("hit_data_c", hit_data_c, "hit_data_c[24][2048]/i");

  ifstream fin[12];
  
  // Open raw files for each board
  for(board_id = 0; board_id < 12; board_id++){     
    if(used_board[board_id] == 1){
      fin[board_id].open(Form("/home/quser/exp/raris25/maikodaq/data/uTPC_%04d_%s%d_00000.raw",
                                runno, ac[ac_id].c_str(), boardno), ios::in | ios::binary);
    
      if(!fin[board_id].is_open()){
        cerr << "Cannot open " << ac[ac_id] << boardno << " raw file." << endl;
      } else {
        cerr << "Opened " << ac[ac_id] << boardno << " raw file." << endl;
      }
    }
    if(board_id == 5){
      // Switch to cathode boards
      printf("Switching to cathode boards\n");
      ac_id = 1;
      boardno = -1;
    }    
    boardno++;      
  }
  
  // Initialization: Read evt_num from the first board and set it as the initial evt_cnt
  if(used_board[0] == 1 && fin[0].is_open()){
    streampos pos = fin[0].tellg();
    char data;
    int local_flag = -1;
    // Search for header 0xeb then 0x90
    while(local_flag == -1){
      if(!fin[0].read((char*)&data, 1)){
        local_flag = -2;
        break;
      }
      if((data & 0xff) == 0xeb){
        fin[0].read(&data, 1);
        if((data & 0xff) == 0x90){
          local_flag = 1;
          break;
        }
      }
    }
    if(local_flag != -2){
      fin[0].read(&data, 1);
      if((data & 0xff) == 0x19){
        fin[0].read(&data, 1);
        if((data & 0xff) == 0x64){
          int init_evt;
          fin[0].read((char*)&init_evt, 4);
          evt_cnt = htonl(init_evt);
     
          cerr << "Initial evt_cnt set to " << evt_cnt << " from board 0." << endl;
        }
      }
    }
    // Reset file pointer for board 0
    fin[0].clear();
    fin[0].seekg(pos);
  }
  
  char data;
  int i_data;
  
  // Main event loop
  while(stop_evt == -1 || evt_cnt < stop_evt){
    boardno = 0;
    // 清空 fADC 数据数组
    for(int i = 0; i < 2; i++){
      for(int j = 0; j < 1025; j++){
        for(int k = 0; k < 24; k++){
          fadc_data[i][j][k] = 0;
        }
      }
    }
    
    // 清空 Hit 数据数组
    for(int k = 0; k < 24; k++){
      for(int j = 0; j < 2048; j++){
        hit_data_a[k][j] = 0;
        hit_data_c[k][j] = 0;
      }
    }
    
    // 重置各板卡计数器
    for(int k = 0; k < 12; k++){
      sca_cnt[k] = 0;
      clk_cnt[k] = 0;
      evt_num[k] = 0;
    }
    
    // 标记本次 event 是否因数据溢出而放弃
    bool event_overflow = false;
    
    for(board_id = 0; board_id < 12; board_id++){      
      ac_id = (board_id < 6) ? 0 : 1;
      
      if(used_board[board_id] == 1) {
        flag = -1;
        // 搜索下一个 event header（0xeb 0x90）
        while(flag == -1){
          if(!fin[board_id].read((char*)&data, 1)){
            flag = -2;
            break;
          }
          if((data & 0xff) == 0xeb){
            fin[board_id].read(&data, 1);
            if((data & 0xff) == 0x90){
              flag = 1;
            }
          }
        }
        if(flag == -2) break;
        
        fin[board_id].read(&data, 1);
        
        if((data & 0xff) == 0x19){
          fin[board_id].read(&data, 1);
          if((data & 0xff) == 0x64){
            int evtcnt;
            fin[board_id].read((char*)&evtcnt, 4);
            evt_num[board_id] = htonl(evtcnt);
            
            // 若该板卡 event 编号超前，则回退
            if(evt_cnt < evt_num[board_id]){
              fin[board_id].seekg(-8, ios::cur);
              continue;
            }
            
            if(evt_cnt == evt_num[board_id]) flag = 2;
            
            int clkcnt;
            int scacnt;
            int fadc_cnt = 1;
            
            fin[board_id].read((char*)&clkcnt, 4);
            clk_cnt[board_id] = htonl(clkcnt);  
            
            fin[board_id].read((char*)&scacnt, 4);
            sca_cnt[board_id] = htonl(scacnt);  
            fin[board_id].read((char*)&i_data, 4);
            if(htonl(i_data) != 0x75504943){
              while(htonl(i_data) != 0x160317ff) {
                if (board_id == 7 && htonl(i_data) == 0x16031400) {
                  break;
                }
                // 检查 FADC 数据填充是否即将超出数组范围
                if(fadc_cnt >= 1025) {
                   event_overflow = true;
                   break;
                }
                
                int fdata = (i_data & 0xffff);
                int f_ch, f_data_val;
                f_ch = ((htonl(fdata) >> 28) & 0xf) - 4;
                fadc_ch = (boardno * 4) + f_ch;
                if(fadc_ch < 0 || fadc_ch >= 24){
                  event_overflow = true;
                  break;
                }
                f_data_val = (htonl(fdata) >> 16) & 0x3ff;
                fadc_data[ac_id][fadc_cnt][fadc_ch] = f_data_val;
                
                fdata = (i_data >> 16) & 0xffff;
                f_ch = ((htonl(fdata) >> 28) & 0xf) - 4;
                fadc_ch = (boardno * 4) + f_ch;
                if(fadc_ch < 0 || fadc_ch >= 24){
                  event_overflow = true;
                  break;
                }
                f_data_val = (htonl(fdata) >> 16) & 0x3ff;
                fadc_data[ac_id][fadc_cnt][fadc_ch] = f_data_val;
                
                for(int f = 0; f < 2; f++){
                  fin[board_id].read((char*)&fdata, 2);
                  f_ch = ((htonl(fdata) >> 28) & 0xf) - 4;
                  fadc_ch = (boardno * 4) + f_ch;
                  if(fadc_ch < 0 || fadc_ch >= 24){
                    event_overflow = true;
                    break;
                  }
                  f_data_val = (htonl(fdata) >> 16) & 0x3ff;
                  fadc_data[ac_id][fadc_cnt][fadc_ch] = f_data_val;
                }
                if(event_overflow) break;
                fadc_cnt++;
                
                if(!fin[board_id].read((char*)&i_data, 4)){
                  flag = -2;
                  break;
                }
              }
            }
            if(event_overflow) break;
            
            if(htonl(i_data) == 0x160317ff){ 
              fin[board_id].read((char*)&i_data, 4);
            }
            while((htonl(i_data) >> 16) == 0x8000){
              hit_clk = (htonl(i_data) & 0x07ff);
              if(hit_clk >= 2048) { 
                event_overflow = true;
                break;
              }
              hitclk[hit_clk] = hit_clk;
              N_hit_clk[board_id] = hit_clk;
              
              for(int i = 0; i < 4; i++){
                fin[board_id].read((char*)&hitdata, 4);
                hitdata = htonl(hitdata);
                hit_ch = boardno * 4 + i;
                if(hit_ch < 0 || hit_ch >= 24) {
                  event_overflow = true;
                  break;
                }
                if(board_id < 6){
                  hit_data_a[hit_ch][hit_clk] = hitdata;
                } else {
                  hit_data_c[hit_ch][hit_clk] = hitdata;
                }
              }
              if(event_overflow) break;
              if(!fin[board_id].read((char*)&i_data, 4)){
                flag = -2;
                break;
              }
            }
            if(event_overflow) break;
            hit_cnt++;
          }
        }
      }
      if(boardno == 5) boardno = -1;
      boardno++;
      
      if(event_overflow) break;  // 若任一板卡出现溢出，则退出当前 event 的读取
    } // End of board loop
    
    // 如果本 event 因数据溢出被放弃，则对所有板卡同步到下一个 event header
    if(event_overflow){
      for(int b = 0; b < 12; b++){
        if(used_board[b] == 1 && fin[b].is_open()){
          sync_to_next_event(fin[b]);
        }
      }
      evt_cnt++;
      continue;
    }
    
    // 检查所有板卡的 evt_num 是否都与当前 evt_cnt 不同（若全都不同，认为该 event 已全部处理完毕）
    bool all_different = true;
    for (int num : evt_num){
      if (num == evt_cnt){
        all_different = false;
        break;
      }
    }
    if (all_different){
      evt_cnt++;
      all_different_evt++;
    }
    
    if(flag == 2){
      tree->Fill();
      trig_rq = sca_cnt[0];
      trig_acpt = sca_cnt[1];
      clk_rq = sca_cnt[2];
      clk_acpt = sca_cnt[3];

      successful_evt_count++;
      evt_cnt++;
      if(evt_cnt % 200 == 0){
        printf("\rParsed events: %d", evt_cnt);
        fflush(stdout);
      }
      flag = -1;
    }
    
    if(flag == -2) break;
    if(quit_flag) break;
  } // End of event loop
  
  // Final summary in English
  printf("\nTotal skipped events: %d\n", all_different_evt);
  printf("Total successfully parsed events: %d\n", successful_evt_count);
  printf("trigger requested: %d\n", trig_rq);
  printf("trigger accepted: %d\n", trig_acpt);
  printf("clock requested: %d\n", clk_rq);
  printf("clock accepted: %d\n", clk_acpt); 
  for(int i = 0; i < 12; i++){
    if(used_board[i] == 1){
      fin[i].close();
    }
  }
  printf("File closed\n");
  tree->Write();
  ofile->Close();
  
  return 0;
}


