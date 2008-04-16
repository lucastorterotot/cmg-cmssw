#include "RecoLuminosity/ROOTSchema/interface/ROOTFileBase.h"

std::string ROOTFileBase::TimeStampLong(){

  time_t rawtime;
  time(&rawtime);

  return ctime(&rawtime);
}

std::string ROOTFileBase::TimeStampShort(){

  time_t rawtime;
  struct tm* timeinfo;

  rawtime = time(NULL);
  timeinfo = localtime(&rawtime);

  std::ostringstream out;
  out.str(std::string());
  out << std::setfill('0') << std::setw(4) << timeinfo->tm_year + 1900;
  out << std::setfill('0') << std::setw(2) << timeinfo->tm_mon + 1;
  out << std::setfill('0') << std::setw(2) << timeinfo->tm_mday;

  return out.str();
}

ROOTFileBase::ROOTFileBase(){
#ifdef DEBUG
  std::cout << "Begin " << __PRETTY_FUNCTION__ << std::endl;
#endif

  Header  = new HCAL_HLX::LUMI_SECTION_HEADER;
  Summary = new HCAL_HLX::LUMI_SUMMARY;
  Detail  = new HCAL_HLX::LUMI_DETAIL;

  EtSum     = new HCAL_HLX::ET_SUM_SECTION[HCAL_HLX_MAX_HLXS];
  Occupancy = new HCAL_HLX::OCCUPANCY_SECTION[HCAL_HLX_MAX_HLXS];
  LHC       = new HCAL_HLX::LHC_SECTION[HCAL_HLX_MAX_HLXS];

  Threshold       = new HCAL_HLX::LUMI_THRESHOLD;
  L1Trigger       = new HCAL_HLX::LEVEL1_TRIGGER;
  HLT             = new HCAL_HLX::HLT;
  TriggerDeadtime = new HCAL_HLX::TRIGGER_DEADTIME;
  RingSet         = new HCAL_HLX::LUMI_HF_RING_SET;

  outputDir = ".";

  m_file = NULL;

#ifdef DEBUG
  std::cout << "End " << __PRETTY_FUNCTION__ << std::endl;
#endif
}

ROOTFileBase::~ROOTFileBase(){

#ifdef DEBUG
  std::cout << "Begin " << __PRETTY_FUNCTION__ << std::endl;
#endif

  delete Header;
  delete Summary;
  delete Detail;

  delete [] EtSum;
  delete [] Occupancy;
  delete [] LHC;

  delete Threshold;
  delete L1Trigger;
  delete HLT;
  delete TriggerDeadtime; 
  delete RingSet;

#ifdef DEBUG
  std::cout << "End " << __PRETTY_FUNCTION__ << std::endl;
#endif
}

void ROOTFileBase::CreateFileName(const HCAL_HLX::LUMI_SECTION &localSection){
#ifdef DEBUG
  std::cout << "Begin " << __PRETTY_FUNCTION__ << std::endl;
#endif

  std::ostringstream outputString;

  // Create directory name
  outputString << outputDir << "/" 
	       << std::setfill('0') << std::setw(9) << localSection.hdr.runNumber;
  
  // Create the directory that will contain the single lumi section file
  mkdir(outputString.str().c_str(), 0777);
  
  // Create file name
  outputString << "/CMS_LUMI_RAW_"
	       << TimeStampShort() << "_"
	       << std::setfill('0') << std::setw(9) << localSection.hdr.runNumber << "_"
	       << localSection.hdr.bCMSLive << "_"
	       << std::setfill('0') << std::setw(4) << localSection.hdr.sectionNumber 
	       << ".root";
  fileName = outputString.str();
  
#ifdef DEBUG
  std::cout << "Output file is " << fileName << std::endl;
  std::cout << "End " << __PRETTY_FUNCTION__ << std::endl;
#endif
}

