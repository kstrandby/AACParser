#include "stdafx.h"
#include "AACParser.h"

#include <fstream>

using namespace std;

AACParser::AACParser()
{
}


AACParser::~AACParser()
{
}

int AACParser::GetNumberOfFrames(std::string filename, AACInfo info)
{
  ifstream inFile;

  inFile.open(filename, ios::in | ios::binary | ios::ate);
  char* allBytes = 0;

  inFile.seekg(0, ios::end);
  size_t size = inFile.tellg();
  inFile.seekg(0, ios::beg);

  allBytes = new char[size + 1]; //  for the '\0'
  inFile.read(allBytes, size);
  allBytes[size] = '\0'; // set '\0' 

  int nFrames = 0;

  for (int i = 0; i < size - 1; i++)
  {
    if (WholeByteIsAllOnes(allBytes[i]))
    {
      if (FirstNibbleIsAllOnes(allBytes[i + 1]) && SecondNibbleMatchesSyncWord(allBytes[i + 1], info))
      {
        nFrames++;
      }
    }
  }
  return nFrames;
}

bool AACParser::SecondNibbleMatchesSyncWord(char wholeByte, AACInfo info)
{
  int MPEGbit = ReadBit(wholeByte, 4);
  bool LayerMatches = ReadBit(wholeByte, 5) == 0 && ReadBit(wholeByte, 6) == 0;
  int CRCbit = ReadBit(wholeByte, 7);

  bool MPEGversionMatches = (info.MPEGversion == 2 && MPEGbit == 1) || (info.MPEGversion == 4 && MPEGbit == 0);
  bool CRCmatches = (info.CRC && CRCbit == 0) || (!info.CRC && CRCbit == 1);

  return LayerMatches && MPEGversionMatches && CRCmatches;
}

bool AACParser::FirstNibbleIsAllOnes(char wholeByte)
{
  for (int i = 0; i < 4; i++)
  {
    int bit = ReadBit(wholeByte, i);
    if (bit != 1)
    {
      return false;
    }
  }
  return true;
}

bool AACParser::WholeByteIsAllOnes(char wholeByte)
{
  for (int i = 0; i < 8; i++)
  {
    int bit = ReadBit(wholeByte, i);
    if (bit != 1)
    {
      return false;
    }
  }
  return true;
}

AACInfo AACParser::Parse(std::string filename)
{
  std::ifstream input;
  input.open(filename, std::ios::binary || std::ios::in);
  AACInfo result;

  if (input.is_open())
  {
    if (!ReadSyncWord(input))
    {
      std::cerr << "Sync word not valid!" << std::endl;
    }

    if (!ReadMPEGVersion(input, result.MPEGversion))
    {
      std::cerr << "MPEG version not valid!" << std::endl;
    }
    if (!ReadLayer(input))
    {
      std::cerr << "Layer not valid!" << std::endl;
    }

    if (!ReadProtectionAbsent(input, result.CRC))
    {
      std::cerr << "Protection absent information not valid!" << std::endl;
    }

    if (!ReadProfile(input, result.profile))
    {
      std::cerr << "Profile information not valid!" << std::endl;
    }

    if (!ReadSamlingFrequencyIndex(input, result.samplingFrequency))
    {
      std::cerr << "Sampling frequency index not valid!" << std::endl;
    }

    if (!ReadPrivateBit(input))
    {
      std::cerr << "Private bit not valid!" << std::endl;
    }

    if (!ReadChannelConfiguration(input, result.channelConfiguration))
    {
      std::cerr << "Channel configuration not valid!" << std::endl;
    }

    return result;
  }

}

bool AACParser::ReadSyncWord(std::ifstream &stream)
{
  char byte1;

  int syncWordPos = 0;

  while (stream.get(byte1))
  {
    syncWordPos++;
    bool nextByte = false;

    for (int i = 0; i < 8; i++)
    {
      int bit = ReadBit(byte1, i);
      if (bit != 1)
      {
        nextByte = true;
        break;
      }

    }
    if (nextByte) continue;

    char byte2;
    stream.get(byte2);
    syncWordPos++;

    for (int i = 0; i < 4; i++)
    {
      int bit = ReadBit(byte2, i);
      if (bit != 1)
      {
        nextByte = true;
        break;
      }

    }

    if (nextByte) continue;

    // if we get to here, syncword has been found, lets rewind to beginning of second byte,
    // so we can keep reading
    stream.seekg(syncWordPos - 1);
    return true;
  }

  for (int i = 0; i < 8; i++)
  {
    int bit = ReadBit(byte1, i);
    if (bit != 1) return false;
  }

  char byte2;
  stream.get(byte2);
  for (int i = 0; i < 4; i++)
  {
    int bit = ReadBit(byte2, i);
    if (bit != 1) return false;
  }
  stream.seekg(1); // move cursor back to byte 1 to be ready to read the rest of the bits
  return true;
}

bool AACParser::ReadMPEGVersion(std::ifstream &stream, int &version)
{
  streamoff curPos = stream.tellg();

  bool result = false;
  char byte;
  stream.get(byte);
  int MPEGbit = ReadBit(byte, 4);
  if (MPEGbit == 0)
  {
    version = 4; // 0 for MPEG-4
    result = true;
  }
  else if (MPEGbit == 1)
  {
    version = 2; // 1 for MPEG-2
    result = true;
  }
  stream.seekg(curPos); // move cursor back to start of the byte to be ready to read the rest of the bits
  return result;
}

