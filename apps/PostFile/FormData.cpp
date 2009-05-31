#include "StdAfx.h"
#include "FormData.h"

FormData::FormData()
{
	m_boundary = GenerateMultipartBoundary();
}

tstring FormData::GetContentTypeHeader() const
{
	tstring contentTypeHeader = _T("Content-Type: multipart/form-data; boundary=") + m_boundary;

	return contentTypeHeader;
}

tstring FormData::GenerateMultipartBoundary()
{
	tstring boundaryPrefix(27, '-');

	// We now need 12 hex digits.
	int r0 = rand() & 0xffff;
	int r1 = rand() & 0xffff;
	int r2 = rand() & 0xffff;

	TCHAR temp[13];
	_stprintf(temp, _T("%04X%04X%04X"), r0, r1, r2);

	return boundaryPrefix + temp;
}

void FormData::AddTextField(const std::string &fieldName, const std::string &fieldValue)
{
	TextField tf;
	tf.fieldName = fieldName;
	tf.fieldValue = fieldValue;

	m_fields.push_back(tf);
}

void FormData::AddFileField(const std::string &fieldName, const std::string &localFileName, HANDLE hCompressedFile)
{
	FileField ff;
	ff.fieldName = fieldName;
	ff.contentType = "application/octet-stream";
	ff.localFileName = localFileName;
	ff.hCompressedFile = hCompressedFile;

	m_files.push_back(ff);
}

std::string FormData::GetFileHeader(const FileField &ff) const
{
	std::string contentDisposition = "Content-Disposition: form-data; name=\"" + ff.fieldName + "\"; filename=\"" + ff.localFileName + "\"";
	std::string contentType = "Content-Type: " + ff.contentType;

	std::string fileHeader;
	fileHeader.append("--" + m_boundary + "\r\n");
	fileHeader.append(contentDisposition + "\r\n");
	fileHeader.append(contentType + "\r\n");
	fileHeader.append("\r\n");

	return fileHeader;
}

std::string FormData::GetFileTrailer(const FileField &ff) const
{
	return std::string("\r\n");
}

std::string FormData::GetTextHeader(const TextField &tf) const
{
	std::string contentDisposition = "Content-Disposition: form-data; name=\"" + tf.fieldName + "\"";

	std::string textHeader;
	textHeader.append("--" + m_boundary + "\r\n");
	textHeader.append(contentDisposition + "\r\n");
	textHeader.append("\r\n");

	return textHeader;
}

std::string FormData::GetTextTrailer(const TextField &tf) const
{
	return std::string("\r\n");
}

DWORD FormData::CalculateTextFieldContentLength(const TextField &tf) const
{
	std::string textHeader = GetTextHeader(tf);

	std::string textTrailer = GetTextTrailer(tf);

	return (DWORD)textHeader.size() + (DWORD)tf.fieldValue.size() + (DWORD)textTrailer.size();
}

/** How much does this file field contribute to the overall length?
 */
DWORD FormData::CalculateFileFieldContentLength(const FileField &ff) const
{
	std::string fileHeader = GetFileHeader(ff);

	DWORD dwLocalFileSize = GetFileSize(ff.hCompressedFile, NULL);

	std::string fileTrailer = GetFileTrailer(ff);

	return (DWORD)fileHeader.size() + dwLocalFileSize + (DWORD)fileTrailer.size();
}

std::string FormData::GetBodyTrailer() const
{
	std::string bodyTrailer;
	bodyTrailer.assign("--" + m_boundary + "--\r\n");
	return bodyTrailer;
}

DWORD FormData::CalculateContentLength() const
{
	DWORD dwContentLength = 0;

	// For each field:
	for (fields_t::const_iterator i = m_fields.begin(); i != m_fields.end(); ++i)
		dwContentLength += CalculateTextFieldContentLength(*i);

	// For each file:
	for (files_t::const_iterator i = m_files.begin(); i != m_files.end(); ++i)
		dwContentLength += CalculateFileFieldContentLength(*i);

	// After all of the fields and files comes the body trailer:
	std::string bodyTrailer = GetBodyTrailer();

	dwContentLength += (DWORD)bodyTrailer.size();

	return dwContentLength;
}

