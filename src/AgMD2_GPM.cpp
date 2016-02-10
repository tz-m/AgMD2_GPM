#include "../include/AgMD2_GPM.h"

///
/// Based on Keysight IVI-C Driver Example Program
///
/// Initializes the driver, reads a few Identity interface properties, and performs a
/// multi-record acquisition.
///
/// See driver help topic "Programming the IVI-C Driver in Various Development Environments"
/// for additional programming information.
///
/// Runs in simulation mode without an instrument.
///

void AgMD2_GPM::Quit()
{
  ViSession session = GetSession();

  // Close the driver.
  AgMD2_close(session);
  printf("Driver closed \n");

  gApplication->Terminate(0);
}

// Utility function to check status error during driver API call.
void AgMD2_GPM::checkApiCall(ViStatus status, char const* functionName)
{
  ViInt32 ErrorCode;
  ViChar ErrorMessage[256];

  if (status > 0) // Warning occurred.
    {
      AgMD2_GetError(VI_NULL, &ErrorCode, sizeof(ErrorMessage), ErrorMessage);
      printf("** Warning during %s: 0x%08x, %s\n", functionName, ErrorCode, ErrorMessage);
    }
  else if (status < 0) // Error occurred.
    {
      
      AgMD2_GetError(VI_NULL, &ErrorCode, sizeof(ErrorMessage), ErrorMessage);
      printf("** Error during %s: 0x%08x, %s\n", functionName, ErrorCode, ErrorMessage);
      Quit();
      throw std::exception();
    }
}

void AgMD2_GPM::initialize_driver()
{
  ViSession session = 0;

  ViBoolean idQuery = VI_TRUE;
  ViBoolean reset = VI_TRUE;

  // Edit resource and options as needed. Resource is ignored if option Simulate=true.
  ViRsrc resource = (ViRsrc)rp.GetResourceName().c_str();

  std::cout << "Resource " << resource << std::endl;

  // An input signal is necessary if the example is run in non simulated mode, otherwise
  // the acquisition will time out.
  ViConstString options = rp.GetOptChar();

  std::cout << "Option string " << options << std::endl;

  // Initialize the driver. See driver help topic "Initializing the IVI-C Driver" for additional information.
  checkApiCall(AgMD2_InitWithOptions(resource, idQuery, reset, options, &session), "AgMD2_InitWithOptions");
  SetSession(session);
  printf("Driver initialized with session %u\n",session);

  
  ViChar str[128] = { '\0' };
  // Read and output a few attributes.
  checkApiCall(AgMD2_GetAttributeViString(session, "", AGMD2_ATTR_SPECIFIC_DRIVER_PREFIX, sizeof(str), str), "AgMD2_GetAttributeViString(AGMD2_ATTR_SPECIFIC_DRIVER_PREFIX)");
  printf("Driver prefix:      %s\n", str);
  checkApiCall(AgMD2_GetAttributeViString(session, "", AGMD2_ATTR_SPECIFIC_DRIVER_REVISION, sizeof(str), str), "AgMD2_GetAttributeViString(AGMD2_ATTR_SPECIFIC_DRIVER_REVISION)");
  printf("Driver revision:    %s\n", str);
  checkApiCall(AgMD2_GetAttributeViString(session, "", AGMD2_ATTR_SPECIFIC_DRIVER_VENDOR, sizeof(str), str), "AgMD2_GetAttributeViString(AGMD2_ATTR_SPECIFIC_DRIVER_VENDOR)");
  printf("Driver vendor:      %s\n", str);
  checkApiCall(AgMD2_GetAttributeViString(session, "", AGMD2_ATTR_SPECIFIC_DRIVER_DESCRIPTION, sizeof(str), str), "AgMD2_GetAttributeViString(AGMD2_ATTR_SPECIFIC_DRIVER_DESCRIPTION)");
  printf("Driver description: %s\n", str);
  checkApiCall(AgMD2_GetAttributeViString(session, "", AGMD2_ATTR_INSTRUMENT_MODEL, sizeof(str), str), "AgMD2_GetAttributeViString(AGMD2_ATTR_INSTRUMENT_MODEL)");
  printf("Instrument model:   %s\n", str);
  checkApiCall(AgMD2_GetAttributeViString(session, "", AGMD2_ATTR_INSTRUMENT_FIRMWARE_REVISION, sizeof(str), str), "AgMD2_GetAttributeViString(AGMD2_ATTR_INSTRUMENT_FIRMWARE_REVISION)");
  printf("Firmware revision:  %s\n", str);
  checkApiCall(AgMD2_GetAttributeViString(session, "", AGMD2_ATTR_INSTRUMENT_INFO_SERIAL_NUMBER_STRING, sizeof(str), str), "AgMD2_GetAttributeViString(AGMD2_ATTR_INSTRUMENT_INFO_SERIAL_NUMBER_STRING)");
  printf("Serial number:      %s\n", str);
  checkApiCall(AgMD2_GetAttributeViString(session, "", AGMD2_ATTR_INSTRUMENT_INFO_OPTIONS, sizeof(str), str), "AgMD2_GetAttributeViString(AGMD2_ATTR_INSTRUMENT_INFO_OPTIONS)");
  printf("Instrument options: %s\n", str);
  ViInt32 channelcount;
  checkApiCall(AgMD2_GetAttributeViInt32(session, "", AGMD2_ATTR_CHANNEL_COUNT, &channelcount), "AgMD2_GetAttributeViInt32(AGMD2_ATTR_CHANNEL_COUNT)");
  printf("Channel count:      %i\n", channelcount);
  ViInt32 bits;
  checkApiCall(AgMD2_GetAttributeViInt32(session, "", AGMD2_ATTR_INSTRUMENT_INFO_NBR_ADC_BITS, &bits), "AgMD2_GetAttributeViInt32(AGMD2_ATTR_INSTRUMENT_INFO_NBR_ADC_BITS)");
  printf("ADC bits:           %i\n", bits);
  printf("Session:            %u\n\n", session);
}

