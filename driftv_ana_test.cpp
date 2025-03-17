#include <TFile.h>
#include <TTree.h>
#include <TString.h>
#include <TMath.h>
#include <cstdio>
#include <vector>
#include <algorithm>

// 计算中值的辅助函数
double getMedian(std::vector<int>& vec) {
   size_t size = vec.size();
   if(size == 0) return 0;
   std::sort(vec.begin(), vec.end());
   if(size % 2 == 0)
      return 0.5 * (vec[size/2 - 1] + vec[size/2]);
   else
      return vec[size/2];
}

int driftv_ana(int runno) {
  
   // 定义输入输出 ROOT 文件和 TTree
   TFile* ifile = new TFile(Form("./rootfile/MAIKo_%05d.root", runno), "READ");
   TTree* tree1 = (TTree*)ifile->Get("tree");
   
   TFile* ofile = new TFile(Form("./driftv/driftv_%04d.root", runno), "RECREATE");
   TTree* tree2 = new TTree("tree", "tree");
   
   int evt_cnt; 
   // hit_data_a 数组：24个通道，每个通道 2048 个 clk（32 个 bit 代表 32 个 strip，合计 768 个strip）
   unsigned int hit_data_a[24][2048] = {0};
   
   // 漂移速度相关变量
   double driftv = 0.0;
   int n_driftv = 0;
   double sum_driftv = 0.0;
   double ave_driftv = 0.0;
   
   // 新增：统计每个 event 总的 pixel hit 数
   int n_pixel_hit = 0;
   
   // 为后续计算，定义用于记录每个事件中触发的最小和最大 strip
   int min_strp = 768;
   int max_strp = 0;
   int median_clk_min=0;
   int median_clk_max=0;
   // 设置输入、输出树的分支
   tree1->SetBranchAddress("evt_cnt", &evt_cnt);
   // 注意：这里将 ROOT 树中的 "hit_data_c" 分支指向 hit_data_a 数组
   tree1->SetBranchAddress("hit_data_a", hit_data_a);
   
   tree2->Branch("runno", &runno, "runno/I");
   tree2->Branch("evt_cnt", &evt_cnt, "evt_cnt/I");
   tree2->Branch("driftv", &driftv, "driftv/D");
   tree2->Branch("ave_driftv", &ave_driftv, "ave_driftv/D");
   tree2->Branch("n_driftv", &n_driftv, "n_driftv/I");
   tree2->Branch("min_strp", &min_strp, "min_strp/I");
   tree2->Branch("max_strp", &max_strp, "max_strp/I");
   tree2->Branch("median_clk_min", &median_clk_min, "median_clk_min/I");
   tree2->Branch("median_clk_max", &median_clk_max, "median_clk_min/I");

   // 新增 branch 用于记录每个 event 总 pixel hit 数
   tree2->Branch("n_pixel_hit", &n_pixel_hit, "n_pixel_hit/I");
   
   // 循环处理每个事件
   Long64_t nentries = tree1->GetEntries();
   for (Long64_t ii = 0; ii < nentries; ii++) {
      tree1->GetEntry(ii);
      driftv = 0;
      // 重置每个 event 的 pixel hit 计数
      n_pixel_hit = 0;
      
      // 对每个事件重置每个 strip 的 clk 数据容器，同时重置 strip 边界
      std::vector<int> clk_values[768];  // 每个 strip 收集所有 hit 的 clk 值
      min_strp = 768;
      max_strp = 0;
      
      // 遍历所有通道和 clk，以及每个通道的32个 bit
      for (int ch = 0; ch < 24; ch++) {
         for (int clk = 0; clk < 2048; clk++) {
            for (int bit = 0; bit < 32; bit++) {
               // 根据原有逻辑计算 strip 编号
               int ch2 = ((ch / 4 + 1) * 4) - (ch % 4) - 1;
               int ch_to_strp = ch2 * 32 + bit;
               
               // 判断该 bit 是否被触发
               if ((hit_data_a[ch][clk] & (1U << bit)) != 0) {
                  // 将当前 clk 值加入对应 strip 的容器中
                  clk_values[ch_to_strp].push_back(clk);
                  
                  // 新增：统计 pixel hit 数，每次触发计数一次
                  n_pixel_hit++;
                  
                  // 更新 strip 的边界，注意：min_strp 只考虑大于50的 strip
                  if ((ch_to_strp > 50) && (ch_to_strp < min_strp))
                     min_strp = ch_to_strp;
                  if (ch_to_strp > max_strp)
                     max_strp = ch_to_strp;
               }
            }
         }
      }
      
      // 对每个 strip 计算中值，并存放到 med_clk 数组中
      double med_clk[768] = {0};
      for (int strp = 0; strp < 768; strp++) {
         if (!clk_values[strp].empty()) {
            med_clk[strp] = getMedian(clk_values[strp]);
         } else {
            med_clk[strp] = 0;  // 没有 hit 数据时设为0（或根据需要设置其他默认值）
         }
      }
      
      // 根据触发的 strip 边界计算漂移速度
      // 只有当至少有一个 strip 被触发时，且 min_strp 与 max_strp 合理时，才计算漂移速度
      if ((min_strp < 768) && (max_strp < 768) && (min_strp < max_strp)) {
         // 这里直接使用各 strip 的中值 clk 作为代表值
          median_clk_max = static_cast<int>(med_clk[max_strp]);
          median_clk_min = static_cast<int>(med_clk[min_strp]);
         
         int delta_clk = median_clk_min - median_clk_max;
         int delta_strp = max_strp - min_strp;
         driftv = (0.4 * delta_strp) / (sqrt(3)*10*delta_clk);
         // 根据原来的条件进行判断，避免噪声和除0错误
         if ((min_strp < 100) && (delta_clk > 200) && (delta_strp < 800)) {
            driftv = (0.4 * delta_strp) / (sqrt(3) * 10 * delta_clk);
            printf("driftv = %f\n", driftv);
            n_driftv++;
            sum_driftv += driftv;
         }
      }
      
      // 计算当前事件累积的平均漂移速度
      if (n_driftv > 0)
         ave_driftv = sum_driftv / n_driftv;
      else
         ave_driftv = 0.0;
      
      //      printf("ave_driftv = %f\n", ave_driftv);
      // 输出当前事件的 pixel hit 数
      // printf("n_pixel_hit = %d\n", n_pixel_hit);
      
      tree2->Fill();
   }
   
   printf("Final ave_driftv = %f\n", ave_driftv);
   printf("Number of driftv events = %d\n", n_driftv);
   
   tree2->Write();
   ofile->Close();
   
   return 0;
}



    

