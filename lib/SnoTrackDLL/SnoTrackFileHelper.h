#pragma once

#include "utf8.h"

class CFileHelper
{
	CFile *m_pFile;

public:
	CFileHelper(CFile *pFile)
		: m_pFile(pFile)
	{
		ASSERT(m_pFile);
	}

	void ReadMagic(unsigned char *magic)
	{
		m_pFile->Read(magic, 8);
	}

	void WriteMagic(const unsigned char *magic)
	{
		m_pFile->Write(magic, 8);
	}

	utf8_string ReadString1()
	{
		utf8_string str;

		unsigned char stringSize1;
		m_pFile->Read(&stringSize1, sizeof(unsigned char));
		utf8_char *p = (utf8_char *)_alloca(stringSize1);
		m_pFile->Read(p, stringSize1);

		str.assign(p, stringSize1);
		return str;
	}

	void WriteString1(const utf8_string &str)
	{
		// We use a std::string here, so that it doesn't mysteriously change size when we go to Unicode.
		std::string temp = str;
		if (temp.size() > 255)
			temp.erase(255);

		unsigned char stringSize = (unsigned char)temp.size();
		m_pFile->Write(&stringSize, sizeof(unsigned char));
		m_pFile->Write(temp.data(), (UINT)temp.size());
	}

	utf8_string ReadString2()
	{
		unsigned short stringSize2;
		m_pFile->Read(&stringSize2, sizeof(unsigned short));

		utf8_char *p = new utf8_char[stringSize2];
		m_pFile->Read(p, stringSize2);

		utf8_string str;
		str.assign(p, stringSize2);
		delete [] p;

		return str;
	}

	void WriteString2(const utf8_string &str)
	{
		std::string temp = str;
		if (temp.size() > SHRT_MAX)
			temp.erase(SHRT_MAX);

		unsigned short stringSize2 = (unsigned short)temp.size();
		m_pFile->Write(&stringSize2, sizeof(unsigned short));
		m_pFile->Write(temp.data(), (UINT)temp.size());
	}

	DWORD ReadInteger()
	{
		DWORD dw;
		m_pFile->Read(&dw, sizeof(DWORD));
		return dw;
	}

	void WriteInteger(DWORD dw)
	{
		m_pFile->Write(&dw, sizeof(DWORD));
	}

	bool ReadBool()
	{
		unsigned char b;
		m_pFile->Read(&b, sizeof(unsigned char));

		return b ? true : false;
	}

	void WriteBool(bool b)
	{
		unsigned char ch = b;
		m_pFile->Write(&ch, sizeof(unsigned char));
	}

	double ReadDouble()
	{
		double result;
		m_pFile->Read(&result, sizeof(double));

		return result;
	}

	void WriteDouble(double d)
	{
		m_pFile->Write(&d, sizeof(double));
	}
};

