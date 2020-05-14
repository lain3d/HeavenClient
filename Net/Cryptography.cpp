//////////////////////////////////////////////////////////////////////////////////
//	This file is part of the continued Journey MMORPG client					//
//	Copyright (C) 2015-2019  Daniel Allendorf, Ryan Payton						//
//																				//
//	This program is free software: you can redistribute it and/or modify		//
//	it under the terms of the GNU Affero General Public License as published by	//
//	the Free Software Foundation, either version 3 of the License, or			//
//	(at your option) any later version.											//
//																				//
//	This program is distributed in the hope that it will be useful,				//
//	but WITHOUT ANY WARRANTY; without even the implied warranty of				//
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the				//
//	GNU Affero General Public License for more details.							//
//																				//
//	You should have received a copy of the GNU Affero General Public License	//
//	along with this program.  If not, see <https://www.gnu.org/licenses/>.		//
//////////////////////////////////////////////////////////////////////////////////
#include "Cryptography.h"

namespace ms
{
	Cryptography::Cryptography(const int8_t* handshake)
	{
#ifdef USE_CRYPTO
		for (size_t i = 0; i < HEADER_LENGTH; i++)
			sendiv[i] = handshake[i + 7];

		for (size_t i = 0; i < HEADER_LENGTH; i++)
			recviv[i] = handshake[i + 11];
#endif
	}

	Cryptography::Cryptography()
	{}

	Cryptography::~Cryptography()
	{}

	void Cryptography::encrypt(int8_t* bytes, size_t length)
	{
#ifdef USE_CRYPTO
		mapleencrypt(bytes, length);
		aesofb(bytes, length, sendiv);
#endif
	}

	void Cryptography::decrypt(int8_t* bytes, size_t length)
	{
#ifdef USE_CRYPTO
		aesofb(bytes, length, recviv);
		mapledecrypt(bytes, length);
#endif
	}

	void Cryptography::create_header(int8_t* buffer, size_t length) const
	{
#ifdef USE_CRYPTO
		static const uint8_t MAPLEVERSION = 83;

		size_t a = ((sendiv[3] << 8) | sendiv[2]) ^MAPLEVERSION;
		size_t b = a ^length;

		buffer[0] = static_cast<int8_t>(a % 0x100);
		buffer[1] = static_cast<int8_t>(a / 0x100);
		buffer[2] = static_cast<int8_t>(b % 0x100);
		buffer[3] = static_cast<int8_t>(b / 0x100);
#else
		int32_t length = static_cast<int32_t>(slength);

		for (int32_t i = 0; i < HEADERLEN; i++)
		{
			buffer[i] = static_cast<int8_t>(length);
			length = length >> 8;
		}
#endif
	}

	size_t Cryptography::check_length(const int8_t* bytes) const
	{
#ifdef USE_CRYPTO
		uint32_t headermask = 0;

		for (size_t i = 0; i < 4; i++)
			headermask |= static_cast<uint8_t>(bytes[i]) << (8 * i);

		return static_cast<int16_t>((headermask >> 16) ^ (headermask & 0xFFFF));
#else
		size_t length = 0;

		for (int32_t i = 0; i < HEADERLEN; i++)
			length += static_cast<uint8_t>(bytes[i]) << (8 * i);

		return length;
#endif
	}

	void Cryptography::mapleencrypt(int8_t* bytes, size_t length) const
	{
		for (size_t j = 0; j < 3; j++)
		{
			int8_t remember = 0;
			int8_t datalen = static_cast<int8_t>(length & 0xFF);

			for (size_t i = 0; i < length; i++)
			{
				int8_t cur = (rollleft(bytes[i], 3) + datalen) ^remember;
				remember = cur;
				cur = rollright(cur, static_cast<int32_t>(datalen) & 0xFF);
				bytes[i] = static_cast<int8_t>((~cur) & 0xFF) + 0x48;
				datalen--;
			}

			remember = 0;
			datalen = static_cast<int8_t>(length & 0xFF);

			for (size_t i = length; i--;)
			{
				int8_t cur = (rollleft(bytes[i], 4) + datalen) ^remember;
				remember = cur;
				bytes[i] = rollright(cur ^ 0x13, 3);
				datalen--;
			}
		}
	}

