int evt_num = 0;
Bool_t bnext = kFALSE;
Bool_t bprev = kFALSE;
Bool_t bdraw = kFALSE;

void webmon(int runno=1546){ 
  TFile *file = new TFile(Form("./rootfile/MAIKo_%05d.root",runno), "read");    
  TTree *tree = (TTree*)file->Get("tree");                                      
                                                                                
  unsigned int hit_data_a[24][2048]={0};                                        
  unsigned int hit_data_c[24][2048]={0};                                        
                                                                                
  tree->SetBranchAddress("hit_data_a", hit_data_a);
  tree->SetBranchAddress("hit_data_c", hit_data_c);  
  
  TH2D *track_a = new TH2D("track_a", "", 768, 0, 768, 2048, 0, 2048);          
  TH2D *track_c = new TH2D("track_c", "", 768, 0, 768, 2048, 0, 2048);          

  track_a->GetXaxis()->SetTitle("strip");
  track_a->GetYaxis()->SetTitle("100MHz clock");
  track_c->GetXaxis()->SetTitle("strip");
  track_c->GetYaxis()->SetTitle("100MHz clock");

  track_a->SetDrawOption("box");
  track_c->SetDrawOption("box");  
  
  track_a->SetName("track_anode");                                              
  track_c->SetName("track_cathode");                                            
                                                                                
  auto serv = new THttpServer("http:8080");                                     
  serv->SetItemField("/","_monitoring","100");
  serv->Register("track/", track_a);                                            
  serv->Register("track/", track_c);                                            
                                                                                
  serv->RegisterCommand("/Previous", "bprev=kTRUE;",
			"button;rootsys/icons/bld_undo.png");
  serv->RegisterCommand("/Next", "bnext=kTRUE;",
			"button;rootsys/icons/bld_redo.png");

  while(kTRUE){
    if(bnext){
      evt_num++;
      bdraw=kTRUE;
      bnext=kFALSE;
    }
    
    if(bprev){
      evt_num--;
      if(evt_num<0) evt_num=0;
      bdraw=kTRUE;
      bprev=kFALSE;
    }
    
    if(bdraw){
      tree->GetEntry(evt_num);
      track_a->Reset();
      track_c->Reset();
      
      track_a->SetTitle(Form("Run:%d, Eve:%d, Anode", runno, evt_num));
      track_c->SetTitle(Form("Run:%d, Eve:%d, Cathode", runno, evt_num));      

      for(int ch=0; ch<24; ch++){                                                   
	for(int clk=0; clk<2048; clk++){                                            
	  for(int i=0; i<32; i++){                                                  
	    int ch2,ch3;                                                            
	    ch2 = ((ch/4+1)*4)-(ch%4) -1;                                           
	    int ch_to_strp;                                                         
	    if((hit_data_a[ch][clk]) & ((unsigned int)0x1<<(i))){
	      ch_to_strp = (ch2)*32 + i;                                            
	      //printf("ch_to_strp=%d\n",ch_to_strp);                               
	      track_a->Fill(ch_to_strp,clk);                                        
	    }
	    if((hit_data_c[ch][clk]) & ((unsigned int)0x1<<(i))){	      
	      ch_to_strp = (ch2)*32 + i;                                            
	      track_c->Fill(ch_to_strp,clk);                                        
	    }                                                                       
	  }                                                                         
	}                                                                           
      }
      bdraw=kFALSE;
    } // end of if(bdraw)
    

    if(!bnext || !bprev){
      gSystem->Sleep(10); // sleep minimal time;
      if(gSystem->ProcessEvents()) break;
    }

  } // end of while(kTRUE)
}
