// NatureViewCrypto.cpp
// Cryptographic code relating to NatureView authorization and file signing
// created from scratch on Oct 10th 2003, by CXH

#include "stdafx.h"
#include "Security.h"
#include "Requester.h"
#include "NatureViewCrypto.h"
#include "Log.h"
#include "UsefulPathString.h"
#include "Project.h" // for Munging

#include "3DNCrypt.h"

#ifdef WCS_BUILD_VNS
// NatureView 384-bit Key ID: 0xBFABCB9FD5043BCD (I think this key fingerprint is incorrect, and goes with the key below)
// N=Public key
#define NV_KEY_VNS_N "\xA8\x9D\x69\x8A\xAE\x75\x73\xE0\xE6\x5E\x8C\xA3\x0F\x9D\xD1\x7B\xFB\xBB\xBD\x7A\x94\xC9\xA3\x8D\x8D\x18\x31\xAE\x63\x94\x22\x42\xB1\xB3\xBE\xAE\x9E\xE4\xB5\xCD\xBF\xAB\xCB\x9F\xD5\x04\x3B\xCD"
// D=Private key
// #define NV_KEY_VNS_D "\x18\xCB\xDA\xD0\xA1\x2F\x63\xDD\x4F\x0D\xE7\x81\x64\x2D\xCB\xFB\xA5\x05\x05\x46\xBB\x87\x10\x85\x84\xBF\x82\x96\x95\xE2\xF6\x73\xE7\x49\x99\x6F\x41\x31\xF4\x4A\xA8\xAF\xB0\xA2\xF1\xCA\xEE\xC7"
// E=Key Exponent = 0x11
#define NV_KEY_VNS_E "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x11"

#else // !WCS_BUILD_VNS = WCS

// WCS NatureView 384-bit Key ID: 0xBFABCB9FD5043BCD
// N=Public key = 0xB67F1AF91D7229ADC9C8F0F65649A6566935352B9BFCA152B6272741CB7726D85BA296E8986D220AD3751FEE2FFC0BE9
#define NV_KEY_WCS_N "\xB6\x7F\x1A\xF9\x1D\x72\x29\xAD\xC9\xC8\xF0\xF6\x56\x49\xA6\x56\x69\x35\x35\x2B\x9B\xFC\xA1\x52\xB6\x27\x27\x41\xCB\x77\x26\xD8\x5B\xA2\x96\xE8\x98\x6D\x22\x0A\xD3\x75\x1F\xEE\x2F\xFC\x0B\xE9"
// D=Private key = 0x35ACE9D0CC6CDF14FF1CFB93BF069A55A6790FA36A1D20631F572C3FD916B618C5C9843B264DC61CA9851A5CFA906955
// #define NV_WCS_KEY_D "\x35\xAC\xE9\xD0\xCC\x6C\xDF\x14\xFF\x1C\xFB\x93\xBF\x06\x9A\x55\xA6\x79\x0F\xA3\x6A\x1D\x20\x63\x1F\x57\x2C\x3F\xD9\x16\xB6\x18\xC5\xC9\x84\x3B\x26\x4D\xC6\x1C\xA9\x85\x1A\x5C\xFA\x90\x69\x55"
// E=Key Exponent = 0x11
#define NV_KEY_WCS_E "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x11"

#endif // !WCS_BUILD_VNS = WCS

// 3D Nature Auth 384-bit Key ID: 0x6E9EFC45993B90E1
// N=Public key
#define AUTH_KEY_N "\xC0\xAE\x11\x31\x9E\xAF\xDF\x51\x59\x40\xFA\x76\x78\xBC\xDA\x40\xEB\xD0\x7F\xA6\x06\xF9\x4F\x55\x28\x01\x10\xDA\xD3\x91\x90\xF0\xCE\x86\x33\x34\x62\x78\xEE\x44\x6E\x9E\xFC\x45\x99\x3B\x90\xE1"
// D=Private key
// #define AUTH_KEY_D "\x0D\x39\x1F\x4C\x30\x89\x8F\x53\x62\xFC\xEE\x0D\x26\x67\x50\x3B\xAB\xCA\x8B\x45\x1E\x98\xA3\x8F\xC3\x7C\x3B\xBB\xEF\x7F\xE8\xD1\x5C\x59\x66\x0D\xA3\x90\xDE\x8E\xBC\x1D\x7C\x3E\x5E\x8B\x50\xD3"
// E=Key Exponent = 0x11
#define AUTH_KEY_E "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x11"


