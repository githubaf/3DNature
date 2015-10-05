// NatureViewCrypto.h
// Cryptographic code relating to NatureView authorization and file signing
// created from scratch on Oct 10th 2003, by CXH

enum
	{
	NVW_KEY_PRERELEASE = 0,
	NVW_KEY_SX1_WCS_6,
	NVW_KEY_SX1_VNS_2,
	NVW_KEY_SX2_WCS_7,
	NVW_KEY_SX2_VNS_3,
	NVW_KEY_SX3_WCS_8,
	NVW_KEY_SX3_VNS_4,
	NVW_KEY_SX4_WCS_9,
	NVW_KEY_SX4_VNS_5
	}; // keys

// Used for "Sign NatureView File" menu
int RequestAndSignNVWFile(int KeyID);

// SX-NVW uses this to sign a just-written NVW file
int AppendSignNVWSceneFile(const char *FilePathAndName, int KeyID);

// SX-NVW uses this to create signatures for externally-referenced (presumably) binary files
int GenerateNVWDependentFileSignature(const char *FilePathAndName, int KeyID, char *SigBuf);

// Security (and therefore AuthorizeGUI) use this to parse input authorization packets
int DecodeArmoredAuthString(const unsigned char *PacketInput, unsigned char *DataOutputSixteen);