	void Cryptography::mapledecrypt(int8_t* bytes, size_t length) const
	{
		for (size_t i = 0; i < 3; i++)
		{
			uint8_t remember = 0;
			uint8_t datalen = static_cast<uint8_t>(length & 0xFF);

			for (size_t j = length; j--;)
			{
				uint8_t cur = rollleft(bytes[j], 3) ^0x13;
				bytes[j] = rollright((cur ^ remember) - datalen, 4);
				remember = cur;
				datalen--;
			}

			remember = 0;
			datalen = static_cast<uint8_t>(length & 0xFF);

			for (size_t j = 0; j < length; j++)
			{
				uint8_t cur = (~(bytes[j] - 0x48)) & 0xFF;
				cur = rollleft(cur, static_cast<int32_t>(datalen) & 0xFF);
				bytes[j] = rollright((cur ^ remember) - datalen, 3);
				remember = cur;
				datalen--;
			}
		}
	}

	void Cryptography::updateiv(uint8_t* iv) const
	{
		static const uint8_t maplebytes[256] =
			{
				0xEC, 0x3F, 0x77, 0xA4, 0x45, 0xD0, 0x71, 0xBF, 0xB7, 0x98, 0x20, 0xFC, 0x4B, 0xE9, 0xB3, 0xE1,
				0x5C, 0x22, 0xF7, 0x0C, 0x44, 0x1B, 0x81, 0xBD, 0x63, 0x8D, 0xD4, 0xC3, 0xF2, 0x10, 0x19, 0xE0,
				0xFB, 0xA1, 0x6E, 0x66, 0xEA, 0xAE, 0xD6, 0xCE, 0x06, 0x18, 0x4E, 0xEB, 0x78, 0x95, 0xDB, 0xBA,
				0xB6, 0x42, 0x7A, 0x2A, 0x83, 0x0B, 0x54, 0x67, 0x6D, 0xE8, 0x65, 0xE7, 0x2F, 0x07, 0xF3, 0xAA,
				0x27, 0x7B, 0x85, 0xB0, 0x26, 0xFD, 0x8B, 0xA9, 0xFA, 0xBE, 0xA8, 0xD7, 0xCB, 0xCC, 0x92, 0xDA,
				0xF9, 0x93, 0x60, 0x2D, 0xDD, 0xD2, 0xA2, 0x9B, 0x39, 0x5F, 0x82, 0x21, 0x4C, 0x69, 0xF8, 0x31,
				0x87, 0xEE, 0x8E, 0xAD, 0x8C, 0x6A, 0xBC, 0xB5, 0x6B, 0x59, 0x13, 0xF1, 0x04, 0x00, 0xF6, 0x5A,
				0x35, 0x79, 0x48, 0x8F, 0x15, 0xCD, 0x97, 0x57, 0x12, 0x3E, 0x37, 0xFF, 0x9D, 0x4F, 0x51, 0xF5,
				0xA3, 0x70, 0xBB, 0x14, 0x75, 0xC2, 0xB8, 0x72, 0xC0, 0xED, 0x7D, 0x68, 0xC9, 0x2E, 0x0D, 0x62,
				0x46, 0x17, 0x11, 0x4D, 0x6C, 0xC4, 0x7E, 0x53, 0xC1, 0x25, 0xC7, 0x9A, 0x1C, 0x88, 0x58, 0x2C,
				0x89, 0xDC, 0x02, 0x64, 0x40, 0x01, 0x5D, 0x38, 0xA5, 0xE2, 0xAF, 0x55, 0xD5, 0xEF, 0x1A, 0x7C,
				0xA7, 0x5B, 0xA6, 0x6F, 0x86, 0x9F, 0x73, 0xE6, 0x0A, 0xDE, 0x2B, 0x99, 0x4A, 0x47, 0x9C, 0xDF,
				0x09, 0x76, 0x9E, 0x30, 0x0E, 0xE4, 0xB2, 0x94, 0xA0, 0x3B, 0x34, 0x1D, 0x28, 0x0F, 0x36, 0xE3,
				0x23, 0xB4, 0x03, 0xD8, 0x90, 0xC8, 0x3C, 0xFE, 0x5E, 0x32, 0x24, 0x50, 0x1F, 0x3A, 0x43, 0x8A,
				0x96, 0x41, 0x74, 0xAC, 0x52, 0x33, 0xF0, 0xD9, 0x29, 0x80, 0xB1, 0x16, 0xD3, 0xAB, 0x91, 0xB9,
				0x84, 0x7F, 0x61, 0x1E, 0xCF, 0xC5, 0xD1, 0x56, 0x3D, 0xCA, 0xF4, 0x05, 0xC6, 0xE5, 0x08, 0x49
			};

		uint8_t mbytes[4] =
			{
				0xF2, 0x53, 0x50, 0xC6
			};

		for (size_t i = 0; i < 4; i++)
		{
			uint8_t ivbyte = iv[i];
			mbytes[0] += maplebytes[mbytes[1] & 0xFF] - ivbyte;
			mbytes[1] -= mbytes[2] ^ maplebytes[ivbyte & 0xFF] & 0xFF;
			mbytes[2] ^= maplebytes[mbytes[3] & 0xFF] + ivbyte;
			mbytes[3] += (maplebytes[ivbyte & 0xFF] & 0xFF) - (mbytes[0] & 0xFF);

			size_t mask = 0;
			mask |= (mbytes[0]) & 0xFF;
			mask |= (mbytes[1] << 8) & 0xFF00;
			mask |= (mbytes[2] << 16) & 0xFF0000;
			mask |= (mbytes[3] << 24) & 0xFF000000;
			mask = (mask >> 0x1D) | (mask << 3);

			for (size_t j = 0; j < 4; j++)
			{
				size_t value = mask >> (8 * j);
				mbytes[j] = static_cast<uint8_t>(value & 0xFF);
			}
		}

		for (size_t i = 0; i < 4; i++)
			iv[i] = mbytes[i];
	}