int RequestAndSignNVWFile(int KeyID)
{
#ifdef WCS_BUILD_DEMO
return(1);
#else // !WCS_BUILD_DEMO
int Success = 0;
#ifdef WCS_BUILD_RTX
const unsigned char *TextOutput = NULL;
const char *FilePathAndName;
FILE *NVWFile;
FileReq ChooseFile;
unsigned char EBuf[48], NBuf[48];
char MiscTextBuf[1000], FileNameBuf[1000];
bool IsNV = false;



// Do dongle/RTX check, just to be safe
if(GlobalApp->SXAuthorized = GlobalApp->Sentinal->CheckAuthFieldRTX() ? 1: 0)
	{
	ChooseFile.SetDefPat(WCS_REQUESTER_PARTIALWILD("nvw"));
	if(ChooseFile.Request(NULL))
		{
		char *MungedPathAndName;
		FilePathAndName = ChooseFile.GetFirstName();
		MungedPathAndName = GlobalApp->MainProj->MungPath(FilePathAndName);
		strcpy(FileNameBuf, MungedPathAndName);
		if(NVWFile = fopen(FileNameBuf, "r"))
			{
			// clear indetification buffer
			memset(MiscTextBuf, 0, 999);
			fgetline(MiscTextBuf, 900, NVWFile, 0, 0);
			if(!strncmp(MiscTextBuf, "<?xml version=\"1.0\" encoding=\"utf-8\"?>", 38))
				{
				memset(MiscTextBuf, 0, 999);
				fgetline(MiscTextBuf, 900, NVWFile, 0, 0);
				if(!strncmp(MiscTextBuf, "<NATUREVIEW>", 12))
					{
					IsNV = true;
					} // if
				} // if

			fclose(NVWFile);
			NVWFile = NULL;

			if(KeyID == NVW_KEY_SX1_VNS_2)
				{
				#ifdef WCS_BUILD_VNS
				memcpy(EBuf, NV_KEY_VNS_E, 48);
				memcpy(NBuf, NV_KEY_VNS_N, 48);
				#endif // WCS_BUILD_VNS
				} // if
			else if (KeyID == NVW_KEY_SX1_WCS_6)
				{
				#ifndef WCS_BUILD_VNS
				memcpy(EBuf, NV_KEY_WCS_E, 48);
				memcpy(NBuf, NV_KEY_WCS_N, 48);
				#endif // !WCS_BUILD_VNS
				} // else
			else
				{
				UserMessageOK("ALERT!", "Unknown NatureView Key!");
				} // else

			if(IsNV)
				{
				if(NVWFile = fopen(FileNameBuf, "rb+"))
					{ // seek to end and erase old signature if necessary
					long EndPos, SigPos = -1;
					bool KeepSearching;

					fseek(NVWFile, 0, SEEK_END); // seek to end.
					EndPos = ftell(NVWFile);
					fseek(NVWFile, 0, SEEK_SET); // seek to start

					for(KeepSearching = 1; KeepSearching;)
						{
						int CurPos = ftell(NVWFile);

						if(CurPos == EndPos)
							{
							SigPos = -1; // didn't find it
							KeepSearching = 0; // stop looking
							} // if

						fgetline(MiscTextBuf, 900, NVWFile, 0, 0);

						if(!strncmp("<!-- SIGNATURE ", MiscTextBuf, 15))
							{
							SigPos = CurPos; // beginning of the line we just read
							KeepSearching = 0;
							} // if
						} // while

					if(SigPos > 0)
						{ // obliterate everything after signature with non-hashing spaces, since we can't truncate
						int Spacing;
						fseek(NVWFile, SigPos, SEEK_SET); // seek to start of sig
						memset(MiscTextBuf, ' ', 999);

						Spacing = EndPos - SigPos;
						while(Spacing > 0)
							{
							if(Spacing > 998)
								{
								fprintf(NVWFile, "\n");
								fwrite(MiscTextBuf, 997, 1, NVWFile);
								fprintf(NVWFile, "\n");
								Spacing -= 999;
								} // if
							else
								{
								fwrite(MiscTextBuf, Spacing - 1, 1, NVWFile);
								fprintf(NVWFile, "\n");
								Spacing -= Spacing; // Spacing = 0, silly!
								} // else
							} // while
						} // if
					fclose(NVWFile);
					} // if
				} // if

			unsigned char AllowECW = 0;
			#ifdef WCS_BUILD_VNS
			AllowECW = 1;
			#endif // WCS_BUILD_VNS
			TextOutput = HashSignEncryptAndEncode(FileNameBuf, IsNV, EBuf, NBuf, AllowECW);

			if(IsNV)
				{
				if(NVWFile = fopen(FileNameBuf, "r+"))
					{
					bool KeepSearching;
					int SigPos = -1, CurPos = -1;

					fseek(NVWFile, 0, SEEK_SET); // seek to start

					for(KeepSearching = 1; KeepSearching;)
						{
						fgetline(MiscTextBuf, 900, NVWFile, 0, 0);
						CurPos = ftell(NVWFile);
						if(feof(NVWFile)) // did we hit the end?
							{
							if(SigPos == -1)
								{
								SigPos = CurPos; // append here
								} // if
							KeepSearching = 0; // stop looking
							} // if
						else
							{
							// are we positioned after a non-blank line?
							if(!isspace(MiscTextBuf[0]) || SkipPastNextSpace(MiscTextBuf)) // does this line have anything other than whitespace?
								{
								SigPos = CurPos; // beginning of the line after the line we just read
								} // if
							} // else
						} // while

					fseek(NVWFile, SigPos, SEEK_SET); // seek to start of sig

					fprintf(NVWFile, "\n<!-- SIGNATURE TYPE=\"NATUREVIEW\" VALUE=\"%s\" -->\n", TextOutput); // ensure we begin on a newline and end with one

					fclose(NVWFile);
					NVWFile = NULL;

					sprintf(MiscTextBuf, "Signed file \"%s\".", FileNameBuf);
					GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, MiscTextBuf);
					sprintf(MiscTextBuf, "Signature: \"%s\".", TextOutput);
					GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, MiscTextBuf);

		/*
						{ // temp test
						unsigned char OutSig[16], TestSig[16];
						unsigned char DBuf[48];
						memcpy(DBuf, NV_KEY_VNS_D, 48);
						if(DecryptPacket(TextOutput, OutSig, DBuf, NBuf))
							{
							if(CalcModifiedHashOfTextFileFromName(FileNameBuf, TestSig))
								{
								if(memcmp(TestSig, OutSig, 16))
									{
									UserMessageOK("SX Signature Test", "Signatures differ!", REQUESTER_ICON_EXCLAMATION);
									} // if
								else
									{
									UserMessageOK("SX Signature Test", "Signatures match!");
									} // else
								} // if
							} // if
						} // scope

		*/
					Success = 1;
					} // if
				} // if
			else
				{ // display signature of binary file
				strcpy(MiscTextBuf, (char *)TextOutput);
				
				GetInputString("Calculated signature for file:\n(Control+C to copy)", "", MiscTextBuf);
				} // else
			} // if
		} // if
	} // if
