// Security.cpp
//
// Hardware key security routines
// Built from scratch on 9/22/97 by Chris 'Xenon' Hanson
// Copyright 1997

#include "stdafx.h"
#include "AppMem.h"
#include "Application.h"
#include "Security.h"
#include "WCSVersion.h"
#include "requester.h"
#include "UsefulPathString.h"
#include "UsefulEndian.h"

#ifdef WCS_BUILD_RTX
#include "NatureViewCrypto.h"
#endif // WCS_BUILD_RTX

#ifdef WCS_BUILD_DEMO
// Annoy user every 5 minutes
#define WCS_SECURITY_DEMO_ANNOY_SECONDS 300
#endif // WCS_BUILD_DEMO

#ifndef WCS_BUILD_DEMO
char *XFT_trJz(void) {return("Security Check Failed, exiting.");}
#endif // !WCS_BUILD_DEMO

Security::Security()
{
#ifdef WCS_BUILD_DEMO
DTime = 0;
#endif // WCS_BUILD_DEMO
#ifdef _WIN32
SPRO = NULL;
FullSerial = 0;
SerA = 0; SerB = 0;
DResp = 0;
SPResp[0] = 0;
SPQuery[0] = 0;
DonglePresentCached = 0;
RenderEngineCached = 0;
DongleFoundInitially = 0;
AttemptNLM = 0;
PreferredNLMHost[0] = NULL;
#endif // _WIN32

} // Security::Security


Security::~Security()
{

#ifndef DISABLE_DONGLE

#ifdef WCS_SECURITY_NETWORK_DONGLE
RNBOsproReleaseLicense(SPRO, 0, NULL); // definitively release license
#endif // WCS_SECURITY_NETWORK_DONGLE


if(SPRO)
	{
	AppMem_Free(SPRO, SPRO_APIPACKET_SIZE);
	SPRO = NULL;
	} // if

#endif // !DISABLE_DONGLE

} // Security::~Security

#ifndef WCS_BUILD_DEMO
int Security::InitDongle(void)
{
#ifndef DISABLE_DONGLE

if(!SPRO)
	{
	if(SPRO = (RBP_SPRO_APIPACKET)AppMem_Alloc(SPRO_APIPACKET_SIZE, APPMEM_CLEAR))
		{
		if(RNBOsproInitialize(SPRO) == SP_SUCCESS)
			{
			return(1);
			} // if
		} // if
	AppMem_Free(SPRO, SPRO_APIPACKET_SIZE);
	SPRO = NULL;
	} // if

return(SPRO ? 1 : 0);

#else  // !DISABLE_DONGLE
return(1);
#endif // !DISABLE_DONGLE

} // Security::InitDongle
#endif // !WCS_BUILD_DEMO


#ifndef WCS_BUILD_DEMO
#ifdef WCS_SECURITY_NETWORK_DONGLE
void Security::PingDongleServer(void)
{
ReadAuthField(WCS_SECURITY_AUTH_FIELD_A);
} // Security::PingDongleServer
#endif // WCS_SECURITY_NETWORK_DONGLE
#endif // !WCS_BUILD_DEMO


#ifndef WCS_BUILD_DEMO
unsigned long Security::FindDongle(void)
{
// wrapper to make sure we only do this once per run
if(!DongleFoundInitially)
	{
#ifdef WCS_SECURITY_NETWORK_DONGLE
	if(GetAttemptNLM())
		{ // indicate whether we should look on the network for a net key
		// not setting the contact server mode, allows the user to specify the NSP_HOST, which allows for
		// license servers to be elsewhere than on the broadcast subnet.
		RNBOsproSetContactServer(SPRO, RNBO_SPN_ALL_MODES); // this prohibits use of NSP_HOST, so we offer SOFTLICENSE= below
		char *PreferredHost = GetPreferredNLMHost();
		if(strlen(PreferredHost))
			{
			RNBOsproSetContactServer(SPRO, GetPreferredNLMHost()); // set preferred server as directed by user with SOFTLICENSE=
			} // if
		GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, "Initiating network license capability.");
		} // if
	else
#endif // WCS_SECURITY_NETWORK_DONGLE
		{
		RNBOsproSetContactServer(SPRO, RNBO_SPN_DRIVER);
		} // else
	if(RNBOsproFindFirstUnit(SPRO, DONGLE_DEV_ID) == SP_SUCCESS)
		{
		DongleFoundInitially = 1;
		} // if
	else
		{
		DongleFoundInitially = 2; // stop looking for it if we were refused initially
		} // else
	} // if

return(DongleFoundInitially);
} // Security::FindDongle
#endif // WCS_BUILD_DEMO



