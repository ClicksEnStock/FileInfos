
#include "Common.h"

// Convert an UTF8 string to wchar_t*
wchar_t* utf8_decode(const std::string &str)
{
	int wchars_num = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
	wchar_t* wstr = new wchar_t[wchars_num];
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wstr, wchars_num);
	return wstr;
}

TCHAR* Extension::ExifMetadata(TCHAR* exifkey)
{
	if (Exiv2::fileExists(imagePath, true))
	{
		Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(imagePath, true);
		if (image.get() != 0)
		{
			image->readMetadata();
			Exiv2::ExifData &exifData = image->exifData();
			wstring wskey = wstring(exifkey);
			string skey(wskey.begin(), wskey.end());
			string svalue = "";
			if (!exifData.empty())
			{
				try {
					Exiv2::Metadatum* metadatum = 0;
					Exiv2::ExifData::iterator pos = exifData.findKey(Exiv2::ExifKey(skey));
					if (pos != exifData.end())
					{
						metadatum = &(*pos);
						if (strcmp(metadatum->key().c_str(), "Exif.Image.XPComment") == 0)
						{
							svalue = metadatum->print(&exifData);
						}
						else
						{
							svalue = metadatum->toString();
						}
					}
					return utf8_decode(svalue);
				}
				catch (Exiv2::AnyError& e) {
					return _T("");
				}
				catch (...) {
					return _T("");
				}
			}
		}
	}
	return _T("");
}