// NVSigCheck.cpp

#include <stdio.h>
#include <string.h>

#include <osg/PositionAttitudeTransform>

#include "3DNCrypt.h"
#include "NVSigCheck.h"
#include "NVScene.h"

extern NVScene MasterScene;



// NatureView 384-bit Key ID: 0xBFABCB9FD5043BCD  (I think this key fingerprint is incorrect, and goes with the key below)
// N=Public key
#define NV_KEY_VNS_N "\xA8\x9D\x69\x8A\xAE\x75\x73\xE0\xE6\x5E\x8C\xA3\x0F\x9D\xD1\x7B\xFB\xBB\xBD\x7A\x94\xC9\xA3\x8D\x8D\x18\x31\xAE\x63\x94\x22\x42\xB1\xB3\xBE\xAE\x9E\xE4\xB5\xCD\xBF\xAB\xCB\x9F\xD5\x04\x3B\xCD"
// D=Private key
#define NV_KEY_VNS_D "\x18\xCB\xDA\xD0\xA1\x2F\x63\xDD\x4F\x0D\xE7\x81\x64\x2D\xCB\xFB\xA5\x05\x05\x46\xBB\x87\x10\x85\x84\xBF\x82\x96\x95\xE2\xF6\x73\xE7\x49\x99\x6F\x41\x31\xF4\x4A\xA8\xAF\xB0\xA2\xF1\xCA\xEE\xC7"

// NatureView 384-bit Key ID: 0xBFABCB9FD5043BCD
// N=Public key
#define NV_KEY_WCS_N "\xB6\x7F\x1A\xF9\x1D\x72\x29\xAD\xC9\xC8\xF0\xF6\x56\x49\xA6\x56\x69\x35\x35\x2B\x9B\xFC\xA1\x52\xB6\x27\x27\x41\xCB\x77\x26\xD8\x5B\xA2\x96\xE8\x98\x6D\x22\x0A\xD3\x75\x1F\xEE\x2F\xFC\x0B\xE9"
// D=Private key
#define NV_KEY_WCS_D "\x35\xAC\xE9\xD0\xCC\x6C\xDF\x14\xFF\x1C\xFB\x93\xBF\x06\x9A\x55\xA6\x79\x0F\xA3\x6A\x1D\x20\x63\x1F\x57\x2C\x3F\xD9\x16\xB6\x18\xC5\xC9\x84\x3B\x26\x4D\xC6\x1C\xA9\x85\x1A\x5C\xFA\x90\x69\x55"


char PreviousCachedSigText[100];
char PreviousCachedFileText[1000];
bool PreviousSigSuccess, PreviousSigValid;

bool GlobalSigInvalid = false;

void SetGlobalSigInvalid(bool NewState)
{
GlobalSigInvalid = NewState;
} // SetGlobalSigInvalid

bool GetGlobalSigInvalid(void)
{
return(GlobalSigInvalid);
} // GetGlobalSigInvalid


void InitSigCaches(void)
{
#ifdef NV_CHECK_SIGNATURES
PreviousCachedFileText[0] = PreviousCachedSigText[0] = NULL;
PreviousSigSuccess = false;
PreviousSigValid = false;
#endif // NV_CHECK_SIGNATURES
} // InitSigCaches

