
// 3DNCrypt.h
// Public interface to 3DNCrypt library facilities


#ifndef _3DNCRYPT_H
#define _3DNCRYPT_H

int CalcHashOfFile(FILE *InFile, unsigned char *SigOutSixteen, unsigned char MaxInputLength);
int CalcHashOfFileFromName(const char *InFileName, unsigned char *SigOutSixteen, unsigned long MaxInputLength, unsigned char AllowECWHash);
int CalcModifiedHashOfTextFile(FILE *InFile, unsigned char *SigOutSixteen);
int CalcModifiedHashOfTextFileFromName(const char *InFileName, unsigned char *SigOutSixteen);
int CalcModifiedHashOfNVWFile(FILE *InFile, unsigned char *SigOutSixteen);
int CalcModifiedHashOfNVWFileFromName(const char *InFileName, unsigned char *SigOutSixteen);
int FormatAndEncryptPacket(unsigned char *PacketBufferFiftyTwo, unsigned char *SigInSixteen, unsigned char *KeyE, unsigned char *KeyN, unsigned char PacketVersion);
int HashSignAndEncrypt(const char *InFileName, bool AsText, unsigned char *PacketBufferFiftyTwo, unsigned char *KeyE, unsigned char *KeyN, unsigned char AllowECWHash);
const unsigned char *HashSignEncryptAndEncode(const char *InFileName, bool AsText, unsigned char *KeyE, unsigned char *KeyN, unsigned char AllowECWHash);

int GetPacketVersion(const unsigned char *EncodedInputPacket);
int DecryptPacket(const unsigned char *EncodedInputPacket, unsigned char *OutputSignature, unsigned char *KeyD, unsigned char *KeyN);

int BinToBase64(const unsigned char *InputData, int InputSize, unsigned char *OutputData, int *OutputSize);
int Base64ToBin(const unsigned char *InputData, int InputSize, unsigned char *OutputData, int *OutputSize);
int ValidateBase64(const unsigned char *Input);

unsigned long CalcHashOfECW(const char *InName, unsigned char *SigOutSixteen);

#endif // _3DNCRYPT_H