#ifndef WCS_BUILD_DEMO
unsigned long Security::CheckDongle(unsigned long Code)
{
#ifndef DISABLE_DONGLE
int Retry;
unsigned long QueryCode = NULL, Response32 = NULL, Data32 = NULL;

if (Code == NULL)
	{
	memcpy(SPQuery, "\xDC\x6C\xAE\xC4", 4);
	memcpy(&QueryCode, "\xDC\x6C\xAE\xC4", 4);
	} // if
else
	{
	SPQuery[0] = (unsigned char)((Code & 0xff000000) >> 24);
	SPQuery[1] = (unsigned char)((Code & 0x00ff0000) >> 16);
	SPQuery[2] = (unsigned char)((Code & 0x0000ff00) >> 8);
	SPQuery[3] = (unsigned char)((Code & 0x000000ff));
	memcpy(&QueryCode, &SPQuery[0], 4);
	} // else

if(InitDongle())
	{
	for(Retry = 0; Retry < 5; Retry++)
		{
		if(FindDongle())
			{
			#ifdef WCS_SECURITY_NETWORK_DONGLE
			static bool ServerNameLogged = false;
			char ServerNameMsg[300];
			char ServerID[200];
			ServerID[0] = NULL;
			// Now check and see if it's via network, and if so, ensure it's a multi-license key
			RNBOsproGetContactServer(SPRO, ServerID, 199);
			if(stricmp(ServerID, "RNBO_SPN_DRIVER")) // are we accessing a network server?
				{ // it's a network key
				unsigned short HardLimit = 0;
				RNBOsproSetHeartBeat(SPRO, WCS_SECURITY_NETWORK_DONGLE_HEARTBEAT); // license expires every so many seconds (increased from default of 120)
				RNBOsproGetHardLimit(SPRO, &HardLimit);
				if(!ServerNameLogged)
					{ // this only fires once
					sprintf(ServerNameMsg, "Located network key on server %s, Hard Limit %d.", ServerID, HardLimit);
					GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, ServerNameMsg);
					ServerNameLogged = true;
					} // if
				if(HardLimit == 1) // single-license key
					{
					unsigned long NetAuthField = 0;
					// is this single-license key specifically enabled for Net sharing?
					NetAuthField = ReadAuthField(WCS_SECURITY_AUTH_FIELD_NETKEY);
					if(NetAuthField != CheckKeySerial())
						{
						RNBOsproReleaseLicense(SPRO, 0, NULL); // definitively release license if it doesn't qualify
						return(0); // not permitted
						} // if
					} // if
				} // if
			#endif // WCS_SECURITY_NETWORK_DONGLE

			if(RNBOsproQuery(SPRO, 40, (RBP_VOID)SPQuery, (RBP_VOID)SPResp, (RBP_DWORD)&DResp, 4) == SP_SUCCESS)
				{
				SetDonglePresentCached(1); // only indicates a key is present, not that it's authorized in any way
				CheckSerial();
				if(FullSerial)
					{
					if(Code == NULL)
						{
						if((SPResp[0] == 0x9B) && (SPResp[1] == 0xA1) && (SPResp[2] == 0xAA) && (SPResp[3] == 0x48))
							{
							return(1);
							} // if
						else
							{
							return(0);
							} // else
						} // if
					Data32 = Response32 = NULL;

					Data32 = SPResp[0];
					Data32 = Data32 << 24;
					Response32 |= Data32;

					Data32 = SPResp[1];
					Data32 = Data32 << 16;
					Response32 |= Data32;

					Data32 = SPResp[2];
					Data32 = Data32 << 8;
					Response32 |= Data32;

					Response32 |= SPResp[3];
					return(Response32);
					} // if
				} // if
			} // if
		Sleep(100);
		} // for
	} // if

return(0);
#else // !DISABLE_DONGLE
return(1);
#endif // !DISABLE_DONGLE

} // Security::CheckDongle



unsigned long Security::CheckSerial(void)
{
#ifndef DISABLE_DONGLE

if(!FullSerial)
	{
	if(InitDongle())
		{
		if(FindDongle())
			{
			if(RNBOsproRead(SPRO, 10, &SerA) == SP_SUCCESS)
				{
				if(RNBOsproRead(SPRO, 16, &SerB) == SP_SUCCESS)
					{
					FullSerial = SerA;
					FullSerial = (FullSerial << 16);
					FullSerial |= SerB;
					} // if
				} // if
			} // if
		} // if
	} // if
return(FullSerial);
#else // !DISABLE_DONGLE
return(1); // serial number 1
#endif // !DISABLE_DONGLE

} // Security::CheckSerial


unsigned long Security::CheckKeySerial(void)
{
#ifndef DISABLE_DONGLE
WORD KS;
unsigned long KeySerial;
KS = 0;
KeySerial = 0;

if(InitDongle())
	{
	if(FindDongle())
		{
		if(RNBOsproRead(SPRO, 0, &KS) == SP_SUCCESS)
			{
			KeySerial = KS;
			} // if
		} // if
	} // if

return(KeySerial);
#else // !DISABLE_DONGLE
return(1); // Key serial #1
#endif // !DISABLE_DONGLE

} // Security::CheckKeySerial


