#include <tinyalsa/asoundlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <string.h>

#include <math.h>
/* MSR API offline test program */
#include "signal_SI_extern.h"
#include "wave_file.h"
#include <sys/time.h>
//#include "ChangeRateInterface.h"

#include <time.h>
#include <HaierWake.h>

#include "tvqe_java.h"
#include "./wav/wave_file.h"

typedef struct {
  		   char Riff[5];
  		   char WAVEfmt[9];
  		   char Cdata[5];
  		   unsigned long sizeOfFile;
  		   unsigned long nSamplesPerSec;
  		   unsigned long navgBytesPerSec;
  		   unsigned long sizeOfData;
		   unsigned short nBlockAlign;
		   unsigned long sizeOfFmt;
		   unsigned short wFormatTag;
		   unsigned short nChannels;
		   unsigned short nBitPerSample;
}WAVEHEADFMT;

//sogou mic start
#define	SG_M_A_SAMPLE_RATE	96000	//
#define	SG_M_A_SAMPLE_NUM_PER_CHAN	(SG_M_A_SAMPLE_RATE*10/1000)	//10msͨ
#define	SG_M_A_TOTAL_SAMPLE_NUM	(SG_M_A_SAMPLE_NUM_PER_CHAN*2)	//10msͨ
#define SG_M_A_NUM 7
#define SG_M_A_REF_NUM 1
#define SG_M_A_CHN_NUM 12 
#define	SG_MIC_SAMPLE_RATE	16000	//

const short flag_s16[] = {0x0001, 0x0002, 0x0003, 0x0004};

const int frame_size = 12;
int data_index = 0;
short frame_buf[12];
short allbuf16_s[SG_M_A_CHN_NUM * 160];
int data_len = 0;
int flag_pos = -1;
int cntall = 0;
int cntallper = 0;
//sogou mic end

//haier mic start
#define	HE_MIC_SAMPLE_RATE	16000	//采样率
#define	HE_SAMPLE_NUM_PER_CHAN	(HE_MIC_SAMPLE_RATE*10/1000)	//10毫秒单通道样点数
#define	HE_TOTAL_SAMPLE_NUM	(HE_SAMPLE_NUM_PER_CHAN*2)	//2通道总样点数
#define HE_NN HE_SAMPLE_NUM_PER_CHAN //80(NB) or 160(WB/SWB)
#define SAVE_MIX_NS_FILE
//haier mic end

#define ID_RIFF 0x46464952
#define ID_WAVE 0x45564157
#define ID_FMT  0x20746d66
#define ID_DATA 0x61746164

#define FORMAT_PCM 1

struct wav_header {
    uint32_t riff_id;
    uint32_t riff_sz;
    uint32_t riff_fmt;
    uint32_t fmt_id;
    uint32_t fmt_sz;
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
    uint32_t data_id;
    uint32_t data_sz;
};

int capturing = 1;
short mic1[160], mic2[160], mic3[160], mic4[160], mic5[160], mic6[160], mic7[160], ref[160];
short outbuffer[SG_M_A_TOTAL_SAMPLE_NUM];

unsigned int capture_sample(HAIER_WAKE_HANDLE wake_handle, FILE *file, unsigned int card, unsigned int device,
                            unsigned int channels, unsigned int rate,
                            enum pcm_format format, unsigned int period_size,
                            unsigned int period_count);
int wake_up_process(HAIER_WAKE_HANDLE wake_handle, short *buffer, int data_size);