void AgMD2_GPM::configure_channels()
{
  ViSession session = GetSession();
  ViInt32 coupling = AGMD2_VAL_VERTICAL_COUPLING_DC;//same for all channels

  // Configure the acquisition.

  auto itr = cpm.begin();
  while (itr != cpm.end())
    {
      ViUInt8 num = itr->first;
      ChannelParams cp = itr->second;
      std::cout << "ChannelParams.Complete() = " << cp.Complete() << std::endl;
      if (!cp.GetUseChannel())
	{
	  itr++;
	  continue;
	}
      printf("Configuring acquisition -- Channel%i\n",num);
      printf("Range:           %g\n", cp.GetChannelRange());
      printf("Offset:          %g\n", cp.GetChannelOffset());
      printf("Coupling:        %s\n", coupling ? "DC" : "AC");
      checkApiCall(AgMD2_ConfigureChannel(session, 
					  cp.GetChannelName(), 
					  cp.GetChannelRange(),
					  cp.GetChannelOffset(),
					  coupling,
					  cp.GetUseChannel()),
		   "AgMD2_ConfigureChannel");
      itr++;
    }
}

void AgMD2_GPM::configure_acquisition()
{
  ViSession session = GetSession();
  printf("\nNumber of records:  %li\n", rp.GetNumRecords());
  printf("Record size:        %li\n", rp.GetRecordSize());
  printf("Sample rate:        %g\n", rp.GetSampleRate());
  checkApiCall(AgMD2_SetAttributeViInt64(session, "", AGMD2_ATTR_NUM_RECORDS_TO_ACQUIRE, 1), "AgMD2_SetAttributeViInt64(AGMD2_ATTR_NUM_RECORDS_TO_ACQUIRE)");
  checkApiCall(AgMD2_SetAttributeViInt64(session, "", AGMD2_ATTR_RECORD_SIZE, rp.GetRecordSize()), "AgMD2_SetAttributeViInt64(AGMD2_ATTR_RECORD_SIZE)");
  checkApiCall(AgMD2_SetAttributeViReal64(session, "", AGMD2_ATTR_SAMPLE_RATE, rp.GetSampleRate()), "AgMD2_SetAttributeViReal64(AGMD2_ATTR_SAMPLE_RATE)");

  // NOTE: The ConfigureAcquisition method (below) doesn't work for some reason, so set the values manually
  //checkApiCall(AgMD2_ConfigureAcquisition(session, 1, rp.GetRecordSize(), rp.GetSampleRate()), "AgMD2_ConfigureAcquisition");
}

