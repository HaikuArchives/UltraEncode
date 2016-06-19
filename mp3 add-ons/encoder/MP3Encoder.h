#ifndef _MP3_ENCODER_H
#define _MP3_ENCODER_H

#include <Encoder.h>
#include <unistd.h>
#include <stdio.h>

using namespace BPrivate;

class MP3Encoder : public Encoder 
{
	public:

		MP3Encoder();
		~MP3Encoder();
	
		status_t	GetCodecInfo(media_codec_info *mci) const;
		status_t	Sniff(media_file_format *mfi, media_format *in_fmt,
						  media_format *out_fmt, media_codec_info *ei);
		status_t	SetFormat(media_file_format *mfi,
							  media_format *in_format,
							  media_format *out_format);
		void		AttachedToTrack();	
		status_t	Encode(const void *in_buffer, int64 num_frames, media_encode_info *info);
		status_t	Flush();


private:

	media_raw_audio_format		m_format;
	char *						m_buffer;		//	FIFO for input data in int16 fmt
	size_t						m_bufAvail;		//	how many bytes are in buffer
	size_t						m_chunkSize;	//	total FIFO size in bytes (1152 frames)
	void *						m_cookie;		//	encoder state
	media_encode_info 			*m_LastEncodeInfo;

	inline size_t xform(int cnt) {	//	how much will some data fill of the buffer?
		return cnt*2/(m_format.format&0xf);
	}
	inline void convert(const char * & buf, size_t & cnt) {
		int togo = m_chunkSize-m_bufAvail;	//	how many dest bytes?
		if (m_format.format == 0x2) {
			//	just copy to fill buffer
			if (togo > cnt) togo = cnt;
			memcpy(m_buffer+m_bufAvail, buf, togo);
			buf += togo;
			cnt -= togo;
			m_bufAvail += togo;
		}
		else {
			togo /= 2;	//	samples, not bytes, to put in buffer
			if (togo > cnt/(m_format.format & 0xf)) {
				togo = cnt/(m_format.format & 0xf);
			}
			int16 * out = (int16 *)(m_buffer+m_bufAvail);
			switch (m_format.format) {
			//	need to convert from input to buffer
			case 0x1:
				for (int i=0; i<togo; ++i) {
					out[i] = buf[i] * 257;	//	closest we can come without fractional bit arithmetic
				}
				break;
			case 0x4:
				for (int i=0; i<togo; ++i) {
					out[i] = ((int32 *)buf)[i]>>16;
				}
				break;
			case 0x11:
				for (int i=0; i<togo; ++i) {
					out[i] = ((int8)(((uchar *)buf)[i]^0x80)) * 257;
				}
				break;
			case 0x24:
				for (int i=0; i<togo; ++i) {
					int s = (int)(((float *)buf)[i] * 32767.0);
					out[i] = (s > 32767) ? 32767 : (s < -32767) ? -32767 : s;
				}
				break;
			default:	//	this should not happen because of argument checking before here
				fprintf(stderr, "MP3Encoder: unknown sample format 0x%x\n", m_format.format);
				abort();
			}
			buf += togo*(m_format.format&0xf);
			m_bufAvail += togo*2;
			cnt -= togo*(m_format.format&0xf);
		}
	}

	status_t	EncodeBuffer(const char * src, int32 src_length);
};

#endif
