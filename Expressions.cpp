
#include "Common.h"

//+-------------------------------------------------------------------
//
//  ConvertValueToString
//  
//  Generate a string for the value in a given PROPVARIANT structure.
//  The most common types are supported; that is, those that can be
//  displayed with printf.  For other types, only an ellipses (...) 
//  is displayed.
//
//  The property to create a string from is in propvar, the resulting
//  string is placed into pwszValue, which is a buffer with space for
//  cchValue characters (including the string terminator).
//
//+-------------------------------------------------------------------

void ConvertValueToString(const PROPVARIANT &propvar, WCHAR *pwszValue, ULONG cchValue)
{
	// Ensure that the output string is terminated

	pwszValue[cchValue - 1] = L'\0';
	--cchValue;

	// Based on the type, put the value into pwszValue as a string.

	switch (propvar.vt)
	{
	case VT_EMPTY:
		wcsncpy_s(pwszValue, cchValue, L"", cchValue);
		break;
	case VT_NULL:
		wcsncpy_s(pwszValue, cchValue, L"", cchValue);
		break;
	case VT_I2:
		_snwprintf_s(pwszValue, cchValue, cchValue, L"%i", propvar.iVal);
		break;
	case VT_I4:
	case VT_INT:
		_snwprintf_s(pwszValue, cchValue, cchValue, L"%li", propvar.lVal);
		break;
	case VT_I8:
		_snwprintf_s(pwszValue, cchValue, cchValue, L"%I64i", propvar.hVal);
		break;
	case VT_UI2:
		_snwprintf_s(pwszValue, cchValue, cchValue, L"%u", propvar.uiVal);
		break;
	case VT_UI4:
	case VT_UINT:
		_snwprintf_s(pwszValue, cchValue, cchValue, L"%lu", propvar.ulVal);
		break;
	case VT_UI8:
		_snwprintf_s(pwszValue, cchValue, cchValue, L"%I64u", propvar.uhVal);
		break;
	case VT_R4:
		_snwprintf_s(pwszValue, cchValue, cchValue, L"%f", propvar.fltVal);
		break;
	case VT_R8:
		_snwprintf_s(pwszValue, cchValue, cchValue, L"%lf", propvar.dblVal);
		break;
	case VT_BSTR:
		_snwprintf_s(pwszValue, cchValue, cchValue, L"\"%s\"",
			propvar.bstrVal);
		break;
	case VT_ERROR:
		_snwprintf_s(pwszValue, cchValue, cchValue, L"0x%08X", propvar.scode);
		break;
	case VT_BOOL:
		_snwprintf_s(pwszValue, cchValue, cchValue, L"%s",
			VARIANT_TRUE == propvar.boolVal ? L"True" : L"False");
		break;
	case VT_I1:
		_snwprintf_s(pwszValue, cchValue, cchValue, L"%i", propvar.cVal);
		break;
	case VT_UI1:
		_snwprintf_s(pwszValue, cchValue, cchValue, L"%u", propvar.bVal);
		break;
	case VT_VOID:
		wcsncpy_s(pwszValue, cchValue, L"", cchValue);
		break;
	case VT_LPSTR:
		if (0 > _snwprintf_s(pwszValue, cchValue, cchValue,
			L"\"%hs\"", propvar.pszVal))
			// String is too large for pwszValue
			wcsncpy_s(pwszValue, cchValue, L"...", cchValue);
		break;
	case VT_LPWSTR:
		if (0 > _snwprintf_s(pwszValue, cchValue, cchValue,
			L"\"%s\"", propvar.pwszVal))
			// String is too large for pwszValue
			wcsncpy_s(pwszValue, cchValue, L"...", cchValue);
		break;
	case VT_FILETIME:
		_snwprintf_s(pwszValue, cchValue, cchValue, L"%08x:%08x",
			propvar.filetime.dwHighDateTime,
			propvar.filetime.dwLowDateTime);
		break;
	case VT_CLSID:
		pwszValue[0] = L'\0';
		StringFromGUID2(*propvar.puuid, pwszValue, cchValue);
		break;
	default:
		wcsncpy_s(pwszValue, cchValue, L"...", cchValue);
		break;
	}

}


//+-------------------------------------------------------------------
//
//  DisplayProperty
//
//  Dump the ID, name, type, and value of a property.
//
//+-------------------------------------------------------------------

