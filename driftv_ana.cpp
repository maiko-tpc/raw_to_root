int driftv_ana(int runno){
  
TFile* ifile;
TTree* tree1;

TFile* ofile;
TTree* tree2;

 int evt_cnt ; 
  
 unsigned int hit_data_a[24][2048]={0}; // 768/32
 //unsigned int hit_data_c[24][2048] ={0};

 double driftv = 0.0;
 int n_driftv = 0;
 double sum_driftv = 0.0;
 double ave_driftv = 0.0;

 int min_clk[768];
 int max_clk[768];
 int mean_clk_1;
 int mean_clk_2;
  
 int min_strp;
 int max_strp;
  
 unsigned int hit_a[2048][768];
 // unsigned int hit_c[2048][768];  
  
 ifile = new TFile(Form("./rootfile/MAIKo_%05d.root",runno),"READ");
 tree1 = (TTree*)ifile->Get("tree");

 ofile = new TFile(Form("./driftv/driftv_%04d.root",runno),"RECREATE");
 tree2 = new TTree("tree","tree"); 

 tree1->SetBranchAddress("evt_cnt",&evt_cnt);
 tree1->SetBranchAddress("hit_data_a",hit_data_a);
 //tree1->SetBranchAddress("hit_data_c",hit_data_c);
 
 tree2->Branch("runno",&runno,"runno/I");
 tree2->Branch("evt_cnt",&evt_cnt,"evt_cnt/I");
 tree2->Branch("driftv",&driftv,"driftv/D");
 tree2->Branch("ave_driftv",&ave_driftv,"ave_driftv/D");
 tree2->Branch("n_driftv",&n_driftv,"n_driftv/I");
 //tree2->Branch("hit_data_a",hit_data_a,"hit_data_a[2048][24]/i");
 //tree2->Branch("hit_data_c",hit_data_c,"hit_data_c[2048][24]/i");

 //for(evt_num = 1; evt_num<2; evt_num++){
    //Tree1->Draw(">>elist",Form("evt_num==%d",evt_num));
    //TEventList *elist = (TEventList*)gDirectory->Get("elist");

     //tree1->GetEntry(elist->GetEntry(0));
 
 for(int ii=0; ii<tree1->GetEntries(); ii++){

 tree1->GetEntry(ii);
   
   for(int i=0; i<768; i++){
     min_clk[i] =2048;
     max_clk[i] =0;
   }
   
   min_strp =768;
   max_strp =0;
   
   for(int ch=0; ch<24; ch++){
     for(int clk=0; clk<2048; clk++){
       for(int i=0; i<32; i++){

	 int ch2;
	 int ch_to_strp;
	 
	 ch2 = ((ch/4+1)*4)-(ch%4) -1;
	 ch_to_strp = (ch2)*32 +i;

	 if( ((hit_data_a[ch][clk]) &((unsigned int)0x1 <<(i))) ==1){
	   //printf("strp=%d\n",ch_to_strp);
	   //hit_c[clk][ch_to_strp] =1;

	   if(clk < min_clk[ch_to_strp]) min_clk[ch_to_strp]=clk;
	   if(clk > max_clk[ch_to_strp]) max_clk[ch_to_strp]=clk;

	   if((ch_to_strp > 50) && (ch_to_strp < min_strp)) min_strp = ch_to_strp;
	   if(ch_to_strp > max_strp) max_strp = ch_to_strp;
	    	  

	 }	  
       }
     }
   }

   mean_clk_1 = 0;
   mean_clk_2 = 0;
   
   //printf("min_strp_c=%d\n",min_strp_c);
   //printf("max_strp_c=%d\n",max_strp_c);

   mean_clk_1 = 0.5*(max_clk[max_strp] + min_clk[max_strp]) ;
   mean_clk_2 = 0.5*(max_clk[min_strp] + min_clk[min_strp]) ;
      
   //printf("mean[max_strp_c] = %d\n",mean_clk_c1);
   //printf("mean[min_strp_c] = %d\n",mean_clk_c2);
   
    
   int delta_strp;
   int delta_clk;

   //printf("min, max_strp = %d, %d \n",min_strp_c,max_strp_c);
   
   delta_strp =  max_strp - min_strp;
   delta_clk = mean_clk_2 - mean_clk_1;

   //printf("delta_strp=%d\n", delta_strp);
   //printf("delta_clk=%d\n", delta_clk);

   driftv=0.0;
   
   if((min_strp<100) && (delta_clk>200) && (delta_strp<500)){
   //if((min_strp_c<100) ){
     
     driftv = (0.4*delta_strp)/(sqrt(3)*10*delta_clk);
     //driftv = (0.4*a)/(sqrt(3)*10);
     printf("driftv=%f\n",driftv);
     n_driftv++;
     
     sum_driftv += driftv;

   }

   ave_driftv = sum_driftv/n_driftv;
   printf("ave_driftv=%f\n",ave_driftv);     
   tree2->Fill();  
 }
 
 printf("ave_driftv=%f\n",ave_driftv);
 printf("n=%d\n",n_driftv);
 
 tree2->Write();
 ofile->Close();
  
 return 0;
}



    