unsigned long Security::WriteAuthField(WORD NewAuth, int FieldLoc)
{
#ifndef DISABLE_DONGLE
WORD WritePass = 0xA52C;

if(InitDongle())
	{
	// Code to write new authorization code to Auth field
	if(FindDongle())
		{
		if(RNBOsproWrite(SPRO, WritePass, FieldLoc, NewAuth, 0) == SP_SUCCESS)
			{
			return(NewAuth);
			} // if
		} // if
	} // if
#endif // !DISABLE_DONGLE

return(0);
} // Security::WriteAuthField


unsigned long Security::ErrorCheckNewAuthorizationValue(unsigned long NewAuthValue)
{
// parameters copied from WCS.cpp around line 781
#ifdef WCS_BUILD_VNS
return(ValidateAuthFieldValue(0, (unsigned char *)"EW", NewAuthValue));
#else // !WCS_BUILD_VNS
return(ValidateAuthFieldValue(0, (unsigned char *)"S"), NewAuthValue));
#endif // !WCS_BUILD_VNS
} // Security::ErrorCheckNewAuthorizationValue



unsigned long Security::CheckAuthField(unsigned char RequiredEditionCode, unsigned char *PermittedEditionCodes, int FieldLoc)
{
unsigned long AppVerNum;

#ifdef WCS_BUILD_VNS
AppVerNum = CalcVNSVerNum(APP_VERS_NUM); // make it impossible to practically recycle codes from WCS to VNS
#else // !WCS_BUILD_VNS
AppVerNum = CalcWCSVerNum(APP_VERS_NUM);
#endif // !WCS_BUILD_VNS

return(CheckAuthFieldVersion(RequiredEditionCode, PermittedEditionCodes, FieldLoc, AppVerNum));

} // Security::CheckAuthField


unsigned long Security::ValidateAuthFieldValue(unsigned char RequiredEditionCode, unsigned char *PermittedEditionCodes, unsigned long AuthField)
{
unsigned long AppVerNum;

#ifdef WCS_BUILD_VNS
AppVerNum = CalcVNSVerNum(APP_VERS_NUM); // make it impossible to practically recycle codes from WCS to VNS
#else // !WCS_BUILD_VNS
AppVerNum = CalcWCSVerNum(APP_VERS_NUM);
#endif // !WCS_BUILD_VNS

return(ValidateAuthFieldVersionValue(RequiredEditionCode, PermittedEditionCodes, AuthField, AppVerNum));

} // Security::ValidateAuthFieldValue



unsigned long Security::CheckAuthFieldVersion(unsigned char RequiredEditionCode, unsigned char *PermittedEditionCodes, int FieldLoc, unsigned long AppVerNum)
{
#ifndef DISABLE_DONGLE
unsigned long KeySerial = 0, AuthField = 0;

if(InitDongle())
	{
	// Verify Auth field
	KeySerial = CheckKeySerial();
	if(!KeySerial || KeySerial == 1) return(0); // no key present or network authorized
	AuthField = ReadAuthField(FieldLoc);
	return(ValidateAuthFieldVersionValue(RequiredEditionCode, PermittedEditionCodes, AuthField, AppVerNum));
	} // if

return(0);

#else // !DISABLE_DONGLE

return(1); // might need to emulate somehow

#endif // !DISABLE_DONGLE

} // Security::CheckAuthFieldVersion