// returns true if Check is successful
// generic binary file signature checking
bool CheckDependentBinaryFileSignature(const char *InputSig, const char *FileNameAndPath)
{
// must be explicitly cleared to succeed
bool SigInvalid = true;

// Handle repeated calls via caching
// (avoids expensive RSA decryption and file reads for MD5 calculation for
// sequential calls for the same object)
if(PreviousSigValid)
	{
	if(PreviousCachedSigText[0])
		{
		if(!strcmp(InputSig, PreviousCachedSigText))
			{
			if(!strcmp(FileNameAndPath, PreviousCachedFileText))
				{
				return(PreviousSigSuccess);
				} // if
			} // if
		} // if
	} // if

if(InputSig && InputSig[0] && FileNameAndPath && FileNameAndPath[0])
	{
	unsigned char NVWMD5Sixteen[17], SigSixteen[17];
	int PacketVersion = 0, MaxLength = 0;
	PacketVersion = GetPacketVersion((unsigned char *)InputSig);
	if(PacketVersion == 1) MaxLength = 0;
	if(PacketVersion == 2) MaxLength = 16384;
	if(CalcHashOfFileFromName(FileNameAndPath, NVWMD5Sixteen, MaxLength, 1)) // calculate hash using binary method (allow ECW hash)
		{
		unsigned char DBuf[48], NBuf[48];

		// Try VNS2 signature first
		memcpy(DBuf, NV_KEY_VNS_D, 48);
		memcpy(NBuf, NV_KEY_VNS_N, 48);
		// this makes debugging easier
		NVWMD5Sixteen[16] = SigSixteen[16] = 0;
		if(DecryptPacket((unsigned char *)InputSig, SigSixteen, DBuf, NBuf))
			{
			// update cache
			strcpy(PreviousCachedSigText, InputSig);
			strcpy(PreviousCachedFileText, FileNameAndPath);
			PreviousSigValid = 1;

			if(memcmp(NVWMD5Sixteen, SigSixteen, 16))
				{
				PreviousSigSuccess = 0;
				//UserMessageOK(NVW_NATUREVIEW_NAMETEXT, "Signatures differ!", REQUESTER_ICON_EXCLAMATION);
				} // if
			else
				{
				SigInvalid = false;
				PreviousSigSuccess = 1;
				//UserMessageOK(NVW_NATUREVIEW_NAMETEXT, "Signatures match!");
				} // else
			} // if
		// if that fails, try WCS6
		if(SigInvalid)
			{
			memcpy(DBuf, NV_KEY_WCS_D, 48);
			memcpy(NBuf, NV_KEY_WCS_N, 48);
			// this makes debugging easier
			NVWMD5Sixteen[16] = SigSixteen[16] = 0;
			if(DecryptPacket((unsigned char *)InputSig, SigSixteen, DBuf, NBuf))
				{
				// update cache
				strcpy(PreviousCachedSigText, InputSig);
				strcpy(PreviousCachedFileText, FileNameAndPath);
				PreviousSigValid = 1;

				if(memcmp(NVWMD5Sixteen, SigSixteen, 16))
					{
					PreviousSigSuccess = 0;
					//UserMessageOK(NVW_NATUREVIEW_NAMETEXT, "Signatures differ!", REQUESTER_ICON_EXCLAMATION);
					} // if
				else
					{
					SigInvalid = false;
					PreviousSigSuccess = 1;
					//UserMessageOK(NVW_NATUREVIEW_NAMETEXT, "Signatures match!");
					} // else
				} // if
			} // if
		} // if
	} // if

return(!SigInvalid);
} // CheckDependentBinaryFileSignature


bool CheckNVWFileSignature(const char *FileNameAndPath)
{
bool SigInvalid = false;

#ifdef NV_CHECK_SIGNATURES
// check signatures

// SigInvalid must be successfully cleared to proceed
SigInvalid = true;
if(MasterScene.CheckSig())
	{
	unsigned char NVWMD5Sixteen[17], SigSixteen[17];
	if(CalcModifiedHashOfNVWFileFromName(FileNameAndPath, NVWMD5Sixteen))
		{
		unsigned char DBuf[48], NBuf[48];

		// Try VNS2 signature
		memcpy(DBuf, NV_KEY_VNS_D, 48);
		memcpy(NBuf, NV_KEY_VNS_N, 48);
		// this makes debugging easier
		NVWMD5Sixteen[16] = SigSixteen[16] = 0;
		if(DecryptPacket((unsigned char *)MasterScene.GetSig(), SigSixteen, DBuf, NBuf))
			{
			if(memcmp(NVWMD5Sixteen, SigSixteen, 16))
				{
				//UserMessageOK(NVW_NATUREVIEW_NAMETEXT, "Signatures differ!", REQUESTER_ICON_EXCLAMATION);
				} // if
			else
				{
				SigInvalid = false;
				//UserMessageOK(NVW_NATUREVIEW_NAMETEXT, "Signatures match!");
				} // else
			} // if

		// if that fails, try WCS6
		if(SigInvalid)
			{
			memcpy(DBuf, NV_KEY_WCS_D, 48);
			memcpy(NBuf, NV_KEY_WCS_N, 48);
			// this makes debugging easier
			NVWMD5Sixteen[16] = SigSixteen[16] = 0;
			if(DecryptPacket((unsigned char *)MasterScene.GetSig(), SigSixteen, DBuf, NBuf))
				{
				if(memcmp(NVWMD5Sixteen, SigSixteen, 16))
					{
					//UserMessageOK(NVW_NATUREVIEW_NAMETEXT, "Signatures differ!", REQUESTER_ICON_EXCLAMATION);
					} // if
				else
					{
					SigInvalid = false;
					//UserMessageOK(NVW_NATUREVIEW_NAMETEXT, "Signatures match!");
					} // else
				} // if
			}

		} // if
	} // if
#endif // NV_CHECK_SIGNATURES

return(SigInvalid);
} // CheckNVWFileSignature
