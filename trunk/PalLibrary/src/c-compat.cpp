//”√”⁄ºÊ»› C ”Ô—‘

#include "pallib.h"

extern "C" int decodeyj1(const void* Source, void** Destination, uint32* Length)
{
	return Pal::Tools::DecodeYJ1(Source, *Destination, *Length) ? 0 : -1;
}

extern "C" int encodeyj1(const void* Source, uint32 SourceLength, void** Destination, uint32* Length)
{
	return Pal::Tools::EncodeYJ1(Source, SourceLength, *Destination, *Length) ? 0 : -1;
}

extern "C" int decodeyj2(const void* Source, void** Destination, uint32* Length)
{
	return Pal::Tools::DecodeYJ2(Source, *Destination, *Length) ? 0 : -1;
}

extern "C" int encodeyj2(const void* Source, uint32 SourceLength, void** Destination, uint32* Length, int bCompatible)
{
	return Pal::Tools::EncodeYJ2(Source, SourceLength, *Destination, *Length) ? 0 : -1;
}

extern "C" int decoderng(const void* Source, void* PrevFrame)
{
	return Pal::Tools::DecodeRNG(Source, PrevFrame) ? 0 : -1;
}

extern "C" int encoderng(const void* PrevFrame, const void* CurFrame, void** Destination, uint32* Length)
{
	return Pal::Tools::EncodeRNG(PrevFrame, CurFrame, *Destination, *Length) ? 0 : -1;
}

extern "C" int decoderle(const void* Rle, void* Destination, sint32 Stride, sint32 Width, sint32 Height, sint32 x, sint32 y)
{
	return Pal::Tools::DecodeRLE(Rle, Destination, Stride, Width, Height, x, y) ? 0 : -1;
}

extern "C" int encoderle(const void* Source, const void *Base, sint32 Stride, sint32 Width, sint32 Height, void** Destination, uint32* Length)
{
	return Pal::Tools::EncodeRLE(Source, Base, Stride, Width, Height, *Destination, *Length) ? 0 : -1;
}

extern "C" int encoderlet(const void* Source, uint8 TransparentColor, sint32 Stride, sint32 Width, sint32 Height, void** Destination, uint32* Length)
{
	return Pal::Tools::EncodeRLE(Source, TransparentColor, Stride, Width, Height, *Destination, *Length) ? 0 : -1;
}


extern "C" int decodeyj1streaminitialize(void** pvState, uint32 uiGrowBy)
{
	return Pal::Tools::DecodeYJ1StreamInitialize(*pvState, uiGrowBy) ? 0 : -1;
}

extern "C" int decodeyj1streaminput(void* pvState, const void* Source, uint32 SourceLength)
{
	return Pal::Tools::DecodeYJ1StreamInput(pvState, Source, SourceLength) ? 0 : -1;
}

extern "C" int decodeyj1streamoutput(void* pvState, void* Destination, uint32* Length)
{
	return Pal::Tools::DecodeYJ1StreamOutput(pvState, Destination, *Length) ? 0 : -1;
}

extern "C" int decodeyj1streamfinished(void* pvState, uint32* AvailableLength)
{
	return Pal::Tools::DecodeYJ1StreamFinished(pvState, *AvailableLength) ? 0 : -1;
}

extern "C" int decodeyj1streamfinalize(void* pvState)
{
	return Pal::Tools::DecodeYJ1StreamFinalize(pvState) ? 0 : -1;
}

extern "C" int decodeyj1streamreset(void* pvState)
{
	return Pal::Tools::DecodeYJ1StreamReset(pvState) ? 0 : -1;
}


extern "C" int decodeyj2streaminitialize(void** pvState, uint32 uiGrowBy)
{
	return Pal::Tools::DecodeYJ2StreamInitialize(*pvState, uiGrowBy) ? 0 : -1;
}

extern "C" int decodeyj2streaminput(void* pvState, const void* Source, uint32 SourceLength)
{
	return Pal::Tools::DecodeYJ2StreamInput(pvState, Source, SourceLength) ? 0 : -1;
}

extern "C" int decodeyj2streamoutput(void* pvState, void* Destination, uint32* Length)
{
	return Pal::Tools::DecodeYJ2StreamOutput(pvState, Destination, *Length) ? 0 : -1;
}

extern "C" int decodeyj2streamfinished(void* pvState, uint32* AvailableLength)
{
	return Pal::Tools::DecodeYJ2StreamFinished(pvState, *AvailableLength) ? 0 : -1;
}

extern "C" int decodeyj2streamfinalize(void* pvState)
{
	return Pal::Tools::DecodeYJ2StreamFinalize(pvState) ? 0 : -1;
}

extern "C" int decodeyj2streamreset(void* pvState)
{
	return Pal::Tools::DecodeYJ2StreamReset(pvState) ? 0 : -1;
}


extern "C" int encodeyj2streaminitialize(void** pvState, uint32 uiSourceLength, uint32 uiGrowBy, int bCompatible)
{
	return Pal::Tools::EncodeYJ2StreamInitialize(*pvState, uiSourceLength, uiGrowBy, bCompatible != 0) ? 0 : -1;
}