unsigned long Security::ValidateAuthFieldVersionValue(unsigned char RequiredEditionCode, unsigned char *PermittedEditionCodes, unsigned long AuthField, unsigned long AppVerNum)
{
#ifndef DISABLE_DONGLE
unsigned long KeySerial = 0, ChallengeCode = 0, VersionLoop, VersionPlusEdition, AuthResponse;
WORD AuthSixteen;
unsigned char EditionCode;

if(InitDongle())
	{
	// Verify Auth field
	KeySerial = CheckKeySerial();
	if(!KeySerial || KeySerial == 1) return(0); // no key present or network authorized
	for(VersionLoop = AppVerNum; VersionLoop < (AppVerNum + WCS_SECURITY_FORWARD_VERSIONS + 1); VersionLoop++)
		{
		AuthResponse = 0;
		VersionPlusEdition = VersionLoop ^ RequiredEditionCode; // Combine RequiredEdition indicator (if present, NOOP if RequiredEdition = 0)
		ChallengeCode = (VersionPlusEdition << 16) | (~VersionPlusEdition << 24);
		ChallengeCode |= KeySerial;
		AuthResponse = CheckDongle(ChallengeCode);
		AuthSixteen = (WORD)(AuthResponse & 0xffff);
		if(AuthSixteen == AuthField)
			{
			return(1);
			} // if
		} // for
	// Check for other editions
	if(PermittedEditionCodes)
		{
		// loop through permitted edition codes, testing each
		while(PermittedEditionCodes[0])
			{
			EditionCode = PermittedEditionCodes[0];
			for(VersionLoop = AppVerNum; VersionLoop < (AppVerNum + WCS_SECURITY_FORWARD_VERSIONS + 1); VersionLoop++)
				{
				AuthResponse = 0;
				VersionPlusEdition = VersionLoop ^ EditionCode; // Combine Edition indicator
				ChallengeCode = (VersionPlusEdition << 16) | (~VersionPlusEdition << 24);
				ChallengeCode |= KeySerial;
				AuthResponse = CheckDongle(ChallengeCode);
				AuthSixteen = (WORD)(AuthResponse & 0xffff);
				if(AuthSixteen == AuthField)
					{
					return(1);
					} // if
				} // for
			PermittedEditionCodes++; // try next permitted edition code
			} // for
		} // if
	} // if

return(0);

#else // !DISABLE_DONGLE

return(1); // might need to emulate somehow

#endif // !DISABLE_DONGLE

} // Security::ValidateAuthFieldVersionValue







unsigned long Security::ReadAuthField(int FieldLoc)
{
#ifndef DISABLE_DONGLE
WORD Auth;
unsigned long AuthField;
Auth = 0;
AuthField = 0;

if(InitDongle())
	{
	if(FindDongle())
		{
		if(RNBOsproRead(SPRO, FieldLoc, &Auth) == SP_SUCCESS)
			{
			AuthField = Auth;
			} // if
		} // if
	} // if

return(AuthField);
#else // !DISABLE_DONGLE

return(0); // may need to emulate somehow
#endif // !DISABLE_DONGLE

} // Security::ReadAuthField


unsigned long Security::DecodeAuthString(char *AString)
{
return(atoi(AString));
} // Security::DecodeAuthString

char *Security::EncodeAuthString(unsigned long ACode)
{
sprintf((char *)CodedString, "%d", ACode);
return((char *)CodedString);
} // Security::EncodeAuthString


unsigned long KaseyKasem[] =
{
70048,
70083,
77014,
110288,
0 // must end with 0
}; // KaseyKasem (the Hit List)

int Security::CheckHL(void)
{
int TheUsualSuspects;
unsigned long Perp;

Perp = CheckSerial();

for(TheUsualSuspects = 0; KaseyKasem[TheUsualSuspects]; TheUsualSuspects++)
	{
	if(Perp == KaseyKasem[TheUsualSuspects])
		{
		GlobalApp->Sentinal->WriteAuthField((WORD)0, WCS_SECURITY_AUTH_FIELD_A); // Mwaaaa haaah haah haah hah ha haaaaaaaa...
		GlobalApp->Sentinal->WriteAuthField((WORD)0, WCS_SECURITY_AUTH_FIELD_B); // Mwaaaa haaah haah haah hah ha haaaaaaaa...
		GlobalApp->Sentinal->WriteAuthField((WORD)0, WCS_SECURITY_AUTH_FIELD_RTX); // Mwaaaa haaah haah haah hah ha haaaaaaaa...
		return(1);
		} // if
	} // for

return(0);
} // Security::CheckHL


unsigned long Security::DoAuth(unsigned char EditionCode, int FieldLoc)
{
char AuthText[512], AuthPrompt[512], *AuthCode;
unsigned char EditionStr[5];

EditionStr[0] = EditionStr[1] = EditionStr[2] = EditionStr[3] = EditionStr[4] = NULL;

if(EditionCode)
	{
	EditionStr[0] = ' ';
	EditionStr[1] = EditionCode;
	EditionStr[2] = 'E';
	} // if

AuthCode = EncodeAuthString(CheckKeySerial());
sprintf(AuthPrompt, "This hardware key has not yet been authorized\nto run %s Version %s%s.\n\nContact a 3D Nature authorization agent\nvia E-Mail at " APP_AUTHEMAIL "\nor during business hours at " APP_AUTHPHONE ".\n\nYou will need to supply the following:\nUpgrade code:%s%s Serial:0x%08x\n\nClick OK to enter your authorization code.", APP_TITLE, APP_VERS, EditionStr, AuthCode, EditionStr, CheckSerial());
UserMessageOK("Authorization Required", AuthPrompt);
sprintf(AuthPrompt, "Upgrade code: %s%s\nSerial: 0x%08x\n\nPlease enter your authorization code:", AuthCode, &EditionStr[1], CheckSerial());
AuthText[0] = 0;
if(GetInputString(AuthPrompt, WCS_REQUESTER_POSDIGITS_ONLY, AuthText))
	{
	if(WriteAuthField((WORD)GlobalApp->Sentinal->DecodeAuthString(AuthText), FieldLoc))
		{
		UserMessageOK("Authorization Updated", "Program will now exit. Please restart " APP_SHORTTITLE ".");
		} // if
	else
		{
		UserMessageOK("Authorization Write Failure", "Contact 3D Nature. Program will now exit.");
		} // if
	GlobalApp->SetTerminate(1);
	return(1);
	} // if
else
	{
	GlobalApp->SetTerminate(1);
	} // else

return(0);
} // Security::DoAuth


