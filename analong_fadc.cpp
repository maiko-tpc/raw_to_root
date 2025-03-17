#include <TFile.h>
#include <TTree.h>
#include <TGraphErrors.h>
#include <TF1.h>
#include <TH1F.h>
#include <TString.h>
#include <TROOT.h>
#include <iostream>
#include <vector>
#include <cmath>
using namespace std;

void analong_fadc(int runno) {
    // Set time window length in clock ticks (assuming a 100 Hz clock)
    // For example, time_win = 1800*100 corresponds to 1800 sec per window
    const int time_win = 1800 * 100;  

    // Open the ROOT file produced by the pulse height macro
    TString infileName = Form("fadc/fadc_%05d.root", runno);
    TFile *infile = TFile::Open(infileName, "READ");
    if (!infile || infile->IsZombie()) {
        cout << "Cannot open file: " << infileName.Data() << endl;
        return;
    }
    
    // Get the TTree "pulse_tree" that contains pulse height information,
    // the branch "time" (extracted from sca_cnt[2]) and the vector branch "pulse_height_a"
    TTree *tree = (TTree*)infile->Get("tree");
    if (!tree) {
        cout << "TTree 'tree' not found in file " << infileName.Data() << endl;
        return;
    }
    
    int totalEntries = tree->GetEntries();
    cout << "Total entries: " << totalEntries << endl;
    
    int i = 0;
    int cnt = 0;
    int cnt2 = 0;
    
    // Create a Gaussian function for fitting
    TF1 *func = new TF1("func", "gaus");
    double mean, sigma, integ;
    
    // Vectors to hold the time window centers, their errors,
    // the Gaussian means, and the corresponding errors
    vector<float> time_cen;
    vector<float> time_err;  
    vector<float> mean_vals;
    vector<float> mean_err;    
    
    // Loop over time windows
    while (1) {
        // Fill histogram "h1" with pulse_height_a[11] for events in the current time window.
        // The expression "pulse_height_a[11]" extracts the pulse height of channel 11.
        cnt = tree->Draw("pulse_height_a[23]>>h1(100,0,1024)",
                         Form("time>%d && time<%d", time_win * i, time_win * (i + 1)));
        // Get total number of events in this time window (for error estimation)
        cnt2 = tree->GetEntries(Form("time>%d && time<%d", time_win * i, time_win * (i + 1)));
        
        // If not enough entries in the window, exit the loop
        if (cnt < 10) break;
        
        // Retrieve the created histogram
        TH1F *h1 = (TH1F*)gROOT->FindObject("h1");
        // Fit the histogram with a Gaussian function.
        // Adjust the fit range (here 0 to 1024) if needed based on your pulse height distribution.
        h1->Fit("func", "", "", 55, 100);
        mean = func->GetParameter(1);
        sigma = func->GetParameter(2);
        
        integ = cnt2;  // use the number of events in the window for error estimation
        
        // Compute the center of the time window in seconds
        time_cen.push_back(time_win * (i + 0.5) / 100.0);
        time_err.push_back(0);
        mean_vals.push_back(mean);
        // Estimate the error on the mean as sigma/sqrt(N)
        mean_err.push_back(sigma / sqrt(integ));
        
        printf("Index = %d, entries = %d\n", i, cnt);
        i++;
    }
    
    // Create a TGraphErrors to display the time evolution of the Gaussian fit mean
    TGraphErrors *gr = new TGraphErrors(i, &time_cen[0], &mean_vals[0],
                                        &time_err[0], &mean_err[0]);
    gr->SetMarkerStyle(20);
    gr->GetHistogram()->SetMinimum(0.0);
    gr->GetHistogram()->SetMaximum(1024);
    
    gr->SetTitle("");
    gr->GetXaxis()->SetTitle("Time (sec)");
    gr->GetYaxis()->SetTitle("fadc_a[11] Pulse Height Mean");
    gr->Draw("AP");
}
