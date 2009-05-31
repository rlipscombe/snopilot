#pragma once
#include "tstring.h"
#include <vector>

class SendFormProgress
{
public:
	virtual void OnProgress(DWORD dwBytesWritten, DWORD dwTotalBytes, DWORD dwBytesPerSecond, DWORD dwSecondsToCompletion) = 0;
	virtual bool CheckCancel() = 0;
};

class FormData
{
	tstring m_boundary;

	struct TextField
	{
		std::string fieldName;
		std::string fieldValue;
	};

	typedef std::vector< TextField> fields_t;
	fields_t m_fields;

	struct FileField
	{
		std::string fieldName;
		std::string contentType;
		std::string localFileName;
		HANDLE hCompressedFile;
	};

	typedef std::vector< FileField > files_t;
	files_t m_files;

public:
	FormData();

	void AddTextField(const std::string &fieldName, const std::string &fieldValue);
	void AddFileField(const std::string &fieldName, const std::string &localFileName, HANDLE hCompressedFile);

	tstring GetContentTypeHeader() const;
	DWORD CalculateContentLength() const;

	HRESULT SendForm(HINTERNET hRequest, DWORD dwCachedBytesPerSecond, SendFormProgress *pProgress);

private:
	tstring GenerateMultipartBoundary();

	std::string GetTextHeader(const TextField &tf) const;
	std::string GetTextTrailer(const TextField &tf) const;
	std::string GetFileHeader(const FileField &ff) const;
	std::string GetFileTrailer(const FileField &ff) const;
	std::string GetBodyTrailer() const;

	DWORD CalculateTextFieldContentLength(const TextField &tf) const;
	DWORD CalculateFileFieldContentLength(const FileField &ff) const;

	HRESULT SendFormFields(HINTERNET hRequest, DWORD dwCachedBytesPerSecond, SendFormProgress *pProgress);

	HRESULT SendTextField(HINTERNET hRequest, const TextField &tf);
	HRESULT SendFileField(HINTERNET hRequest, const FileField &ff, DWORD dwCachedBytesPerSecond, SendFormProgress *pProgress);

	HRESULT SendFileContents(HANDLE hCompressedFile, HINTERNET hRemoteFile, DWORD dwCachedBytesPerSecond, SendFormProgress *pProgress);
};