#ifdef WCS_BUILD_RTX


// armored auth packet
// 0x00 version 0
// 0xff magic
// 0x55 magic
// 0xcd more magic
// [16 bits] flagbitson
// [16 bits] notflagbitson (~flagbitson) (a check word)
// [16 bits] basic RTX authorization field value
// [16 bits] keyID that this code was created for


// returns 1 if all ok, 0 if not, and ~0 if code is not for this key
unsigned long Security::HandleArmoredAuth(char *InputString)
{
unsigned char OutputBuf[32]; // really only use 16, but we over-pad
unsigned long KeySer = 0;
unsigned short IntendedKeySer;

if(InputString && InputString[0])
	{
	// trim leading and trailing whitespace
	if(isspace(InputString[0]))
		{
		InputString = SkipPastNextSpace(InputString); // handles skipping over tabs too
		} // if
	if(InputString[0])
		{
		InputString = TrimTrailingSpaces(InputString);
		} // if
	if(DecodeArmoredAuthString((unsigned char *)InputString, OutputBuf))
		{
		KeySer = CheckKeySerial();
		// check version and magic bits
		if(OutputBuf[0] == 0) // version 0 are SX codes
			{
			if(OutputBuf[1] == 0xff && OutputBuf[2] == 0x55 && OutputBuf[3] == 0xcd)
				{
				unsigned short FlagBitsOn, NotFlagBitsOn, RTXVersionAuth;
				memcpy(&FlagBitsOn, &OutputBuf[4], 2);
				memcpy(&NotFlagBitsOn, &OutputBuf[6], 2);
				memcpy(&RTXVersionAuth, &OutputBuf[8], 2);
				memcpy(&IntendedKeySer, &OutputBuf[10], 2);
				// IntendedKeySer
				#ifdef BYTEORDER_BIGENDIAN
				SimpleEndianFlip16U(FlagBitsOn, &FlagBitsOn);
				SimpleEndianFlip16U(NotFlagBitsOn, &NotFlagBitsOn);
				SimpleEndianFlip16U(RTXVersionAuth, &RTXVersionAuth);
				SimpleEndianFlip16U(IntendedKeySer, &IntendedKeySer);
				#endif // BYTEORDER_BIGENDIAN
				if((FlagBitsOn ^ 0xffff) == NotFlagBitsOn)
					{
					if(IntendedKeySer == KeySer)
						{
						unsigned long AuthField = 0;
						unsigned long NewAuthField = 0;
						AuthField = ReadAuthField(WCS_SECURITY_RTX_OPT_FIELD);

						//NewAuthField = AuthField | FlagBitsOn;
						NewAuthField = FlagBitsOn; // this allows us to turn them off as well, but we have to know which ones were already on when we gen the code

						// write the aforementioned flag bits to the key
						WriteAuthField((unsigned short)NewAuthField, WCS_SECURITY_RTX_OPT_FIELD);

						// write the basic RTX authorization field
						WriteAuthField((WORD)RTXVersionAuth, WCS_SECURITY_AUTH_FIELD_RTX);
						return(1);
						} // if
					else
						{ // auth packet not for this keyID
						return(0xffffffff);
						} // else
					} // if
				else
					{
					// invalid packet
					} // else
				} // if
			} // if
		else if(OutputBuf[0] == 1) // Version 1 is network authorization
			{
			if(OutputBuf[1] == 0xff && OutputBuf[2] == 0x55 && OutputBuf[3] == 0xcd)
				{
				unsigned short AuthBits, NotAuthBits /*, RTXVersionAuth */;
				memcpy(&AuthBits, &OutputBuf[4], 2);
				memcpy(&NotAuthBits, &OutputBuf[6], 2);
				//memcpy(&RTXVersionAuth, &OutputBuf[8], 2);
				memcpy(&IntendedKeySer, &OutputBuf[10], 2);
				// IntendedKeySer
				#ifdef BYTEORDER_BIGENDIAN
				SimpleEndianFlip16U(AuthBits, &AuthBits);
				SimpleEndianFlip16U(NotAuthBits, &NotAuthBits);
				//SimpleEndianFlip16U(RTXVersionAuth, &RTXVersionAuth);
				SimpleEndianFlip16U(IntendedKeySer, &IntendedKeySer);
				#endif // BYTEORDER_BIGENDIAN
				if((AuthBits ^ 0xffff) == NotAuthBits)
					{
					if(IntendedKeySer == KeySer)
						{
						unsigned long NewAuthField = 0;
						NewAuthField = AuthBits;

						// write the aforementioned flag bits to the key
						WriteAuthField((unsigned short)NewAuthField, WCS_SECURITY_AUTH_FIELD_NETKEY);

						return(1);
						} // if
					else
						{ // auth packet not for this keyID
						return(0xffffffff);
						} // else
					} // if
				else
					{
					// invalid packet
					} // else
				} // if
			} // else
		} // if
	} // if

return(0);
} // Security::HandleArmoredAuth




