
#include <stdio.h>
#include <time.h>
#include <string.h>

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


#define	SAMPLE_RATE	96000	//采样率
#define	SAMPLE_NUM_PER_CHAN	(SAMPLE_RATE*10/1000)	//10ms单通道样点数
#define	TOTAL_SAMPLE_NUM	(SAMPLE_NUM_PER_CHAN*2)	//10ms两通道总样点数
#define MIC_NUM 7
#define REF_NUM 1
#define ALL_NUM 12 
#define	MIC_RATE	16000	//采样率

const short flag_s16[2] = {-3856, 3855}; //signed 16bit value f0f0，0f0f

const int frame_size = 12;
int data_index = 0;
short frame_buf[12];
short allbuf16_s[ALL_NUM * 160];
int data_len = 0;
int flag_pos = -1;
int cntall = 0;
int cntallper = 0;

int main(int argc, char* argv[])
{
	int numWrite = 0, i, totalSampleCountper = 0, totalSampleCountsparate = 0, dataRead = 0;

	FILE  *fpsparate, *fp_input;
	FILE *fp_mic1, *fp_mic2, *fp_mic3, *fp_mic4, *fp_refL, *fp_refR;

	WAVEHEADFMT waveheadmic1, waveheadmic2, waveheadmic3, waveheadmic4, waveheadrefL, waveheadrefR;
	WAVEHEADFMT waveheadsparate;
	
	short mic1[160], mic2[160], mic3[160], mic4[160], refL[160], refR[160];

	short outbuffer[TOTAL_SAMPLE_NUM];
	short frameCount = 0;
	int outputIndex = 0;


    short data10ms[TOTAL_SAMPLE_NUM];//参考通道和信号通道

	if(argc != 3)
	{
		printf("usage: %s input_file output_file\n",argv[0]);
		return -1;
	}

	fp_input = fopen(argv[1], "rb");
	if(fp_input == NULL)
	{
		printf("Can't open input file!\n");
		return 0;
	}
	fseek(fp_input, 44L, SEEK_SET);

	fpsparate = fopen(argv[2], "wb");
	if(fpsparate == NULL)
	{
		printf("Can't open output file!\n");
	}
	fseek(fpsparate, 44L, SEEK_SET);

	fp_mic1 = fopen("mic1.wav", "wb");
	if(fp_mic1 == NULL)
	{
		printf("Can't open mic1.wav file!\n");
	}
	fp_mic2 = fopen("mic2.wav", "wb");
	if(fp_mic2 == NULL)
	{
		printf("Can't open mic2.wav file!\n");
	}
	fp_mic3 = fopen("mic3.wav", "wb");
	if(fp_mic1 == NULL)
	{
		printf("Can't open mic3.wav file!\n");
	}
	fp_mic4 = fopen("mic4.wav", "wb");
	if(fp_mic4 == NULL)
	{
		printf("Can't open mic4.wav file!\n");
	}

	fp_refL = fopen("refL.wav", "wb");
	if(fp_refL == NULL)
	{
		printf("Can't open refL.wav file!\n");
	}

	fp_refR = fopen("refR.wav", "wb");
	if(fp_refR == NULL)
	{
		printf("Can't open refR.wav file!\n");
	}


	while(!feof(fp_input))
	{
		dataRead = fread(data10ms, sizeof(short), TOTAL_SAMPLE_NUM, fp_input);

	//	printf("dataRead = %d\n", dataRead);

		//	perror("zzzzzzzzzz");
		if(dataRead < TOTAL_SAMPLE_NUM)
		{
			//printf("dataRead = %d\n", dataRead);
			//perror("zzzzzzzzzz");
			break;
		}

		for (i = 0; i < TOTAL_SAMPLE_NUM; i++) {
			if (data_index <= frame_size - 1) {
				frame_buf[data_index] = data10ms[i];
			}
			data_index++;
			if (flag_pos == -1 && (data10ms[i] >= flag_s16[0]-5 && data10ms[i] <= flag_s16[0]+5)) {
				flag_pos = 0;
			} else if (flag_pos == 0) {
				if (data10ms[i] >= flag_s16[0]-5 && data10ms[i] <= flag_s16[0]+5) {			//若不加此项，flag_pos会被置为-1
					flag_pos = 0;
				} else if (data10ms[i] >= flag_s16[1]-5 && data10ms[i] <= flag_s16[1]+5) {
				if (data_index == frame_size) {
					memcpy(outbuffer+cntall, frame_buf, frame_size*2);
					cntall += frame_size;

					mic1[cntallper] = frame_buf[6];
					mic2[cntallper] = frame_buf[7];
					mic3[cntallper] = frame_buf[8];
					mic4[cntallper] = frame_buf[9];
					refL[cntallper] = frame_buf[2];
					refR[cntallper] = frame_buf[3];

					cntallper++;
					if(cntallper >= 160)
					{
						cntallper = 0;
						fwrite(mic1, sizeof(short), 160, fp_mic1);
						fwrite(mic2, sizeof(short), 160, fp_mic2);
						fwrite(mic3, sizeof(short), 160, fp_mic3);
						fwrite(mic4, sizeof(short), 160, fp_mic4);
						
						fwrite(refL, sizeof(short), 160, fp_refL);
						fwrite(refR, sizeof(short), 160, fp_refR);
					}

					if(cntall >= TOTAL_SAMPLE_NUM)
					{
						cntall = 0;
						fwrite(outbuffer, sizeof(short), TOTAL_SAMPLE_NUM, fpsparate);
						totalSampleCountsparate += TOTAL_SAMPLE_NUM;
						totalSampleCountper += 160;
					}
				}
				
				flag_pos = -1;
				data_index = 0;
			} else {
				flag_pos = -1;
			}
		}
	}

	}

	PrepareWaveHead( &waveheadsparate, 
		ALL_NUM,
		MIC_RATE,
		16,
		totalSampleCountsparate * sizeof(unsigned short)
		);
	WriteWaveHead( &waveheadsparate, fpsparate);

	PrepareWaveHead( &waveheadmic1, 
		1,
		MIC_RATE,
		16,
		totalSampleCountper * sizeof(unsigned short)
		);
	WriteWaveHead( &waveheadmic1, fp_mic1);

	PrepareWaveHead( &waveheadmic2, 
		1,
		MIC_RATE,
		16,
		totalSampleCountper * sizeof(unsigned short)
		);
	WriteWaveHead( &waveheadmic2, fp_mic2);

	PrepareWaveHead( &waveheadmic3, 
		1,
		MIC_RATE,
		16,
		totalSampleCountper * sizeof(unsigned short)
		);
	WriteWaveHead( &waveheadmic3, fp_mic3);

	PrepareWaveHead( &waveheadmic4, 
		1,
		MIC_RATE,
		16,
		totalSampleCountper * sizeof(unsigned short)
		);
	WriteWaveHead( &waveheadmic4, fp_mic4);



	PrepareWaveHead( &waveheadrefL, 
		1,
		MIC_RATE,
		16,
		totalSampleCountper * sizeof(unsigned short)
		);
	WriteWaveHead( &waveheadrefL, fp_refL);

	PrepareWaveHead( &waveheadrefR, 
		1,
		MIC_RATE,
		16,
		totalSampleCountper * sizeof(unsigned short)
		);
	WriteWaveHead( &waveheadrefR, fp_refR);

	fclose(fp_input);

	fclose(fpsparate);
	
	fclose(fp_mic1);
	fclose(fp_mic2);
	fclose(fp_mic3);
	fclose(fp_mic4);
	fclose(fp_refL);
 	fclose(fp_refR);
	return 0;
}

