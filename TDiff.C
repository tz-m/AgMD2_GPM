void TDiff(std::string filename)
{
  TTree *tree = new TTree("tdiff","tdiff");
  tree->ReadFile(filename.c_str(),"tdiff");

  TF1 * exposum = new TF1("exposum","exp([0]-1e-9*x/[1])+exp([2]-1e-9*x/[3])",0,20e9);
  exposum->SetParameters(6,0.178,4,0.5);
  exposum->SetParLimits(0,1,10);
  exposum->SetParLimits(1,0,10);
  exposum->SetParLimits(2,1,10);
  exposum->SetParLimits(3,0,10);


  TCanvas * c = new TCanvas("tdiff","tdiff",1600,900);
  tree->Draw("tdiff>>td(600,0,20e9)");
  c->SetLogy();
  TH1F * td = (TH1F*)gDirectory->Get("td");
  td->Fit("exposum","R");
  td->GetXaxis()->SetTitle("deltaT [ns]");
  gStyle->SetOptStat(11);
  gStyle->SetOptFit(1);
}