#endif // WCS_BUILD_RTX

#endif // !WCS_BUILD_DEMO



#ifdef WCS_BUILD_DEMO
void Security::TimeCheck(void)
{
time_t now;

time(&now);

if((now - DTime) > WCS_SECURITY_DEMO_ANNOY_SECONDS)
	{
	DTime = now;
	UserMessageDemo("");
	} // if

} // Security::TimeCheck
#endif // WCS_BUILD_DEMO


#ifdef WCS_FORESTRY_WIZARD
unsigned long Security::CheckAuthFieldForestry()
{

#ifdef WCS_BUILD_DEMO
// We want the Demo version to be able to show off the Forestry Edition
return(1);
#else // !WCS_BUILD_DEMO
if(CheckAuthField('W', NULL, WCS_SECURITY_AUTH_FIELD_B))
	{
	return (1);
	} // if
#endif // !WCS_BUILD_DEMO

return(0);
} // Security::CheckAuthFieldForestry
#endif // WCS_FORESTRY_WIZARD


unsigned long Security::CheckAuthFieldSX2(void)
{
#ifdef WCS_BUILD_SX2
#ifdef WCS_BUILD_DEMO
// We want the Demo version to be able to show off the SX2
return(1);
#else // !WCS_BUILD_DEMO
//if(CheckAuthField('W', NULL, WCS_SECURITY_AUTH_FIELD_B))
//	{
	return (CheckAuthFieldRTX(2));
//	} // if
#endif // !WCS_BUILD_DEMO
#endif // WCS_BUILD_SX2

//return(0);
} // Security::CheckAuthFieldSX2


unsigned long Security::CheckAuthSE()
{

#ifdef WCS_BUILD_DEMO
// We don't want the Demo version to be able to show off Scripting
return(0);
#else // !WCS_BUILD_DEMO
if(CheckAuthFieldVersion('S', NULL, WCS_SECURITY_AUTH_FIELD_A, CalcWCSVerNum(5))) // WCS_SECURITY_AUTH_FIELD_A is the WCS slot, WCS 5 or later supported SE
	{
	return (1);
	} // if
#endif // !WCS_BUILD_DEMO

return(0);
} // Security::CheckAuthSE


#ifdef WCS_BUILD_RTX

#if (!defined(DISABLE_DONGLE) && !defined(WCS_BUILD_DEMO))
unsigned long Security::CheckAuthFieldRTX(unsigned long Version)
{
unsigned long KeySerial = 0, AuthField = 0, ChallengeCode = 0, VersionLoop, VersionPlusEdition, AuthResponse, AppVerNum;
WORD AuthSixteen;

#ifdef WCS_BUILD_VNS
AppVerNum = Version + 200; // make it impossible to practically recycle codes from WCS to VNS to RTX. VNSRTX1=201
#else // !WCS_BUILD_VNS must be WCS
AppVerNum = Version + 100; // make it impossible to practically recycle codes from WCS to VNS to RTX. WCSRTX1=101
#endif // !WCS_BUILD_VNS must be WCS


// CXH NOTE: As of 1/17/05 I have realized that for some reason we've been issuing all VNS-SX codes for all SX-1 authorizations.
// I'm not sure why this is, and even the SX authorization generator program hitherto has no button to indicate whether you want
// to generate a WCS or VNS code.

if(InitDongle())
	{
	// Verify Auth field
	KeySerial = CheckKeySerial();
	if(!KeySerial) return(0);
	AuthField = ReadAuthField(WCS_SECURITY_AUTH_FIELD_RTX);
	for(VersionLoop = AppVerNum; VersionLoop < (AppVerNum + WCS_SECURITY_FORWARD_VERSIONS + 1); VersionLoop++)
		{
		AuthResponse = 0;
		VersionPlusEdition = VersionLoop;
		ChallengeCode = (VersionPlusEdition << 16) | (~VersionPlusEdition << 24);
		ChallengeCode |= KeySerial;
		AuthResponse = CheckDongle(ChallengeCode);
		AuthSixteen = (WORD)(AuthResponse & 0xffff);
		if(AuthSixteen == AuthField)
			{
			return(1);
			} // if
		} // for

	#ifndef WCS_BUILD_VNS // WCS builds only
	// WCSRTX1 will accept a code for VNSRTX1 for ease of testing, but not vice versa
	AppVerNum = Version + 200; // make it impossible to practically recycle codes from WCS to VNS to RTX. VNSRTX1=201
	for(VersionLoop = AppVerNum; VersionLoop < (AppVerNum + WCS_SECURITY_FORWARD_VERSIONS + 1); VersionLoop++)
		{
		AuthResponse = 0;
		VersionPlusEdition = VersionLoop;
		ChallengeCode = (VersionPlusEdition << 16) | (~VersionPlusEdition << 24);
		ChallengeCode |= KeySerial;
		AuthResponse = CheckDongle(ChallengeCode);
		AuthSixteen = (WORD)(AuthResponse & 0xffff);
		if(AuthSixteen == AuthField)
			{
			return(1);
			} // if
		} // for
	#endif // !WCS_BUILD_VNS (WCS builds only)

	} // if

return(0);
} // Security::CheckAuthFieldRTX
#endif // neither DEMO nor DISABLE_DONGLE


