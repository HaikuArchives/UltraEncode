
#if !defined(mp3_encode_h)
#define mp3_encode_h

/*	create a context cookie */
extern "C" int mp3_init(int stereo, float framerate, float bitrate, void ** cookie);
/*	encode successive blocks; NULL src for flush; flush (only) before calling done()	*/
extern "C" int mp3_encode(void * cookie, const void * src, int size, void * dest);
/*	dispose cookie	*/
extern "C" int mp3_done(void * cookie);

#endif	//	mp3_encode_h

