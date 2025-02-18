int eventlist(int runno){
  
TFile* ifile;
TTree* tree1;

 int evt_cnt ; 
  
 unsigned int hit_data_a[24][2048] ={0}; // 768/32
 unsigned int hit_data_c[24][2048] ={0};

 int min_clk_c[768];
 int max_clk_c[768];
 int mean_clk_c1;
 int mean_clk_c2;
  
 int min_strp_c;
 int max_strp_c;
  
 unsigned int hit_c[2048][768];  
  
 ifile = new TFile(Form("./rootfile/MAIKo_%05d.root",runno),"READ");
 tree1 = (TTree*)ifile->Get("tree");

 ofstream ofs(Form("./run%05d.list",runno));

 tree1->SetBranchAddress("evt_cnt",&evt_cnt);
 // tree1->SetBranchAddress("hit_data_a",hit_data_a);
 tree1->SetBranchAddress("hit_data_c",hit_data_c);
  
 //tree2->Branch("hit_data_a",hit_data_a,"hit_data_a[2048][24]/i");
 //tree2->Branch("hit_data_c",hit_data_c,"hit_data_c[2048][24]/i");

 for(int ii=0; ii<tree1->GetEntries(); ii++){

 tree1->GetEntry(ii);
   
   for(int i=0; i<768; i++){
     min_clk_c[i] =2048;
     max_clk_c[i] =0;
   }
   
   min_strp_c =768;
   max_strp_c =0;
   
   for(int ch=0; ch<24; ch++){
     for(int clk=0; clk<2048; clk++){
       for(int i=0; i<32; i++){

	 int ch2;
	 int ch_to_strp;
	 
	 ch2 = ((ch/4+1)*4)-(ch%4) -1;
	 ch_to_strp = (ch2)*32 +i;

	 if( ((hit_data_c[ch][clk]) &((unsigned int)0x1 <<(i))) ==1){
	   //printf("strp=%d\n",ch_to_strp);
	   //hit_c[clk][ch_to_strp] =1;

	   if(clk < min_clk_c[ch_to_strp]) min_clk_c[ch_to_strp]=clk;
	   if(clk > max_clk_c[ch_to_strp]) max_clk_c[ch_to_strp]=clk;

	   if((ch_to_strp > 50) && (ch_to_strp < min_strp_c)) min_strp_c = ch_to_strp;
	   if(ch_to_strp > max_strp_c) max_strp_c = ch_to_strp;
	    	  

	 }	  
       }
     }
   }

   mean_clk_c1 = 0;
   mean_clk_c2 = 0;
   
   //printf("min_strp_c=%d\n",min_strp_c);
   //printf("max_strp_c=%d\n",max_strp_c);

   mean_clk_c1 = 0.5*(max_clk_c[max_strp_c] + min_clk_c[max_strp_c]) ;
   mean_clk_c2 = 0.5*(max_clk_c[min_strp_c] + min_clk_c[min_strp_c]) ;
      
   //printf("mean[max_strp_c] = %d\n",mean_clk_c1);
   //printf("mean[min_strp_c] = %d\n",mean_clk_c2);
   
    
   int delta_strp;
   int delta_clk;

   //printf("min, max_strp = %d, %d \n",min_strp_c,max_strp_c);
   
   delta_strp =  max_strp_c - min_strp_c;
   delta_clk = mean_clk_c2 - mean_clk_c1;

   //printf("delta_strp=%d\n", delta_strp);
   //printf("delta_clk=%d\n", delta_clk);

   //if(delta_strp<700){
     ofs<<evt_cnt<<endl;
     //}
   
 }
 
  
 return 0;
}



    