#endif // WCS_BUILD_RTX

return(Success);
#endif // !WCS_BUILD_DEMO

} // RequestAndSignNVWFile

int AppendSignNVWSceneFile(const char *FilePathAndName, int KeyID)
{
#ifdef WCS_BUILD_DEMO
return(1);
#else // !WCS_BUILD_DEMO

int Success = 0;
#ifdef WCS_BUILD_RTX
const unsigned char *TextOutput = NULL;
FILE *NVWFile;
unsigned char EBuf[48], NBuf[48];

// we won't do a dongle check here, as we might be called repeatedly,
// and you can't call this unless you've already passed a dongle check

if(KeyID == NVW_KEY_SX1_VNS_2)
	{
	#ifdef WCS_BUILD_VNS
	memcpy(EBuf, NV_KEY_VNS_E, 48);
	memcpy(NBuf, NV_KEY_VNS_N, 48);
	#endif // WCS_BUILD_VNS
	} // if
else if (KeyID == NVW_KEY_SX1_WCS_6)
	{
	#ifndef WCS_BUILD_VNS
	memcpy(EBuf, NV_KEY_WCS_E, 48);
	memcpy(NBuf, NV_KEY_WCS_N, 48);
	#endif // !WCS_BUILD_VNS
	} // else
else
	{
	UserMessageOK("ALERT!", "Unknown NatureView Key!");
	} // else
TextOutput = HashSignEncryptAndEncode(FilePathAndName, true, EBuf, NBuf, 0);

if(NVWFile = fopen(FilePathAndName, "r+"))
	{
	fseek(NVWFile, 0, SEEK_END); // seek to end.

	fprintf(NVWFile, "<!-- SIGNATURE TYPE=\"NATUREVIEW\" VALUE=\"%s\" -->\n", TextOutput);

	fclose(NVWFile);
	NVWFile = NULL;
	Success = 1;
	} // if
#endif // WCS_BUILD_RTX

return(Success);

#endif // !WCS_BUILD_DEMO
} // AppendSignNVWSceneFile