void ROOTFileBase::CreateTree(const HCAL_HLX::LUMI_SECTION & localSection){
#ifdef DEBUG
  std::cout << "Begin " << __PRETTY_FUNCTION__ << std::endl;
#endif

  m_file = new TFile(fileName.c_str(), "UPDATE");  

  if(!m_file){
    std::cout << " *** Couldn't make or open file: " << fileName << " *** " << std::endl;
    exit(1);
  } 

  m_file->cd();
  
  m_tree  = new TTree("LumiTree","");

  m_tree->Bronch("Header.",  "HCAL_HLX::LUMI_SECTION_HEADER", &Header,  1);
  m_tree->Bronch("Summary.", "HCAL_HLX::LUMI_SUMMARY",        &Summary, 1);
  m_tree->Bronch("Detail.",  "HCAL_HLX::LUMI_DETAIL",         &Detail,  1);

  m_tree->Bronch("Threshold.",        "HCAL_HLX::LUMI_THRESHOLD",   &Threshold, 1);
  m_tree->Bronch("Level1_Trigger.",   "HCAL_HLX::LEVEL1_TRIGGER",   &L1Trigger, 1);
  m_tree->Bronch("HLT.",              "HCAL_HLX::HLT",              &HLT,       1);
  m_tree->Bronch("Trigger_Deadtime.", "HCAL_HLX::TRIGGER_DEADTIME", &TriggerDeadtime, 1);
  m_tree->Bronch("HF_Ring_Set.",      "HCAL_HLX::LUMI_HF_RING_SET", &RingSet,1);

  for(int i = 0; i < HCAL_HLX_MAX_HLXS; i++){
    EtSumPtr[i] = &EtSum[i];
    MakeBranch(localSection.etSum[i], &EtSumPtr[i], i);

    OccupancyPtr[i] = &Occupancy[i];
    MakeBranch(localSection.occupancy[i], &OccupancyPtr[i], i);

    LHCPtr[i] = &LHC[i];
    MakeBranch(localSection.lhc[i], &LHCPtr[i], i);
  }

#ifdef DEBUG
  std::cout << "End " << __PRETTY_FUNCTION__ << std::endl;
#endif
}

void ROOTFileBase::FillTree(const HCAL_HLX::LUMI_SECTION& localSection){
  
#ifdef DEBUG
  std::cout << "Begin " << __PRETTY_FUNCTION__ << std::endl;
#endif
  
  for(int i = 0; i < HCAL_HLX_MAX_HLXS; i++){
    memcpy(&EtSum[i],     &localSection.etSum[i],     sizeof(HCAL_HLX::ET_SUM_SECTION));
    memcpy(&Occupancy[i], &localSection.occupancy[i], sizeof(HCAL_HLX::OCCUPANCY_SECTION));
    memcpy(&LHC[i],       &localSection.lhc[i],       sizeof(HCAL_HLX::LHC_SECTION));
  }

  memcpy(Header,  &localSection.hdr,         sizeof (localSection.hdr));
  memcpy(Summary, &localSection.lumiSummary, sizeof(HCAL_HLX::LUMI_SUMMARY));
  memcpy(Detail,  &localSection.lumiDetail,  sizeof(HCAL_HLX::LUMI_DETAIL));

  InsertInformation();

  m_tree->Fill();

#ifdef DEBUG
  //  m_tree->Print();
  std::cout << "End " << __PRETTY_FUNCTION__ << std::endl;
#endif
}

