#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include "TSystem.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include "TGraph.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TH2D.h"
#include "TH1D.h"
#include "TLatex.h"
#include <iostream>

//------------------------------------------------------------
// 从输入文件中读取 TTree，并克隆到内存中，确保后续读取数据不会出错
TTree* GetInputTree(int runno) {
    TFile* file = new TFile(Form("./rootfile/MAIKo_0%04d.root", runno), "READ");
    if (!file || file->IsZombie()) {
        printf("Error opening file ./rootfile/MAIKo_0%04d.root.\n", runno);
        if (file) delete file;
        return nullptr;
    }
    TTree* tree = (TTree*)file->Get("tree");
    if (!tree) {
        printf("Error: tree 'tree' not found in file.\n");
        file->Close();
        delete file;
        return nullptr;
    }
    // 克隆整个树到内存中
    TTree* tree_copy = tree->CloneTree(-1);
    file->Close();
    delete file;
    return tree_copy;
}

//------------------------------------------------------------
// 利用输入 TTree 中的 hit_data_a 填充 TGraph
void FillTrackGraph(TTree* tree, int eventno, unsigned int hit_data_a[24][2048], TGraph* graph) {
    tree->GetEntry(eventno - 1);
    int pointIndex = 0;
    for (int ch = 0; ch < 24; ch++) {
        for (int clk = 0; clk < 2048; clk++) {
            for (int i = 0; i < 32; i++) {
                int ch2 = ((ch / 4 + 1) * 4) - (ch % 4) - 1;
                if (hit_data_a[ch][clk] & (1U << i)) {
                    int ch_to_strip = ch2 * 32 + i;
                    graph->SetPoint(pointIndex++, ch_to_strip, clk);
                }
            }
        }
    }
}

//------------------------------------------------------------
// 从输入 TTree中获取 100Hz 时钟计数值
int GetClk100Hz(TTree* tree, int eventno) {
    int sca_cnt[12] = {0};
    tree->SetBranchAddress("sca_cnt", sca_cnt);
    tree->GetEntry(eventno - 1);
    return sca_cnt[2];  // 假定索引2为100Hz时钟
}

//------------------------------------------------------------
// 对某个事件进行轨迹拟合，返回拟合直线斜率 p1
double TrackFitting(TTree* tree, int eventno) {
    int evt_cnt = 0;
    unsigned int hit_data_a[24][2048] = {0};
    tree->SetBranchAddress("evt_cnt", &evt_cnt);
    tree->SetBranchAddress("hit_data_a", hit_data_a);
    
    Long64_t nentries = tree->GetEntries();
    if (eventno >= nentries) {
        printf("Error: event number %d is out of range. Total events: %lld\n", eventno, nentries);
        return 0.0;
    }
    
    TGraph* graph = new TGraph();
    FillTrackGraph(tree, eventno, hit_data_a, graph);
    
    TF1* f1 = new TF1("f1", "pol1", 0, 2048);
    // 静默拟合（不输出信息）
    graph->Fit(f1, "Q");
    double p1 = f1->GetParameter(1);
    
    delete f1;
    delete graph;
    return p1;
}

//------------------------------------------------------------
// 可选：绘制拟合结果并保存调试用图片（如果需要调试，可以打开下面这段代码）
// hit_data_a 可通过 tree->GetEntry(eventno-1) 填充后传入
void DrawAndSaveCanvas(TTree* tree, int eventno, int runno, double p0, double p1, unsigned int hit_data_a[24][2048]) {
    TGraph* graph = new TGraph();
    FillTrackGraph(tree, eventno, hit_data_a, graph);
    TCanvas* c1 = new TCanvas("c1", "c1", 700, 1000);
    c1->cd();
    gPad->SetGrid(1, 1);
    graph->SetMarkerStyle(20);
    graph->Draw("AP");
    
    TLatex* title = new TLatex();
    title->SetNDC();
    title->SetTextSize(0.05);
    title->DrawLatex(0.1, 0.9, "Anode");
    title->DrawLatex(0.1, 0.95, Form("Run%d, eve%d", runno, eventno));
    title->DrawLatex(0.1, 0.85, Form("p0: %.2f, p1: %.2f", p0, p1));
    
    c1->Update();
    c1->SaveAs(Form("./driftv/fit_%04d_event_%04d.png", runno, eventno));
    
    delete title;
    delete c1;
    delete graph;
}