bool AACParser::ReadLayer(std::ifstream & stream)
{
  streamoff curPos = stream.tellg();

  bool result = false;
  char byte;
  stream.get(byte);
  for (int i = 5; i <= 6; i++) // layer is 6th and 7th bit
  {
    if (ReadBit(byte, i) != 0)
      return result;
  }
  result = true;
  stream.seekg(curPos); // move cursor back to start of the byte to be ready to read the rest of the bits
  return result;
}

bool AACParser::ReadProtectionAbsent(std::ifstream & stream, bool & crc)
{
  bool result = false;
  char byte;
  stream.get(byte);
  int protectionAbsent = ReadBit(byte, 7);
  if (protectionAbsent == 1) // no CRC
  {
    crc = false;
    result = true;
  }
  else if (protectionAbsent == 0) // CRC 
  {
    crc = true;
    result = true;
  }

  return result;
}

bool AACParser::ReadProfile(std::ifstream & stream, std::string & profile)
{
  streamoff curPos = stream.tellg();

  bool result = false;
  char byte;
  stream.get(byte);
  int nProfile = ReadBits(byte, 0, 2);
  switch (nProfile)
  {
  case 1:
    profile = "AAC Main";
    result = true;
    break;
  case 2:
    profile = "AAC LC (Low Complexity)";
    result = true;
    break;
  case 3:
    profile = "AAC SSR (Scalable Sample Rate)";
    result = true;
    break;
  case 4:
    profile = "AAC LTP (Long Term Prediction)";
    result = true;
    break;
  default:
    result = false;
    break;
  }
  stream.seekg(curPos); // rewind to start of the byte to be ready to read rest of the bits
  return result;
}

bool AACParser::ReadSamlingFrequencyIndex(std::ifstream & stream, int & samplingFrequency)
{
  streamoff curPos = stream.tellg();

  bool result = true;
  char byte;
  stream.get(byte);
  int samplingFrequencyIndex = ReadBits(byte, 2, 4);
  switch (samplingFrequencyIndex)
  {
  case 0:
    samplingFrequency = 96000;
    break;
  case 1:
    samplingFrequency = 88200;
    break;
  case 2:
    samplingFrequency = 64000;
    break;
  case 3:
    samplingFrequency = 48000;
    break;
  case 4:
    samplingFrequency = 44100;
    break;
  case 5:
    samplingFrequency = 32000;
    break;
  case 6:
    samplingFrequency = 24000;
    break;
  case 7:
    samplingFrequency = 22050;
    break;
  case 8:
    samplingFrequency = 16000;
    break;
  case 9:
    samplingFrequency = 12000;
    break;
  case 10:
    samplingFrequency = 11025;
    break;
  case 11:
    samplingFrequency = 8000;
    break;
  case 12:
    samplingFrequency = 7350;
    break;
  default:
    result = false;
    break;
  }
  stream.seekg(curPos); // rewind to third byte 
  return result;
}

bool AACParser::ReadPrivateBit(std::ifstream & stream)
{
  streamoff curPos = stream.tellg();

  char byte;
  stream.get(byte);
  ReadBit(byte, 6); // just read the bit and return true, we dont care what it is
  stream.seekg(curPos);
  return true;
}

bool AACParser::ReadChannelConfiguration(std::ifstream & stream, std::string & channelConfiguration)
{
  bool result = true;

  // get last 1 bit in 3rd byte
  char byte;
  stream.get(byte);
  int lastBit = ReadBit(byte, 7);

  streamoff curPos = stream.tellg();

  // get first 2 bits of 4th byte
  stream.get(byte);
  int firstTwoBits = ReadBits(byte, 0, 2);

  // merge the bits to get the result
  int channelConfig = lastBit & (firstTwoBits << 1);
  switch (channelConfig)
  {
  case 0:
    channelConfiguration = "Channel configuration = 0 - Channel configuration is sent via inband PCE";
    break;
  case 1:
    channelConfiguration = "Channel configuration = 1 - 1 channel: front center";
    break;
  case 2:
    channelConfiguration = "Channel configuration = 2 - 2 channels: front-left, front-right";
    break;
  case 3:
    channelConfiguration = "Channel configuration = 3 - 3 channels: front-center, front-left, front-right";
    break;
  case 4:
    channelConfiguration = "Channel configuration = 4 - 4 channels: front-center, front-left, front-right, back-center";
    break;
  case 5:
    channelConfiguration = "Channel configuration = 5 - 5 channels: front-center, front-left, front-right, back-left, back-right";
    break;
  case 6:
    channelConfiguration = "Channel configuration = 6 - 6 channels: front-center, front-left, front-right, back-left, back-right, LFE-channel";
    break;
  case 7:
    channelConfiguration = "Channel configuration = 7 - 8 channels: front-center, front-left, front-right, side-left, side-right, back-left, back-right, LFE-channel";
    break;
  case 8: // channel configuration 8-15 reserved
  case 9:
  case 10:
  case 11:
  case 12:
  case 13:
  case 14:
  case 15:
  default:
    result = false;
    break;
  }
  stream.seekg(curPos);

  return result;
}

unsigned int AACParser::ReadBits(char byte, int offset, int numOfBits)
{
  unsigned int mask = (((0x1 << numOfBits) - 0x1) << offset);
  return (byte & mask) >> offset;
}

unsigned int AACParser::ReadBit(char byte, int offset)
{
  unsigned int mask = 0x1 << (7 - offset);
  return (byte & mask) >> (7 - offset);
}

