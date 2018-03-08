#include <stdio.h>
#include <stdlib.h>
#include <MediaFormats.h>
#include <private/media/EncoderPlugin.h>
#include <string.h>

#include "MP3Encoder.h"
#include "mp3_encode.h"

#if NDEBUG
 #define FPRINTF (void)
 #define assert(x) (void)0
#else
 #define FPRINTF fprintf
 #define assert(x) do { if (!(x)) { fprintf(stderr, "ASSERT FAILED!\n%s:%d: %s\n", __FILE__, __LINE__, #x); debugger("assert failed!"); } } while (0)
#endif

#define WAVE_FORMAT_MPEG 0x0050


struct mpeg1waveformat {

	int16 wFormatTag;
	int16 nChannels;
	int32 nSamplesPerSec;
	int32 nAvgBytesPerSec;
	int16 nBlockAlign;
	int16 wBitsPerSample;
	int16 cbSize;

	int16 fwHeadLayer;
	int32 dwHeadBitrate;
	int16 fwHeadMode;
	int16 fwHeadModeExt;
	int16 wHeadEmphasis;
	int16 fwHeadFlags;
	int32 dwPTSLow;
	int32 dwPTSHigh;

};

#define MPEG_MODEXT_NONE 0x0001
#define MPEG_MODEXT_INTENSITY 0x0002
#define MPEG_MODEXT_MSSTEREO 0x0004
#define MPEG_MODEXT_INT_MS 0x0008

#define MPEG_RATE_SINGLE 0x1
#define MPEG_RATE_DUAL 0x2

static struct {
	int32 bitrate;
	uint32 caps;
} mpeg_bitrate_lay3[] = {
	{ 0, 0 },			/* free form */
	{ 32000, MPEG_RATE_SINGLE },
	{ 40000, MPEG_RATE_SINGLE },
	{ 48000, MPEG_RATE_SINGLE },
	{ 56000, MPEG_RATE_SINGLE },
	{ 64000, MPEG_RATE_SINGLE | MPEG_RATE_DUAL },
	{ 80000, MPEG_RATE_SINGLE },
	{ 96000, MPEG_RATE_SINGLE | MPEG_RATE_DUAL },
	{ 112000, MPEG_RATE_SINGLE | MPEG_RATE_DUAL },
	{ 128000, MPEG_RATE_SINGLE | MPEG_RATE_DUAL },
	{ 160000, MPEG_RATE_SINGLE | MPEG_RATE_DUAL },
	{ 192000, MPEG_RATE_SINGLE | MPEG_RATE_DUAL },
	{ 224000, MPEG_RATE_DUAL },
	{ 256000, MPEG_RATE_DUAL },
	{ 320000, MPEG_RATE_DUAL },
	-1			/* forbidden */
};

#define MPEG_EMPHASIS_NONE 0x0001
#define MPEG_EMPHASIS_5015 0x0002
#define MPEG_EMPHASIS_CCITT 0x0004

#define MPEG_LAYER1 0x0001
#define MPEG_LAYER2 0x0002
#define MPEG_LAYER3 0x0004

#define MPEG_STEREO 0x0001
#define MPEG_JOINTSTEREO 0x0002
#define MPEG_DUALCHANNEL 0x0004
#define MPEG_SINGLECHANNEL 0x0008

#define MPEG_PRIVATEBIT 0x0001
#define MPEG_COPYRIGHT 0x0002
#define MPEG_ORIGINALHOME 0x0004
#define MPEG_PROTECTIONBIT 0x0008
#define MPEG_ID_MPEG1 0x0010


static	media_format			s_wavFormat;
static	media_format			s_mpegFormat;


extern "C" void register_encoder(void);
extern "C" Encoder *instantiate_encoder(void);

Encoder *instantiate_encoder(void) 
{
	return new MP3Encoder();
}

