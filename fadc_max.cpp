#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>
#include <TCanvas.h>
#include <iostream>
#include <vector>
#include <set>
#include <map>
std::vector<int> listHitEvents(int runno) {
    TFile *file = new TFile(Form("./rootfile/MAIKo_%05d.root", runno), "read");
    TTree *tree = (TTree*)file->Get("tree");
    unsigned int hit_data_a[24][2048] = {0};
    unsigned int hit_data_c[24][2048] = {0};
    int evt_num;
    int globalMaxClk;
    int globalMinClk;
    tree->SetBranchAddress("hit_data_a", hit_data_a);
    tree->SetBranchAddress("hit_data_c", hit_data_c);
    tree->SetBranchAddress("evt_num", &evt_num);

    std::vector<int> hitEvents; // 存储满足条件的事件编号
    int nEntries = tree->GetEntries(); // 获取事件总数
    for (int entry = 0; entry < nEntries; ++entry) {
        tree->GetEntry(entry);
	globalMinClk = INT_MAX;
	globalMaxClk = 0;

        bool hasHitIn0To200 = false;
        bool hasHitAbove300 = false;
        std::map<int, std::pair<int, int>> cathodeStripsClk; // 存储有hit的unique strip号和对应的clk范围

        // 计算Cathode track的长度，并获取clk范围
        for (int ch = 0; ch < 24; ++ch) {
            for (int clk = 0; clk < 2048; ++clk) {
                for (int i = 0; i < 32; ++i) {
                    int ch2 = ((ch / 4 + 1) * 4) - (ch % 4) - 1;
                    int ch_to_strp = ch2 * 32 + i;

                    if ((hit_data_c[ch][clk]) & ((unsigned int)0x1 << i)) {
                       globalMinClk = std::min(globalMinClk, clk);
                       globalMaxClk = std::max(globalMaxClk, clk);
                        if (cathodeStripsClk.find(ch_to_strp) == cathodeStripsClk.end()) {
                            cathodeStripsClk[ch_to_strp] = std::make_pair(clk, clk);
                        } else {
                            cathodeStripsClk[ch_to_strp].first = std::min(cathodeStripsClk[ch_to_strp].first, clk);
                            cathodeStripsClk[ch_to_strp].second = std::max(cathodeStripsClk[ch_to_strp].second, clk);
                        }
                    }

                    if (ch_to_strp > 300 && (hit_data_a[ch][clk]) & ((unsigned int)0x1 << i)) {
                        hasHitAbove300 = true;
                    }
                }
            }
        }

        // 检查Anode的0～200 strip中的hit
        for (int ch = 0; ch < 24 && !hasHitIn0To200; ++ch) {
            for (int clk = 0; clk < 2048 && !hasHitIn0To200; ++clk) {
                for (int i = 0; i < 32; ++i) {
                    int ch2 = ((ch / 4 + 1) * 4) - (ch % 4) - 1;
                    int ch_to_strp = ch2 * 32 + i;

                    if (ch_to_strp >= 0 && ch_to_strp <= 200 && ((hit_data_a[ch][clk]) & ((unsigned int)0x1 << i))) {
                        hasHitIn0To200 = true;
                    }
                }
            }
        }

        // 如果满足所有条件，记录事件号
        bool cathodeConditionMet = false;
printf("Max Clk: %d, Min Clk: %d\n", globalMaxClk, globalMinClk);
	if (globalMaxClk - globalMinClk <= 100) {
                cathodeConditionMet = true;
                printf("hoge");
              //  break;
            }
        

        if (hasHitIn0To200 && !hasHitAbove300 && cathodeStripsClk.size() > 300 && cathodeConditionMet) {
            hitEvents.push_back(entry);
        }
    }

    // 输出满足条件的事件号
    std::cout << "Events with hits in Anode 0-200 strip, no hits above 300, Cathode track length > 300 and clk range <= 100:" << std::endl;
    for (int event : hitEvents) {
        std::cout << event << std::endl;
    }
return hitEvents;
    delete file;
}



void analyze_fadc_data(int runno, int chosen) {
 std::vector<int> eventList = listHitEvents(runno);
    // 打开ROOT文件
    TFile *file = new TFile(Form("./rootfile/MAIKo_%05d.root", runno), "READ");
    TH1D *integ_hist = new TH1D("integ_hist", "Histogram of integ[chosen]", 40, 0, 8000);
    if (file->IsZombie()) {
        std::cout << "Error opening file" << std::endl;
        return;
    }

    // 获取TTree
    TTree *tree = (TTree*)file->Get("tree");
    if (!tree) {
        std::cout << "Tree not found" << std::endl;
        return;
    }
    unsigned int fadc_data[2][1025][24]={0};
    // 设置fadc_data分支
    tree->SetBranchAddress("fadc_data", &fadc_data);
    TH2D *fadc_a[24];
    TH2D *fadc_c[24];
    // 创建直方图
    double fadc_c_bl[24];  
    int integ[24];
    // 遍历TTree
    Long64_t nentries = tree->GetEntries();
    for (Long64_t evt :eventList) {
        tree->GetEntry(evt);
          for(int i=0; i<24; i++){

    int j = 6*(i%4)+i/4+1;

    double sum = 0;
    int baseline_cnt=0;
        // 将fadc_data的值填充到直方图中
           for(int clk=0; clk<1025; clk++){
	    
      if (clk >= 500 && clk < 1000) {
            sum += fadc_data[1][clk][i];
            baseline_cnt++;
        }
        integ[i] += fadc_data[1][clk][i];
    }
    if (baseline_cnt > 0) {
        fadc_c_bl[i] = sum / baseline_cnt;
    } else {
        fadc_c_bl[i] = 0; // In case there's no data in the range
    }
    integ[i]=-integ[i]+1025*fadc_c_bl[i];

    }
    // 绘制直方图
    integ_hist->Fill(integ[chosen]);

    // 清理

}
integ_hist->Draw();
}


