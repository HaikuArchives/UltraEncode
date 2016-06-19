#ifndef __ENCODELIB_H__
#define __ENCODELIB_H__

extern "C" {

#define         SBLIMIT                 32

/* Structure for Reading Layer II Allocation Tables from File */
typedef struct {
    unsigned int    steps;
    unsigned int    bits;
    unsigned int    group;
    unsigned int    quant;
} sb_alloc, *alloc_ptr;

typedef sb_alloc al_table[SBLIMIT][16];

typedef struct {
    int version;
    int lay;
    int error_protection;
    int bitrate_index;
    int sampling_frequency;
    int padding;
    int extension;
    int mode;
    int mode_ext;
    int copyright;
    int original;
    int emphasis;
} layer, *the_layer;

/* Parent Structure Interpreting some Frame Parameters in Header */
typedef struct {
    layer       *header;        /* raw header information */
    int         actual_mode;    /* when writing IS, may forget if 0 chs */
    al_table    *alloc;         /* bit allocation table read in */
    int         tab_num;        /* number of table as loaded */
    int         stereo;         /* 1 for mono, 2 for stereo */
    int         jsbound;        /* first band of joint stereo coding */
    int         sblimit;        /* total number of sub bands */
} frame_params;


//	MPEG Header Definitions - Mode Values
#define         MPEG_AUDIO_ID           1
#define			MPEG_PHASE2_LSF			0
#define         MPG_MD_STEREO           0
#define         MPG_MD_JOINT_STEREO     1
#define         MPG_MD_DUAL_CHANNEL     2
#define         MPG_MD_MONO             3

//	Mode Extention
#define         MPG_MD_LR_LR             0
#define         MPG_MD_LR_I              1
#define         MPG_MD_MS_LR             2
#define         MPG_MD_MS_I              3


//	Psycho
#define DFLT_LAY        3      /* default encoding layer is III */
#define DFLT_MOD        'j'    /* default mode is stereo */
#define DFLT_PSY        2      /* default psych model is 2 */
#define DFLT_SFQ        44.1   /* default input sampling rate is 44.1 kHz */
#define DFLT_EMP        'n'    /* default de-emphasis is none */
#define DFLT_EXT        ".mp3" /* default output file extension */


void 	OpenEncoder(char *inPath, char *outPath, layer *info);
void 	CloseEncoder(layer *info);
int 	MakeFrame();
void 	TimeStatus(int frameNum, int totalframes, int samp_rate);
int		CurrentFrame();
int		TotalFrames();

};

#endif