extern "C" int encodeyj2streaminput(void* pvState, const void* Source, uint32 SourceLength, int bFinished)
{
	return Pal::Tools::EncodeYJ2StreamInput(pvState, Source, SourceLength, bFinished != 0) ? 0 : -1;
}

extern "C" int encodeyj2streamoutput(void* pvState, void* Destination, uint32* Length, uint32* Bits)
{
	return Pal::Tools::EncodeYJ2StreamOutput(pvState, Destination, *Length, *Bits) ? 0 : -1;
}

extern "C" int encodeyj2streamfinished(void* pvState)
{
	return Pal::Tools::EncodeYJ2StreamFinished(pvState) ? 0 : -1;
}

extern "C" int encodeyj2streamfinalize(void* pvState)
{
	return Pal::Tools::EncodeYJ2StreamFinalize(pvState) ? 0 : -1;
}


extern "C" GRFFILE* grfopen(const char* grffile, const char* base, int create, int truncate)
{
	GRFFILE* grf;

	if (Pal::Tools::GRF::GRFopen(grffile, base, (create != 0), (truncate != 0), grf))
		return grf;
	else
		return NULL;
}

extern "C" int grfclose(GRFFILE* stream)
{
	return Pal::Tools::GRF::GRFclose(stream) ? 0 : -1;
}

extern "C" int grfflush(GRFFILE* stream)
{
	return Pal::Tools::GRF::GRFflush(stream) ? 0 : -1;
}

extern "C" int grfgettype(GRFFILE* stream)
{
	int type;

	if (Pal::Tools::GRF::GRFgettype(stream, type))
		return type;
	else
		return -1;
}

extern "C" char* grfenumname(GRFFILE* stream, const char* prevname)
{
	char* name;

	if (Pal::Tools::GRF::GRFenumname(stream, prevname, name))
		return name;
	else
		return NULL;
}


extern "C" int grferror(GRFFILE* stream)
{
	return Pal::Tools::GRF::GRFerror(stream);
}

extern "C" void grfclearerr(GRFFILE* stream)
{
	Pal::Tools::GRF::GRFclearerr(stream);
}


extern "C" FILE* grfopenfile(GRFFILE* stream, const char* name, const char* mode)
{
	FILE* fp;

	if (Pal::Tools::GRF::GRFopenfile(stream, name, mode, fp))
		return fp;
	else
		return NULL;
}

extern "C" int grfappendfile(GRFFILE* stream, const char* name)
{
	return Pal::Tools::GRF::GRFappendfile(stream, name) ? 0 : -1;
}

extern "C" int grfremovefile(GRFFILE* stream, const char* name)
{
	return Pal::Tools::GRF::GRFremovefile(stream, name) ? 0 : -1;
}

extern "C" int grfrenamefile(GRFFILE* stream, const char* oldname, const char* newname)
{
	return Pal::Tools::GRF::GRFrenamefile(stream, oldname, newname) ? 0 : -1;
}

extern "C" int grfgetfileattr(GRFFILE* stream, const char* name, int attr, void* value)
{
	return Pal::Tools::GRF::GRFgetfileattr(stream, name, attr, value) ? 0 : -1;
}

extern "C" int grfsetfileattr(GRFFILE* stream, const char* name, int attr, const void* value)
{
	return Pal::Tools::GRF::GRFsetfileattr(stream, name, attr, value) ? 0 : -1;
}


extern "C" int grfseekfile(GRFFILE* stream, const char* name)
{
	return Pal::Tools::GRF::GRFseekfile(stream, name) ? 0 : -1;
}

extern "C" int grfeof(GRFFILE* stream)
{
	return Pal::Tools::GRF::GRFeof(stream);
}

extern "C" long grfseek(GRFFILE* stream, long offset, int origin)
{
	long pos;

	if (Pal::Tools::GRF::GRFseek(stream, offset, origin, pos))
		return pos;
	else
		return -1;
}

extern "C" long grftell(GRFFILE* stream)
{
	long pos;

	if (Pal::Tools::GRF::GRFtell(stream, pos))
		return pos;
	else
		return -1;
}

extern "C" long grfread(GRFFILE* stream, void* buffer, long size)
{
	uint32 len;

	if (Pal::Tools::GRF::GRFread(stream, buffer, size, len))
		return (long)len;
	else
		return -1;
}

extern "C" int grfgetattr(GRFFILE* stream, int attr, void* value)
{
	return Pal::Tools::GRF::GRFgetattr(stream, attr, value) ? 0 : -1;
}

extern "C" int grfpackage(const char* pszGRF, const char* pszBasePath, const char* pszNewFile)
{
	return Pal::Tools::GRF::GRFPackage(pszGRF, pszBasePath, pszNewFile) ? 0 : -1;
}

extern "C" int grfextract(const char* pszGRF, const char* pszBasePath, const char* pszNewFile)
{
	return Pal::Tools::GRF::GRFExtract(pszGRF, pszBasePath, pszNewFile) ? 0 : -1;
}