void register_encoder()
{
	status_t err;
	media_format				tempFormat, mediaFormat;
	media_format_description	formatDescription;
	BMediaFormats				formatObject;

	formatObject.Lock();

	/* register as a WAV codec */
	memset(&mediaFormat, 0, sizeof(media_format));
	mediaFormat.type = B_MEDIA_ENCODED_AUDIO;
	mediaFormat.u.encoded_audio = media_encoded_audio_format::wildcard;
	memset(&formatDescription, 0, sizeof(media_format_description));
	formatDescription.family = B_WAV_FORMAT_FAMILY;
	formatDescription.u.wav.codec = WAVE_FORMAT_MPEG;
	err = formatObject.MakeFormatFor(formatDescription, mediaFormat, &tempFormat);
	if (err == B_MEDIA_DUPLICATE_FORMAT)
		formatObject.GetFormatFor(formatDescription, &tempFormat);
	s_wavFormat = tempFormat;

	/* register as a MPEG codec */
	memset(&mediaFormat, 0, sizeof(media_format));
	mediaFormat.type = B_MEDIA_ENCODED_AUDIO;
	mediaFormat.u.encoded_audio = media_encoded_audio_format::wildcard;
	memset(&formatDescription, 0, sizeof(media_format_description));
	formatDescription.family = B_MPEG_FORMAT_FAMILY;
	formatDescription.u.mpeg.id = B_MPEG_1_AUDIO_LAYER_3;
	err = formatObject.MakeFormatFor(formatDescription, mediaFormat, &tempFormat);
	if (err == B_MEDIA_DUPLICATE_FORMAT)
		formatObject.GetFormatFor(formatDescription, &tempFormat);
	s_mpegFormat = tempFormat;

	formatObject.Unlock();
}

MP3Encoder::MP3Encoder()
{
	memset(&m_format, 0, sizeof(m_format));
	m_buffer = 0;
	m_bufAvail = 0;
	m_chunkSize = 1152*4;	//	stereo, 16 bit
	m_cookie = 0;
}

MP3Encoder::~MP3Encoder()
{
	if (m_cookie != 0) {
		mp3_done(m_cookie);
	}
	free(m_buffer);
}

//---------------------------------------------------------------------
//	GetCodecInfo
//---------------------------------------------------------------------
//
//


status_t MP3Encoder::GetCodecInfo(media_codec_info *mci) const
{
	strcpy(mci->pretty_name, "MP3 Compression");
	strcpy(mci->short_name, "mp3");
	return B_OK;
}


//---------------------------------------------------------------------
//	Accepted Format
//---------------------------------------------------------------------
//
//

status_t MP3Encoder::AcceptedFormat(const media_format* proposedInputFormat,
									media_format* _acceptedInputFormat)
{
	status_t err = B_OK;
	media_format* out_fmt;
	
	if (!proposedInputFormat) 
	{
		FPRINTF(stderr, "B_BAD_VALUE\n");
		return B_BAD_VALUE;
	}
	
	
	if (proposedInputFormat->type != B_MEDIA_RAW_AUDIO) 
	{
		proposedInputFormat->type = B_MEDIA_RAW_AUDIO;
		proposedInputFormat->u.raw_audio = media_raw_audio_format::wildcard;
	}
			
	proposedInputFormat->deny_flags = B_MEDIA_MAUI_UNDEFINED_FLAGS;
	proposedInputFormat->require_flags = 0;
	out_fmt->deny_flags = B_MEDIA_MAUI_UNDEFINED_FLAGS;
	out_fmt->require_flags = 0;
	
	*out_fmt = s_wavFormat;
	if (proposedInputFormat->u.raw_audio.frame_rate <= 1.0) 
	{
		out_fmt->u.encoded_audio.output.frame_rate = 44100.0;
	}
	else if (fabs(proposedInputFormat->u.raw_audio.frame_rate/44100.0-1.0) < 0.01) 
	{
		out_fmt->u.encoded_audio.output.frame_rate = 44100.0;
	}
	else if (fabs(proposedInputFormat->u.raw_audio.frame_rate/48000.0-1.0) < 0.01) 
	{
		out_fmt->u.encoded_audio.output.frame_rate = 48000.0;
	}
	else if (fabs(proposedInputFormat->u.raw_audio.frame_rate/32000.0-1.0) < 0.01) 
	{
		out_fmt->u.encoded_audio.output.frame_rate = 32000.0;
	}
	else 
	{
		FPRINTF(stderr, "BAD FRAME RATE (%g)\n", proposedInputFormat->u.raw_audio.frame_rate);
		return B_MEDIA_BAD_FORMAT;
	}
	out_fmt->type = B_MEDIA_ENCODED_AUDIO;
	out_fmt->u.encoded_audio.output.channel_count = proposedInputFormat->u.raw_audio.channel_count;
	out_fmt->u.encoded_audio.output.format = media_raw_audio_format::B_AUDIO_SHORT;
	out_fmt->u.encoded_audio.output.byte_order = 0;
	out_fmt->u.encoded_audio.output.buffer_size = 0;
	out_fmt->u.encoded_audio.frame_size = 0;
	out_fmt->u.encoded_audio.bit_rate = 128000.0;	//	should be set from parameters

	m_format = proposedInputFormat->u.raw_audio;
	m_format.frame_rate = out_fmt->u.encoded_audio.output.frame_rate;
	m_chunkSize = 1152*2*m_format.channel_count;

	if(_acceptedInputFormat != NULL)
	{
		_acceptedInputFormat = out_fmt;
	}
	return B_OK;
}

