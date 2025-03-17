using namespace std;

void ana_amp(int runno, int ch) {                                                                  
    gStyle->SetPalette(56);
    
    TFile *file = new TFile(Form("./rootfile/MAIKo_%05d.root", runno), "READ");                    
    if (!file || file->IsZombie()) {
        cout << "Error: Cannot open file!" << endl;
        return;
    }

    TTree *tree = (TTree*)file->Get("tree");
    if (!tree) {
        cout << "Error: TTree not found!" << endl;
        file->Close();
        return;
    }

    unsigned int fadc_data[2][1025][24] = {0};
    int sca_cnt[12];
    int evt_cnt = 0;
    
    tree->SetBranchAddress("fadc_data", fadc_data);
    tree->SetBranchAddress("evt_cnt", &evt_cnt);
    tree->SetBranchAddress("sca_cnt", sca_cnt);

    Long64_t nEntries = tree->GetEntries();
    tree->GetEntry(nEntries-1);
    int last_sca_cnt_2 = sca_cnt[2];  
    
    cout << "Last event sca_cnt[2]: " << last_sca_cnt_2 << endl;


    int minutes = 30;  
    int freq = 100;    //100Hz
    int div = std::ceil(static_cast<double>(last_sca_cnt_2) / (60.0 * minutes * freq));  
    cout << "hist number : "<< div<<endl;

    TH1F* h[div];
    for (int i = 0; i < div; ++i) {
        h[i] = new TH1F(Form("pulse_hight_%d", i), "Pulse Hight", 200, 0, 200);
    }
    TGraph* gr = new TGraph(div);    
    for (Long64_t event = 0; event < nEntries; event++) {
        tree->GetEntry(event);

        double fadc_c_min[24], fadc_c_max[24], fadc_c_bl[24];
        fadc_c_min[ch] = DBL_MAX;
        fadc_c_max[ch] = -DBL_MAX;

        double sum = 0;
        int baseline_cnt = 0;
        int minIndex = -1;

        for (int clk = 0; clk < 1025; clk++) {
            double currentValue = fadc_data[0][clk][ch]; 

	    /*
            if (currentValue < fadc_c_min[ch] && currentValue != 0) {
                fadc_c_min[ch] = currentValue;
                minIndex = clk;
            }*/
            if (currentValue > fadc_c_max[ch]) {
	      fadc_c_max[ch] = currentValue;
	    }
            if(clk >= 500 && clk < 1000) {
                sum += currentValue;
                baseline_cnt++;
            }
        }

        fadc_c_bl[ch] = (baseline_cnt > 0) ? sum / baseline_cnt : 0;
        //if (minIndex == -1) continue;

        
        int index = std::ceil(static_cast<double>(sca_cnt[2]) / (60.0 * minutes * freq)) - 1;
        if ( index < div && fadc_c_max[ch] - fadc_c_bl[ch] > 0) {
            h[index]->Fill(fadc_c_max[ch] - fadc_c_bl[ch]);
        }

	cout << "\r" << event+1 << "/" << nEntries << " [" << event*100/nEntries << "%]    ";
    }
    cout << endl;
    for (int i = 0; i < div; i++){
      int bin_max = h[i]->GetMaximumBin();
      h[i]->Fit("gaus","","",60,250);
      TF1* fit = h[i]->GetFunction("gaus");
      double mean = fit->GetParameter(1);
      double clk_time = i * minutes * freq + minutes * freq/2; 
      gr->SetPoint(i,clk_time,mean);
    }

    //output file
    TFile* fout= new TFile(Form("./rootfile/MAIKo_%05d_Ch%d.root", runno,ch),"recreate"); 
    
    //vector<TCanvas*> c(div); 
    for (int i = 0; i < div; ++i) {
      h[i]->Write();
      //c[i] = new TCanvas(Form("c%d",i),Form("c%d",i),500,500);
      //   h[i]->Draw();
    }
    gr->SetMarkerStyle(20);
    gr->SetMarkerColor(2);
    gr->GetXaxis()->SetTitle("clk");
    gr->GetYaxis()->SetTitle("Pulse Hight");
    gr->Write();
    //fout->Close();
    return;
}
