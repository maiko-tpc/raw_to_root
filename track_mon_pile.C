void track_mon_pile(int runno, int evtnum)
{
  gStyle->SetPalette(56);
  
  TFile *file = new TFile(Form("./rootfile/MAIKo_%05d.root",runno), "read");
  TTree *tree = (TTree*)file->Get("tree");

  unsigned int hit_data_a[24][2048]={0};
  unsigned int hit_data_c[24][2048]={0};
  int evt_num;

  //unsigned int hit_a[768][2048];
  unsigned int hit_c[768][2048];
  
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
  title->SetTextSize(0.1);

  //c->Clear();                                                                    
  //track_a->Reset();
  //track_c->Reset();
  
  for(int cnt=0; cnt<300; cnt++){
    
    tree->Draw(">>elist",Form("evt_num==%d",(evtnum+cnt)));
    TEventList *elist = (TEventList*)gDirectory->Get("elist");
    tree->GetEntry(elist->GetEntry(0));

    int min_clk_c[768], max_clk_c[768];
    int mean_clk_c1, mean_clk_c2;
    int delta_clk;

    int ch_to_strp;
    int clk;
    
    for(int i=0; i<768; i++){
      min_clk_c[i] =2048;
      max_clk_c[i] =0;
      for(int j=0; j<2048; j++){
	//hit_a[i][j]=0;
	hit_c[i][j]=0;
      }
    }
  
    
    for(int ch=0; ch<24; ch++){
      for(clk=0; clk<2048; clk++){
	//for(int strp=0;strp<768;++strp){
	for(int i=0; i<32; i++){
	
	  //if(i==0) printf("ch=%d, ch2=%d\n",ch,ch2);	    	
	  int ch2,ch3;   
	  ch2 = ((ch/4+1)*4)-(ch%4) -1;
	  
	  ch_to_strp = (ch2)*32 + i;
	  
	  if((hit_data_a[ch][clk]) & ((unsigned int)0x1<<(i))){
	    track_a->Fill(ch_to_strp,clk);
	  }
	    
	  if((hit_data_c[ch][clk]) & ((unsigned int)0x1<<(i))){
	      
	    hit_c[ch_to_strp][clk] = 1;

	    if(clk < min_clk_c[ch_to_strp]) min_clk_c[ch_to_strp] =clk;
	    if(clk > max_clk_c[ch_to_strp]) max_clk_c[ch_to_strp] =clk;

	    //track_c->Fill(ch_to_strp,clk);
	  }
	}	  
      }
    }
    
    //mean_clk_c1 =0;
    //mean_clk_c2 =0;

    mean_clk_c1 =0.5*(max_clk_c[300] + min_clk_c[300]);
    mean_clk_c2 =0.5*(max_clk_c[10] + min_clk_c[10]);
    
    delta_clk = mean_clk_c2 - mean_clk_c1;
    
    //if((mean_clk_c2 < 1000)&&(delta_clk < 400) &&(delta_clk>0)){
    //if(hit_c[500][clk]==0){
    
    for(clk=0; clk<2048; clk++){
	for(ch_to_strp=0; ch_to_strp<768; ch_to_strp++){
	  
	  //if((hit_data_a[ch][clk]) & ((unsigned int)0x1<<(i))){
	  //track_a->Fill(ch_to_strp,clk);
	  //}
	  //if(hit_a[ch_to_strp][clk]==1) track_a->Fill(ch_to_strp,clk);

	  if(hit_c[ch_to_strp][clk]==1) track_c->Fill(ch_to_strp,clk);
	  
	}
      }
    }

  //}
  
  pad_a->cd();
  track_a->SetStats(kFALSE);
  track_a->GetXaxis()->SetLabelSize(0.05);
  track_a->GetYaxis()->SetLabelSize(0.05);
  track_a->Draw("col");
  title->DrawLatexNDC(0.25, 0.85, "anode image");

  pad_c->cd();
  track_c->SetStats(kFALSE);
  track_c->GetXaxis()->SetLabelSize(0.05);
  track_c->GetYaxis()->SetLabelSize(0.05);
  track_c->Draw("col");
  title->DrawLatexNDC(0.15, 0.85, "cathode image");
  c->Update();

//  c->Print(Form("%d_%d.png", RunNo, EvtNo));                                       

}
