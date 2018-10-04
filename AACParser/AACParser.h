#pragma once
#include <iostream>

struct AACInfo
{
  int MPEGversion;
  bool CRC;
  std::string profile;
  int samplingFrequency;
  std::string channelConfiguration;

  void PrintInfo()
  {
    std::cout << "File information: " << std::endl
      << "MPEG version: \t\tMPEG-" << MPEGversion << std::endl
      << "CRC present: \t\t" << (CRC ? "Yes" : "No") << std::endl
      << "Profile: \t\t" << profile.c_str() << std::endl
      << "Sampling frequency: \t" << samplingFrequency << "Hz" << std::endl
      << "Channel configuration: \t" << channelConfiguration.c_str() << std::endl;
  }
};

class AACParser
{
public:
	AACParser();
	~AACParser();

  int GetNumberOfFrames(std::string filename, AACInfo info);
	AACInfo Parse(std::string filename);

private:
	unsigned int ReadBit(char byte, int offset);
	unsigned int ReadBits(char byte, int offset, int numOfBits);
  bool FirstNibbleIsAllOnes(char wholeByte);
  bool SecondNibbleMatchesSyncWord(char wholeByte, AACInfo info);
  bool WholeByteIsAllOnes(char wholeByte);

	bool ReadSyncWord(std::ifstream &stream);
	bool ReadMPEGVersion(std::ifstream &stream, int &version);
	bool ReadLayer(std::ifstream &stream);
	bool ReadProtectionAbsent(std::ifstream &stream, bool &crc);
	bool ReadProfile(std::ifstream &stream, std::string &profile);
	bool ReadSamlingFrequencyIndex(std::ifstream &stream, int &samplingFrequency);
	bool ReadPrivateBit(std::ifstream &stream);
	bool ReadChannelConfiguration(std::ifstream &stream, std::string &channelConfiguration);

};