//---------------------------------------------------------------------
//	Sniff
//---------------------------------------------------------------------
//
//

status_t MP3Encoder::Sniff(media_file_format *mff, media_format *in_fmt,
	media_format *out_fmt, media_codec_info *mci)
{
	status_t err = B_OK;

	if (!in_fmt || !out_fmt || !mci || !mff) 
	{
		FPRINTF(stderr, "B_BAD_VALUE\n");
		return B_BAD_VALUE;
	}
		
	if (mff->family != B_WAV_FORMAT_FAMILY) 
	{
		//FPRINTF(stderr, "B_MISMATCHED_VALUES\n");
		return B_MISMATCHED_VALUES;
	}
	
	if (in_fmt->type != B_MEDIA_RAW_AUDIO) 
	{
		in_fmt->type = B_MEDIA_RAW_AUDIO;
		in_fmt->u.raw_audio = media_raw_audio_format::wildcard;
	}
			
	in_fmt->deny_flags = B_MEDIA_MAUI_UNDEFINED_FLAGS;
	in_fmt->require_flags = 0;
	out_fmt->deny_flags = B_MEDIA_MAUI_UNDEFINED_FLAGS;
	out_fmt->require_flags = 0;
	
	*out_fmt = s_wavFormat;
	if (in_fmt->u.raw_audio.frame_rate <= 1.0) 
	{
		out_fmt->u.encoded_audio.output.frame_rate = 44100.0;
	}
	else if (fabs(in_fmt->u.raw_audio.frame_rate/44100.0-1.0) < 0.01) 
	{
		out_fmt->u.encoded_audio.output.frame_rate = 44100.0;
	}
	else if (fabs(in_fmt->u.raw_audio.frame_rate/48000.0-1.0) < 0.01) 
	{
		out_fmt->u.encoded_audio.output.frame_rate = 48000.0;
	}
	else if (fabs(in_fmt->u.raw_audio.frame_rate/32000.0-1.0) < 0.01) 
	{
		out_fmt->u.encoded_audio.output.frame_rate = 32000.0;
	}
	else 
	{
		FPRINTF(stderr, "BAD FRAME RATE (%g)\n", in_fmt->u.raw_audio.frame_rate);
		return B_MEDIA_BAD_FORMAT;
	}
	out_fmt->type = B_MEDIA_ENCODED_AUDIO;
	out_fmt->u.encoded_audio.output.channel_count = in_fmt->u.raw_audio.channel_count;
	out_fmt->u.encoded_audio.output.format = media_raw_audio_format::B_AUDIO_SHORT;
	out_fmt->u.encoded_audio.output.byte_order = 0;
	out_fmt->u.encoded_audio.output.buffer_size = 0;
	out_fmt->u.encoded_audio.frame_size = 0;
	out_fmt->u.encoded_audio.bit_rate = 128000.0;	//	should be set from parameters

	// Fill in the encoder info struct 
	strcpy(mci->pretty_name, "MP3 Encoder");
	strcpy(mci->short_name, "mp3");

	m_format = in_fmt->u.raw_audio;
	m_format.frame_rate = out_fmt->u.encoded_audio.output.frame_rate;
	m_chunkSize = 1152*2*m_format.channel_count;

	return B_OK;
}