#ifdef DISABLE_DONGLE
unsigned long Security::CheckAuthFieldRTX( long unsigned int Version )
{

#ifdef WCS_BUILD_DEMO
return (1);	// demo needs to be able to show SX
#else // !WCS_BUILD_DEMO
return(0); // might need to emulate somehow
#endif // !WCS_BUILD_DEMO

} // Security::CheckAuthFieldRTX
#endif // DISABLE_DONGLE


unsigned long Security::CheckFormatRTX(unsigned long FormatEnum) // not the flag, the format
{
unsigned short FormatFlag = 0, FormatAvailable = 1;

// translate format to flag, or result if a non-optional format
switch(FormatEnum)
	{
	case WCS_SECURITY_RTX_FORMAT_NVE:
	case WCS_SECURITY_RTX_FORMAT_VRML_WEB:
	case WCS_SECURITY_RTX_FORMAT_3DS:
	case WCS_SECURITY_RTX_FORMAT_LW:
	case WCS_SECURITY_RTX_FORMAT_VTP:
		{
		return(1);
		} // 
	case WCS_SECURITY_RTX_FORMAT_STL:
		{
		FormatFlag = WCS_SECURITY_RTX_FLAG_VRMLSTL;
		#ifndef WCS_BUILD_VNS
		FormatAvailable = 0;	// only available in VNS
		#endif // ! WCS_BUILD_VNS
		break;
		} // 
	case WCS_SECURITY_RTX_FORMAT_VRMLSTL:
		{
		FormatFlag = WCS_SECURITY_RTX_FLAG_VRMLSTL;
		#ifndef WCS_BUILD_VNS
		FormatAvailable = 0;	// only available in VNS
		#endif // ! WCS_BUILD_VNS
		break;
		} // 
	case WCS_SECURITY_RTX_FORMAT_OPENFLIGHT:
		{
		FormatFlag = WCS_SECURITY_RTX_FLAG_OPENFLIGHT;
		#ifndef WCS_BUILD_VNS
		FormatAvailable = 0;	// only available in VNS
		#endif // ! WCS_BUILD_VNS
		break;
		} // 
	case WCS_SECURITY_RTX_FORMAT_TERRAPAGE:
		{
		FormatFlag = WCS_SECURITY_RTX_FLAG_TERRAPAGE;
		FormatAvailable = 0; // not available yet
		break;
		} // 
	case WCS_SECURITY_RTX_FORMAT_MAYA:
		{
		FormatFlag = WCS_SECURITY_RTX_FLAG_MAYA;
		FormatAvailable = 0; // not available yet
		break;
		} // 
	case WCS_SECURITY_RTX_FORMAT_SOFTIMAGE:
		{
		FormatFlag = WCS_SECURITY_RTX_FLAG_SOFTIMAGE;
		FormatAvailable = 0; // not available yet
		break;
		} // 
	case WCS_SECURITY_RTX_FORMAT_GIS:
		{
		FormatAvailable = 0;	// only available in VNS+SX2, when properly authorized
		#ifdef WCS_BUILD_SX2
		#ifdef WCS_BUILD_VNS
		#ifndef WCS_BUILD_DEMO
		if(CheckAuthFieldSX2()) return(1);
		#else // WCS_BUILD_DEMO
		return(1); // show it, it's only a demo
		#endif // !WCS_BUILD_DEMO
		#endif // WCS_BUILD_VNS
		#endif // WCS_BUILD_SX2
		break;
		} // 
	case WCS_SECURITY_RTX_FORMAT_WCSVNS:
		{
		FormatAvailable = 0;	// only available in SX2, when properly authorized
		#ifdef WCS_BUILD_SX2
		#ifndef WCS_BUILD_DEMO
		if(CheckAuthFieldSX2()) return(1);
		#else // WCS_BUILD_DEMO
		return(1); // show it, it's only a demo
		#endif // !WCS_BUILD_DEMO
		#endif // WCS_BUILD_SX2
		break;
		} // 
	case WCS_SECURITY_RTX_FORMAT_COLLADA:
		{
		FormatAvailable = 0;	// only available in SX2, when properly authorized
		#ifdef WCS_BUILD_SX2
		#ifndef WCS_BUILD_DEMO
		if(CheckAuthFieldSX2()) return(1);
		#else // WCS_BUILD_DEMO
		return(1); // show it, it's only a demo (actually, hidden elsewhere in ExportControlGUI::FormatSpecificAuth until ready
		#endif // !WCS_BUILD_DEMO
		#endif // WCS_BUILD_SX2
		break;
		} // 
	case WCS_SECURITY_RTX_FORMAT_FBX:
		{
		FormatAvailable = 0;	// only available in SX2, when properly authorized
		#ifdef WCS_BUILD_SX2
		#ifndef WCS_BUILD_DEMO
		if(CheckAuthFieldSX2()) return(1);
		#else // WCS_BUILD_DEMO
		return(1); // show it, it's only a demo
		#endif // !WCS_BUILD_DEMO
		#endif // WCS_BUILD_SX2
		break;
		} // 
	case WCS_SECURITY_RTX_FORMAT_KML:
		{
		FormatAvailable = 0;	// only available in SX2, when properly authorized
		#ifdef WCS_BUILD_SX2
		#ifndef WCS_BUILD_DEMO
		if(CheckAuthFieldSX2()) return(1);
		#else // WCS_BUILD_DEMO
		return(1); // show it, it's only a demo
		#endif // !WCS_BUILD_DEMO
		#endif // WCS_BUILD_SX2
		break;
		} // 
	case WCS_SECURITY_RTX_FORMAT_WW:
		{
		FormatAvailable = 0;	// only available in SX2, when properly authorized
		#ifdef WCS_BUILD_SX2
		#ifndef WCS_BUILD_DEMO
		if(CheckAuthFieldSX2()) return(1);
		#else // WCS_BUILD_DEMO
		return(1); // show it, it's only a demo
		#endif // !WCS_BUILD_DEMO
		#endif // WCS_BUILD_SX2
		break;
		} // 
	default:
		{
		return(0); // dunno what format you're talking about
		} // default
	} // switch

if(!FormatFlag)
	{
	return(0); // handle unknown errors
	} // if

#ifndef DISABLE_DONGLE
unsigned long AuthField = 0;

#ifdef WCS_BUILD_DEMO

if (FormatAvailable)
	return (1);

#else // WCS_BUILD_DEMO

if (! FormatAvailable)
	return (0);

if(InitDongle())
	{
	// Check RTX format auth field
	AuthField = ReadAuthField(WCS_SECURITY_RTX_OPT_FIELD);
	if((AuthField & FormatFlag) == FormatFlag)
		{
		return(1);
		} // if
	} // if

#endif // WCS_BUILD_DEMO

return(0);

#else // !DISABLE_DONGLE

return(0); // might need to emulate somehow

#endif // !DISABLE_DONGLE



} // Security::CheckFormatRTX