	int8_t Cryptography::rollleft(int8_t data, size_t count) const
	{
		int32_t mask = (data & 0xFF) << (count % 8);

		return static_cast<int8_t>((mask & 0xFF) | (mask >> 8));
	}

	int8_t Cryptography::rollright(int8_t data, size_t count) const
	{
		int32_t mask = ((data & 0xFF) << 8) >> (count % 8);

		return static_cast<int8_t>((mask & 0xFF) | (mask >> 8));
	}

	void Cryptography::aesofb(int8_t* bytes, size_t length, uint8_t* iv) const
	{
		size_t blocklength = 0x5B0;
		size_t offset = 0;

		while (offset < length)
		{
			uint8_t miv[16];

			for (size_t i = 0; i < 16; i++)
				miv[i] = iv[i % 4];

			size_t remaining = length - offset;

			if (remaining > blocklength)
				remaining = blocklength;

			for (size_t x = 0; x < remaining; x++)
			{
				size_t relpos = x % 16;

				if (relpos == 0)
					aesencrypt(miv);

				bytes[x + offset] ^= miv[relpos];
			}

			offset += blocklength;
			blocklength = 0x5B4;
		}

		updateiv(iv);
	}

	void Cryptography::aesencrypt(uint8_t* bytes) const
	{
		uint8_t round = 0;
		addroundkey(bytes, round);

		for (round = 1; round < 14; round++)
		{
			subbytes(bytes);
			shiftrows(bytes);
			mixcolumns(bytes);
			addroundkey(bytes, round);
		}

		subbytes(bytes);
		shiftrows(bytes);
		addroundkey(bytes, round);
	}

	void Cryptography::addroundkey(uint8_t* bytes, uint8_t round) const
	{
		// This key is already expanded
		// Only works for versions lower than version 118
		static const uint8_t maplekey[256] =
			{
				0x13, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0xB4, 0x00, 0x00, 0x00,
				0x1B, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x33, 0x00, 0x00, 0x00, 0x52, 0x00, 0x00, 0x00,
				0x71, 0x63, 0x63, 0x00, 0x79, 0x63, 0x63, 0x00, 0x7F, 0x63, 0x63, 0x00, 0xCB, 0x63, 0x63, 0x00,
				0x04, 0xFB, 0xFB, 0x63, 0x0B, 0xFB, 0xFB, 0x63, 0x38, 0xFB, 0xFB, 0x63, 0x6A, 0xFB, 0xFB, 0x63,
				0x7C, 0x6C, 0x98, 0x02, 0x05, 0x0F, 0xFB, 0x02, 0x7A, 0x6C, 0x98, 0x02, 0xB1, 0x0F, 0xFB, 0x02,
				0xCC, 0x8D, 0xF4, 0x14, 0xC7, 0x76, 0x0F, 0x77, 0xFF, 0x8D, 0xF4, 0x14, 0x95, 0x76, 0x0F, 0x77,
				0x40, 0x1A, 0x6D, 0x28, 0x45, 0x15, 0x96, 0x2A, 0x3F, 0x79, 0x0E, 0x28, 0x8E, 0x76, 0xF5, 0x2A,
				0xD5, 0xB5, 0x12, 0xF1, 0x12, 0xC3, 0x1D, 0x86, 0xED, 0x4E, 0xE9, 0x92, 0x78, 0x38, 0xE6, 0xE5,
				0x4F, 0x94, 0xB4, 0x94, 0x0A, 0x81, 0x22, 0xBE, 0x35, 0xF8, 0x2C, 0x96, 0xBB, 0x8E, 0xD9, 0xBC,
				0x3F, 0xAC, 0x27, 0x94, 0x2D, 0x6F, 0x3A, 0x12, 0xC0, 0x21, 0xD3, 0x80, 0xB8, 0x19, 0x35, 0x65,
				0x8B, 0x02, 0xF9, 0xF8, 0x81, 0x83, 0xDB, 0x46, 0xB4, 0x7B, 0xF7, 0xD0, 0x0F, 0xF5, 0x2E, 0x6C,
				0x49, 0x4A, 0x16, 0xC4, 0x64, 0x25, 0x2C, 0xD6, 0xA4, 0x04, 0xFF, 0x56, 0x1C, 0x1D, 0xCA, 0x33,
				0x0F, 0x76, 0x3A, 0x64, 0x8E, 0xF5, 0xE1, 0x22, 0x3A, 0x8E, 0x16, 0xF2, 0x35, 0x7B, 0x38, 0x9E,
				0xDF, 0x6B, 0x11, 0xCF, 0xBB, 0x4E, 0x3D, 0x19, 0x1F, 0x4A, 0xC2, 0x4F, 0x03, 0x57, 0x08, 0x7C,
				0x14, 0x46, 0x2A, 0x1F, 0x9A, 0xB3, 0xCB, 0x3D, 0xA0, 0x3D, 0xDD, 0xCF, 0x95, 0x46, 0xE5, 0x51,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
			};

		uint8_t offset = round * 16;

		for (uint8_t i = 0; i < 16; i++)
			bytes[i] ^= maplekey[i + offset];
	}

