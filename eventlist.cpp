int eventlist(int runno) {

  TFile* ifile;
  TTree* tree1;

  int evt_cnt;

  // anode和cathode的数据数组
  unsigned int hit_data_a[24][2048] = {0}; // anode数据
  unsigned int hit_data_c[24][2048] = {0}; // cathode数据

  // 打开ROOT文件和树
  ifile = new TFile(Form("./rootfile/MAIKo_%05d.root", runno), "READ");
  tree1 = (TTree*)ifile->Get("tree");

  ofstream ofs(Form("./run%05d.list", runno));

  // 设置分支地址
  tree1->SetBranchAddress("evt_cnt", &evt_cnt);
  tree1->SetBranchAddress("hit_data_a", hit_data_a);
  tree1->SetBranchAddress("hit_data_c", hit_data_c);

  int nEntries = tree1->GetEntries();
  for (int ii = 0; ii < nEntries; ii++) {
    tree1->GetEntry(ii);
    if(ii%1000==0){
      cout<<"prossed data:"<<100*(ii+1)/nEntries<<" %!           \r"<<flush;
    }

    // 统计cathode的总像素数
    int cathode_pixel_count = 0;
    // 用于标记cathode中每个strip是否hit（用于beam轴区域判断）
    bool cathode_strip_hit[768] = { false };

    // 遍历cathode数据：ch, clk, bit
    for (int ch = 0; ch < 24; ch++) {
      for (int clk = 0; clk < 2048; clk++) {
        unsigned int word = hit_data_c[ch][clk];
        for (int bit = 0; bit < 32; bit++) {
          if (word & ((unsigned int)1 << bit)) {
            cathode_pixel_count++;  // 累计像素hit数
            // 使用给定的转换公式得到strip号
            int ch2 = ((ch / 4 + 1) * 4) - (ch % 4) - 1;
            int strip = ch2 * 32 + bit;
            // 如果strip处于300～500区域，则标记hit
            if (strip >= 300 && strip <= 500) {
              cathode_strip_hit[strip] = true;
            }
          }
        }
      }
    }

    // 统计beam轴区域（300～500）中hit的唯一strip数
    int cathode_region_hit = 0;
    for (int strip = 300; strip <= 500; strip++) {
      if (cathode_strip_hit[strip]) {
        cathode_region_hit++;
      }
    }

    // 处理anode数据，统计总像素hit数
    int anode_pixel = 0;
    for (int ch = 0; ch < 24; ch++) {
      for (int clk = 0; clk < 2048; clk++) {
        unsigned int word = hit_data_a[ch][clk];
        for (int bit = 0; bit < 32; bit++) {
          if (word & ((unsigned int)1 << bit)) {
            anode_pixel++;
          }
        }
      }
    }
    
    // 按条件挑选事件：
    //  - cathode的beam轴区域（300～500）中唯一hit strip数 ≥ 30
    //  - cathode图像中的像素hit数 > 300
    //  - anode图像中的像素hit数 > 300
    if (cathode_region_hit >= 50 && cathode_pixel_count > 5000 && anode_pixel > 5000) {
      ofs << evt_cnt << endl;
    }
  }
  
  return 0;
}




    

