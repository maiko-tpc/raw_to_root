{
  const int time_win = 1800*100;  // 100 Hz clock
  double fit_min = 1.3;
  double fit_max = 2.0;
  
  TFile *infile = new TFile("driftv/driftv_1419.root"); // with flow
  //  TFile *infile = new TFile("driftv/driftv_1431.root");  // w/o flow
  TTree* tree2 = (TTree*)infile->Get("tree");

  int a = tree2->GetEntries();
  cout << a << endl;
  
  int i=0;
  int cnt=0;
  int cnt2;
  
  TF1 *func = new TF1("func", "gaus");
  double mean, sigma, integ;

  vector<float> time_cen;
  vector<float> time_err;  
  vector<float> v_ave;
  vector<float> v_err;    

  int bin1, bin2;
  
  while(1){
    cnt = tree2->Draw("driftv*100>>h1(100,0,3)",
    		     Form("time>%d && time<%d", time_win*i, time_win*(i+1)));
    cnt2 = tree2->GetEntries(Form("time>%d && time<%d && driftv*100>%f && driftv*100<%f",
				  time_win*i, time_win*(i+1), fit_min, fit_max));

    h1->Fit("func","","", fit_min, fit_max);
    mean = func->GetParameter(1);
    sigma = func->GetParameter(2);    

    integ = cnt2;
    
    time_cen.push_back(time_win*(i+0.5)/100.0);
    time_err.push_back(0);
    v_ave.push_back(mean);
    v_err.push_back(sigma/sqrt(integ));
    
    printf("index=%d, cnt=%d\n", i, cnt);
    if(cnt<10) break;
    i++;
  }

  TGraphErrors *gr = new TGraphErrors(i, &time_cen[0], &v_ave[0],
				      &time_err[0], &v_err[0]);
  gr->SetMarkerStyle(20);
  gr->GetHistogram()->SetMinimum(0.0);
  gr->GetHistogram()->SetMaximum(3.0);

  gr->SetTitle("");
  gr->GetXaxis()->SetTitle("Time (sec)");
  gr->GetYaxis()->SetTitle("Drift Speed (cm/us)");  
  
  gr->Draw("AP");
}
