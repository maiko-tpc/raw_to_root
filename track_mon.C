void track_mon(int runno, int evtnum)
{
  gStyle->SetPalette(56);
  
  TFile *file = new TFile(Form("./rootfile/MAIKo_%05d.root",runno), "read");
  TTree *tree = (TTree*)file->Get("tree");

  unsigned int hit_data_a[24][2048]={0};
  unsigned int hit_data_c[24][2048]={0};
  int evt_num;
  int clk_cnt[12];
#define N_ac 2
#define N_hit_clk 2048
#define N_hit_ch 768
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

  /*
   unsigned int** hit_data_a;
  hit_data_a = (unsigned int**)malloc(sizeof(unsigned int*)*N_hit_clk);
  if(hit_data_a==NULL) exit(1);
  for(int i=0; i<N_hit_clk; i++){
    hit_data_a[i] = (unsigned int*)malloc(sizeof(unsigned int)*N_hit_ch);
    if(hit_data_a[i]==NULL) exit(1);
  }
  */
  
  tree->SetBranchAddress("hit_data_a",hit_data_a);
  tree->SetBranchAddress("hit_data_c",hit_data_c);
  tree->SetBranchAddress("evt_num",&evt_num);
  tree->SetBranchAddress("clk_cnt",&clk_cnt);
  TCanvas *c = new TCanvas("c", "c", 700, 500);
  TPad *pad_a = new TPad("pad_a", "pad_a", 0, 0, 0.5, 1);
  TPad *pad_c = new TPad("pad_c", "pad_c", 0.5, 0, 1, 1);
  TH2D *track_a = new TH2D("track_a", "", 768, 0, 768, 2048, 0, 2048);
  TH2D *track_c = new TH2D("track_c", "", 768, 0, 768, 2048, 0, 2048);

  pad_a->SetTopMargin(0.02);
  pad_a->SetBottomMargin(0.12);
  pad_a->SetRightMargin(0.04);
  pad_a->SetLeftMargin(0.2);
  pad_a->Draw();
  pad_c->SetTopMargin(0.02);
  pad_c->SetBottomMargin(0.12);
  pad_c->SetRightMargin(0.1);
  pad_c->SetLeftMargin(0.1);
  pad_c->Draw();
  pad_a->SetTicks(1);
  pad_c->SetTicks(1);

  TLatex *title = new TLatex();
  title->SetTextFont(132);
  title->SetTextSize(0.07);
  title->SetTextAlign(11);  

  tree->Draw(">>elist",Form("evt_num==%d",evtnum));
  //  tree->Draw(">>elist",Form("clk_cnt==%d",clk_cnt));
  TEventList *elist = (TEventList*)gDirectory->Get("elist");
  tree->GetEntry(elist->GetEntry(0));
  //  c->Clear();                                                                    
  track_a->Reset();
  track_c->Reset();

  for(int ch=0; ch<24; ch++){
    for(int clk=0; clk<2048; clk++){
    //for(int strp=0;strp<768;++strp){      
      for(int i=0; i<32; i++){
	
	//if(i==0) printf("ch=%d, ch2=%d\n",ch,ch2);	    	
	int ch2,ch3;   
	ch2 = ((ch/4+1)*4)-(ch%4) -1;

	/*
	if(ch2>7 & ch2<16){
	  ch3 = ch2 + 8;
	}else if(ch2>15){
	  ch3 = ch2 - 8;
	}
	*/
	
	int ch_to_strp;
	    
	if((hit_data_a[ch][clk]) & ((unsigned int)0x1<<(i))){
	  //printf("%d\n",hit_data_a[strp/32][clk]);
	      
	  ch_to_strp = (ch2)*32 + i;
	  //printf("ch_to_strp=%d\n",ch_to_strp);
	  
	  track_a->Fill(ch_to_strp,clk);
	}
	    
	if((hit_data_c[ch][clk]) & ((unsigned int)0x1<<(i))){
	      
	  ch_to_strp = (ch2)*32 + i;

	  //printf("i=%d , ch_to_strp=%d\n",i,ch_to_strp);
	  
	  track_c->Fill(ch_to_strp,clk);
	}
      }	  
    }
  }

  
  pad_a->cd();
  gPad->SetGrid(1,1);
  track_a->SetStats(kFALSE);
  track_a->GetXaxis()->SetLabelSize(0.05);
  track_a->GetYaxis()->SetLabelSize(0.05);
  track_a->SetTitleSize(0.06,"XY");
  //  track_a->SetTitle(";strip number (ch);drift time (10 ns)");                        
  track_a->SetTitle(";strip number;100MHz clock");
  track_a->SetLineColor(1);
  track_a->Draw("box");
  title->DrawLatex(50, 1750, "Anode");
  title->DrawLatex(50, 1860, Form("Run%d, eve%d", runno, evtnum));


  pad_c->cd();
  gPad->SetGrid(1,1);
  track_c->SetStats(kFALSE);
  track_c->GetXaxis()->SetLabelSize(0.05);
  track_c->GetYaxis()->SetLabelSize(0.05);
  track_c->SetTitleSize(0.06,"X");
  track_c->SetTitle(";strip number (ch);");
  track_c->SetLineColor(1);
  track_c->Draw("box");
  title->DrawLatex(50, 1750, "Cathode");
  title->DrawLatex(50, 1860, Form("Run%d, eve%d", runno, evtnum));

  c->Update();
  
  
  c->Print(Form("%d_%d.pdf", runno, evtnum));
  for(int i=0; i<12; i++){
        cout<< hex << clk_cnt[i]<<endl;
  }
  /*    
  pad_a->cd();
  track_a->SetStats(kFALSE);
  track_a->GetXaxis()->SetLabelSize(0.05);
  track_a->GetYaxis()->SetLabelSize(0.05);
  track_a->SetLineColor(1);
  track_a->Draw("box");
  title->DrawLatex(50, 1750, "Anode");
  title->DrawLatex(50, 1860, Form("Run%d, eve%d", runno, evtnum));  
  
  
  pad_c->cd();
  track_c->SetStats(kFALSE);
  track_c->GetXaxis()->SetLabelSize(0.05);
  track_c->GetYaxis()->SetLabelSize(0.05);
  track_c->SetLineColor(1);
  track_c->Draw("box");
  title->DrawLatex(50, 1750, "Cathode");
  title->DrawLatex(50, 1860, Form("Run%d, eve%d", runno, evtnum));  
  c->Update();
  */
  //  c->Print(Form("%d_%d.png", RunNo, EvtNo));                                       

}