int ReadWaveHead( WAVEHEADFMT *wav_in, FILE *file_in ) 
{
    int tmp;

	fseek( file_in, 0, SEEK_SET );
	if ( fread ( wav_in->Riff, 1, 4, file_in ) < 4 ) return 0;
	if ( fread ( &(wav_in->sizeOfFile), 1, 4, file_in) < 4 ) return 0 ;
	if ( fread ( wav_in->WAVEfmt, 1, 8, file_in ) < 8 ) return 0;
	if ( fread ( &(wav_in->sizeOfFmt), 1, 4, file_in ) < 4 ) return 0;
	if ( fread ( &(wav_in->wFormatTag), 1, 2, file_in ) < 2 ) return 0;
	if ( fread ( &(wav_in->nChannels), 1, 2, file_in ) < 2 ) return 0;
	if ( fread ( &(wav_in->nSamplesPerSec), 1, 4, file_in ) < 4 ) return 0;
	if ( fread ( &(wav_in->navgBytesPerSec), 1, 4, file_in ) < 4 ) return 0;
	if ( fread ( &(wav_in->nBlockAlign), 1, 2, file_in ) < 2 ) return 0;
	if ( fread ( &(wav_in->nBitPerSample), 1, 2, file_in ) < 2 ) return 0;
	if ( fread ( wav_in->Cdata, 1, 4, file_in) < 4 ) return 0;
	if ( fread ( &(wav_in->sizeOfData), 1, 4, file_in ) < 4 ) return 0;

	tmp = ftell( file_in );
	fseek( file_in, 0, SEEK_END );
	wav_in->sizeOfFile = ftell( file_in );
	wav_in->sizeOfData = wav_in->sizeOfFile - 44;

	fseek( file_in, tmp, SEEK_SET );

	return 1;
}

int WriteWaveHead( WAVEHEADFMT *wav_out, FILE *file_out ) 
{
	
	fseek( file_out, 0, SEEK_SET );
	if ( fwrite ( wav_out->Riff, 1, 4, file_out ) < 4 ) return 0;
	if ( fwrite ( &(wav_out->sizeOfFile), 1, 4, file_out) < 4 ) return 0 ;
	if ( fwrite ( wav_out->WAVEfmt, 1, 8, file_out ) < 8 ) return 0;
	if ( fwrite ( &(wav_out->sizeOfFmt), 1, 4, file_out ) < 4 ) return 0;
	if ( fwrite ( &(wav_out->wFormatTag), 1, 2, file_out ) < 2 ) return 0;
	if ( fwrite ( &(wav_out->nChannels), 1, 2, file_out ) < 2 ) return 0;
	if ( fwrite ( &(wav_out->nSamplesPerSec), 1, 4, file_out ) < 4 ) return 0;
	if ( fwrite ( &(wav_out->navgBytesPerSec), 1, 4, file_out ) < 4 ) return 0;
	if ( fwrite ( &(wav_out->nBlockAlign), 1, 2, file_out ) < 2 ) return 0;
	if ( fwrite ( &(wav_out->nBitPerSample), 1, 2, file_out ) < 2 ) return 0;
	if ( fwrite ( wav_out->Cdata, 1, 4, file_out) < 4 ) return 0;
	if ( fwrite ( &(wav_out->sizeOfData), 1, 4, file_out ) < 4 ) return 0;

	return 1;
}

int PrepareWaveHead( WAVEHEADFMT *wav_head, 
					 unsigned short nChannels,
					 unsigned long nSamplesPerSec,
					 unsigned short nBitPerSample,
					 unsigned long sizeOfData
				   )
{
	strcpy ( wav_head->Riff, "RIFF" );
	strcpy ( wav_head->WAVEfmt, "WAVEfmt " );
	strcpy ( wav_head->Cdata, "data" );

	wav_head->sizeOfFmt = 16;
	wav_head->wFormatTag = 1;
	wav_head->nChannels = nChannels;
	wav_head->nSamplesPerSec = nSamplesPerSec;
	wav_head->nBitPerSample = nBitPerSample;
	wav_head->sizeOfData = sizeOfData;
	wav_head->sizeOfFile = wav_head->sizeOfData + 44;
	wav_head->nBlockAlign = wav_head->nChannels * ( wav_head->nBitPerSample / 8 );
	wav_head->navgBytesPerSec= wav_head->nSamplesPerSec * wav_head->nBlockAlign;
	
	return 1;
}