void Extension::AppendProperty(const PROPVARIANT &propvar, const STATPROPSTG &statpropstg)
{
	WCHAR wsz[MAX_PATH + 1];
	if (NULL != statpropstg.lpwstrName)
		propertyStr += statpropstg.lpwstrName;

	ConvertValueToString(propvar, wsz, sizeof(wsz) / sizeof(wsz[0]));
	propertyStr += wsz;

}


//+-------------------------------------------------------------------
//
//  DisplayPropertySet
//
//  Dump all the properties into a given property set.
//
//+-------------------------------------------------------------------

void Extension::AppendPropertySet(FMTID fmtid, const WCHAR *pwszStorageName, IPropertyStorage *pPropStg)
{
	IEnumSTATPROPSTG *penum = NULL;
	HRESULT hr = S_OK;
	STATPROPSTG statpropstg;
	PROPVARIANT propvar;
	PROPSPEC propspec;
	PROPID propid;
	WCHAR *pwszFriendlyName = NULL;

	// This string will hold a string-formatted FMTID. It must
	// be 38 characters, plus the terminator character.
	// For best practice, create a moderately longer string.
	WCHAR wszFMTID[64] = { L"" };

	PropVariantInit(&propvar);
	memset(&statpropstg, 0, sizeof(statpropstg));

	try
	{
		// Display the ID of the property set.

		StringFromGUID2(fmtid,
			wszFMTID,
			sizeof(wszFMTID) / sizeof(wszFMTID[0]));
		wprintf(L"\n Property Set %s\n", wszFMTID);

		// If this is a common property set, display.

		if (FMTID_SummaryInformation == fmtid)
			wprintf(L"   (SummaryInformation property set)\n");
		else if (FMTID_DocSummaryInformation == fmtid)
			wprintf(L"   (DocumentSummaryInformation property set)\n");
		else if (FMTID_UserDefinedProperties == fmtid)
			wprintf(L"   (UserDefined property set)\n");

		// Also display the name of the storage that contains
		// this property set.

		wprintf(L"   in \"%s\":\n", pwszStorageName);

		// If this property set has a friendly name, display it now.
		// (Property names are stored in the special dictionary
		// property - the name of the property set is indicated by
		// naming the dictionary property itself.)

		propid = PID_DICTIONARY;
		pwszFriendlyName = NULL;

		hr = pPropStg->ReadPropertyNames(1, &propid,
			&pwszFriendlyName);
		if (S_OK == hr)
		{
			wprintf(L"   (Friendly name is \"%s\")\n\n",
				pwszFriendlyName);
			CoTaskMemFree(pwszFriendlyName);
			pwszFriendlyName = NULL;
		}
		else
			wprintf(L"\n");

		// Get a property enumerator.

		hr = pPropStg->Enum(&penum);
		if (FAILED(hr))
			throw L"Failed IPropertyStorage::Enum";

		// Get the first property in the enumeration.

		hr = penum->Next(1, &statpropstg, NULL);

		// Loop through and display each property.  The 'Next'
		// call above, and at the bottom of the while loop,
		// will return S_OK if it returns another property,
		// S_FALSE if there are no more properties,
		// and anything else is an error.

		while (S_OK == hr)
		{

			// Read the property out of the property set

			PropVariantInit(&propvar);
			propspec.ulKind = PRSPEC_PROPID;
			propspec.propid = statpropstg.propid;

			hr = pPropStg->ReadMultiple(1, &propspec, &propvar);
			if (FAILED(hr))
				throw L"Failed IPropertyStorage::ReadMultiple";

			// Display the property value, type, and so on.

			AppendProperty(propvar, statpropstg);

			// Free buffers allocated during the read, and
			// by the enumerator.

			PropVariantClear(&propvar);
			CoTaskMemFree(statpropstg.lpwstrName);
			statpropstg.lpwstrName = NULL;

			// Move to the next property in the enumeration

			hr = penum->Next(1, &statpropstg, NULL);
		}
		if (FAILED(hr)) throw L"Failed IEnumSTATPROPSTG::Next";
	}
	catch (LPCWSTR pwszErrorMessage)
	{
		wprintf(L"Error in DumpPropertySet: %s (hr = %08x)\n",
			pwszErrorMessage, hr);
	}

	if (NULL != penum)
		penum->Release();

	if (NULL != statpropstg.lpwstrName)
		CoTaskMemFree(statpropstg.lpwstrName);

	PropVariantClear(&propvar);
}