void AgMD2_GPM::configure_triggers()
{
  // Configure the trigger.
  ViSession session = GetSession();

  ViReal64 triggerdelay = rp.GetTriggerDelay()*rp.GetRecordSize()/rp.GetSampleRate();

  auto itr = cpm.begin();
  while (itr != cpm.end())
    {
      ViUInt8 num = itr->first;
      ChannelParams cp = itr->second;

      if (!cp.GetActiveTrigger())
	{
	  itr++;
	  continue;
	}

      bool activeTrigger = cp.GetActiveTrigger();
      printf("Configuring trigger %i -- %s\n",num,cp.GetChannelName());
      printf("Source:          %s\n", cp.GetTriggerSource());
      printf("Level:           %g\n", cp.GetTriggerLevel());
      printf("Slope:           %i\n", cp.GetTriggerSlope());
      printf("Delay:           %g\n", triggerdelay);
      printf("Active Trigger:  %s\n", (activeTrigger) ? "true" : "false");
      checkApiCall(AgMD2_ConfigureEdgeTriggerSource(session,
						    cp.GetTriggerSource(),
						    cp.GetTriggerLevel(),
						    cp.GetTriggerSlope()),
		   "AgMD2_ConfigureEdgeTriggerSource");
      checkApiCall(AgMD2_SetAttributeViInt32(session,
					     cp.GetTriggerSource(),
					     AGMD2_ATTR_TRIGGER_TYPE,
					     AGMD2_VAL_EDGE_TRIGGER),
		   "AgMD2_SetAttributeViInt32(AGMD2_ATTR_TRIGGER_TYPE)");
      checkApiCall(AgMD2_SetAttributeViInt32(session,
					     cp.GetTriggerSource(),
					     AGMD2_ATTR_TRIGGER_COUPLING,
					     AGMD2_VAL_TRIGGER_COUPLING_DC),
		   "AgMD2_SetAttributeViInt32(AGMD2_ATTR_TRIGGER_COUPLING)");

      checkApiCall(AgMD2_SetAttributeViString(session, 
					      "", 
					      AGMD2_ATTR_ACTIVE_TRIGGER_SOURCE, 
					      cp.GetTriggerSource()), 
		   "AgMD2_SetAttributeViString(AGMD2_ATTR_ACTIVE_TRIGGER_SOURCE)"); 
  
      itr++;
    }
  
  checkApiCall(AgMD2_SetAttributeViReal64(session, "", AGMD2_ATTR_TRIGGER_DELAY, triggerdelay), "AgMD2_SetAttributeViReal64(AGMD2_ATTR_TRIGGER_DELAY)");
}

void AgMD2_GPM::calibrate()
{
  ViSession session = GetSession();
  // Calibrate the instrument.
  printf("Performing self-calibration\n");
  checkApiCall(AgMD2_SelfCalibrate(session), "AgMD2_SelfCalibrate");
}


void AgMD2_GPM::initialize_parameters()
{
  rp.Reset();
  cpm.clear();
  
  std::string line;
  std::ifstream defparams("params.txt");
  std::string partype; // put global, or channel number
  std::string parname;
  std::string parval;
  if (defparams.is_open())
    while (std::getline(defparams,line))
      {
	std::stringstream ss(line);
	ss >> partype;
	
	if (partype[0] == '#') continue;

	ss >> parname >> parval;

	if (partype == "global") 
	  {
	    if (parname == "Draw")
	      {
		if (parval == "true")
		  {
		    rp.SetDraw(true);
		  }
		else if (parval == "false")
		  {
		    rp.SetDraw(false);
		  }
	      }
	    if (parname == "ResourceName")
	    {
	      rp.SetResourceName(parval);
	    }
	    else if (parname == "NumRecords") 
	      {
		rp.SetNumRecords(std::stoll(parval));
	      }
	    else if (parname == "RecordSize") 
	      {
		rp.SetRecordSize(std::stoll(parval));
	      }
	    else if (parname == "SampleRate") 
	      {
		rp.SetSampleRate(std::stold(parval));
	      }
	    else if (parname == "TimeoutInMS") 
	      {
		rp.SetTimeoutInMS(std::stol(parval));
	      }
	    else if (parname == "TriggerDelay") 
	      {
		rp.SetTriggerDelay(std::stod(parval));
	      }
	    else if (parname == "EdgeDetectThreshold") 
	      {
		rp.SetEdgeThreshold(std::stoll(parval));
	      }
	    else if (parname == "BaselineCalcFraction") 
	      {
		rp.SetBaselineFraction(std::stod(parval));
	      }
	    else if (parname == "TriggerSigmaThreshold") 
	      {
		rp.SetTriggerSigmaThreshold(std::stod(parval));
	      }
	  }
	else 
	  {
	    ViUInt8 num = std::stoi(partype);

	    if (num >= 1 && num <= 8)
	      {
		if (cpm.count(num) == 0)
		  {
		    cpm.emplace(num,ChannelParams(num));
		  }
		ChannelParams *cp = &cpm.at(num);

		if (parname == "UseChannel")
		  {
		    if (parval == "true") cp->UpdateUseChannel(true);
		    else if (parval == "false") cp->UpdateUseChannel(false);
		  }
		else if (parname == "ChannelNickname") 
		  {
		    cp->UpdateChannelNickname((ViString)parval.c_str());
		  }
		else if (parname == "ChannelPolarity") 
		  {
		    cp->UpdateChannelPolarity(std::stoi(parval));
		  }
		else if (parname == "ChannelRange") 
		  {
		    cp->UpdateChannelRange(std::stod(parval));
		  }
		else if (parname == "ChannelOffset") 
		  {
		    cp->UpdateChannelOffset(std::stod(parval));
		  }
		else if (parname == "TriggerLevel") 
		  {
		    cp->UpdateTriggerLevel(std::stod(parval));
		  }
		else if (parname == "TriggerSlope") 
		  {
		    cp->UpdateTriggerSlope(std::stoi(parval));
		  }
		else if (parname == "ActiveTrigger")
		  {
		    if (parval == "true") cp->UpdateActiveTrigger(true);
		    else if (parval == "false") cp->UpdateActiveTrigger(false);
		  }
	      }
	  }
      }
}