HRESULT FormData::SendFormFields(HINTERNET hRequest, DWORD dwCachedBytesPerSecond, SendFormProgress *pProgress)
{
	// For each field...
	for (fields_t::const_iterator i = m_fields.begin(); i != m_fields.end(); ++i)
	{
		HRESULT hr = SendTextField(hRequest, *i);
		if (FAILED(hr))
			return hr;
	}

	// For each file...
	for (files_t::const_iterator i = m_files.begin(); i != m_files.end(); ++i)
	{
		HRESULT hr = SendFileField(hRequest, *i, dwCachedBytesPerSecond, pProgress);
		if (FAILED(hr))
			return hr;
	}

	// After the last file:
	std::string bodyTrailer = GetBodyTrailer();
	DWORD dwBytesWritten;
	if (!InternetWriteFile(hRequest, bodyTrailer.data(), (DWORD)bodyTrailer.size(), &dwBytesWritten))
	{
		TRACE("InternetWriteFile failed, error = %d (0x%x)\n", GetLastError(), GetLastError());
		return HRESULT_FROM_WIN32(GetLastError());
	}

	return S_OK;
}

HRESULT FormData::SendTextField(HINTERNET hRequest, const TextField &tf)
{
	std::string textHeader = GetTextHeader(tf);

	DWORD dwBytesWritten;
	if (!InternetWriteFile(hRequest, textHeader.data(), (DWORD)textHeader.size(), &dwBytesWritten))
	{
		TRACE("InternetWriteFile failed, error = %d (0x%x)\n", GetLastError(), GetLastError());
		return HRESULT_FROM_WIN32(GetLastError());
	}

	if (tf.fieldValue.size())
	{
		if (!InternetWriteFile(hRequest, tf.fieldValue.data(), (DWORD)tf.fieldValue.size(), &dwBytesWritten))
		{
			TRACE("InternetWriteFile failed, error = %d (0x%x)\n", GetLastError(), GetLastError());
			return HRESULT_FROM_WIN32(GetLastError());
		}
	}

	std::string textTrailer = GetTextTrailer(tf);
	if (!InternetWriteFile(hRequest, textTrailer.data(), (DWORD)textTrailer.size(), &dwBytesWritten))
	{
		TRACE("InternetWriteFile failed, error = %d (0x%x)\n", GetLastError(), GetLastError());
		return HRESULT_FROM_WIN32(GetLastError());
	}

	return S_OK;
}

HRESULT FormData::SendFileField(HINTERNET hRequest, const FileField &ff, DWORD dwCachedBytesPerSecond, SendFormProgress *pProgress)
{
	std::string fileHeader = GetFileHeader(ff);

	DWORD dwBytesWritten;
	if (!InternetWriteFile(hRequest, fileHeader.data(), (DWORD)fileHeader.size(), &dwBytesWritten))
	{
		TRACE("InternetWriteFile failed, error = %d (0x%x)\n", GetLastError(), GetLastError());
		return HRESULT_FROM_WIN32(GetLastError());
	}

	HRESULT hr  = SendFileContents(ff.hCompressedFile, hRequest, dwCachedBytesPerSecond, pProgress);
	if (FAILED(hr))
		return hr;

	// After the file:
	std::string fileTrailer = GetFileTrailer(ff);
	if (!InternetWriteFile(hRequest, fileTrailer.data(), (DWORD)fileTrailer.size(), &dwBytesWritten))
	{
		TRACE("InternetWriteFile failed, error = %d (0x%x)\n", GetLastError(), GetLastError());
		return HRESULT_FROM_WIN32(GetLastError());
	}

	return S_OK;
}