//+-------------------------------------------------------------------
//
//  DisplayPropertySetsInStorage
//
//  Dump the property sets in the top level of a given storage.
//
//+-------------------------------------------------------------------

void Extension::AppendPropertySetsInStorage(const WCHAR *pwszStorageName, IPropertySetStorage *pPropSetStg)
{
	IEnumSTATPROPSETSTG *penum = NULL;
	HRESULT hr = S_OK;
	IPropertyStorage *pPropStg = NULL;
	STATPROPSETSTG statpropsetstg;

	try
	{
		// Get a property-set enumerator, which only enumerates
		// the property sets at this level of the storage, not 
		// its child objects.

		hr = pPropSetStg->Enum(&penum);
		if (FAILED(hr))
			throw L"failed IPropertySetStorage::Enum";

		// Get the first property set in the enumeration.
		// (The field used to open the property set is
		// statpropsetstg.fmtid.

		memset(&statpropsetstg, 0, sizeof(statpropsetstg));
		hr = penum->Next(1, &statpropsetstg, NULL);

		// Loop through all the property sets.

		while (S_OK == hr)
		{
			// Open the property set.

			hr = pPropSetStg->Open(statpropsetstg.fmtid,
				STGM_READ | STGM_SHARE_EXCLUSIVE,
				&pPropStg);
			if (FAILED(hr))
				throw L"failed IPropertySetStorage::Open";

			// Display the properties in the property set.

			AppendPropertySet(statpropsetstg.fmtid,
				pwszStorageName,
				pPropStg);

			pPropStg->Release();
			pPropStg = NULL;

			// Get the FMTID of the next property set in the
			// enumeration.

			hr = penum->Next(1, &statpropsetstg, NULL);

		}
		if (FAILED(hr)) throw L"Failed IEnumSTATPROPSETSTG::Next";

		// Special-case handling for the UserDefined property set:
		// This property set actually lives inside the well-known
		// DocumentSummaryInformation property set. It is the only
		// property set which is allowed to live inside another
		// (and exists for legacy compatibility).  It does not get
		// included in a normal enumeration, so verify that it is
		// done explicitly. Look for it when the end of the
		// enumerator is reached.

		hr = pPropSetStg->Open(FMTID_UserDefinedProperties,
			STGM_READ | STGM_SHARE_EXCLUSIVE,
			&pPropStg);
		if (SUCCEEDED(hr))
		{
			AppendPropertySet(FMTID_UserDefinedProperties,
				pwszStorageName,
				pPropStg);
			pPropStg->Release();
			pPropStg = NULL;
		}

	}
	catch (LPCWSTR pwszErrorMessage)
	{
		wprintf(L"Error in DumpPropertySetsInStorage: %s(hr = % 08x)\n",
			pwszErrorMessage, hr);
	}

	if (NULL != pPropStg)
		pPropStg->Release();
	if (NULL != penum)
		penum->Release();
}


//+-------------------------------------------------------------------
//
//  DisplayStorageTree
//
//  Dump all the property sets in the given storage and recursively in
//  all its child objects.
//
//+-------------------------------------------------------------------