void sigint_handler(int sig)
{
    capturing = 0;
}
int wake_init(HAIER_WAKE_HANDLE * handle)
{
	const char* model = "/mnt/sdcard/haier/model_file/test_20170912.nnet.mars.q";
	const char* cmvn = "/mnt/sdcard/haier/model_file/cmvn_test";
	double threshold = 0.6;
	int ret = 0;
	
	if((ret = haier_wake_init(handle, model, cmvn, threshold)) != HAIER_WAKE_OK)
	{
		printf("haier_wake_init failed return %d !!!!!!\r\n", ret);
		haier_wake_release(*handle);
		return ret;
	}

	return HAIER_WAKE_OK;
}
void wake_deinit(HAIER_WAKE_HANDLE handle)
{
	haier_wake_release(handle);
}

unsigned int capture_sample(HAIER_WAKE_HANDLE wake_handle, FILE *file, unsigned int card, unsigned int device,
                            unsigned int channels, unsigned int rate,
                            enum pcm_format format, unsigned int period_size,
                            unsigned int period_count)
{
    struct pcm_config config;
    struct pcm *pcm;
    short *buffer;
    unsigned int size;
    unsigned int bytes_read = 0;

    config.channels = channels;
    config.rate = rate;
    config.period_size = period_size;
    config.period_count = period_count;
    config.format = format;
    config.start_threshold = 0;
    config.stop_threshold = 0;
    config.silence_threshold = 0;

    pcm = pcm_open(card, device, PCM_IN, &config);
    if (!pcm || !pcm_is_ready(pcm))
	{
        fprintf(stderr, "Unable to open PCM device (%s)\n", pcm_get_error(pcm));
        return 0;
    }
    
    size = pcm_frames_to_bytes(pcm, pcm_get_buffer_size(pcm));
    buffer = (short *)malloc(size);
    if(!buffer)
	{
        fprintf(stderr, "Unable to allocate %d bytes\n", size);
        free(buffer);
        pcm_close(pcm);
        return 0;
    }
	fprintf(stderr, "*** buffer_size = %d, size = %d\r\n", pcm_get_buffer_size(pcm), size);

    fprintf(stderr, "@@@ Capturing sample: %u ch, %u hz, %u bit, size %u\n", channels, rate, pcm_format_to_bits(format), size);

	while(capturing && !pcm_read(pcm, buffer, size))
	{
		wake_up_process(wake_handle, buffer, size);
        if (fwrite(buffer, 1, size, file) != size)
		{
            fprintf(stderr,"Error capturing sample\n");
            break;
        }
        bytes_read += size;
    }

    free(buffer);
    pcm_close(pcm);
    return pcm_bytes_to_frames(pcm, bytes_read);
}
int wake_up_cnt = 0;
int wake_up_process(HAIER_WAKE_HANDLE wake_handle, short *buffer, int data_size)
{
	int data_len = 320;
	int ret = 0;
	double confidence;
	short * data10ms = buffer;
	int i;
	
	if(data_size < SG_M_A_TOTAL_SAMPLE_NUM)
	{
		printf("dataRead = %d, %d\r\n", data_size, SG_M_A_TOTAL_SAMPLE_NUM);
		return -1;
	}
	data_len = data_size>>1;
	//printf("dataRead = %d, %d\r\n", data_len, SG_M_A_TOTAL_SAMPLE_NUM);
	for(i = 0; i < data_len; i++)
	{
		if(data_index <= frame_size - 1)
		{
			frame_buf[data_index] = data10ms[i];
		}
		data_index++;
		if(flag_pos == -1 && (data10ms[i] == flag_s16[0] && data10ms[i+1] == flag_s16[1]))
		{
			//printf("%d: data_index: %d, flag_pos: %d, data10ms[%d]: %d, flag_s16[0]: %d\r\n", __LINE__, data_index, flag_pos, i, data10ms[i], flag_s16[0]);
			flag_pos = 0;
		}
		else if (flag_pos == 0)
		{
			if (data10ms[i] == flag_s16[0] && data10ms[i+1] == flag_s16[1])
			{
				//printf("%d: data_index: %d, flag_pos: %d, data10ms[%d]: %d, flag_s16[0]: %d\r\n", __LINE__, data_index, flag_pos, i, data10ms[i], flag_s16[0]);
				//����Ӵflag_posᱻΪ-1
				flag_pos = 0;
			}
			else if(data10ms[i] == flag_s16[1] && data10ms[i+1] == flag_s16[2])
			{
				flag_pos = 0;
			}
			else if(data10ms[i] == flag_s16[2] && data10ms[i+1] == flag_s16[3])
			{
				flag_pos = 0;
			}
			else if (data10ms[i] == flag_s16[3])//-5 && data10ms[i] <= flag_s16[1]+5)
			{
				//printf("%d: data_index: %d, flag_pos: %d, data10ms[%d]: %d, flag_s16[0]: %d\r\n", __LINE__, data_index, flag_pos, i, data10ms[i], flag_s16[0]);
				if (data_index == frame_size)
				{
					/*printf("%d: 0x%04x,0x%04x,0x%04x,0x%04x,   0x%04x,0x%04x,0x%04x,0x%04x,0x%04x,0x%04x,0x%04x,0x%04x,0x%04x,0x%04x,0x%04x,0x%04x\r\n",
							__LINE__, data10ms[i-3], data10ms[i-2], data10ms[i-1], data10ms[i], frame_buf[0], frame_buf[1], frame_buf[2], frame_buf[3], frame_buf[4], frame_buf[5],
							frame_buf[6], frame_buf[7], frame_buf[8], frame_buf[9], frame_buf[10], frame_buf[11]);
					mic5[cntallper] = frame_buf[0];
					mic6[cntallper] = frame_buf[1];
					mic7[cntallper] = frame_buf[2];
					mic4[cntallper] = frame_buf[3];
					mic3[cntallper] = frame_buf[4];
					ref[cntallper] = frame_buf[5];
					mic1[cntallper] = frame_buf[6];
					mic2[cntallper] = frame_buf[7];
					*/
					mic3[cntallper] = frame_buf[4];
					cntallper++;
					if(cntallper >= 160)
					{
						cntallper = 0;
						ret = haier_wake_process(wake_handle, (char*)mic3, sizeof(mic3), &confidence);
						if(ret == HAIER_WAKE_OK)
						{
							printf("##############################################\r\n");
							printf("##############################################\r\n");
							printf("############ find wake up word %d ###########\n", ++wake_up_cnt);
							printf("##############################################\r\n");
							printf("##############################################\r\n\r\n\r\n");
							
						}
					}
				}
				flag_pos = -1;
				data_index = 0;
			}
			else
			{
				flag_pos = -1;
			}
		}
	}

	return 0;
}
typedef struct
{
	HAIER_WAKE_HANDLE wake_handle;
}hndl_s;
int main(int argc, char **argv)
{
    FILE *file;
    struct wav_header header;
    unsigned int card = 0;
    unsigned int device = 0;
    unsigned int channels = 2;
    unsigned int rate = 96000;
    unsigned int bits = 16;
    unsigned int frames;
    unsigned int period_size = 960;
    unsigned int period_count = 2;
    enum pcm_format format;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s file.wav [-D card] [-d device] [-c channels] "
                "[-r rate] [-b bits] [-p period_size] [-n n_periods]\n", argv[0]);
		fprintf(stderr, "%s wakeup.pcm -D 1 -d 0 -c 2 -r 96000 -b 16\r\n", argv[0]);
        return 1;
    }

    file = fopen(argv[1], "wb");
    if (!file) {
        fprintf(stderr, "Unable to create file '%s'\n", argv[1]);
        return 1;
    }

    /* parse command line arguments */
    argv += 2;
    while (*argv) {
        if (strcmp(*argv, "-d") == 0) {
            argv++;
            if (*argv)
                device = atoi(*argv);
        } else if (strcmp(*argv, "-c") == 0) {
            argv++;
            if (*argv)
                channels = atoi(*argv);
        } else if (strcmp(*argv, "-r") == 0) {
            argv++;
            if (*argv)
                rate = atoi(*argv);
        } else if (strcmp(*argv, "-b") == 0) {
            argv++;
            if (*argv)
                bits = atoi(*argv);
        } else if (strcmp(*argv, "-D") == 0) {
            argv++;
            if (*argv)
                card = atoi(*argv);
        } else if (strcmp(*argv, "-p") == 0) {
            argv++;
            if (*argv)
                period_size = atoi(*argv);
        } else if (strcmp(*argv, "-n") == 0) {
            argv++;
            if (*argv)
                period_count = atoi(*argv);
        }
        if (*argv)
            argv++;
    }

	wake_init(&wake_handle);
    header.riff_id = ID_RIFF;
    header.riff_sz = 0;
    header.riff_fmt = ID_WAVE;
    header.fmt_id = ID_FMT;
    header.fmt_sz = 16;
    header.audio_format = FORMAT_PCM;
    header.num_channels = channels;
    header.sample_rate = rate;

    switch (bits) {
    case 32:
        format = PCM_FORMAT_S32_LE;
        break;
    case 24:
        format = PCM_FORMAT_S24_LE;
        break;
    case 16:
        format = PCM_FORMAT_S16_LE;
        break;
    default:
        fprintf(stderr, "%d bits is not supported.\n", bits);
        return 1;
    }

    header.bits_per_sample = pcm_format_to_bits(format);
    header.byte_rate = (header.bits_per_sample / 8) * channels * rate;
    header.block_align = channels * (header.bits_per_sample / 8);
    header.data_id = ID_DATA;

    /* leave enough room for header */
    fseek(file, sizeof(struct wav_header), SEEK_SET);

    /* install signal handler and begin capturing */
    signal(SIGINT, sigint_handler);
    frames = capture_sample(wake_handle, file, card, device, header.num_channels,
                            header.sample_rate, format,
                            period_size, period_count);
    printf("Captured %d frames\n", frames);

    /* write header now all information is known */
    header.data_sz = frames * header.block_align;
    header.riff_sz = header.data_sz + sizeof(header) - 8;
    fseek(file, 0, SEEK_SET);
    fwrite(&header, sizeof(struct wav_header), 1, file);

    fclose(file);
	wake_deinit(wake_handle);

    return 0;
}