//------------------------------------------------------------
// 主函数：进行漂移速度拟合，并将结果保存到输出 ROOT 文件中
void driftv_fit(int runno) {
    // 打开并克隆输入树
    TTree* inputTree = GetInputTree(runno);
    if (!inputTree) return;
    Long64_t nentries = inputTree->GetEntries();
    std::cout << "Run " << runno << " contains " << nentries << " events." << std::endl;
    
    // 创建输出文件，并确保在创建 TTree 前先打开文件
    TFile* filev = new TFile(Form("./driftv/fit_%04d.root", runno), "RECREATE");
    if (!filev || filev->IsZombie()) {
        printf("Error opening output file ./driftv/fit_%04d.root.\n", runno);
        delete filev;
        return;
    }
    
    // 创建输出 TTree，该树自动关联到 filev（正确的创建顺序）
    TTree* treev = new TTree("driftv_tree", "Tree with drift velocity");
    int out_runno = runno;
    int evt_cnt = 0;
    int clk100Hz = 0;
    double driftv = 0.0;
    double p1 = 0.0;  // 拟合参数
    
    treev->Branch("runno", &out_runno, "runno/I");
    treev->Branch("evt_cnt", &evt_cnt, "evt_cnt/I");
    treev->Branch("clk100Hz", &clk100Hz, "clk100Hz/I");
    treev->Branch("driftv", &driftv, "driftv/D");
    treev->Branch("p1", &p1, "p1/D");
    
    // 依次对部分事件进行处理（这里以 event 从 1000 开始，每隔 3000 取一个）
    for (int eventno = 1000; eventno < nentries; eventno += 3000) {
        if (eventno % 1000 == 0)
            std::cout << "Progress: " << eventno << " / " << nentries << " completed." << std::endl;
        
        // 从输入树中获取 evt_cnt（可根据需要修改）
        inputTree->SetBranchAddress("evt_cnt", &evt_cnt);
        inputTree->GetEntry(eventno - 1);
        
        // 进行轨迹拟合，获取斜率 p1
        p1 = TrackFitting(inputTree, eventno);
        // 获取 100Hz 时钟值
        clk100Hz = GetClk100Hz(inputTree, eventno);
        
        // 根据拟合结果计算漂移速度（公式可根据实际情况调整）
        if (p1 != 0)
            driftv = 0.4 / (p1 * sqrt(3));
        else
            driftv = 0.0;
        
        treev->Fill();
        
        // 可选：调试时绘制图片
        // unsigned int hit_data_a[24][2048] = {0};
        // inputTree->SetBranchAddress("hit_data_a", hit_data_a);
        // DrawAndSaveCanvas(inputTree, eventno, runno, 0.0, p1, hit_data_a);
    }
    
    // 写入并关闭输出文件
    treev->Write();
    filev->Write();
    filev->Close();
    
    // 为绘制图像再打开一次输出文件（只读方式）
    TFile* filev_read = new TFile(Form("./driftv/fit_%04d.root", runno), "READ");
    TTree* treev_read = (TTree*)filev_read->Get("driftv_tree");
    TCanvas* c2 = new TCanvas("c2", "c2", 700, 500);
    treev_read->Draw("driftv:clk100Hz", Form("runno==%d", runno));
    c2->SaveAs(Form("./driftv/fit_%04d_driftv_vs_clk100Hz.pdf", runno));
    
    filev_read->Close();
    
    // 清理内存
    delete inputTree;
    delete c2;
    delete filev_read;
}