void Extension::AppendStorageTree(const WCHAR *pwszStorageName, IStorage *pStg)
{
	IPropertySetStorage *pPropSetStg = NULL;
	IStorage *pStgChild = NULL;
	WCHAR *pwszChildStorageName = NULL;
	IEnumSTATSTG *penum = NULL;
	HRESULT hr = S_OK;
	STATSTG statstg;

	memset(&statstg, 0, sizeof(statstg));

	try
	{
		// Dump the property sets at this storage level

		hr = pStg->QueryInterface(IID_IPropertySetStorage,
			reinterpret_cast<void**>(&pPropSetStg));
		if (FAILED(hr))
			throw
			L"Failed IStorage::QueryInterface(IID_IPropertySetStorage)";

		AppendPropertySetsInStorage(pwszStorageName, pPropSetStg);

		// Get an enumerator for this storage.

		hr = pStg->EnumElements(NULL, NULL, NULL, &penum);
		if (FAILED(hr)) throw L"failed IStorage::Enum";

		// Get the name of the first element (stream/storage)
		// in the enumeration.  As usual, 'Next' will return
		// S_OK if it returns an element of the enumerator,
		// S_FALSE if there are no more elements, and an
		// error otherwise.

		hr = penum->Next(1, &statstg, 0);

		// Loop through all the child objects of this storage.

		while (S_OK == hr)
		{
			// Verify that this is a storage that is not a property
			// set, because the property sets are displayed above).
			// If the first character of its name is the '\005'
			// reserved character, it is a stream /storage property
			// set.

			if (STGTY_STORAGE == statstg.type
				&&
				L'\005' != statstg.pwcsName[0])
			{
				// Indicates normal storage, not a propset.
				// Open the storage.

				ULONG cchChildStorageName;

				hr = pStg->OpenStorage(statstg.pwcsName,
					NULL,
					STGM_READ | STGM_SHARE_EXCLUSIVE,
					NULL, 0,
					&pStgChild);
				if (FAILED(hr))
					throw L"failed IStorage::OpenStorage";

				// Compose a name of the form 
				// "Storage\ChildStorage\..." for display purposes.
				// First, allocate it.

				cchChildStorageName = wcslen(pwszStorageName)
					+ wcslen(statstg.pwcsName)
					+ 2  // For two "\" chars
					+ 1; // For string terminator

				pwszChildStorageName =
					new WCHAR[cchChildStorageName];
				if (NULL == pwszChildStorageName)
				{
					hr = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
					throw L"couldn't allocate memory";
				}

				// Terminate the name.

				pwszChildStorageName[cchChildStorageName - 1] = L'\0';
				--cchChildStorageName;

				// Build the name.

				wcsncpy_s(pwszChildStorageName, cchChildStorageName,
					pwszStorageName, cchChildStorageName);
				cchChildStorageName -= wcslen(pwszStorageName);

				wcsncat_s(pwszChildStorageName, cchChildStorageName,
					L"\\", cchChildStorageName);
				cchChildStorageName -= 2;

				wcsncat_s(pwszChildStorageName, cchChildStorageName,
					statstg.pwcsName, cchChildStorageName);

				// Dump all property sets under this child storage.

				AppendStorageTree(pwszChildStorageName, pStgChild);

				pStgChild->Release();
				pStgChild = NULL;

				delete[] pwszChildStorageName;
				pwszChildStorageName = NULL;
			}

			// Move to the next element in the enumeration of 
			// this storage.

			CoTaskMemFree(statstg.pwcsName);
			statstg.pwcsName = NULL;

			hr = penum->Next(1, &statstg, 0);
		}
		if (FAILED(hr)) throw L"failed IEnumSTATSTG::Next";
	}
	catch (LPCWSTR pwszErrorMessage)
	{
		wprintf(L"Error in DumpStorageTree: %s (hr = %08x)\n",
			pwszErrorMessage, hr);
	}

	// Cleanup before returning.

	if (NULL != statstg.pwcsName)
		CoTaskMemFree(statstg.pwcsName);
	if (NULL != pStgChild)
		pStgChild->Release();
	if (NULL != pStg)
		pStg->Release();
	if (NULL != penum)
		penum->Release();
	if (NULL != pwszChildStorageName)
		delete[] pwszChildStorageName;

}


wstring Extension::GetFileInfos(LPCWSTR filePath)
{
	HRESULT hr = S_OK;
	IStorage *pStg = NULL;

	// Open the root storage.

	hr = StgOpenStorageEx(filePath,
		STGM_READ | STGM_SHARE_DENY_WRITE,
		STGFMT_ANY,
		0,
		NULL,
		NULL,
		IID_IStorage,
		reinterpret_cast<void**>(&pStg));
	
	// Dump all the properties in all the property sets within this
	// storage.

	if (FAILED(hr))
	{
		//wprintf(L"Error: couldn't open storage \"%s\" (hr = % 08x)\n",filePath, hr);
	}
	else
	{
		//Displaying all property sets
		AppendStorageTree(filePath, pStg);
		pStg->Release();
	}
	/*WIN32_FILE_ATTRIBUTE_DATA fInfo;
	GetFileAttributesEx(filePath, GetFileExInfoStandard, &fInfo);
	fInfo.dwFileAttributes*/
	return propertyStr;
}