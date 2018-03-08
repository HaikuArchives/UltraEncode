#include <stdio.h>
#include <OS.h>
#include <assert.h>

#include "mp3_encode.h"

extern "C" {
#include "blade/codec.h"
};


/*	This extra function call layer is necessary to guard the	*/
/*	globals used by the Blade codec. We might work on removing	*/
/*	it sometime.	*/

static int32 inited;
sem_id g_lock;

int
mp3_init(
	int stereo,
	float framerate,
	float bitrate,
	void ** cookie)
{
	//fprintf(stderr, "mp3_init(%s, %g, %g)\n", stereo ? "stereo" : "mono", framerate, bitrate);


	int32 v = atomic_or(&inited, 1);
	if (v == 0) {
		g_lock = create_sem(1, "EncoderGlobals");
		atomic_or(&inited, 2);
	}
	else while (!(v & 2)) {
		snooze(10000);
		v = atomic_or(&inited, 0);
	}
	thread_info tinfo;
	get_thread_info(find_thread(NULL), &tinfo);
	//fprintf(stderr,
	//		"thread %d (%s) tries to lock blade_mp3_globals\n",
	//		tinfo.thread,
	//		tinfo.name);
	status_t ret = acquire_sem(g_lock);
	if (ret == B_OK) 
	{
		//fprintf(stderr,
		//		"blade_mp3_globals holder is thread %d (%s)\n",
		//		tinfo.thread,
		//		tinfo.name);

		CodecInitIn info;
		info.frequency = (int32)framerate;
		info.mode = (stereo ? 0 : 3);
		info.bitrate = (int32)(128000.0/1000);
		info.emphasis = 0;
		info.fPrivate = 0;
		info.fCRC = 0;
		info.fCopyright = 1;
		info.fOriginal = 1;

		CodecInitOut * p = codecInit(&info);
		if (p == 0) 
		{
			//fprintf(stderr, "codecInit returns error\n");
			ret = B_ERROR;
		}
		else 
		{
			//fprintf(stderr, "p->nSamples = %d\n", p->nSamples);
			assert(p->nSamples == 1152*(stereo ? 2 : 1));
			*cookie = (void *)1;
		}
	}
	return ret;
}

int
mp3_encode(
	void * cookie,
	const void * src,
	int size,
	void * dest)
{
	if (cookie == 0) return B_BAD_VALUE;
	if (!src) {
		return codecExit((char *)dest);
	}
	return codecEncodeChunk(size/2, (short *)const_cast<void *>(src), (char *)dest);
}

int
mp3_done(
	void * cookie)
{
	if (cookie == 0) return B_BAD_VALUE;
	thread_info info;
	get_thread_info(find_thread(NULL), &info);
	//fprintf(stderr, "thread %d (%s) releases blade_mp3_globals\n", info.thread, info.name);
	return release_sem(g_lock);
}