template< class T >
void ROOTFileBase::MakeBranch(const T &in, T **out, int HLXNum){

#ifdef DEBUG
  std::cout << "Begin " << __PRETTY_FUNCTION__ << std::endl;
#endif
  
  const std::string typeName = typeid(T).name();
  std::string className;
  std::string branchName;
  std::ostringstream numString;

  if(typeName == "N8HCAL_HLX11LHC_SECTIONE"){
    className = "HCAL_HLX::LHC_SECTION";
    branchName = "LHC";
  }else if(typeName == "N8HCAL_HLX17OCCUPANCY_SECTIONE"){
    className = "HCAL_HLX::OCCUPANCY_SECTION";
    branchName = "Occupancy";
  }else if(typeName == "N8HCAL_HLX14ET_SUM_SECTIONE"){
    className = "HCAL_HLX::ET_SUM_SECTION";
    branchName = "ETSum";
  }

#ifdef DEBUG
  std::cout << "Class: " << className << std::endl;
  std::cout << "Class: " << typeid(T).name() << std::endl;
#endif
  
  numString << std::setfill('0') << std::setw(2) << HLXNum;
  branchName = branchName + numString.str() + ".";
  m_tree->Bronch(branchName.c_str(), className.c_str(), out, 1);

#ifdef DEBUG
  std::cout << "End " << __PRETTY_FUNCTION__ << std::endl;
#endif
}

void ROOTFileBase::InsertInformation(){
  // This information will eventually come from the lms cell
  Threshold->OccThreshold1Set1 = 51;
  Threshold->OccThreshold2Set1 = 52;
  Threshold->OccThreshold1Set2 = 53;
  Threshold->OccThreshold2Set2 = 54;
  Threshold->ETSum             = 55;

  L1Trigger->L1lineNumber  = 71;
  L1Trigger->L1Scaler      = 72;
  L1Trigger->L1RateCounter = 73;
  
  HLT->TriggerPath    = 81;
  HLT->InputCount     = 82;
  HLT->AcceptCount    = 83;
  HLT->PrescaleFactor = 84;

  TriggerDeadtime->TriggerDeadtime = 91;

  RingSet->Set1Rings = "33L,34L";
  RingSet->Set2Rings = "35S,36S";
  RingSet->EtSumRings = "33L,34L,35S,36S";
}

void ROOTFileBase::CloseTree(){
#ifdef DEBUG
  std::cout << "Begin " << __PRETTY_FUNCTION__ << std::endl;
#endif

  m_file->Write();
  m_file->Close();

  //delete m_tree; // NO!!! root does this when you delete m_file
  if(m_file != NULL){
    delete m_file;
    m_file = NULL;
    m_tree = NULL;
  }

#ifdef DEBUG
  std::cout << "End " << __PRETTY_FUNCTION__ << std::endl;
#endif
}

void ROOTFileBase::Concatenate(const HCAL_HLX::LUMI_SECTION& localSection){
#ifdef DEBUG
  std::cout << "Begin " << __PRETTY_FUNCTION__ << std::endl;
#endif

  std::ostringstream outputString;
  std::string outFileName;
  std::string tempString;
  std::vector<std::string> FileList;

  outputString << outputDir
	       << "/CMS_LUMI_RAW_"
	       << TimeStampShort()
	       << "_"
	       << std::setfill('0') << std::setw(9) << localSection.hdr.runNumber
	       << "_"
	       << localSection.hdr.bCMSLive
	       << ".root";
  outFileName = outputString.str();
  
  outputString.str(std::string());
  outputString << outputDir << "/" << std::setfill('0') << std::setw(9) << localSection.hdr.runNumber;  
  outputString << "/CMS_LUMI_RAW_"
	       << TimeStampShort() << "_"
	       << std::setfill('0') << std::setw(9) << localSection.hdr.runNumber << "_"
	       << localSection.hdr.bCMSLive << "_*.root";

  TChain Temp("LumiTree");
  Temp.Add(outputString.str().c_str());

  //  Temp.Merge(outFileName.c_str()); // causes leaks
  /*
  TFile* OutputFile = new TFile(outFileName.c_str(),"RECREATE");
  TTree* OutputTree = Temp.CloneTree();
  
  OutputFile->Write();
  OutputFile->Close();
  delete OutputFile;
  */
#ifdef DEBUG
  std::cout << "End " << __PRETTY_FUNCTION__ << std::endl;
#endif
}