unsigned long Security::CheckImageFormatRTX(const char *FormatName)
{

#ifndef WCS_BUILD_VNS // WCS doesn't have ECW DLL support
if (! stricmp(FormatName, "ERMapper ECW"))
	return (0);
if (! stricmp(FormatName, "JPEG 2000"))
	return (0);
if (! stricmp(FormatName, "Lossless JPEG 2000"))
	return (0);
if (! stricmp(FormatName, "JPEG 2000 DEM"))
	return (0);
if (! stricmp(FormatName, "Lossless JPEG 2000 DEM"))
	return (0);
#endif // !WCS_BUILD_VNS

#ifndef DISABLE_DONGLE
if (! CheckAuthFieldSX2())
	{
	if (! stricmp(FormatName, "ERMapper ECW"))
		return (0);
	if (! stricmp(FormatName, "JPEG 2000"))
		return (0);
	if (! stricmp(FormatName, "Lossless JPEG 2000"))
		return (0);
	if (! stricmp(FormatName, "JPEG 2000 DEM"))
		return (0);
	if (! stricmp(FormatName, "Lossless JPEG 2000 DEM"))
		return (0);
	} // if
#endif // DISABLE_DONGLE

return (1);

} // Security::CheckImageFormatRTX

#endif // WCS_BUILD_RTX
