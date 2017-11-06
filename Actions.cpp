
#include "Common.h"

void Extension::SetFile(TCHAR* filePath)
{
	wstring wspath = wstring(filePath);
	string spath(wspath.begin(), wspath.end());
	imagePath = spath;
}

void Extension::SetExifMetadata(TCHAR* exifkey, TCHAR* exifvalue)
{
	if (Exiv2::fileExists(imagePath, true))
	{
		Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(imagePath, true);
		if (image.get() != 0)
		{
			image->readMetadata();
		}
		Exiv2::ExifData &exifData = image->exifData();
		wstring wskey = wstring(exifkey);
		const string skey(wskey.begin(), wskey.end());
		wstring wsvalue = wstring(exifvalue);
		string svalue(wsvalue.begin(), wsvalue.end());
		if (!exifData.empty()) 
		{
			try {
				Exiv2::Metadatum* metadatum = 0;
				Exiv2::ExifData::iterator pos = exifData.findKey(Exiv2::ExifKey(skey));
				if (pos != exifData.end())
				{
					metadatum = &(*pos);
					Exiv2::Value::AutoPtr value;
					value = metadatum->getValue();
					if (value.get() == 0 )
					{
						value = Exiv2::Value::create(Exiv2::TypeId::string);
					}
					int rc = value->read(svalue);
					if (0 == rc) 
					{
						if (metadatum) 
						{
							metadatum->setValue(value.get());
						}
						else 
						{
							exifData.add(Exiv2::ExifKey(skey), value.get());
						}
						image->writeMetadata();
					}
					else {
						// Error, failed to read 
					}
				}
			}
			catch (Exiv2::AnyError& e) {

			}
			catch (...) {

			}
		}
		else
		{
			Exiv2::Value::AutoPtr v = Exiv2::Value::create(Exiv2::string);
			// Set the value to a string
			v->read(svalue);
			Exiv2::ExifKey key(skey);
			exifData.add(key, v.get());
			image->setExifData(exifData);
			image->writeMetadata();
		}
	}
}