HRESULT FormData::SendForm(HINTERNET hRequest, DWORD dwCachedBytesPerSecond, SendFormProgress *pProgress)
{
	// Using POST to send a file:
	// Add the Content-Type header:
	tstring contentTypeHeader = GetContentTypeHeader();
	HttpAddRequestHeaders(hRequest, contentTypeHeader.c_str(), -1, HTTP_ADDREQ_FLAG_ADD);

	DWORD dwContentLength = CalculateContentLength();

	INTERNET_BUFFERS buffersIn;
	memset(&buffersIn, 0, sizeof(INTERNET_BUFFERS));
	buffersIn.dwStructSize = sizeof(INTERNET_BUFFERS);
	buffersIn.dwBufferTotal = dwContentLength;

	BOOL bResult = HttpSendRequestEx(hRequest, &buffersIn, NULL, HSR_INITIATE, reinterpret_cast<DWORD_PTR>(this));
	if (!bResult)
	{
		TRACE("HttpSendRequestEx failed, error = %d (0x%x)\n", GetLastError(), GetLastError());
		return HRESULT_FROM_WIN32(GetLastError());
	}

	HRESULT hr = SendFormFields(hRequest, dwCachedBytesPerSecond, pProgress);
	if (FAILED(hr))
		return hr;

	bResult = HttpEndRequest(hRequest, NULL, 0, reinterpret_cast<DWORD_PTR>(this));
	if (!bResult)
	{
		TRACE("HttpEndRequest failed, error = %d (0x%x)\n", GetLastError(), GetLastError());
		return HRESULT_FROM_WIN32(GetLastError());
	}

	return S_OK;
}

HRESULT FormData::SendFileContents(HANDLE hCompressedFile, HINTERNET hRemoteFile, DWORD dwCachedBytesPerSecond, SendFormProgress *pProgress)
{
	ASSERT(pProgress);

	DWORD dwLocalFileSize = GetFileSize(hCompressedFile, NULL);
	DWORD dwTotalBytesSent = 0;
	DWORD dwBytesPerSecond = dwCachedBytesPerSecond;
	DWORD dwTimeStarted = GetTickCount() / 1000;
	DWORD dwTimeLast = dwTimeStarted;

	// Hugo says that 256Kb blocks mean that progress is only reported every 15 seconds or so.
	// We could reduce to 16Kb block, which would mean once a second, but we only report progress
	// every two seconds, anyway (see the end of the loop).
	DWORD cbBuffer = 8 * 1024;	// 8KB
	BYTE *pBuffer = (BYTE *)malloc(cbBuffer);

	DWORD dwSecondsToCompletion = dwLocalFileSize / dwBytesPerSecond;
	pProgress->OnProgress(0, dwLocalFileSize, dwBytesPerSecond, dwSecondsToCompletion);
	for (;;)
	{
		if (pProgress->CheckCancel())
		{
			free(pBuffer);
			return E_USER_CANCELLED;
		}

		// Read a bunch of the source file:
		DWORD dwBytesRead;
		if (!ReadFile(hCompressedFile, pBuffer, cbBuffer, &dwBytesRead, NULL))
		{
			free(pBuffer);
			TRACE("ReadFile failed, error = %d (0x%x)\n", GetLastError(), GetLastError());
			return HRESULT_FROM_WIN32(GetLastError());
		}

		if (dwBytesRead == 0)
			break;

		// Write that to the other end:
		DWORD dwBytesWritten;
		if (!InternetWriteFile(hRemoteFile, pBuffer, dwBytesRead, &dwBytesWritten))
		{
			free(pBuffer);
			TRACE("InternetWriteFile failed, error = %d (0x%x)\n", GetLastError(), GetLastError());
			return HRESULT_FROM_WIN32(GetLastError());
		}

		dwTotalBytesSent += dwBytesWritten;
		
		// Avoid reporting progress too often.
		DWORD dwTimeNow = GetTickCount() / 1000;
		if (dwTimeNow != dwTimeLast)
		{
			DWORD dwSecondsTaken = dwTimeNow - dwTimeStarted;
			dwBytesPerSecond = dwTotalBytesSent / dwSecondsTaken;

			DWORD dwBytesRemaining = dwLocalFileSize - dwTotalBytesSent;
			DWORD dwSecondsToCompletion = dwBytesRemaining / dwBytesPerSecond;

			pProgress->OnProgress(dwTotalBytesSent, dwLocalFileSize, dwBytesPerSecond, dwSecondsToCompletion);
			dwTimeLast = dwTimeNow;
		}
	}

	pProgress->OnProgress(dwLocalFileSize, dwLocalFileSize, dwBytesPerSecond, 0);
	free(pBuffer);

	return S_OK;
}

