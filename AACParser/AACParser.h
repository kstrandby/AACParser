#pragma once
#include <iostream>

class AACParser
{
public:
	AACParser();
	~AACParser();

	void Parse(std::string filename);

private:
	unsigned int ReadBit(char byte, int offset);
	unsigned int ReadBits(char byte, int offset, int numOfBits);

	bool ReadSyncWord(std::ifstream &stream);
	bool ReadMPEGVersion(std::ifstream &stream, int &version);
	bool ReadLayer(std::ifstream &stream);
	bool ReadProtectionAbsent(std::ifstream &stream, bool &crc);
	bool ReadProfile(std::ifstream &stream, std::string &profile);
	bool ReadSamlingFrequencyIndex(std::ifstream &stream, int &samplingFrequency);
	bool ReadPrivateBit(std::ifstream &stream);
	bool ReadChannelConfiguration(std::ifstream &stream, std::string &channelConfiguration);

	void PrintInfo(int MPEGversion, bool crc, std::string profile, int samplingFrequency, std::string channelConfiguration);
};

