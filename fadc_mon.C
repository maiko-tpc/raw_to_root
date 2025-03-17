#include <algorithm>
void fadc_mon(int runno, int evtcnt){
  gStyle->SetPalette(56);
  
  TFile *file = new TFile(Form("./rootfile/MAIKo_%05d.root",runno), "read");
  TTree *tree = (TTree*)file->Get("tree");

  unsigned int fadc_data[2][1025][24]={0};
  int evt_cnt;
  
  tree->SetBranchAddress("fadc_data",fadc_data);
  tree->SetBranchAddress("evt_cnt",&evt_cnt);

  TCanvas *c1 = new TCanvas("c1", "c1", 1000, 500);
  TCanvas *c2 = new TCanvas("c2", "c2", 1000, 500);
  c1->Divide(6,4);
  c2->Divide(6,4);
  TCanvas *c3 = new TCanvas("c3", "c3", 1000, 600);
  
  TH2D *fadc_a[24];
  TH2D *fadc_c[24];
double fadc_c_min[24]; // Array to store minimum values
double fadc_c_bl[24];  // Array to store baseline values
double fadc_c_max[24]; // Array to store maximum values

  for(int i=0; i<24; i++){

    int j = 6*(i%4)+i/4+1;

    fadc_a[i]= new TH2D(Form("fadc_a%d",i), Form("fadc_a%d",i), 1024, 0, 1023, 1024, 0, 1023);
    fadc_c[i]= new TH2D(Form("fadc_c%d",i), Form("fadc_c%d",i), 1024, 0, 1023, 1024, 0, 1023);
  
  TLatex *title = new TLatex();
  title->SetTextFont(132);
  title->SetTextSize(0.7);
  title->SetTextAlign(11);  

  tree->Draw(">>elist",Form("evt_cnt==%d",evtcnt));
  TEventList *elist = (TEventList*)gDirectory->Get("elist");
  tree->GetEntry(elist->GetEntry(0));
  
    fadc_a[i]->Reset();
    fadc_c[i]->Reset();
    fadc_c_min[i] = DBL_MAX;
   // fadc_c_max[i] = -DBL_MAX; // Initialize to the lowest possible value
    double sum = 0;
    int baseline_cnt = 0;// Initialize to the lowest possible value
    int minIndex = -1; // 存储最小值的索引
    for(int clk=0; clk<1025; clk++){
	    
      fadc_a[i]->Fill(clk,fadc_data[0][clk][i]);

	
      fadc_c[i]->Fill(clk,fadc_data[1][clk][i]);


        // 如果当前值比上一个值大，则可能是局部最大值
        double currentValue = fadc_data[1][clk][i];
        if(currentValue < fadc_c_min[i] && currentValue != 0){
            fadc_c_min[i] = currentValue;
            minIndex = clk;
        }
        // Calculate baseline
        if (clk >= 500 && clk < 1000) {
            sum += fadc_data[1][clk][i];
            baseline_cnt++;
        }
    }

    if (baseline_cnt > 0) {
        fadc_c_bl[i] = sum / baseline_cnt;
    } else {
        fadc_c_bl[i] = 0; // In case there's no data in the range
    }
    if (minIndex == -1) continue;

    fadc_c_max[i] = -DBL_MAX;
    double currentMax = -DBL_MAX; 
        for(int clk=0; clk < minIndex; clk++){
        double currentValue = fadc_data[1][clk][i];
        if(currentValue > currentMax){
            currentMax = currentValue;
        }
    }
    fadc_c_max[i] = currentMax;
// 先创建TGraph对象的数组---------------------
// 创建 TGraph 对象数组并设置横纵轴范围为 0 到 1023
TGraph *graph_fadc_a[24];
for (int i = 0; i < 24; ++i) {
    graph_fadc_a[i] = new TGraph();
    graph_fadc_a[i]->SetName(Form("graph_fadc_a%d", i));
    // 填充数据
    for (int clk = 0; clk < 1025; ++clk) {
        graph_fadc_a[i]->SetPoint(clk, clk, fadc_data[0][clk][i]);
    }
    // 固定横轴范围：0 到 1023
    graph_fadc_a[i]->GetXaxis()->SetLimits(0, 1023);
    // 固定纵轴范围：0 到 1023
    graph_fadc_a[i]->SetMinimum(0);
    graph_fadc_a[i]->SetMaximum(1023);

    // 设置点的样式和大小
    graph_fadc_a[i]->SetMarkerStyle(20);
    graph_fadc_a[i]->SetMarkerSize(0.1);
}

// 绘制 Anode 图形
for (int i = 0; i < 24; ++i) {
    int j = 6 * (i % 4) + i / 4 + 1;
    c1->cd(j);
    gPad->SetTopMargin(0.02);
    gPad->SetBottomMargin(0.12);
    gPad->SetRightMargin(0.04);
    gPad->SetLeftMargin(0.2);
    graph_fadc_a[i]->Draw("AL");
    title->DrawLatex(50, 1750, "Anode");
    title->DrawLatex(50, 1860, Form("Run%d, eve%d", runno, evtcnt));
}

// 创建 TGraph 对象数组并设置横纵轴范围为 0 到 1023
TGraph *graph_fadc_c[24];
for (int i = 0; i < 24; ++i) {
    graph_fadc_c[i] = new TGraph();
    graph_fadc_c[i]->SetName(Form("graph_fadc_c%d", i));
    for (int clk = 0; clk < 1025; ++clk) {
        graph_fadc_c[i]->SetPoint(clk, clk, fadc_data[1][clk][i]);
    }
    // 固定横轴范围：0 到 1023
    graph_fadc_c[i]->GetXaxis()->SetLimits(0, 1023);
    // 固定纵轴范围：0 到 1023
    graph_fadc_c[i]->SetMinimum(0);
    graph_fadc_c[i]->SetMaximum(1023);

    // 设置点的样式和大小
    graph_fadc_c[i]->SetMarkerStyle(20);
    graph_fadc_c[i]->SetMarkerSize(0.1);
}

// 绘制 Cathode 图形
for (int i = 0; i < 24; ++i) {
    int j = 6 * (i % 4) + i / 4 + 1;
    c2->cd(j);
    gPad->SetTopMargin(0.02);
    gPad->SetBottomMargin(0.12);
    gPad->SetRightMargin(0.04);
    gPad->SetLeftMargin(0.2);
    graph_fadc_c[i]->Draw("AL");
    title->DrawLatex(50, 1750, "Cathode");
    title->DrawLatex(50, 1860, Form("Run%d, eve%d", runno, evtcnt));
}


c3->cd();

// 设置画布边距
gPad->SetTopMargin(0.02);
gPad->SetBottomMargin(0.12);
gPad->SetRightMargin(0.04);
gPad->SetLeftMargin(0.2);
graph_fadc_c[7]->GetXaxis()->SetTitle("50 MHz clock");
graph_fadc_c[7]->GetYaxis()->SetTitle("FADC_V (ch)");
graph_fadc_c[7]->GetXaxis()->SetLabelSize(0.05);
graph_fadc_c[7]->GetYaxis()->SetLabelSize(0.05);
graph_fadc_c[7]->GetXaxis()->SetTitleSize(0.05);
graph_fadc_c[7]->GetYaxis()->SetTitleSize(0.05);
// 绘制阴极的第一个图
graph_fadc_c[7]->Draw("AL"); // 使用"AP"选项绘制：轴（Axes）和点（Points）

// 绘制标题等
//title->DrawLatex(50, 1750, "Cathode");
//title->DrawLatex(50, 1860, Form("Run%d, eve%d", runno, evtcnt));

  }
  
c1->Update();
 c2->Update();  
 c3->Update();                                     
for (int i = 0; i < 24; i++) {
    std::cout << "Channel " << i << ":" << std::endl;
    std::cout << "  Minimum Value: " << fadc_c_min[i] << std::endl;
    std::cout << "  Maximum Value: " << fadc_c_max[i] << std::endl;
    std::cout << "  Baseline (Avg 500-999ch): " << fadc_c_bl[i] << std::endl;
    std::cout << std::endl; // For better readability
} 
double minVal = *std::min_element(fadc_c_min, fadc_c_min + 11);
double maxVal = *std::max_element(fadc_c_min, fadc_c_min + 11);

// 计算最大值和最小值的比率
double ratio = 0;
if (minVal != 0) {  // 避免除以零
    ratio = maxVal / minVal;
} else {
    std::cout << "Minimum value is zero, cannot compute ratio." << std::endl;
}

// 输出比率
std::cout << "Ratio of max to min in fadc_c_min: " << ratio << std::endl;
}