	void Cryptography::subbytes(uint8_t* bytes) const
	{
		// Rijndael substitution box
		static const uint8_t subbox[256] =
			{
				0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76,
				0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0, 0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0,
				0xB7, 0xFD, 0x93, 0x26, 0x36, 0x3F, 0xF7, 0xCC, 0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15,
				0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07, 0x12, 0x80, 0xE2, 0xEB, 0x27, 0xB2, 0x75,
				0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0, 0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84,
				0x53, 0xD1, 0x00, 0xED, 0x20, 0xFC, 0xB1, 0x5B, 0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF,
				0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9, 0x02, 0x7F, 0x50, 0x3C, 0x9F, 0xA8,
				0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5, 0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2,
				0xCD, 0x0C, 0x13, 0xEC, 0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73,
				0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88, 0x46, 0xEE, 0xB8, 0x14, 0xDE, 0x5E, 0x0B, 0xDB,
				0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C, 0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79,
				0xE7, 0xC8, 0x37, 0x6D, 0x8D, 0xD5, 0x4E, 0xA9, 0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08,
				0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6, 0xE8, 0xDD, 0x74, 0x1F, 0x4B, 0xBD, 0x8B, 0x8A,
				0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E, 0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E,
				0xE1, 0xF8, 0x98, 0x11, 0x69, 0xD9, 0x8E, 0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF,
				0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F, 0xB0, 0x54, 0xBB, 0x16
			};

		for (uint8_t i = 0; i < 16; i++)
			bytes[i] = subbox[bytes[i]];
	}

	void Cryptography::shiftrows(uint8_t* bytes) const
	{
		uint8_t remember = bytes[1];
		bytes[1] = bytes[5];
		bytes[5] = bytes[9];
		bytes[9] = bytes[13];
		bytes[13] = remember;

		remember = bytes[10];
		bytes[10] = bytes[2];
		bytes[2] = remember;

		remember = bytes[3];
		bytes[3] = bytes[15];
		bytes[15] = bytes[11];
		bytes[11] = bytes[7];
		bytes[7] = remember;

		remember = bytes[14];
		bytes[14] = bytes[6];
		bytes[6] = remember;
	}

	uint8_t Cryptography::gmul(uint8_t x) const
	{
		return (x << 1) ^ (0x1B & (uint8_t) ((int8_t) x >> 7));
	}

	void Cryptography::mixcolumns(uint8_t* bytes) const
	{
		for (uint8_t i = 0; i < 16; i += 4)
		{
			uint8_t cpy0 = bytes[i];
			uint8_t cpy1 = bytes[i + 1];
			uint8_t cpy2 = bytes[i + 2];
			uint8_t cpy3 = bytes[i + 3];

			uint8_t mul0 = gmul(bytes[i]);
			uint8_t mul1 = gmul(bytes[i + 1]);
			uint8_t mul2 = gmul(bytes[i + 2]);
			uint8_t mul3 = gmul(bytes[i + 3]);

			bytes[i] = mul0 ^ cpy3 ^ cpy2 ^ mul1 ^ cpy1;
			bytes[i + 1] = mul1 ^ cpy0 ^ cpy3 ^ mul2 ^ cpy2;
			bytes[i + 2] = mul2 ^ cpy1 ^ cpy0 ^ mul3 ^ cpy3;
			bytes[i + 3] = mul3 ^ cpy2 ^ cpy1 ^ mul0 ^ cpy0;
		}
	}
}