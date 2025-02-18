void fadc_mon(int runno, int evtcnt)
{
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
  
  //TPad *pad_a[6];
  //TPad *pad_c[6];

  /*
  TPad *pad_a0 = new TPad("pad_a0", "pad_a0", 0, 0.5, 0.15, 1);
  TPad *pad_a1 = new TPad("pad_a1", "pad_a1", 0.15, 0.5, 0.3, 1);
  TPad *pad_a2 = new TPad("pad_a2", "pad_a2", 0.3, 0.5, 0.45, 1);
  TPad *pad_a3 = new TPad("pad_a3", "pad_a3", 0.45, 0.5, 0.6, 1);
  TPad *pad_a4 = new TPad("pad_a4", "pad_a4", 0.6, 0.5, 0.75, 1);
  TPad *pad_a5 = new TPad("pad_a5", "pad_a5", 0.75, 0.5, 0.9, 1);

  TPad *pad_c0 = new TPad("pad_c0", "pad_c0", 0, 0, 0.15, 0.5);
  TPad *pad_c1 = new TPad("pad_c1", "pad_c1", 0.15, 0, 0.3, 0.5);
  TPad *pad_c2 = new TPad("pad_c2", "pad_c2", 0.3, 0, 0.45, 0.5);
  TPad *pad_c3 = new TPad("pad_c3", "pad_c3", 0.45, 0, 0.6, 0.5);	
  TPad *pad_c4 = new TPad("pad_c4", "pad_c4", 0.6, 0, 0.75, 0.5);
  TPad *pad_c5 = new TPad("pad_c5", "pad_c5", 0.75, 0, 0.9, 0.5);
  */
  
  TH2D *fadc_a[24];
  TH2D *fadc_c[24];

  for(int i=0; i<24; i++){

    int j = 6*(i%4)+i/4+1;
    
    //pad_a[i]= new TPad(Form("pad_a%d",i), "pad_a", i*0.15, 0.5, 0.15*(i+1), 1);
    //pad_c[i]= new TPad(Form("pad_c%d",i), "pad_c", i*0.15, 0, 0.15*(i+1), 0.5);
    
    fadc_a[i]= new TH2D(Form("fadc_a%d",i), Form("fadc_a%d",i), 2048, 0, 1025, 800, 0, 800);
    fadc_c[i]= new TH2D(Form("fadc_c%d",i), Form("fadc_c%d",i), 2048, 0, 1025, 800, 0, 800);
  
  
  /*
  pad_c->SetTopMargin(0.02);
  pad_c->SetBottomMargin(0.12);
  pad_c->SetRightMargin(0.1);
  pad_c->SetLeftMargin(0.1);
  pad_c->Draw();
  pad_c->SetTicks(1);
  */
  
  TLatex *title = new TLatex();
  title->SetTextFont(132);
  title->SetTextSize(0.7);
  title->SetTextAlign(11);  

  tree->Draw(">>elist",Form("evt_cnt==%d",evtcnt));
  TEventList *elist = (TEventList*)gDirectory->Get("elist");
  tree->GetEntry(elist->GetEntry(0));
  //  c->Clear();                                                                    

    
    fadc_a[i]->Reset();
    fadc_c[i]->Reset();
        
    for(int clk=0; clk<1025; clk++){
      //      for(int ch=0; ch<24; ch++){
      
	//int ch2;   
	//ch2 = ((ch/4+1)*4)-(ch%4) -1;
   	//int ch_to_strp;
	    
      fadc_a[i]->Fill(clk,fadc_data[0][clk][i]);
	//printf("fadc= %u\n",fadc_data[0][clk][0]);
	
      fadc_c[i]->Fill(clk,fadc_data[1][clk][i]);
	//printf("fadc= %u",fadc_data[1][0][clk]);
	
	//	}
    }


    
    c1->cd(j);
    gPad->SetTopMargin(0.02);
    gPad->SetBottomMargin(0.12);
    gPad->SetRightMargin(0.04);
    gPad->SetLeftMargin(0.2);
    gPad->Draw();
    gPad->SetTicks(1);
  
    fadc_a[i]->SetStats(kFALSE);
    fadc_a[i]->GetXaxis()->SetLabelSize(0.05);
    fadc_a[i]->GetYaxis()->SetLabelSize(0.05);
    fadc_a[i]->SetLineColor(1);
    fadc_a[i]->Draw();
    title->DrawLatex(50, 1750, "Anode");
    title->DrawLatex(50, 1860, Form("Run%d, eve%d", runno, evtcnt));  
      
      
    c2->cd(j);
    gPad->SetTopMargin(0.02);
    gPad->SetBottomMargin(0.12);
    gPad->SetRightMargin(0.04);
    gPad->SetLeftMargin(0.2);
    gPad->Draw();
    gPad->SetTicks(1);
  
      
    fadc_c[i]->SetStats(kFALSE);
    fadc_c[i]->GetXaxis()->SetLabelSize(0.05);
    fadc_c[i]->GetYaxis()->SetLabelSize(0.05);
    fadc_c[i]->SetLineColor(1);
    fadc_c[i]->Draw();
    title->DrawLatex(50, 1750, "Cathode");
    title->DrawLatex(50, 1860, Form("Run%d, eve%d", runno, evtcnt));  

  }
  
c1->Update();
 c2->Update();
//  c->Print(Form("%d_%d.png", RunNo, EvtNo));                                       

}