volatile sig_atomic_t sig_caught = 0;

void handler(int)
{
  sig_caught = 1;
}

int AgMD2_GPM::app()
{
  initialize_parameters();
  bool complete = true;
  if (!rp.Complete()) {
    rp.Print();
    complete = false;
  }
  auto itr = cpm.begin();
  while (itr != cpm.end())
    {
      if (!itr->second.GetUseChannel())
	{
	  itr++;
	  continue;
	}
      if (!itr->second.Complete())
	{
	  printf("Channel%i:\n",itr->first);
	  itr->second.Print();
	  complete = false;
	}
      itr++;
    }
  if (!complete) return -1;

  printf("  AgMD2_GPM\n\n");

  initialize_driver();

  configure_channels();

  configure_acquisition();

  configure_triggers();

  calibrate();

  struct sigaction action;
  memset(&action, 0, sizeof(struct sigaction));
  action.sa_handler = handler;
  sigaction(SIGINT, &action, NULL);

  ViSession session = GetSession();

  Header head;
  std::cout << "Header size is " << sizeof(Header) << " bytes long." << std::endl;

  time_t now = time(0);
  struct tm tstruct;
  char filename[30];
  tstruct = *localtime(&now);
  strftime(filename,sizeof(filename),"GPM_%Y%m%d_%H%M%S.dat", &tstruct);
  
  std::ofstream fileout(filename,std::ios::out|std::ios::binary);
  if (!fileout.is_open())
    {
      std::cerr << "error: open file for output failed!" << std::endl;
      return -1;
    }
  
  TCanvas* canv;
  std::map<ViUInt8, TH1I*> histMap;
  std::map<ViUInt8, TGaxis*> axMap;

  if (rp.GetDraw())
    {
      canv = new TCanvas("c","DigiView",1400, 800);
      canv->Divide(4,2);

      itr = cpm.begin();
      while (itr != cpm.end())
	{
	  if (!itr->second.GetUseChannel())
	    {
	      itr++;
	      continue;
	    }
	  TString name = "h"; name += itr->first;
	  TString title = itr->second.GetChannelNickname();
	  histMap.emplace(itr->first,new TH1I(name.Data(),title.Data(),rp.GetRecordSize(),0,0));
	  axMap.emplace(itr->first,new TGaxis(1,0,1,1,0,1,510,"+L"));
	  itr++;
	}
    }
  
  bool saturation_flag = false;
  unsigned int success = 0;

  int i_record;
  bool print = false;
  try
    {
      for (i_record = 0; success < rp.GetNumRecords(); ++i_record)
	{
	  if (i_record % 100 == 0)
	    {
	      print = true;
	    }
	  else
	    {
	      print = false;
	    }

	  if (sig_caught) break;
      
	  head.eventNumber = i_record;

	  if (print)
	    {
	      std::cout << "Event " << i_record << " -- Init" << std::flush;
	    }

	  checkApiCall(AgMD2_InitiateAcquisition(session), "AgMD2_InitiateAcquisition");

	  if (print)
	    {
	      std::cout << ", Waiting" << std::flush;
	    }
	  // this is cheating vvvvvvvvv
	  //checkApiCall(AgMD2_SendSoftwareTrigger(session), "AgMD2_SendSoftwareTrigger");
  
	  checkApiCall(AgMD2_WaitForAcquisitionComplete(session, rp.GetTimeoutInMS()), "AgMD2_WaitForAcquisitionComplete");

	  if (print)
	    {
	      std::cout << ", Acquiring" << std::flush;
	    }

	  checkApiCall(AgMD2_QueryMinWaveformMemory(session, 8, 1, 0, rp.GetRecordSize(), &head.memsize), "AgMD2_QueryMinWaveformMemory");
      
	  if (print)
	    {
	  std::cout << ", Writing " << std::flush;
	    }
	  
	  saturation_flag = false;

	  itr = cpm.begin();

	  while (itr != cpm.end())
	    {
	      if (!itr->second.GetUseChannel())
		{
		  itr++;
		  continue;
		}
	      head.channelNumber = itr->second.GetChannelNumber();
	      if (!rp.GetDraw() && head.channelNumber < 5)
		{
		  ++itr;
		  continue;
		}
	      ViChar* dataArray = new ViChar[head.memsize];
	      checkApiCall(AgMD2_FetchWaveformInt8(session,
						   itr->second.GetChannelName(),
						   head.memsize,
						   dataArray,
						   &head.actualPoints,
						   &head.firstValidPoint,
						   &head.initialXOffset,
						   &head.initialXTimeSeconds,
						   &head.initialXTimeFraction,
						   &head.xIncrement,
						   &head.scaleFactor,
						   &head.scaleOffset),
			   "AgMD2_FetchWaveformInt8");

	      if (rp.GetDraw())
		{
		  histMap[head.channelNumber]->Reset();
		  histMap[head.channelNumber]->GetYaxis()->SetRangeUser(-128,128);
		  histMap[head.channelNumber]->SetBins(head.actualPoints-1,head.initialXOffset,head.initialXOffset+(head.xIncrement * head.actualPoints));
		}
	      for (int i = 0; i < head.actualPoints; i++)
		{
		  int pol = itr->second.GetChannelPolarity();
		  int val = pol*dataArray[i+head.firstValidPoint];
	 
		  if (rp.GetDraw())
		    {
		      histMap[head.channelNumber]->SetBinContent(i,val);
		    }
		  if ((val >= 127 || val <= -127) && (int)head.channelNumber > 4)
		  //if ((val >= 127) && (int)head.channelNumber > 4)
		    //if (false)
		    {
		      saturation_flag = true;
		    }
		}

	      if (saturation_flag)
		{
		  delete[] dataArray;
		  if (print)
		    {
		      std::cout << "X" << std::flush;
		    }
		  break;
		}
	      
	      if (head.channelNumber >= 5)
		{
		  fileout.write((char*)&head,sizeof(Header));
		  fileout.write((char*)dataArray,head.memsize*sizeof(ViChar));
		}
	      delete[] dataArray;
	      if (print)
		{
		  std::cout << (int)head.channelNumber << std::flush;
		}	      
	      itr++;
	    }
	  if (print)
	    {
	      std::cout << ", Finished" << std::flush;
	    }

	  if (!saturation_flag)
	    {
	      if (print)
		{
		  std::cout << ", " << success << " Recorded" << std::flush;
		}
	      success++;
	      if (rp.GetDraw())
		{
		  int canvnumber = 1;
		  for (std::map<ViUInt8, TH1I*>::iterator hitr = histMap.begin(); hitr != histMap.end(); ++hitr) 
		    {
		      canv->cd(canvnumber);
		      histMap[hitr->first]->Draw();
		      canv->Update();
		      axMap[hitr->first]->DrawAxis(gPad->GetUxmax(),gPad->GetUymin(),gPad->GetUxmax()-0.00000001,gPad->GetUymax(),gPad->GetUymin()*head.scaleFactor+head.scaleOffset,gPad->GetUymax()*head.scaleFactor+head.scaleOffset,510,"+L");
		      canv->Update();
		      canvnumber++;
		    }
		  canv->Modified();
		  canv->Update();
		}
	    }
	  /*
	  if (i_record % 100 == 0)
	    {
	      std::cout << "Event " << i_record << ", " << success << " recorded." << std::endl;
	    }
	  */
	  if (print)
	    {
	      std::cout << std::endl << std::flush;
	    }
	}
    }
  catch (std::exception e) {}

  fileout.close();
  
  std::cout << "\n " << i_record << " events recorded." << std::endl;

  Quit();

  return 0;
}
