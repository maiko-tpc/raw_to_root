#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include "TSystem.h"
#include "TGraph.h"
#include "TF1.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>  // for atoi

int driftv_ana(int runno) {
  
  TFile* ifile;
  TTree* tree1;
  
  TFile* ofile;
  TTree* tree2;
  
  int evt_cnt; 
  unsigned int hit_data_a[24][2048] = {0}; // 768/32
  
  double driftv = 0.0;
  // 移除了 n_driftv, sum_driftv, ave_driftv
  
  int min_clk[768];
  int max_clk[768];
  int time;
  int n_pixel = 0;
  
  // 打开输入 ROOT 文件
  ifile = new TFile(Form("./rootfile/MAIKo_%05d.root", runno), "READ");
  if (!ifile || ifile->IsZombie()) {
    printf("Error opening file ./rootfile/MAIKo_%05d.root\n", runno);
    return -1;
  }
  tree1 = (TTree*)ifile->Get("tree");
  if (!tree1) {
    printf("Error: tree 'tree' not found in file.\n");
    return -1;
  }
  
  // 创建输出 ROOT 文件和 TTree
  ofile = new TFile(Form("./driftv/driftv_%04d.root", runno), "RECREATE");
  tree2 = new TTree("tree", "tree");
  
  int sca_cnt[12];
  
  // 设置输入 tree 的分支地址
  tree1->SetBranchAddress("evt_cnt", &evt_cnt);
  tree1->SetBranchAddress("hit_data_a", hit_data_a);
  tree1->SetBranchAddress("sca_cnt", sca_cnt);
  
  // 设置输出 tree 的分支（已移除 ave_driftv, n_driftv）
  tree2->Branch("runno", &runno, "runno/I");
  tree2->Branch("evt_cnt", &evt_cnt, "evt_cnt/I");
  tree2->Branch("driftv", &driftv, "driftv/D");
  tree2->Branch("n_pixel", &n_pixel, "n_pixel/I");
  tree2->Branch("time", &time, "time/I");
  
  Long64_t nentries = tree1->GetEntries();
  // 设置进度条更新步长
  int progress_step = nentries / 100;
  if (progress_step == 0) progress_step = 1;
  
  for (Long64_t ii = 0; ii < nentries; ii++) {
    tree1->GetEntry(ii);
    time = sca_cnt[2];
    n_pixel = 0;
    
    // 初始化 min_clk 和 max_clk 数组
    for (int i = 0; i < 768; i++) {
      min_clk[i] = 2048;
      max_clk[i] = 0;
    }
    
    // 遍历所有通道，更新每个条带的最小和最大时钟值
    for (int ch = 0; ch < 24; ch++) {
      for (int clk = 0; clk < 2048; clk++) {
        for (int i = 0; i < 32; i++) {
          int ch2 = ((ch / 4 + 1) * 4) - (ch % 4) - 1;
          int ch_to_strp = ch2 * 32 + i;
          
          // 判断 hit_data_a 中的第 i 位是否为 1
          if ((hit_data_a[ch][clk] & (1U << i)) != 0) {
            n_pixel++;
            if (clk < min_clk[ch_to_strp]) min_clk[ch_to_strp] = clk;
            if (clk > max_clk[ch_to_strp]) max_clk[ch_to_strp] = clk;
          }
        }
      }
    }
    
    // 如果 n_pixel 小于 300，则不进行分析
    if (n_pixel < 300) {
      driftv = 0.0;
    } else {
      // 针对条带范围 500 到 600 内的轨迹进行直线拟合
      TGraph *gr = new TGraph();
      int nPoints = 0;
      for (int strp = 500; strp <= 600; strp++) {
        // 仅选取有信号的条带
        if (min_clk[strp] < 2048 && max_clk[strp] > 0) {
          double mean_clk = 0.5 * (min_clk[strp] + max_clk[strp]);
          // 横坐标为平均时钟值，纵坐标为条带号
          gr->SetPoint(nPoints, mean_clk, strp);
          nPoints++;
        }
      }
      
      if (nPoints >= 2) {
        // 安静模式下拟合直线 "pol1"
        gr->Fit("pol1", "Q");
        TF1 *fit = gr->GetFunction("pol1");
        double slope = fit->GetParameter(1); // 斜率 = d(strp)/d(clk)
        // 使用转换因子计算漂移速度
        driftv = (0.4 / (sqrt(3) * 10)) * slope;
      } else {
        driftv = 0.0;
      }
      
      delete gr; // 释放内存
    }
    
    tree2->Fill();
    
    // 更新进度条，每处理 progress_step 个事件打印一次进度
    if (ii % progress_step == 0) {
      double progress = (double)ii / nentries * 100;
      printf("Progress: %3.1f%%\r", progress);
      fflush(stdout);
    }
  }
  
  // 最后输出 100%
  printf("Progress: 100.0%%\n");
  
  tree2->Write();
  ofile->Close();
  ifile->Close();
  
  return 0;
}

#ifdef COMPILE_STANDALONE
int main(int argc, char* argv[]) {
  if (argc < 2) {
    printf("Usage: %s runno\n", argv[0]);
    return 1;
  }
  int runno = atoi(argv[1]);
  return driftv_ana(runno);
}
#endif






    