status_t MP3Encoder::SetFormat( media_file_format *mfi,
								media_format *in_fmt,
								media_format *out_fmt)
{
	//FPRINTF(stderr, "MP3Encoder::SetFormat() - ENTER -\n");
	media_codec_info temp;
	status_t err = Sniff(mfi, in_fmt, out_fmt, &temp);
	return err;
}


void MP3Encoder::AttachedToTrack()
{
	if (!m_buffer) m_buffer = (char *)malloc(m_chunkSize);
	mpeg1waveformat header;
	header.wFormatTag = WAVE_FORMAT_MPEG;
	header.nChannels = m_format.channel_count;
	header.nSamplesPerSec = (int32)m_format.frame_rate;
	header.nAvgBytesPerSec = (int32)(128000.0/8);
	header.nBlockAlign = 1;
	header.wBitsPerSample = 0;
	header.cbSize = 22;
	header.fwHeadLayer = MPEG_LAYER3;
	header.dwHeadBitrate = (int32)128000.0;
	header.fwHeadMode = (m_format.channel_count != 1) ? MPEG_STEREO : MPEG_SINGLECHANNEL;
	header.fwHeadModeExt = 0;
	//	We might want to allow for settable emphasis?
	header.wHeadEmphasis = MPEG_EMPHASIS_NONE;
	//	We might want to add protection here (CRC)
	header.fwHeadFlags = MPEG_COPYRIGHT | MPEG_ORIGINALHOME | MPEG_ID_MPEG1;
	header.dwPTSLow = 0;
	header.dwPTSHigh = 0;

	AddTrackInfo(0, (const char*)&header, sizeof(header));

	(void)mp3_init(m_format.channel_count != 1, m_format.frame_rate, 128000.0, &m_cookie);
}

status_t MP3Encoder::EncodeBuffer( const char *src, int32 src_length)
{
	assert(src_length == 1152*m_format.channel_count*2);

	char * output = (char *)alloca(src_length);	/*	assume each packet will shrink, which is safe for the blade codec	*/
	int outputSize = mp3_encode(m_cookie, src, src_length, output);

	if (outputSize > 0) 
	{
		m_LastEncodeInfo->flags |= B_MEDIA_KEY_FRAME;
		return WriteChunk(output, outputSize, m_LastEncodeInfo);
	}
	return B_OK;
}

status_t MP3Encoder::Encode( const void *in_buffer, int64 num_frames, media_encode_info *info)
{
	char *buf = (char *)in_buffer;
	size_t src_length = num_frames * m_format.channel_count * (m_format.format & 0xf);
	status_t err;

	m_LastEncodeInfo = info;

	if (!m_cookie) 
	{
		return B_NO_INIT;
	}

	if (m_bufAvail > 0) 
	{
		if (m_bufAvail + xform(src_length) < m_chunkSize) 
		{
			//	just copy into FIFO
			char *buf = (char *)in_buffer;
			convert(buf, src_length);
			return B_OK;
		}
		
		//	copy part into FIFO
		convert(buf, src_length);
		
		//	encode FIFO
		err = EncodeBuffer(m_buffer, m_chunkSize);
		
		//	update variables
		m_bufAvail = 0;
		if (err < 0) 
		{
			return err;
		}
	}

	while (xform(src_length) >= m_chunkSize) 
	{
		//	encode from buffer
		if (m_format.format == 0x2) 
		{
			err = EncodeBuffer(buf, m_chunkSize);
			buf += m_chunkSize;
			src_length -= m_chunkSize;
		}
		else 
		{
			convert(buf, src_length);
			err = EncodeBuffer(m_buffer, m_chunkSize);
			m_bufAvail = 0;
		}
		if (err < 0) 
		{
			return err;
		}
	}
	
	convert(buf, src_length);
	
	return B_OK;
}

status_t
MP3Encoder::Flush()
{
	if (m_cookie == 0) return B_OK;
	if (m_bufAvail > 0) 
	{
		memset(m_buffer+m_bufAvail, 0, m_chunkSize-m_bufAvail);
		status_t err = EncodeBuffer(m_buffer, m_chunkSize);
		if (err < B_OK) return err;	
	}
	
	return WriteChunk(m_buffer, mp3_encode(m_cookie, NULL, 0, m_buffer), m_LastEncodeInfo);
}