#if 1
void ae()
{
	FILE *dl_fd, *ul_fd, *e_fd;
	short dl_buf[HE_NN], ul_buf[HE_NN], e_buf[HE_NN];
	//char  input_filename[MAX_FILENAME_LEN], ref_filename[MAX_FILENAME_LEN], output_filename[MAX_FILENAME_LEN];
	int aec_mode = COMM; //模式：COMM=0--通信；VR=1--识别
	int Fs;
	int frm_cnt = 0;
	int ul_chan = 0, dl_chan = 0;
	int ul_Fs = 0, dl_Fs = 0;
	long   datasize = 0;
	int   ul_val = 0, dl_val = 0;
	short input[HE_NN * 2] = { 0 };
	int i;
#ifdef  USE_SPEEX
	SpeexEchoState *st;
	SpeexPreprocessState *den;
	int  sampleRate = 8000;

	st = speex_echo_state_init(HE_NN, TAIL);
	den = speex_preprocess_state_init(HE_NN, sampleRate);
	speex_echo_ctl(st, SPEEX_ECHO_SET_SAMPLING_RATE, &sampleRate);
	speex_preprocess_ctl(den, SPEEX_PREPROCESS_SET_ECHO_STATE, st);
	printf("Speex AEC init successfully\n");
#endif

	if (argc != 4)
	{
		fprintf(stderr, "testecho mic_signal.sw speaker_signal.sw output.sw\n");
		exit(1);
	}
#if 0
	strcpy(input_filename, TEST_WAV_DIR);
	strcat(input_filename, argv[1]);
	ul_fd = fopen(input_filename, "rb");

	strcpy(ref_filename, TEST_WAV_DIR);
	strcat(ref_filename, argv[2]);
	dl_fd = fopen(ref_filename, "rb");

	strcpy(output_filename, OUT_WAV_DIR);
	strcat(output_filename, argv[3]);
	e_fd = fopen(output_filename, "wb");
#else
	ul_fd = fopen(argv[1], "rb");

	dl_fd = fopen(argv[2], "rb");

	e_fd = fopen(argv[3], "wb");
#endif

	if (ul_fd)
	{
		printf("ul:%s\n", argv[1]);
		ul_val = WVReadHead(ul_fd, &ul_chan, &ul_Fs, &datasize);
	}
	if (dl_fd)
	{
		printf("dl:%s\n", argv[2]);
		dl_val = WVReadHead(dl_fd, &dl_chan, &dl_Fs, &datasize);
	}
	if (ul_Fs != dl_Fs)
	{
		printf("ulfs=%d and dlfs=%d input have different samplerate\n", ul_Fs, dl_Fs);
		return -1;
	}
	else
	{
		Fs = ul_Fs;
	}

	if (e_fd)
	{
		fwrite(input, 1, WV_LHMIN, e_fd);
	}

	TVqe_Java_Init(HE_MIC_SAMPLE_RATE, aec_mode, 1, 1, 1);

#ifdef  USE_SPEEX
	TVqe_Java_Set_Parameter("AEC_Active", "0"); //
#else
	TVqe_Java_Set_Parameter("AEC_Active", "1"); //
#endif
	TVqe_Java_Set_Parameter("NS_Active", "1");  //
	TVqe_Java_Set_Parameter("AGC_Active", "1"); //


	while (!feof(ul_fd) && !feof(dl_fd))
	{
		if (frm_cnt == 0)
		{
			printf("WELCOME! This is Webrtc AEC test.\n");

			if (ul_chan == 2)
			{
				printf("Dual microphones\n");
			}
			else
			{
				printf("Single microphone\n");
			}

			if (Fs == 16000)
			{
				printf("Wide band\n");
			}
			else
			{
				printf("Narrow band\n");
			}
		}
		printf("frame count:%d\r", frm_cnt);

		fread(input, sizeof(short), HE_NN*ul_chan, ul_fd);
		if (ul_chan == 2)
		{
			for (i = 0; i < HE_NN; i++)
			{
				ul_buf[i] = input[2 * i];
			}
		}
		else
		{
			for (i = 0; i < HE_NN; i++)
			{
				ul_buf[i] = input[i];
			}
		}

		fread(dl_buf, sizeof(short), HE_NN, dl_fd);
#ifdef  USE_SPEEX
		{
			speex_echo_cancellation(st, ul_buf, dl_buf, e_buf);
			speex_preprocess_run(den, e_buf);
			TVqe_Java_Process(e_buf, dl_buf, e_buf, aec_mode);
		}
#else
		TVqe_Java_Process(ul_buf, dl_buf, e_buf, aec_mode);
#endif
		fwrite(e_buf, sizeof(short), HE_NN, e_fd);

		frm_cnt++;
	}

	if (e_fd)
	{
		long int filelen = 0;
		filelen = FLfileSize(e_fd);
		fseek(e_fd, 0, SEEK_SET);
		WVWritehead(e_fd, 1, Fs, filelen - WV_LHMIN);
	}

	//webrtc::WebRtcAec_Free(aecInst);
	TVqe_Java_Free();
#ifdef  USE_SPEEX
	speex_echo_state_destroy(st);
	speex_preprocess_state_destroy(den);
#endif
	fclose(e_fd);
	fclose(dl_fd);
	fclose(ul_fd);


	return 0;
}#endif

