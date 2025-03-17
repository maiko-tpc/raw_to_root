#include <TFile.h>
#include <TTree.h>
#include <TString.h>
#include <vector>
#include <iostream>
#include <limits>
using namespace std;

void fadc_pulse(int runno) {
    // Open the original ROOT file
    TString infileName = Form("./rootfile/MAIKo_%05d.root", runno);
    TFile *infile = TFile::Open(infileName, "READ");
    if (!infile || infile->IsZombie()) {
        cout << "Cannot open file: " << infileName.Data() << endl;
        return;
    }
    
    // Retrieve the TTree from the file
    TTree *tree = (TTree*)infile->Get("tree");
    if (!tree) {
        cout << "TTree 'tree' not found in file " << infileName.Data() << endl;
        return;
    }
    
    // Declare variables to read data from the original TTree
    // fadc_data: [2][1025][24] array; channel 0 for anode (fadc_a) and 1 for cathode (fadc_c)
    unsigned int fadc_data[2][1025][24] = {0};
    int evt_cnt = 0;
    // Assume sca_cnt has at least 3 elements (allocated 10 elements here), where the third element (index 2) is used as time.
    int sca_cnt[12] = {0};
    
    // Set branch addresses
    tree->SetBranchAddress("fadc_data", fadc_data);
    tree->SetBranchAddress("evt_cnt", &evt_cnt);
    tree->SetBranchAddress("sca_cnt", sca_cnt);
    
    // Create an output ROOT file and TTree
    TString outfileName = Form("fadc/fadc_%05d.root", runno);
    TFile *outfile = new TFile(outfileName, "RECREATE");
    TTree *outtree = new TTree("tree", "Pulse Heights");
    
    // Variables to store output results
    vector<double> pulse_height_a; // Pulse heights for anode channels (24 channels)
    vector<double> pulse_height_c; // Pulse heights for cathode channels (24 channels)
    int out_evt_cnt;
    double out_time;  // Time extracted from sca_cnt[2]
    
    // Set branches for the output tree
    outtree->Branch("evt_cnt", &out_evt_cnt, "evt_cnt/I");
    outtree->Branch("time", &out_time, "time/D");
    outtree->Branch("pulse_height_a", &pulse_height_a);
    outtree->Branch("pulse_height_c", &pulse_height_c);
    
    Long64_t nentries = tree->GetEntries();
    cout << "Total events: " << nentries << endl;
    
    // Loop over each event
    for (Long64_t i = 0; i < nentries; i++) {
        tree->GetEntry(i);
        out_evt_cnt = evt_cnt;
        out_time = sca_cnt[2];  // Extract the third element as time
        
        // Clear and resize vectors for the current event
        pulse_height_a.clear();
        pulse_height_c.clear();
        pulse_height_a.resize(24, 0);
        pulse_height_c.resize(24, 0);
        
        // Process anode data (fadc_a): pulse height = global maximum - baseline
        for (int ch = 0; ch < 24; ch++) {
            // Calculate baseline as the average from clock tick 500 to 999
            double baseline_a = 0;
            int count_a = 0;
            for (int clk = 500; clk < 1000; clk++) {
                baseline_a += fadc_data[0][clk][ch];
                count_a++;
            }
            if (count_a > 0) baseline_a /= count_a;
            
            // Find the global maximum over clock ticks 0 to 1024
            double max_a = -std::numeric_limits<double>::infinity();
            for (int clk = 0; clk < 1025; clk++) {
                double value = fadc_data[0][clk][ch];
                if (value > max_a) max_a = value;
            }
            
            pulse_height_a[ch] = max_a - baseline_a;
        }
        
        // Process cathode data (fadc_c): pulse height = global minimum - baseline
        for (int ch = 0; ch < 24; ch++) {
            // Calculate baseline as the average from clock tick 500 to 999
            double baseline_c = 0;
            int count_c = 0;
            for (int clk = 500; clk < 1000; clk++) {
                baseline_c += fadc_data[1][clk][ch];
                count_c++;
            }
            if (count_c > 0) baseline_c /= count_c;
            
            // Find the global minimum over clock ticks 0 to 1024, ignoring zero values if desired
            double min_c = std::numeric_limits<double>::infinity();
            for (int clk = 0; clk < 1025; clk++) {
                double value = fadc_data[1][clk][ch];
                if (value < min_c && value != 0)
                    min_c = value;
            }
            
            pulse_height_c[ch] = min_c - baseline_c;
        }
        
        // Fill the output tree for the current event
        outtree->Fill();
    }
    
    // Write the output tree to the file and close all files
    outfile->cd();
    outtree->Write();
    outfile->Close();
    infile->Close();
    
    cout << "Processing complete. Results saved in " << outfileName.Data() << endl;
}