int GenerateNVWDependentFileSignature(const char *FilePathAndName, int KeyID, char *SigBuf)
{
#ifdef WCS_BUILD_DEMO
return(1);
#else // !WCS_BUILD_DEMO
int Success = 0;
#ifdef WCS_BUILD_RTX
const unsigned char *TextOutput = NULL;
unsigned char EBuf[48], NBuf[48];

// we won't do a dongle check here, as we might be called repeatedly,
// and you can't call this unless you've already passed a dongle check

if(KeyID == NVW_KEY_SX1_VNS_2)
	{
	#ifdef WCS_BUILD_VNS
	memcpy(EBuf, NV_KEY_VNS_E, 48);
	memcpy(NBuf, NV_KEY_VNS_N, 48);
	#endif // WCS_BUILD_VNS
	} // if
else if (KeyID == NVW_KEY_SX1_WCS_6)
	{
	#ifndef WCS_BUILD_VNS
	memcpy(EBuf, NV_KEY_WCS_E, 48);
	memcpy(NBuf, NV_KEY_WCS_N, 48);
	#endif // !WCS_BUILD_VNS
	} // else
else
	{
	UserMessageOK("ALERT!", "Unknown NatureView Key!");
	} // else

unsigned char AllowECW = 0;
#ifdef WCS_BUILD_VNS
AllowECW = 1;
#endif // WCS_BUILD_VNS

if(TextOutput = HashSignEncryptAndEncode(FilePathAndName, false, EBuf, NBuf, AllowECW))
	{
	strcpy(SigBuf, (char *)TextOutput);
	Success = 1;
	} // if
#endif // WCS_BUILD_RTX

return(Success);

#endif // !WCS_BUILD_DEMO
} // GenerateNVWDependentFileSignature


// This decodes a packet to see if we like it
int DecodeArmoredAuthString(const unsigned char *PacketInput, unsigned char *DataOutputSixteen)
{
#ifdef WCS_BUILD_DEMO
return(0);
#else // !WCS_BUILD_DEMO
#ifdef WCS_BUILD_RTX
unsigned char EBuf[48], NBuf[48];

// we publicly decode with the easy E/N combo, keeping the hard D/N pair elsewhere (in the authorize tool)
memcpy(EBuf, AUTH_KEY_E, 48);
memcpy(NBuf, AUTH_KEY_N, 48);

//clear output buffer
memset(DataOutputSixteen, 0, 16);


if(DecryptPacket(PacketInput, DataOutputSixteen, EBuf, NBuf))
	{
	return(1);
	} // if
#endif // WCS_BUILD_RTX

return(0);
#endif // !WCS_BUILD_DEMO
} // DecodeArmoredAuthString


/*
#define CRYPT_OUTBUFSIZE	1000
unsigned char EncoderEncodedOutText[CRYPT_OUTBUFSIZE]; // longer than it needs to be
unsigned char EncoderPacketBufferFiftyTwo[52];
// This is not used in WCS/VNS, just in the authorization tool
const unsigned char *EncodeAuthString(const unsigned char *InputDataSixteen)
{
unsigned char DBuf[48], NBuf[48];

// we encode with the hard D/N combo
memcpy(DBuf, AUTH_KEY_D, 48);
memcpy(NBuf, AUTH_KEY_N, 48);

//clear output buffer
memset(EncoderPacketBufferFiftyTwo, 0, 52);

if(FormatAndEncryptPacket(EncoderPacketBufferFiftyTwo, (unsigned char *)InputDataSixteen, DBuf, NBuf)) // we keep D private, and let E be public
	{
	int OutputSize = 0;
	memset(EncoderEncodedOutText, 0, CRYPT_OUTBUFSIZE);
	if(BinToBase64(EncoderPacketBufferFiftyTwo, 51, EncoderEncodedOutText, &OutputSize))
		{
		return(EncoderEncodedOutText);
		} // if
	} // if

return(NULL);
} // EncodeAuthString

*/
